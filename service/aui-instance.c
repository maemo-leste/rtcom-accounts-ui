/*
 * aui-instance.c
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

#include <dbus/dbus-glib-lowlevel.h>
#include <dbus/dbus.h>
#include <hildon/hildon.h>
#include <telepathy-glib/telepathy-glib.h>

#include "accounts-ui.h"

#include "aui-instance.h"

struct _AuiInstancePrivate
{
  DBusGConnection *dbus_gconnection;
  gchar *object_path;
  GtkWidget *accounts_ui;
  GdkNativeWindow parent_xid;
  gboolean close_on_finish : 1; /* 0x01 0xFE*/
  gboolean unmapped : 1; /* 0x02 0xFD*/
  DBusGMethodInvocation *context;
};

typedef struct _AuiInstancePrivate AuiInstancePrivate;

#define PRIVATE(instance) \
  ((AuiInstancePrivate *) \
   aui_instance_get_instance_private((AuiInstance *)(instance)))

G_DEFINE_TYPE_WITH_PRIVATE(
  AuiInstance,
  aui_instance,
  G_TYPE_OBJECT
)

enum
{
  PROP_DBUS_CONNECTION = 1,
  PROP_PARENT_XID,
  PROP_VISIBLE
};

enum
{
  CLOSED,
  PROPERTY_CHANGED,
  LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

static int
aui_instance_close(AuiInstance *instance, GError **error)
{
  AuiInstancePrivate *priv;

  g_return_val_if_fail(AUI_IS_INSTANCE(instance), FALSE);

  priv = PRIVATE(instance);

  if (priv->accounts_ui)
  {
    gtk_widget_destroy(priv->accounts_ui);
    return TRUE;
  }

  g_set_error(error, DBUS_GERROR, 0, "No dialog");

  return FALSE;
}

#include "dbus-glib-marshal-aui-instance.h"

static GObject *
accounts_ui_constructor(GType type, guint n_construct_properties,
                        GObjectConstructParam *construct_properties)
{
  GObject *instance = G_OBJECT_CLASS(aui_instance_parent_class)->
    constructor(type, n_construct_properties, construct_properties);
  AuiInstancePrivate *priv = PRIVATE(instance);

  if (priv->dbus_gconnection)
  {
    dbus_g_connection_register_g_object(priv->dbus_gconnection,
                                        priv->object_path, instance);
  }
  else
    g_clear_object(&instance);

  return instance;
}

static void
accounts_ui_destroy_cb(GtkWidget *accounts_ui, AuiInstance *instance)
{
  AuiInstancePrivate *priv = PRIVATE(instance);

  g_object_ref(instance);

  g_signal_emit(instance, signals[CLOSED], 0);

  priv->accounts_ui = NULL;
  g_object_unref(instance);
}

static void
accounts_ui_realize_cb(GtkWidget *accounts_ui, AuiInstance *instance)
{
  AuiInstancePrivate *priv = PRIVATE(instance);

  if (priv->parent_xid)
  {
    GdkWindow *window = gdk_window_foreign_new(priv->parent_xid);

    if (window)
    {
      gdk_window_set_transient_for(accounts_ui->window, window);
      g_object_unref(window);
    }
  }
}

static void
accounts_ui_unmap_event_cb(GtkWidget *accounts_ui, GdkEventAny *event,
                           AuiInstance *instance)
{
  AuiInstancePrivate *priv = PRIVATE(instance);

  if (priv->unmapped)
    gtk_widget_destroy(accounts_ui);

  priv->unmapped = FALSE;
}

static void
accounts_ui_unmap_cb(GtkWidget *accounts_ui, AuiInstance *instance)
{
  PRIVATE(instance)->unmapped = TRUE;
}

static void
accounts_ui_dispose(GObject *object)
{
  AuiInstancePrivate *priv = PRIVATE(object);

  if (priv->accounts_ui)
  {
    g_signal_handlers_disconnect_matched(
      priv->accounts_ui, G_SIGNAL_MATCH_DATA | G_SIGNAL_MATCH_FUNC, 0, 0, NULL,
      accounts_ui_destroy_cb, object);
    g_signal_handlers_disconnect_matched(
      priv->accounts_ui, G_SIGNAL_MATCH_DATA | G_SIGNAL_MATCH_FUNC, 0, 0, NULL,
      accounts_ui_realize_cb, object);
    g_signal_handlers_disconnect_matched(
      priv->accounts_ui, G_SIGNAL_MATCH_DATA | G_SIGNAL_MATCH_FUNC, 0, 0, NULL,
      accounts_ui_unmap_event_cb, object);
    g_signal_handlers_disconnect_matched(
      priv->accounts_ui, G_SIGNAL_MATCH_DATA | G_SIGNAL_MATCH_FUNC, 0, 0, NULL,
      accounts_ui_unmap_cb, object);
    gtk_widget_destroy(priv->accounts_ui);
    priv->accounts_ui = NULL;
  }

  if (priv->dbus_gconnection)
  {
    dbus_g_connection_unref(priv->dbus_gconnection);
    priv->dbus_gconnection = NULL;
  }

  G_OBJECT_CLASS(aui_instance_parent_class)->dispose(object);
}

static void
accounts_ui_finalize(GObject *object)
{
  AuiInstancePrivate *priv = PRIVATE(object);

  g_free(priv->object_path);

  G_OBJECT_CLASS(aui_instance_parent_class)->finalize(object);
}

static void
accounts_ui_get_property(GObject *object, guint property_id, GValue *value,
                         GParamSpec *pspec)
{
  AuiInstancePrivate *priv = PRIVATE(object);

  switch (property_id)
  {
    case PROP_VISIBLE:
    {
      if (priv->accounts_ui)
        g_value_set_boolean(value, gtk_widget_get_visible(priv->accounts_ui));
      else
        g_value_set_boolean(value, FALSE);

      break;
    }
    default:
    {
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object,
                                        property_id,
                                        pspec);
      break;
    }
  }
}

static void
aui_instance_set_parent(AuiInstance *instance, guint parent_xid)
{
  AuiInstancePrivate *priv = PRIVATE(instance);
  GValue v = G_VALUE_INIT;

  g_return_if_fail(AUI_IS_INSTANCE(instance));

  if (parent_xid != priv->parent_xid)
  {
    GdkWindow *window = gdk_window_foreign_new(parent_xid);
    GHashTable *properties;

    accounts_ui_set_parent(priv->accounts_ui, window);

    if (window)
      g_object_unref(window);

    priv->parent_xid = parent_xid;
    properties = g_hash_table_new((GHashFunc)&g_str_hash,
                                  (GEqualFunc)&g_str_equal);

    g_value_init(&v, G_TYPE_UINT);
    g_value_set_uint(&v, priv->parent_xid);
    g_hash_table_insert(properties, "com.nokia.Accounts.UI.ParentXid", &v);
    g_signal_emit(instance, signals[PROPERTY_CHANGED], 0, properties);
    g_hash_table_destroy(properties);
  }
}

static void
aui_instance_set_visible(AuiInstance *instance, gboolean visible)
{
  AuiInstancePrivate *priv = PRIVATE(instance);
  GValue v = G_VALUE_INIT;

  g_return_if_fail(AUI_IS_INSTANCE(instance));

  if (priv->accounts_ui &&
      (gtk_widget_get_visible(priv->accounts_ui) != visible))
  {
    GHashTable *properties;

    if (visible)
      gtk_widget_show(priv->accounts_ui);
    else
      gtk_widget_hide(priv->accounts_ui);

    properties = g_hash_table_new((GHashFunc)&g_str_hash,
                                  (GEqualFunc)&g_str_equal);

    g_value_init(&v, G_TYPE_BOOLEAN);
    g_value_set_uint(&v, visible);
    g_hash_table_insert(properties, "com.nokia.Accounts.UI.Visible", &v);
    g_signal_emit(instance, signals[PROPERTY_CHANGED], 0, properties);
    g_hash_table_destroy(properties);
  }
}

static void
accounts_ui_set_property(GObject *object, guint property_id,
                         const GValue *value, GParamSpec *pspec)
{
  switch (property_id)
  {
    case PROP_PARENT_XID:
    {
      aui_instance_set_parent(AUI_INSTANCE(object), g_value_get_uint(value));
      break;
    }
    case PROP_VISIBLE:
    {
      aui_instance_set_visible(AUI_INSTANCE(object),
                               g_value_get_boolean(value));
      break;
    }
    case PROP_DBUS_CONNECTION:
    {
      AuiInstancePrivate *priv = PRIVATE(object);

      g_assert(priv->dbus_gconnection == NULL);

      priv->dbus_gconnection = g_value_dup_boxed(value);
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
aui_instance_class_init(AuiInstanceClass *klass)
{
  GObjectClass *object_class =
    G_OBJECT_CLASS(klass);

  object_class->constructor = accounts_ui_constructor;
  object_class->dispose = accounts_ui_dispose;
  object_class->finalize = accounts_ui_finalize;
  object_class->get_property = accounts_ui_get_property;
  object_class->set_property = accounts_ui_set_property;

  g_object_class_install_property(
    object_class, PROP_DBUS_CONNECTION,
    g_param_spec_boxed(
      "dbus-connection",
      "dbus-connection",
      "dbus-connection",
      DBUS_TYPE_G_CONNECTION,
      G_PARAM_CONSTRUCT_ONLY | G_PARAM_WRITABLE));
  g_object_class_install_property(
    object_class, PROP_PARENT_XID,
    g_param_spec_uint(
      "parent-xid",
      "parent-xid",
      "parent-xid",
      0, G_MAXUINT32, 0,
      G_PARAM_WRITABLE));
  g_object_class_install_property(
    object_class, PROP_VISIBLE,
    g_param_spec_boolean(
      "visible",
      "visible",
      "visible", TRUE,
      G_PARAM_WRITABLE | G_PARAM_READABLE));
  signals[CLOSED] = g_signal_new(
      "closed", G_TYPE_FROM_CLASS(klass),
      G_SIGNAL_RUN_LAST,
      0, NULL, NULL, g_cclosure_marshal_VOID__VOID,
      G_TYPE_NONE,
      0);
  signals[PROPERTY_CHANGED] = g_signal_new(
      "property-changed", G_TYPE_FROM_CLASS(klass),
      G_SIGNAL_RUN_LAST,
      0, NULL, NULL,
      g_cclosure_marshal_VOID__BOXED,
      G_TYPE_NONE,
      1, TP_HASH_TYPE_STRING_VARIANT_MAP);

  dbus_g_object_type_install_info(
    G_TYPE_FROM_CLASS(klass), &dbus_glib_aui_instance_object_info);
}

static void
aui_instance_init(AuiInstance *instance)
{
  static uint instance_id = 0;
  AuiInstancePrivate *priv = PRIVATE(instance);

  priv->accounts_ui = g_object_new(ACCOUNTS_TYPE_UI, NULL);

  g_assert(ACCOUNTS_IS_UI(priv->accounts_ui));

  g_signal_connect(priv->accounts_ui, "destroy",
                   G_CALLBACK(accounts_ui_destroy_cb), instance);
  g_signal_connect(priv->accounts_ui, "realize",
                   G_CALLBACK(accounts_ui_realize_cb), instance);
  g_signal_connect(priv->accounts_ui, "unmap-event",
                   G_CALLBACK(accounts_ui_unmap_event_cb), instance);
  g_signal_connect(priv->accounts_ui, "unmap",
                   G_CALLBACK(accounts_ui_unmap_cb), instance);
  priv->object_path = g_strdup_printf("/com/nokia/AccountsUI/instance%u",
                                      ++instance_id);
}

AuiInstance *
aui_instance_new(DBusGConnection *dbus_gconnection,
                 guint xid)
{
  return g_object_new(AUI_TYPE_INSTANCE,
                      "dbus-connection",
                      dbus_gconnection,
                      "parent-xid", xid,
                      NULL);
}

const gchar *
aui_instance_get_object_path(AuiInstance *instance)
{
  g_return_val_if_fail(AUI_IS_INSTANCE(instance), NULL);

  return PRIVATE(instance)->object_path;
}

GHashTable *
aui_instance_get_properties(AuiInstance *instance)
{
  GValue *v;
  AuiInstancePrivate *priv;
  GHashTable *properties;

  g_return_val_if_fail(AUI_IS_INSTANCE(instance), NULL);

  priv = PRIVATE(instance);
  properties = g_hash_table_new_full((GHashFunc)&g_str_hash,
                                     (GEqualFunc)&g_str_equal,
                                     (GDestroyNotify)g_free,
                                     (GDestroyNotify)g_free);
  v = g_new0(GValue, 1);
  g_value_init(v, G_TYPE_UINT);
  g_value_set_uint(v, priv->parent_xid);
  g_hash_table_insert(properties,
                      g_strdup("com.nokia.Accounts.UI.ParentXid"), v);

  v = g_new0(GValue, 1);
  g_value_init(v, G_TYPE_BOOLEAN);

  if (priv->accounts_ui)
    g_value_set_boolean(v, gtk_widget_get_visible(priv->accounts_ui));
  else
    g_value_set_boolean(v, FALSE);

  g_hash_table_insert(properties,
                      g_strdup("com.nokia.Accounts.UI.Visible"), v);

  return properties;
}

void
aui_instance_action_open_accounts_list(AuiInstance *instance, GError **error)
{
  AuiInstancePrivate *priv = PRIVATE(instance);

  g_return_if_fail(AUI_IS_INSTANCE(instance));
  g_return_if_fail(ACCOUNTS_IS_UI(priv->accounts_ui));

  priv = PRIVATE(instance);

  if (gtk_widget_get_visible(priv->accounts_ui))
    priv->close_on_finish = FALSE;
  else
    accounts_ui_show(priv->accounts_ui);
}

struct auieditdata
{
  gchar *service_name;
  gchar *user_name;
};

static void
auieditdata_destroy(struct auieditdata *data)
{
  g_free(data->service_name);
  g_free(data->user_name);
  g_slice_free(struct auieditdata, data);
}

static void
on_requested_dialog_destroy(GtkWidget *accounts_ui, AuiInstance *instance)
{
  AuiInstancePrivate *priv;

  g_return_if_fail(AUI_IS_INSTANCE(instance));

  priv = PRIVATE(instance);

  if (priv->close_on_finish)
  {
    gtk_widget_hide(priv->accounts_ui);
    g_idle_add((GSourceFunc)gtk_widget_destroy, priv->accounts_ui);
    priv->accounts_ui = NULL;
  }
}

static void
on_accounts_ui_initialized(AuiInstance *instance)
{
  AuiInstancePrivate *priv = PRIVATE(instance);
  struct auieditdata *data;
  GtkWidget *dialog;

  data = g_object_get_data(&instance->parent, "auieditdata");

  if (!data)
    return;

  g_return_if_fail(priv->context);

  if (data->user_name)
  {
    dialog = accounts_ui_dialogs_get_edit_account(priv->accounts_ui,
                                                  data->service_name,
                                                  data->user_name);
  }
  else
  {
    dialog = accounts_ui_dialogs_get_new_account(priv->accounts_ui,
                                                 data->service_name);
  }

  g_object_set_data(&instance->parent, "auieditdata", NULL);

  if (dialog)
  {
    g_signal_connect_after(dialog, "destroy",
                           G_CALLBACK(on_requested_dialog_destroy), instance);

    gtk_widget_show(dialog);
    dbus_g_method_return(priv->context, priv->object_path,
                         aui_instance_get_properties(instance));
  }
  else
  {
    GError error;

    error.domain = DBUS_GERROR;
    error.code = DBUS_GERROR_INVALID_ARGS;
    error.message = "Couldn't open dialog";
    dbus_g_method_return_error(priv->context, &error);
    g_object_unref(instance);
  }

  priv->context = NULL;
}

static void
accounts_ui_initialized_cb(GtkWidget *accounts_ui, GParamSpec *pspec,
                           AuiInstance *instance)
{
  gboolean initialized = FALSE;

  g_object_get(accounts_ui,
               "initialized", &initialized,
               NULL);

  if (initialized)
  {
    g_signal_handlers_disconnect_matched(
      accounts_ui, G_SIGNAL_MATCH_DATA | G_SIGNAL_MATCH_FUNC,
      0, 0, NULL, accounts_ui_initialized_cb, instance);
    on_accounts_ui_initialized(instance);
  }
}

gboolean
aui_instance_action_new_account(AuiInstance *instance,
                                const gchar *service_name,
                                const gchar *on_finish,
                                DBusGMethodInvocation *context)
{
  AuiInstancePrivate *priv;
  struct auieditdata *data;
  gboolean initialized = FALSE;

  g_return_val_if_fail(AUI_IS_INSTANCE(instance), FALSE);
  g_return_val_if_fail(service_name != NULL, FALSE);

  priv = PRIVATE(instance);

  if (gtk_widget_get_visible(priv->accounts_ui))
  {
    dbus_g_method_return(context, priv->object_path,
                         aui_instance_get_properties(instance));
    return FALSE;
  }

  priv->context = context;

  if (on_finish && !strcmp(on_finish, "close"))
    priv->close_on_finish = TRUE;

  data = g_slice_new0(struct auieditdata);
  data->service_name = g_strdup(service_name);
  g_object_set_data_full(G_OBJECT(instance), "auieditdata", data,
                         (GDestroyNotify)auieditdata_destroy);
  g_object_get(G_OBJECT(priv->accounts_ui),
               "initialized", &initialized,
               NULL);

  if (initialized)
    on_accounts_ui_initialized(instance);
  else
  {
    g_signal_connect(priv->accounts_ui, "notify::initialized",
                     G_CALLBACK(accounts_ui_initialized_cb), instance);
  }

  return TRUE;
}

gboolean
aui_instance_action_edit_account(AuiInstance *instance,
                                 const gchar *account_name,
                                 const char *on_finish,
                                 DBusGMethodInvocation *context)
{
  AuiInstancePrivate *priv;
  GStrv parameters;
  gboolean initialized = FALSE;

  g_return_val_if_fail(AUI_IS_INSTANCE(instance), FALSE);
  g_return_val_if_fail(account_name != NULL, FALSE);

  priv = PRIVATE(instance);

  if (gtk_widget_get_visible(priv->accounts_ui))
  {
    dbus_g_method_return(context, priv->object_path,
                         aui_instance_get_properties(instance));
    return FALSE;
  }

  if (on_finish && !strcmp(on_finish, "close"))
    priv->close_on_finish = TRUE;

  parameters = g_strsplit(account_name, "/", 3);

  if (parameters && parameters[0] && parameters[1] && parameters[2])
  {
    struct auieditdata *data = g_slice_new(struct auieditdata);
    gchar *service_name = g_strconcat(parameters[0], "/", parameters[1], NULL);

    g_free(parameters[0]);
    g_free(parameters[1]);

    priv->context = context;

    data->service_name = service_name;
    data->user_name = parameters[2];
    g_object_set_data_full(G_OBJECT(instance), "auieditdata", data,
                           (GDestroyNotify)auieditdata_destroy);
    g_object_get(G_OBJECT(priv->accounts_ui),
                 "initialized", &initialized,
                 NULL);

    if (initialized)
      on_accounts_ui_initialized(instance);
    else
    {
      g_signal_connect(priv->accounts_ui, "notify::initialized",
                       G_CALLBACK(accounts_ui_initialized_cb), instance);
    }

    g_free(parameters);
    return TRUE;
  }
  else
  {
    GError error;

    error.domain = DBUS_GERROR;
    error.code = DBUS_GERROR_INVALID_ARGS;
    error.message = "Expected <cm_name/protocol_name>/<account>";
    dbus_g_method_return_error(context, &error);
    g_strfreev(parameters);
  }

  return FALSE;
}
