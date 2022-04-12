/*
 * accounts-wizard-dialog.c
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
#include <libaccounts/account-dialog-context.h>
#include <libaccounts/account-error.h>

#include <libintl.h>

#include "accounts-wizard-dialog.h"

struct _AccountsWizardDialogPrivate
{
  AccountItem *account_item;
  AccountPluginManager *manager;
  GtkListStore *store;
  GtkWidget *tree_view;
  AccountPlugin *plugin;
  AccountService *service;
  AccountEditContext *context;
  GtkWidget *vbox;
  GtkWidget *dialog;
  GtkWidget *button_sign_in;
  gchar *cancel_text;
};

typedef struct _AccountsWizardDialogPrivate AccountsWizardDialogPrivate;

#define PRIVATE(wizard) \
  ((AccountsWizardDialogPrivate *)accounts_wizard_dialog_get_instance_private \
     ((AccountsWizardDialog *)(wizard)))

G_DEFINE_TYPE_WITH_PRIVATE(
  AccountsWizardDialog,
  accounts_wizard_dialog,
  GTK_TYPE_DIALOG
)

enum
{
  PROP_MANAGER = 1,
  PROP_ACCOUNT_ITEM,
  PROP_SERVICE,
  PROP_CANCEL_TEXT
};

enum
{
  DELETE_ACCOUNT,
  LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

enum
{
  COLUMN_NAME,
  COLUMN_ICON,
  COLUMN_SERVICE,
  COLUMN_PRIORITY
};

static void
on_delete_account_response(GtkDialog *note, int response_id,
                           AccountsWizardDialog *wizard)
{
  gtk_widget_destroy(GTK_WIDGET(note));

  if (response_id == GTK_RESPONSE_OK)
  {
    g_signal_emit(wizard, signals[DELETE_ACCOUNT], 0,
                  PRIVATE(wizard)->account_item);
  }
}

static void
accounts_wizard_dialog_destroy(GtkObject *object)
{
  AccountsWizardDialogPrivate *priv = PRIVATE(object);

  g_clear_object(&priv->manager);
  g_clear_object(&priv->context);
  g_clear_object(&priv->plugin);
  g_clear_object(&priv->service);
  g_clear_object(&priv->account_item);
  g_clear_object(&priv->vbox);
  g_clear_object(&priv->store);

  if (priv->cancel_text)
  {
    g_free(priv->cancel_text);
    priv->cancel_text = NULL;
  }

  GTK_OBJECT_CLASS(accounts_wizard_dialog_parent_class)->destroy(object);
}

static void
accounts_wizard_dialog_set_property(GObject *object, guint property_id,
                                    const GValue *value, GParamSpec *pspec)
{
  AccountsWizardDialogPrivate *priv = PRIVATE(object);

  switch (property_id)
  {
    case PROP_ACCOUNT_ITEM:
    {
      priv->account_item = g_value_dup_object(value);
      break;
    }
    case PROP_SERVICE:
    {
      priv->service = g_value_dup_object(value);
      break;
    }
    case PROP_CANCEL_TEXT:
    {
      g_free(priv->cancel_text);
      priv->cancel_text = g_value_dup_string(value);
      break;
    }
    case PROP_MANAGER:
    {
      priv->manager = g_value_dup_object(value);
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
accounts_wizard_dialog_get_property(GObject *object, guint property_id,
                                    GValue *value, GParamSpec *pspec)
{
  AccountsWizardDialogPrivate *priv = PRIVATE(object);

  switch (property_id)
  {
    case PROP_ACCOUNT_ITEM:
    {
      g_value_set_object(value, priv->account_item);
      break;
    }
    case PROP_SERVICE:
    {
      g_value_set_object(value, priv->service);
      break;
    }
    case PROP_CANCEL_TEXT:
    {
      g_value_set_string(value, priv->cancel_text);
      break;
    }
    case PROP_MANAGER:
    {
      g_value_set_object(value, priv->manager);
      break;
    }
    default:
    {
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
      break;
    }
  }
}

#ifdef DISABLE_PORTRAIT
static void
accounts_wizard_dialog_realize(GtkWidget *widget)
{
  GdkAtom atom;
  long portrait = 0;

  GTK_WIDGET_CLASS(accounts_wizard_dialog_parent_class)->realize(widget);

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
confirm_delete(GtkDialog *dialog)
{
  AccountsWizardDialogPrivate *priv = PRIVATE(dialog);
  const gchar *format;
  gchar *name;
  gchar *description;
  GtkWidget *note;

  if (priv->account_item->connected)
    format = _("accounts_fi_delete_account_online");
  else
    format = _("accounts_fi_delete_account_offline");

  g_object_get(priv->account_item, "name", &name, NULL);

  if (!name)
    name = g_strdup("");

  description = g_strdup_printf(format, name);
  g_free(name);
  note = hildon_note_new_confirmation(GTK_WINDOW(dialog), description);
  g_free(description);
  gtk_window_set_modal(GTK_WINDOW(note), FALSE);
  gtk_window_set_destroy_with_parent(GTK_WINDOW(note), TRUE);
  g_signal_connect(note, "response",
                   G_CALLBACK(on_delete_account_response), dialog);
  gtk_widget_show(note);
}

static void
_operation_async(AccountEditContext *context, GError *error, GtkDialog *dialog)
{
  AccountsWizardDialogPrivate *priv = PRIVATE(dialog);

  g_signal_handlers_disconnect_matched(
    context, G_SIGNAL_MATCH_DATA | G_SIGNAL_MATCH_FUNC, 0, 0, NULL,
    _operation_async, dialog);

  if (priv->button_sign_in)
    gtk_widget_set_sensitive(priv->button_sign_in, TRUE);

  if (!error || ((error->domain == ACCOUNT_ERROR) &&
                 (error->code == ACCOUNT_ERROR_CONNECTION_FAILED)))
  {
    gtk_dialog_response(dialog, GTK_RESPONSE_OK);
  }
}

static void
accounts_wizard_dialog_response(GtkDialog *dialog, gint response_id)
{
  AccountsWizardDialogPrivate *priv = PRIVATE(dialog);

  void (*response)(GtkDialog *, gint);

  switch (response_id)
  {
    case 1:
    {
      GError *error = NULL;

      if (!account_dialog_context_finish(ACCOUNT_DIALOG_CONTEXT(priv->context),
                                         &error))
      {
        if (error && (error->code == ACCOUNT_ERROR_OPERATION_ASYNC))
        {
          if (priv->button_sign_in)
            gtk_widget_set_sensitive(priv->button_sign_in, FALSE);

          g_signal_connect(priv->context, "operation-async",
                           G_CALLBACK(_operation_async), dialog);
          g_error_free(error);
        }

        break;
      }
    }
    /* fall through */
    case GTK_RESPONSE_OK:
    /* fall through */
    case GTK_RESPONSE_DELETE_EVENT:
    {
      gtk_widget_destroy(GTK_WIDGET(dialog));
      break;
    }
    case 100:
    {
      confirm_delete(dialog);
      break;
    }
    default:
    {
      break;
    }
  }

  response = GTK_DIALOG_CLASS(accounts_wizard_dialog_parent_class)->response;

  if (response)
    response(dialog, response_id);
}

static void
create_signin_page(AccountsWizardDialog *wizard, AccountService *service)
{
  AccountsWizardDialogPrivate *priv = PRIVATE(wizard);

  GtkWidget *dialog;
  GError *error = NULL;

  if (priv->service == service)
  {
    if (priv->context)
    {
      g_object_unref(service);
      goto create_dialog;
    }
  }
  else if (priv->context)
  {
    account_dialog_context_cancel(ACCOUNT_DIALOG_CONTEXT(priv->context));
    g_object_unref(priv->context);
  }

  if (priv->service)
    g_object_unref(priv->service);

  priv->service = service;
  priv->context = account_service_begin_new(service);

create_dialog:
  dialog = account_dialog_context_start(ACCOUNT_DIALOG_CONTEXT(priv->context),
                                        &error);

  if (error)
  {
    if (error->message && *error->message)
      hildon_banner_show_information(GTK_WIDGET(wizard), NULL, error->message);

    g_error_free(error);
  }
  else if (dialog)
  {
    const char *msgid = _("accountwizard_ti_login");
    gchar *login_text = g_strdup_printf(msgid, service->display_name);

    priv->dialog = dialog;
    gtk_container_remove(GTK_CONTAINER(GTK_DIALOG(wizard)->vbox), priv->vbox);
    gtk_container_add(GTK_CONTAINER(GTK_DIALOG(wizard)->vbox), priv->dialog);
    gtk_window_set_title(GTK_WINDOW(wizard), login_text);
    priv->button_sign_in = gtk_dialog_add_button(
        GTK_DIALOG(wizard), _("accounts_bd_sign_in"), 1);
    gtk_widget_show_all(GTK_WIDGET(wizard));
    g_free(login_text);
  }
}

static void
service_activated_cb(GtkTreeView *self, GtkTreePath *path,
                     GtkTreeViewColumn *column, AccountsWizardDialog *dialog)
{
  GtkTreeModel *model = gtk_tree_view_get_model(self);
  GtkTreeIter iter;

  if (gtk_tree_model_get_iter(model, &iter, path))
  {
    AccountService *service;

    gtk_tree_model_get(model, &iter, COLUMN_SERVICE, &service, -1);

    if (service)
      create_signin_page(dialog, service);
    else
      g_warning("%s: unable to find service.", __FUNCTION__);
  }
}

static gint
sort_func(GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b,
          gpointer user_data)
{
  gint res;
  gchar *nameb;
  gchar *namea;
  gint priority_b;
  gint priority_a;

  gtk_tree_model_get(model, a,
                     COLUMN_NAME, &namea,
                     COLUMN_PRIORITY, &priority_a,
                     -1);
  gtk_tree_model_get(model, b,
                     COLUMN_NAME, &nameb,
                     COLUMN_PRIORITY, &priority_b, -1);

  if (priority_a < priority_b)
    res = -1;
  else if (priority_a > priority_b)
    res = 1;
  else if (!namea)
  {
    if (!nameb)
      res = 0;
    else
      res = -1;
  }
  else if (!nameb)
    res = 1;
  else
    res = strcmp(namea, nameb);

  g_free(namea);
  g_free(nameb);

  return res;
}

static GObject *
accounts_wizard_dialog_constructor(GType type, guint n_construct_properties,
                                   GObjectConstructParam *construct_properties)
{
  GObject *object;
  AccountsWizardDialogPrivate *priv;

  object = G_OBJECT_CLASS(accounts_wizard_dialog_parent_class)->constructor(
      type, n_construct_properties, construct_properties);

  priv = PRIVATE(object);

  if (!priv->manager)
    g_warning("No plugin manager");

  if (priv->account_item)
  {
    AccountPlugin *plugin = account_item_get_plugin(priv->account_item);
    GError *error = NULL;
    const gchar *title;

    if (plugin)
    {
      priv->plugin = g_object_ref(plugin);
      priv->context = account_plugin_begin_edit(plugin, priv->account_item);
      priv->dialog = account_dialog_context_start(
          ACCOUNT_DIALOG_CONTEXT(priv->context), &error);
    }
    else
      g_warning("Failed to resolve plugin for item %p", priv->account_item);

    if (error)
    {
      g_warning("Error: %s", error->message);
      g_error_free(error);
      return object;
    }

    if (!priv->dialog)
      return object;

    title = account_dialog_context_get_page_title(
        ACCOUNT_DIALOG_CONTEXT(priv->context));

    if (!title)
      title = "";

    gtk_container_add(GTK_CONTAINER(GTK_DIALOG(object)->vbox), priv->dialog);
    gtk_window_set_title(GTK_WINDOW(object), title);
    gtk_dialog_add_button(GTK_DIALOG(object),
                          dgettext("hildon-libs", "wdgt_bd_delete"), 100);

    if (priv->account_item->draft)
    {
      priv->button_sign_in = gtk_dialog_add_button(GTK_DIALOG(object),
                                                   _("accounts_bd_sign_in"), 1);
    }
    else
    {
      gtk_dialog_add_button(GTK_DIALOG(object),
                            dgettext("hildon-libs", "wdgt_bd_save"), 1);
    }

    gtk_widget_show_all(GTK_DIALOG(object)->action_area);
  }
  else
  {
    if (priv->service)
      create_signin_page(ACCOUNTS_WIZARD_DIALOG(object), priv->service);
    else
    {
      GList *plugins;
      GList *p;
      GtkCellRenderer *cell;
      GtkTreeViewColumn *column;
      GtkWidget *pannable_area;

      priv->store = gtk_list_store_new(4, G_TYPE_STRING, GDK_TYPE_PIXBUF,
                                       ACCOUNT_TYPE_SERVICE, G_TYPE_INT);
      gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(priv->store), 0,
                                      sort_func, NULL, NULL);
      gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(priv->store), 0,
                                           GTK_SORT_ASCENDING);
      plugins = account_plugin_manager_list(priv->manager);

      for (p = plugins; p; p = p->next)
      {
        if (p->data)
        {
          GList *services = account_plugin_list_services(p->data);
          GList *s;

          for (s = services; s; s = s->next)
          {
            AccountService *service = s->data;

            if (service)
            {
              const gchar *display_name;
              gint priority;

              g_object_ref(service->icon);
              priority = account_service_get_priority(service);
              display_name = account_service_get_display_name(service);
              gtk_list_store_insert_with_values(
                priv->store, NULL, 0,
                COLUMN_NAME, display_name,
                COLUMN_ICON, service->icon,
                COLUMN_SERVICE, service,
                COLUMN_PRIORITY, priority,
                -1);
            }
          }

          g_list_free(services);
        }
      }

      g_list_free(plugins);
      priv->tree_view = gtk_tree_view_new_with_model(
          GTK_TREE_MODEL(priv->store));

      g_signal_connect(priv->tree_view, "row-activated",
                       G_CALLBACK(service_activated_cb), object);

      cell = gtk_cell_renderer_pixbuf_new();
      column = gtk_tree_view_column_new_with_attributes(
          "Icon", cell, "pixbuf", COLUMN_ICON, NULL);
      gtk_tree_view_append_column(GTK_TREE_VIEW(priv->tree_view), column);

      cell = gtk_cell_renderer_text_new();
      column = gtk_tree_view_column_new_with_attributes(
          "Service", cell, "text", COLUMN_NAME, NULL);
      gtk_tree_view_append_column(GTK_TREE_VIEW(priv->tree_view), column);

      pannable_area = g_object_new(HILDON_TYPE_PANNABLE_AREA,
                                   "height-request", 350,
                                   NULL);
      priv->vbox = gtk_vbox_new(FALSE, 4);
      gtk_box_pack_start(GTK_BOX(priv->vbox), pannable_area, FALSE, FALSE, 0);
      gtk_container_add(GTK_CONTAINER(pannable_area), priv->tree_view);
      gtk_widget_show_all(priv->vbox);
      gtk_container_add(GTK_CONTAINER(GTK_DIALOG(object)->vbox), priv->vbox);
      gtk_window_set_title(GTK_WINDOW(object),
                           _("accountwizard_ti_select_service"));
      priv->dialog = priv->vbox;
      g_object_ref_sink(priv->vbox);
      return object;
    }
  }

  return object;
}

static void
accounts_wizard_dialog_class_init(AccountsWizardDialogClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS(klass);

  GTK_OBJECT_CLASS(klass)->destroy = accounts_wizard_dialog_destroy;

  object_class->set_property = accounts_wizard_dialog_set_property;
  object_class->get_property = accounts_wizard_dialog_get_property;
  object_class->constructor = accounts_wizard_dialog_constructor;

#ifdef DISABLE_PORTRAIT
  GTK_WIDGET_CLASS(klass)->realize = accounts_wizard_dialog_realize;
#endif

  GTK_DIALOG_CLASS(klass)->response = accounts_wizard_dialog_response;

  g_object_class_install_property(
    object_class, PROP_MANAGER,
    g_param_spec_object(
      "manager",
      NULL,
      NULL,
      ACCOUNT_TYPE_PLUGIN_MANAGER,
      G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE));
  g_object_class_install_property(
    object_class, PROP_ACCOUNT_ITEM,
    g_param_spec_object(
      "account-item",
      NULL,
      NULL,
      ACCOUNT_TYPE_ITEM,
      G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE));
  g_object_class_install_property(
    object_class, PROP_SERVICE,
    g_param_spec_object(
      "service",
      NULL,
      NULL,
      ACCOUNT_TYPE_SERVICE,
      G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE));
  g_object_class_install_property(
    object_class, PROP_CANCEL_TEXT,
    g_param_spec_string(
      "cancel-text",
      NULL,
      NULL,
      NULL,
      G_PARAM_READWRITE));

  signals[DELETE_ACCOUNT] =
    g_signal_new("delete-account",
                 G_TYPE_FROM_CLASS(klass),
                 G_SIGNAL_ACTION | G_SIGNAL_RUN_LAST,
                 G_STRUCT_OFFSET(AccountsWizardDialogClass, delete_account),
                 NULL, NULL,
                 g_cclosure_marshal_VOID__POINTER,
                 G_TYPE_NONE,
                 1, G_TYPE_POINTER);
}

static void
accounts_wizard_dialog_init(AccountsWizardDialog *wizard)
{
  gtk_window_set_destroy_with_parent(
    GTK_WINDOW(wizard), TRUE);
  gtk_window_set_modal(GTK_WINDOW(wizard),
                       FALSE);
}

GtkWidget *
accounts_wizard_dialog_new(GtkWindow *window,
                           AccountPluginManager
                           *manager,
                           AccountItem *item,
                           AccountService *
                           service)
{
  GtkWidget *wizard =
    g_object_new(ACCOUNTS_TYPE_WIZARD_DIALOG,
                 "has-separator",
                 FALSE,
                 "manager",
                 manager,
                 "account-item",
                 item,
                 "service",
                 service,
                 NULL);

  gtk_window_set_resizable(GTK_WINDOW(wizard),
                           FALSE);

  if (window)
    gtk_window_set_transient_for(GTK_WINDOW(
                                   wizard),
                                 window);

  return wizard;
}

gboolean
accounts_wizard_dialog_run(AccountsWizardDialog *dlg)
{
  GMainLoop *loop =
    g_main_loop_new(NULL, FALSE);

  g_signal_connect_swapped(dlg, "destroy",
                           G_CALLBACK(
                             g_main_loop_quit),
                           loop);
  gtk_widget_show(GTK_WIDGET(dlg));

  GDK_THREADS_LEAVE();

  g_main_loop_run(loop);

  GDK_THREADS_ENTER();

  g_main_loop_unref(loop);

  return TRUE;
}
