/*
 * accounts-ui.c
 *
 * Copyright (C) 2022 Ivaylo Dimitrov <ivo.g.dimitrov.75@gmail.com>
 *
 * This library is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "config.h"

#include <X11/Xatom.h>
#include <gdk/gdkx.h>
#include <hildon/hildon.h>

#include <errno.h>
#include <libintl.h>
#include <sys/vfs.h>

#include "accounts-wizard-dialog.h"

#include "accounts-ui.h"

struct _AccountsUIPrivate
{
  GtkListStore *store;
  AccountPluginManager *account_plugin_manager;
  GtkWidget *pannable_area;
  GtkWidget *tree_view;
  GtkWidget *label;
  GtkWidget *button_new;
  GdkPixbuf *avatar_icon;
  guint plugins_initialized_lock;
  gboolean initialized : 1;   /* 0x01 */
  gboolean wizard_active : 1; /* 0x02 */
  gboolean show : 1;          /* 0x04 */
  GdkWindow *parent_window;
};

typedef struct _AccountsUIPrivate AccountsUIPrivate;

/*
 */
#define PRIVATE(ui) \
  ((AccountsUIPrivate *) \
   accounts_ui_get_instance_private((AccountsUI *)(ui)))

static void
accounts_list_iface_init(AccountsListIface *iface);

G_DEFINE_TYPE_WITH_CODE(
  AccountsUI,
  accounts_ui,
  GTK_TYPE_DIALOG,
  G_IMPLEMENT_INTERFACE(
    ACCOUNTS_TYPE_LIST,
    accounts_list_iface_init);
  G_ADD_PRIVATE(AccountsUI);
)

enum
{
  PROP_INITIALIZED = 1,
  PROP_PARENT_WINDOW
};

enum
{
  COLUMN_AVATAR,
  COLUMN_NAME,
  COLUMN_DISPLAY_NAME,
  COLUMN_SERVICE_NAME,
  COLUMN_SERVICE_ICON,
  COLUMN_ENABLED,
  COLUMN_DRAFT,
  COLUMN_ACCOUNT_ITEM
};

static void
update_store(GtkTreeModel *model, AccountItem *account_item, int column,
             gpointer value, GType value_type)
{
  GtkTreeIter iter;

  if (!model || !account_item)
    return;

  if (!gtk_tree_model_get_iter_first(model, &iter))
    return;

  do
  {
    AccountItem *item = NULL;
    gtk_tree_model_get(model, &iter, COLUMN_ACCOUNT_ITEM, &item, -1);

    if (item)
      g_object_unref(item);

    if (account_item == item)
    {
      switch (value_type)
      {
        case G_TYPE_POINTER:
        /* fall-through */
        case G_TYPE_STRING:
        {
          gtk_list_store_set(GTK_LIST_STORE(model), &iter, column, value, -1);
          break;
        }
        case G_TYPE_BOOLEAN:
        {
          gboolean bval = GPOINTER_TO_INT(value);

          gtk_list_store_set(GTK_LIST_STORE(model), &iter, column, bval, -1);
          break;
        }
        default:
        {
          g_warn_if_reached();
          break;
        }
      }

      break;
    }
  }
  while (gtk_tree_model_iter_next(model, &iter));
}

static void
name_notify_cb(AccountItem *account_item, GParamSpec *pspec, AccountsUI *ui)
{
  AccountsUIPrivate *priv = PRIVATE(ui);
  gpointer name = NULL;

  g_object_get(account_item, "name", &name, NULL);
  update_store(GTK_TREE_MODEL(priv->store), account_item,
               COLUMN_NAME, name, G_TYPE_STRING);
  g_free(name);
}

static void
avatar_notify_cb(AccountItem *account_item, GParamSpec *pspec, AccountsUI *ui)
{
  AccountsUIPrivate *priv = PRIVATE(ui);
  gpointer avatar = NULL;
  gboolean supports_avatar = FALSE;

  g_object_get(account_item,
               "avatar", &avatar,
               "supports-avatar", &supports_avatar,
               NULL);

  if (!avatar)
  {
    if (supports_avatar && priv->avatar_icon)
      avatar = g_object_ref((gpointer)priv->avatar_icon);
  }

  update_store(GTK_TREE_MODEL(priv->store), account_item,
               COLUMN_AVATAR, avatar, G_TYPE_POINTER);

  if (avatar)
    g_object_unref(avatar);
}

static void
draft_notify_cb(AccountItem *account_item, GParamSpec *pspec, AccountsUI *ui)
{
  AccountsUIPrivate *priv = PRIVATE(ui);
  gboolean draft = FALSE;

  g_object_get(account_item, "draft", &draft, NULL);
  update_store(GTK_TREE_MODEL(priv->store), account_item, COLUMN_DRAFT,
               GINT_TO_POINTER(draft), G_TYPE_BOOLEAN);
}

static void
enabled_notify_cb(AccountItem *account_item, GParamSpec *pspec, AccountsUI *ui)
{
  AccountsUIPrivate *priv = PRIVATE(ui);
  gboolean enabled = FALSE;

  g_object_get(account_item, "enabled", &enabled, NULL);
  update_store(GTK_TREE_MODEL(priv->store), account_item,
               COLUMN_ENABLED, GINT_TO_POINTER(enabled), G_TYPE_BOOLEAN);
}

static void
display_name_notify_cb(AccountItem *account_item, GParamSpec *pspec,
                       AccountsUI *ui)
{
  AccountsUIPrivate *priv = PRIVATE(ui);
  gpointer display_name = NULL;

  g_object_get(account_item, "display-name", &display_name, NULL);
  update_store(GTK_TREE_MODEL(priv->store), account_item,
               COLUMN_DISPLAY_NAME, display_name, G_TYPE_STRING);
  g_free(display_name);
}

static gboolean
_disconnect_all_items(GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter,
                      gpointer data)
{
  gpointer item;

  gtk_tree_model_get(model, iter, COLUMN_ACCOUNT_ITEM, &item, -1);

  if (item)
  {
    g_signal_handlers_disconnect_matched(
      item, G_SIGNAL_MATCH_DATA | G_SIGNAL_MATCH_FUNC, 0, 0, NULL,
      name_notify_cb, data);
    g_signal_handlers_disconnect_matched(
      item, G_SIGNAL_MATCH_DATA | G_SIGNAL_MATCH_FUNC, 0, 0, NULL,
      avatar_notify_cb, data);
    g_signal_handlers_disconnect_matched(
      item, G_SIGNAL_MATCH_DATA | G_SIGNAL_MATCH_FUNC, 0, 0, NULL,
      draft_notify_cb, data);
    g_signal_handlers_disconnect_matched(
      item, G_SIGNAL_MATCH_DATA | G_SIGNAL_MATCH_FUNC, 0, 0, NULL,
      enabled_notify_cb, data);
    g_signal_handlers_disconnect_matched(
      item, G_SIGNAL_MATCH_DATA | G_SIGNAL_MATCH_FUNC, 0, 0, NULL,
      display_name_notify_cb, data);
    g_object_unref(item);
  }

  return FALSE;
}

static void
accounts_ui_finalize(GObject *object)
{
  G_OBJECT_CLASS(accounts_ui_parent_class)->finalize(object);
}

static void
accounts_ui_get_property(GObject *object, guint property_id, GValue *value,
                         GParamSpec *pspec)
{
  AccountsUIPrivate *priv = PRIVATE(object);

  switch (property_id)
  {
    case PROP_INITIALIZED:
    {
      g_value_set_boolean(value, priv->initialized);
      break;
    }
    case PROP_PARENT_WINDOW:
    {
      g_value_set_object(value, priv->parent_window);
      break;
    }
    default:
    {
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
      break;
    }
  }
}

static void
accounts_ui_set_property(GObject *object, guint property_id,
                         const GValue *value, GParamSpec *pspec)
{
  AccountsUIPrivate *priv = PRIVATE(object);

  switch (property_id)
  {
    case PROP_INITIALIZED:
    {
      priv->initialized = g_value_get_boolean(value);
      break;
    }
    case PROP_PARENT_WINDOW:
    {
      if (priv->parent_window)
        g_object_unref(priv->parent_window);

      priv->parent_window = g_value_dup_object(value);
      break;
    }
    default:
    {
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
      break;
    }
  }
}

static void
accounts_ui_destroy(GtkObject *object)
{
  AccountsUIPrivate *priv = PRIVATE(object);

  if (priv->account_plugin_manager)
    g_clear_object(&priv->account_plugin_manager);

  if (priv->store)
  {
    gtk_tree_model_foreach(GTK_TREE_MODEL(priv->store),
                           _disconnect_all_items, object);
    gtk_list_store_clear(priv->store);
    g_clear_object(&priv->store);
  }

  if (priv->avatar_icon)
    g_clear_object(&priv->avatar_icon);

  if (priv->parent_window)
    g_clear_object(&priv->parent_window);

  GTK_OBJECT_CLASS(accounts_ui_parent_class)->destroy(object);
}

#ifdef DISABLE_PORTRAIT
static void
accounts_ui_realize(GtkWidget *widget)
{
  GdkAtom atom;
  long portrait = 0;

  GTK_WIDGET_CLASS(accounts_ui_parent_class)->realize(widget);

  /* FIXME :( */
  atom = gdk_atom_intern_static_string("_HILDON_PORTRAIT_MODE_SUPPORT");
  gdk_property_change(widget->window, atom, gdk_x11_xatom_to_atom(XA_CARDINAL),
                      32, GDK_PROP_MODE_REPLACE, (gpointer)&portrait, 1);
  atom = gdk_atom_intern_static_string("_HILDON_PORTRAIT_MODE_REQUEST");
  gdk_property_change(widget->window, atom, gdk_x11_xatom_to_atom(XA_CARDINAL),
                      32, GDK_PROP_MODE_REPLACE, (gpointer)&portrait, 1);
}
#endif

static void
accounts_ui_size_request(GtkWidget *widget, GtkRequisition *requisition)
{
  AccountsUIPrivate *priv = PRIVATE(widget);
  GtkWidget *pannable_area;
  gint height;
  GtkRequisition req;

  gtk_widget_size_request(priv->tree_view, &req);
  pannable_area = gtk_widget_get_ancestor(priv->tree_view,
                                          HILDON_TYPE_PANNABLE_AREA);
  height = req.height;

  if (req.height >= 350)
    height = 350;

  req.height = height;
  g_object_set(pannable_area, "height-request", height, NULL);

  GTK_WIDGET_CLASS(accounts_ui_parent_class)->size_request(widget, requisition);
}

static void
accounts_ui_class_init(AccountsUIClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS(klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

  object_class->finalize = accounts_ui_finalize;
  object_class->set_property = accounts_ui_set_property;
  object_class->get_property = accounts_ui_get_property;

  GTK_OBJECT_CLASS(klass)->destroy = accounts_ui_destroy;
#ifdef DISABLE_PORTRAIT
  widget_class->realize = accounts_ui_realize;
#endif
  widget_class->size_request = accounts_ui_size_request;

  g_object_class_install_property(
    object_class, PROP_INITIALIZED,
    g_param_spec_boolean(
      "initialized",
      "initialized",
      "Whether the widget has been initialized",
      FALSE,
      G_PARAM_READABLE));
  g_object_class_install_property(
    object_class, PROP_PARENT_WINDOW,
    g_param_spec_object(
      "parent-window",
      "",
      "",
      GDK_TYPE_WINDOW,
      G_PARAM_STATIC_BLURB | G_PARAM_STATIC_NICK | G_PARAM_READWRITE));
}

static gboolean
foreach_func(GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter,
             gpointer data)
{
  gpointer item;
  GList **l = data;

  gtk_tree_model_get(model, iter, COLUMN_ACCOUNT_ITEM, &item, -1);

  if (item)
  {
    g_object_unref(item);
    *l = g_list_prepend(*l, item);
  }

  return FALSE;
}

static GList *
_accounts_list_get_all(AccountsList *accounts_list)
{
  AccountsUIPrivate *priv;
  GList *l = NULL;

  g_return_val_if_fail(ACCOUNTS_IS_UI(accounts_list), NULL);

  priv = PRIVATE(accounts_list);

  if (priv->store)
    gtk_tree_model_foreach(GTK_TREE_MODEL(priv->store), foreach_func, &l);

  return l;
}

static void
select_first_row(GtkTreeView *tree_view)
{
  if (tree_view)
  {
    GtkTreePath *path = gtk_tree_path_new_first();
    gtk_tree_view_set_cursor(tree_view, path, NULL, FALSE);
    gtk_tree_path_free(path);
  }
}

static void
_accounts_list_remove(AccountsList *accounts_list, AccountItem *account_item)
{
  AccountsUIPrivate *priv;
  GtkTreeIter iter;

  g_return_if_fail(ACCOUNTS_IS_UI(accounts_list));
  g_return_if_fail(ACCOUNT_IS_ITEM(account_item));

  priv = PRIVATE(accounts_list);
  priv->store = priv->store;

  if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(priv->store), &iter))
  {
    do
    {
      AccountItem *item = NULL;

      gtk_tree_model_get(GTK_TREE_MODEL(priv->store), &iter,
                         COLUMN_ACCOUNT_ITEM, &item,
                         -1);

      if (item)
        g_object_unref(item);

      if (item == account_item)
      {
        g_signal_handlers_disconnect_matched(
          item, G_SIGNAL_MATCH_DATA | G_SIGNAL_MATCH_FUNC, 0, 0, NULL,
          name_notify_cb, accounts_list);
        g_signal_handlers_disconnect_matched(
          item, G_SIGNAL_MATCH_DATA | G_SIGNAL_MATCH_FUNC, 0, 0, NULL,
          avatar_notify_cb, accounts_list);
        g_signal_handlers_disconnect_matched(
          item, G_SIGNAL_MATCH_DATA | G_SIGNAL_MATCH_FUNC, 0, 0, NULL,
          draft_notify_cb, accounts_list);
        g_signal_handlers_disconnect_matched(
          item, G_SIGNAL_MATCH_DATA | G_SIGNAL_MATCH_FUNC, 0, 0, NULL,
          enabled_notify_cb, accounts_list);
        g_signal_handlers_disconnect_matched(
          item, G_SIGNAL_MATCH_DATA | G_SIGNAL_MATCH_FUNC, 0, 0, NULL,
          display_name_notify_cb, accounts_list);
        gtk_list_store_remove(priv->store, &iter);
        break;
      }
    }
    while (gtk_tree_model_iter_next(GTK_TREE_MODEL(priv->store), &iter));
  }

  if (priv->store->length)
    select_first_row(GTK_TREE_VIEW(priv->tree_view));
  else
  {
    gtk_widget_show(priv->label);
    gtk_widget_hide(priv->pannable_area);
  }
}

static void
_accounts_list_add(AccountsList *accounts_list, AccountItem *account_item)
{
  AccountsUIPrivate *priv;
  gpointer service_icon = NULL;
  gpointer avatar = NULL;
  gpointer service_name = NULL;
  gpointer display_name = NULL;
  gpointer name = NULL;
  gboolean draft = FALSE;
  gboolean enabled = FALSE;
  gboolean supports_avatar = FALSE;

  g_return_if_fail(ACCOUNTS_IS_UI(accounts_list));
  g_return_if_fail(ACCOUNT_IS_ITEM(account_item));

  priv = PRIVATE(accounts_list);

  g_object_get(account_item,
               "name", &name,
               "avatar", &avatar,
               "enabled", &enabled,
               "draft", &draft,
               "display-name", &display_name,
               "service-name", &service_name,
               "service-icon", &service_icon,
               "supports-avatar", &supports_avatar,
               NULL);
  g_signal_connect(account_item, "notify::name",
                   G_CALLBACK(name_notify_cb), accounts_list);
  g_signal_connect(account_item, "notify::avatar",
                   G_CALLBACK(avatar_notify_cb), accounts_list);
  g_signal_connect(account_item, "notify::enabled",
                   G_CALLBACK(enabled_notify_cb), accounts_list);
  g_signal_connect(account_item, "notify::draft",
                   G_CALLBACK(draft_notify_cb), accounts_list);
  g_signal_connect(account_item, "notify::display-name",
                   G_CALLBACK(display_name_notify_cb), accounts_list);

  if (!avatar && supports_avatar && priv->avatar_icon)
    avatar = g_object_ref(priv->avatar_icon);

  gtk_list_store_insert_with_values(priv->store, NULL, 0,
                                    COLUMN_AVATAR, avatar,
                                    COLUMN_NAME, name,
                                    COLUMN_DISPLAY_NAME, display_name,
                                    COLUMN_SERVICE_NAME, service_name,
                                    COLUMN_SERVICE_ICON, service_icon,
                                    COLUMN_ENABLED, enabled,
                                    COLUMN_DRAFT, draft,
                                    COLUMN_ACCOUNT_ITEM, account_item,
                                    -1);
  g_free(name);
  g_free(display_name);
  g_free(service_name);

  if (avatar)
    g_object_unref(avatar);

  if (service_icon)
    g_object_unref(service_icon);

  if (priv->store->length == 1)
  {
    gtk_widget_hide(priv->label);
    gtk_widget_show(priv->pannable_area);
    select_first_row(GTK_TREE_VIEW(priv->tree_view));
  }
}

static void
accounts_list_iface_init(AccountsListIface *iface)
{
  iface->add = _accounts_list_add;
  iface->get_all = _accounts_list_get_all;
  iface->remove = _accounts_list_remove;
}

static void
on_dialog_destroy(GtkObject *object, gpointer user_data)
{
  AccountsUI *ui = user_data;

  if (PRIVATE(ui)->store->length)
    gtk_widget_show(GTK_WIDGET(ui));
  else
  {
    gtk_widget_hide(GTK_WIDGET(ui));
    g_idle_add((GSourceFunc)&gtk_widget_destroy, ui);
  }
}

static void
on_wizard_dialog_destroy(GtkObject *object, gpointer user_data)
{
  AccountsUI *ui = user_data;
  AccountsUIPrivate *priv = PRIVATE(ui);

  priv->wizard_active = FALSE;
}

static void
plugin_initialization_done(AccountsUI *ui)
{
  AccountsUIPrivate *priv = PRIVATE(ui);

  g_return_if_fail(priv->plugins_initialized_lock > 0);

  priv->plugins_initialized_lock--;

  if (!priv->plugins_initialized_lock)
  {
    priv->initialized = TRUE;
    g_object_notify(G_OBJECT(ui), "initialized");

    if (!priv->store || priv->store->length)
    {
      if (priv->show)
        gtk_widget_show(GTK_WIDGET(ui));
    }
    else if (priv->show)
    {
      GtkWidget *dialog = accounts_ui_dialogs_get_new_account(GTK_WIDGET(ui),
                                                              NULL);

      if (dialog)
      {
        gtk_widget_show(dialog);
        g_signal_connect_after(dialog, "destroy",
                               G_CALLBACK(on_dialog_destroy), ui);
      }
    }
  }
}

static void
on_plugin_initialized(AccountPlugin *plugin, GParamSpec *pspec, AccountsUI *ui)
{
  gboolean initialized = FALSE;

  g_object_get(plugin, "initialized", &initialized, NULL);

  if (initialized)
  {
    g_signal_handlers_disconnect_matched(
      plugin, G_SIGNAL_MATCH_DATA | G_SIGNAL_MATCH_FUNC,
      0, 0, NULL, on_plugin_initialized, ui);
    plugin_initialization_done(ui);
  }
}

static gboolean
idle_plugin_initialization_done(gpointer user_data)
{
  AccountsUI *ui = user_data;

  /* we must do the last call in idle, otherwise the caller will not have a
   * a chance to call accounts_ui_show() if there are no plugins or all plugins
   * are immediately initialized */
  plugin_initialization_done(ui);
  g_object_unref(ui);

  return G_SOURCE_REMOVE;
}

static void
init_plugins(AccountsUI *ui)
{
  GList *plugin_paths = g_list_prepend(NULL, PLUGINLIBDIR);
  AccountsUIPrivate *priv = PRIVATE(ui);
  GList *plugins;

  priv->account_plugin_manager =
    account_plugin_manager_new(plugin_paths, ACCOUNTS_LIST(ui));
  g_list_free(plugin_paths);

  plugins = account_plugin_manager_list(priv->account_plugin_manager);

  priv->plugins_initialized_lock = 1;

  if (plugins)
  {
    GList *l;

    for (l = plugins; l; l = l->next)
    {
      gboolean initialized = FALSE;

      priv->plugins_initialized_lock++;

      g_object_get(l->data, "initialized", &initialized, NULL);

      if (!initialized)
      {
        g_signal_connect(l->data, "notify::initialized",
                         G_CALLBACK(on_plugin_initialized), ui);
      }
      else
        plugin_initialization_done(ui);
    }

    g_list_free(plugins);
  }
  else
    gtk_widget_set_sensitive(priv->button_new, FALSE);

  g_idle_add_full(G_PRIORITY_HIGH_IDLE, idle_plugin_initialization_done,
                  g_object_ref(ui), NULL);
}

static gint
sort_func(GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b,
          gpointer user_data)
{
  gint rv;

  if (GPOINTER_TO_INT(user_data) == COLUMN_ENABLED)
  {
    gboolean ena = FALSE;
    gboolean enb = FALSE;

    gtk_tree_model_get(model, a, COLUMN_ENABLED, &ena, -1);
    gtk_tree_model_get(model, b, COLUMN_ENABLED, &enb, -1);

    rv = enb - ena;
  }
  else
  {
    gchar *vala = NULL;
    gchar *valb = NULL;

    gtk_tree_model_get(model, a, user_data, &vala, -1);
    gtk_tree_model_get(model, b, user_data, &valb, -1);

    if (vala)
    {
      if (valb)
        rv = strcmp(vala, valb);
      else
        rv = 1;
    }
    else
      rv = -(valb != 0);

    g_free(vala);
    g_free(valb);
  }

  return rv;
}

static const char *
get_text_color(const gchar *id)
{
  static char buf[40];
  GdkColor color;
  GtkStyle *style = gtk_rc_get_style_by_paths(
      gtk_settings_get_default(), NULL, NULL, GTK_TYPE_LABEL);

  if (gtk_style_lookup_color(style, id, &color))
  {
    sprintf(buf, "#%02x%02x%02x",
            color.red >> 8, color.green >> 8, color.blue >> 8);
  }

  return buf;
}

static void
status_data_func(GtkTreeViewColumn *tree_column, GtkCellRenderer *cell,
                 GtkTreeModel *tree_model, GtkTreeIter *iter, gpointer data)
{
  const char *color = get_text_color("ActiveTextColor");
  const char *span;
  gchar *markup;
  gboolean draft;
  gboolean enabled;

  gtk_tree_model_get(tree_model, iter,
                     COLUMN_ENABLED, &enabled,
                     COLUMN_DRAFT, &draft,
                     -1);

  if (draft)
    span = _("accounts_fi_draft");
  else if (enabled)
    span = _("accounts_fi_enabled");
  else
    span = _("accounts_fi_disabled");

  markup = g_markup_printf_escaped(
      "<span size=\"x-small\" foreground=\"%s\">%s</span>", color, span);
  g_object_set(cell, "markup", markup, NULL);
  g_free(markup);
}

static void
user_name_data_func(GtkTreeViewColumn *tree_column, GtkCellRenderer *cell,
                    GtkTreeModel *tree_model, GtkTreeIter *iter, gpointer data)
{
  gchar *display_name = NULL;
  gchar *name = NULL;

  gtk_tree_model_get(tree_model, iter,
                     COLUMN_NAME, &name,
                     COLUMN_DISPLAY_NAME, &display_name,
                     -1);

  if (name && *name)
  {
    if (display_name && *display_name)
    {
      gchar *markup = g_markup_printf_escaped(
          "%s\n<span size=\"x-small\" foreground=\"%s\">%s</span>", name,
          get_text_color("SecondaryTextColor"), display_name);

      g_object_set(cell, "markup", markup, NULL);
      g_free(markup);
    }
    else
      g_object_set(cell, "text", name, NULL);
  }
  else if (display_name && *display_name)
    g_object_set(cell, "text", NULL);
  else
    g_object_set(cell, "text", "", NULL);

  g_free(name);
  g_free(display_name);
}

static void
on_content_resize(GtkWidget *widget, GtkRequisition *requisition,
                  AccountsUI *ui)
{
  gtk_widget_queue_resize(GTK_WIDGET(ui));
}

static void
new_clicked_cb(GtkButton *button, AccountsUI *ui)
{
  GtkWidget *widget = accounts_ui_dialogs_get_new_account(GTK_WIDGET(ui), NULL);

  if (widget)
    gtk_widget_show(widget);
}

static void
delete_account(GtkWidget *wizard, AccountItem *account_item,
               AccountsList *account_list)
{
  if (account_item)
    accounts_list_remove(account_list, account_item);

  gtk_widget_destroy(wizard);
}

static void
tree_view_row_activated_cb(GtkTreeView *self, GtkTreePath *path,
                           GtkTreeViewColumn *column, gpointer user_data)
{
  AccountsUI *ui = user_data;
  AccountsUIPrivate *priv = PRIVATE(ui);
  GtkTreeIter iter;
  AccountItem *item = NULL;

  if (priv->wizard_active)
    return;

  if (gtk_tree_model_get_iter(GTK_TREE_MODEL(priv->store), &iter, path))
  {
    gtk_tree_model_get(GTK_TREE_MODEL(priv->store), &iter,
                       COLUMN_ACCOUNT_ITEM, &item,
                       -1);
  }

  if (item)
  {
    GtkWidget *wizard;

    priv->wizard_active = TRUE;
    wizard = accounts_wizard_dialog_new(
        GTK_WINDOW(ui), priv->account_plugin_manager, item, NULL);
    g_signal_connect(wizard, "delete-account",
                     G_CALLBACK(delete_account), ui);
    g_signal_connect(wizard, "destroy",
                     G_CALLBACK(on_wizard_dialog_destroy), ui);
    gtk_widget_show(wizard);
    g_object_unref(item);
  }
}

static void
accounts_ui_init(AccountsUI *ui)
{
  GtkWidget *vbox = GTK_DIALOG(ui)->vbox;
  AccountsUIPrivate *priv = PRIVATE(ui);
  GtkWidget *tree_view;
  GtkTreeViewColumn *column;
  GtkCellRenderer *renderer;

  priv->store = gtk_list_store_new(8, GDK_TYPE_PIXBUF, G_TYPE_STRING,
                                   G_TYPE_STRING, G_TYPE_STRING,
                                   GDK_TYPE_PIXBUF, G_TYPE_INT, G_TYPE_INT,
                                   ACCOUNT_TYPE_ITEM);

  gtk_tree_sortable_set_sort_func(
    GTK_TREE_SORTABLE(priv->store), COLUMN_NAME,
    sort_func, GINT_TO_POINTER(COLUMN_NAME), NULL);
  gtk_tree_sortable_set_sort_func(
    GTK_TREE_SORTABLE(priv->store), COLUMN_SERVICE_NAME,
    sort_func, GINT_TO_POINTER(COLUMN_SERVICE_NAME), NULL);
  gtk_tree_sortable_set_sort_func(
    GTK_TREE_SORTABLE(priv->store), COLUMN_ENABLED,
    sort_func, GINT_TO_POINTER(COLUMN_ENABLED), NULL);

  gtk_tree_sortable_set_sort_column_id(
    GTK_TREE_SORTABLE(priv->store), COLUMN_NAME, GTK_SORT_ASCENDING);

  priv->pannable_area = g_object_new(HILDON_TYPE_PANNABLE_AREA,
                                     "hscrollbar-policy", GTK_POLICY_NEVER,
                                     "vscrollbar-policy", GTK_POLICY_AUTOMATIC,
                                     NULL);
  tree_view = g_object_new(GTK_TYPE_TREE_VIEW,
                           "model", priv->store,
                           "headers-visible", FALSE,
                           NULL);

  gtk_tree_view_set_search_column(GTK_TREE_VIEW(tree_view), COLUMN_NAME);
  gtk_tree_selection_set_mode(
    gtk_tree_view_get_selection(GTK_TREE_VIEW(tree_view)),
    GTK_SELECTION_BROWSE);

  column = g_object_new(GTK_TYPE_TREE_VIEW_COLUMN, NULL);
  renderer = g_object_new(GTK_TYPE_CELL_RENDERER_PIXBUF,
                          "stock-size", HILDON_ICON_SIZE_FINGER,
                          "xpad", 16,
                          NULL);
  gtk_tree_view_column_pack_start(column, renderer, FALSE);
  gtk_tree_view_column_add_attribute(column, renderer, "pixbuf",
                                     COLUMN_SERVICE_ICON);
  gtk_tree_view_column_set_sort_column_id(column, COLUMN_SERVICE_NAME);
  gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), column);

  column = g_object_new(GTK_TYPE_TREE_VIEW_COLUMN,
                        "sizing", TRUE,
                        "expand", TRUE,
                        NULL);
  renderer = g_object_new(GTK_TYPE_CELL_RENDERER_TEXT,
                          "xalign", 0.0f,
                          "ellipsize", 3,
                          NULL);
  gtk_tree_view_column_pack_start(column, renderer, TRUE);
  gtk_tree_view_column_set_sort_column_id(column, COLUMN_NAME);
  gtk_tree_view_append_column(GTK_TREE_VIEW( tree_view), column);
  gtk_tree_view_column_set_cell_data_func(
    column, renderer, user_name_data_func, NULL, NULL);

  column = g_object_new(GTK_TYPE_TREE_VIEW_COLUMN,
                        "sizing", TRUE,
                        "expand", 0,
                        NULL);
  renderer = g_object_new(GTK_TYPE_CELL_RENDERER_TEXT,
                          "alignment", PANGO_ALIGN_LEFT,
                          "xpad", 16,
                          "xalign", 1.0,
                          NULL);
  gtk_tree_view_column_pack_start(column, renderer, FALSE);
  gtk_tree_view_column_set_sort_column_id(column, COLUMN_ENABLED);
  gtk_tree_view_column_set_cell_data_func(
    column, renderer, status_data_func, NULL, NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), column);

  column = g_object_new(GTK_TYPE_TREE_VIEW_COLUMN, NULL);
  renderer = g_object_new(GTK_TYPE_CELL_RENDERER_PIXBUF,
                          "stock-size", HILDON_ICON_SIZE_FINGER,
                          NULL);
  gtk_tree_view_column_pack_start(column, renderer, FALSE);
  gtk_tree_view_column_add_attribute(column, renderer, "pixbuf", COLUMN_AVATAR);
  gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), column);

  priv->tree_view = tree_view;

  g_signal_connect(tree_view, "row-activated",
                   G_CALLBACK(tree_view_row_activated_cb), ui);

  priv->label = g_object_new(GTK_TYPE_LABEL,
                             "label", _("accounts_ia_no_accounts"),
                             "xalign", 0.5,
                             "yalign", 0.5,
                             "visible", TRUE,
                             NULL);

  hildon_helper_set_logical_color(priv->label, GTK_RC_FG, GTK_STATE_NORMAL,
                                  "SecondaryTextColor");
  gtk_widget_set_no_show_all(priv->label, TRUE);
  gtk_container_add(GTK_CONTAINER(vbox), priv->label);
  gtk_container_add(GTK_CONTAINER(vbox), priv->pannable_area);
  gtk_container_add(GTK_CONTAINER(priv->pannable_area), priv->tree_view);

  g_signal_connect(priv->tree_view, "size-request",
                   G_CALLBACK(on_content_resize), ui);
  gtk_widget_set_no_show_all(priv->pannable_area, TRUE);
  gtk_widget_show(priv->tree_view);
  priv->button_new = gtk_button_new_with_label(_("accounts_bd_new"));
  gtk_widget_set_size_request(priv->button_new, 160, 70);
  gtk_widget_set_name(priv->button_new, "GtkButton-finger");
  g_signal_connect(priv->button_new, "clicked",
                   G_CALLBACK(new_clicked_cb), ui);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(ui)->action_area), priv->button_new,
                     FALSE, TRUE, 0);
  gtk_widget_show_all(vbox);
  gtk_widget_show(priv->button_new);

  priv->avatar_icon = gtk_icon_theme_load_icon(
      gtk_icon_theme_get_default(),
      "general_default_avatar", HILDON_ICON_PIXEL_SIZE_FINGER, 0, NULL);

  init_plugins(ui);
  gtk_window_set_title(GTK_WINDOW(ui), _("accounts_ti_accounts"));
  gtk_window_set_default_size(GTK_WINDOW(ui), -1, 280);
  gtk_window_set_modal(GTK_WINDOW(ui), FALSE);
  gtk_dialog_set_has_separator(GTK_DIALOG(ui), FALSE);
}

GtkWidget *
accounts_ui_dialogs_get_edit_account(GtkWidget *accounts_ui,
                                     const char *service_name,
                                     const char *user_name)
{
  AccountsUIPrivate *priv;
  GtkTreeIter iter;

  g_return_val_if_fail(ACCOUNTS_IS_UI(accounts_ui), NULL);
  g_return_val_if_fail(user_name != NULL, NULL);

  priv = PRIVATE(accounts_ui);

  g_return_val_if_fail(priv->initialized, NULL);

  if (priv->wizard_active)
    return NULL;

  if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(priv->store), &iter))
  {
    do
    {
      AccountService *service;
      AccountItem *account;
      gchar *_service_name;
      gchar *_user_name;

      gtk_tree_model_get(GTK_TREE_MODEL(priv->store), &iter,
                         COLUMN_ACCOUNT_ITEM, &account,
                         -1);
      g_object_get(account, "name", &_user_name, NULL);
      service = account_item_get_service(account);
      g_object_get(service, "name", &_service_name, NULL);

      if (!g_strcmp0(_user_name, user_name) &&
          !g_strcmp0(_service_name, service_name))
      {
        GtkWidget *wizard;

        g_free(_user_name);
        g_free(_service_name);
        priv->wizard_active = TRUE;

        wizard = accounts_wizard_dialog_new(
            GTK_WINDOW(accounts_ui), priv->account_plugin_manager,
            account, service);
        gtk_window_set_resizable(GTK_WINDOW(wizard), FALSE);
        g_signal_connect(wizard, "delete-account",
                         G_CALLBACK(delete_account), accounts_ui);
        g_signal_connect(wizard, "destroy",
                         G_CALLBACK(on_wizard_dialog_destroy), accounts_ui);
        g_object_unref(account);

        if (!gtk_widget_get_visible(accounts_ui) && priv->parent_window)
        {
          gtk_widget_realize(wizard);
          gdk_window_set_transient_for(wizard->window, priv->parent_window);
        }

        return wizard;
      }

      g_free(_user_name);
      g_free(_service_name);
      g_object_unref(account);
    }
    while (gtk_tree_model_iter_next(GTK_TREE_MODEL(priv->store), &iter));
  }

  g_warning("Unknown account %s for service %s", user_name, service_name);

  return NULL;
}

void
accounts_ui_set_parent(GtkWidget *accounts_ui, GdkWindow *parent)
{
  g_return_if_fail(ACCOUNTS_IS_UI(accounts_ui));

  g_object_set(accounts_ui, "parent-window", parent, NULL);

  if (parent)
  {
    if (gtk_widget_get_realized(accounts_ui))
    {
      gdk_window_set_transient_for(accounts_ui->window, parent);
      gtk_widget_hide(accounts_ui);
      gtk_widget_show(accounts_ui);
    }
  }
}

void
accounts_ui_show(GtkWidget *accounts_ui)
{
  g_return_if_fail(ACCOUNTS_IS_UI(accounts_ui));

  PRIVATE(accounts_ui)->show = TRUE;
}

GtkWidget *
accounts_ui_dialogs_get_new_account(GtkWidget *accounts_ui,
                                    const char *service_name)
{
  const char *dir;
  GtkWidget *wizard;
  AccountsUIPrivate *priv;
  AccountService *service = NULL;
  struct statfs stat;
  int res;

  g_return_val_if_fail(ACCOUNTS_IS_UI(accounts_ui), NULL);

  dir = g_get_home_dir();

  if ((res = statfs(dir, &stat)))
    g_warning("Error \"%s\" while checking file system", strerror(errno));

  if (res || (((double)stat.f_bavail * 100.0) / ((double)stat.f_blocks) <= 2.0))
  {
    GtkWidget *widget;

    if (gtk_widget_get_mapped(accounts_ui))
      widget = accounts_ui;
    else
    {
      widget = GTK_WIDGET(gtk_window_get_transient_for(
                            GTK_WINDOW(accounts_ui)));
    }

    hildon_banner_show_informationf(
      widget, NULL, dgettext("ke-recv", "cerm_device_memory_full"),
      _("accounts_fi_device_memory_full_error"));

    return NULL;
  }

  priv = PRIVATE(accounts_ui);

  if (service_name && *service_name)
  {
    GList *plugins = account_plugin_manager_list(priv->account_plugin_manager);
    GList *p;

    for (p = plugins; p; p = p->next)
    {
      if (p->data)
      {
        GList *services = account_plugin_list_services(p->data);
        GList *s;

        for (s = services; s; s = s->next)
        {
          if (s->data)
          {
            if (!g_strcmp0(account_service_get_name(s->data), service_name))
            {
              service = s->data;
              break;
            }
          }
        }

        g_list_free(services);
      }
    }

    g_list_free(plugins);

    if (!service)
      return NULL;
  }

  wizard = accounts_wizard_dialog_new(
      GTK_WINDOW(accounts_ui), priv->account_plugin_manager, NULL, service);
  g_signal_connect(wizard, "destroy",
                   G_CALLBACK(on_wizard_dialog_destroy), accounts_ui);

  if (!gtk_widget_get_visible(accounts_ui) && priv->parent_window)
  {
    gtk_widget_realize(wizard);
    gdk_window_set_transient_for(wizard->window, priv->parent_window);
  }

  return wizard;
}

gboolean
accounts_ui_get_is_empty(GtkWidget *accounts_ui)
{
  return gtk_widget_get_visible(PRIVATE(accounts_ui)
                                ->label);
}
