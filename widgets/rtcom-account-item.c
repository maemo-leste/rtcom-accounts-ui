/*
 * rtcom-account-item.c
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

#include <hildon/hildon.h>
#include <telepathy-glib/telepathy-glib.h>

#include "rtcom-account-marshal.h"

#include "rtcom-account-item.h"

G_DEFINE_TYPE(
  RtcomAccountItem,
  rtcom_account_item,
  ACCOUNT_TYPE_ITEM
)

enum
{
  PROP_ACCOUNT = 1
};

enum
{
  STORE_SETTINGS,
  NAME_CHANGED,
  CONNECTION_STATUS_CHANGED,
  UPDATED,
  LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

enum
{
  AVATAR_SET = 0x1,
  DISPLAY_NAME_SET = 0x2,
  NICKNAME_SET = 0x4,
  SVCF_SET = 0x8,
  ENABLED_SET = 0x10
};

static GdkPixbuf *
avatar_to_pixbuf(const guchar *data, gsize len, const char *mime_type)
{
  GdkPixbufLoader *loader;
  GdkPixbuf *pixbuf;

  if (!data || !mime_type || !*mime_type)
    return NULL;

  loader = gdk_pixbuf_loader_new_with_mime_type(mime_type, NULL);

  if (!loader)
    return NULL;

  gdk_pixbuf_loader_set_size(loader, HILDON_ICON_SIZE_FINGER,
                             HILDON_ICON_SIZE_FINGER);
  gdk_pixbuf_loader_write(loader, data, len, NULL);
  gdk_pixbuf_loader_close(loader, NULL);
  pixbuf = gdk_pixbuf_loader_get_pixbuf(loader);

  if (pixbuf)
    g_object_ref(pixbuf);

  g_object_unref(loader);

  return pixbuf;
}

static void
get_avatar_ready_cb(TpProxy *proxy, const GValue *out_Value,
                    const GError *error, gpointer user_data,
                    GObject *weak_object)
{
  if (error)
  {
    g_warning("%s: Could not get new avatar data %s", __FUNCTION__,
              error->message);
  }
  else if (!G_VALUE_HOLDS(out_Value, TP_STRUCT_TYPE_AVATAR))
  {
    g_warning("%s: Avatar had wrong type: %s", __FUNCTION__,
              G_VALUE_TYPE_NAME(out_Value));
  }
  else
  {
    AccountItem *item = ACCOUNT_ITEM(weak_object);
    GValueArray *array;
    const GArray *avatar;
    const gchar *mime_type;

    array = g_value_get_boxed(out_Value);
    tp_value_array_unpack(array, 2, &avatar, &mime_type);

    if (item->avatar)
    {
      g_object_unref(item->avatar);
      item->avatar = NULL;
    }

    if (avatar)
    {
      item->avatar =
        avatar_to_pixbuf((guchar *)avatar->data, avatar->len, mime_type);
    }

    g_object_notify(G_OBJECT(item), "avatar");
  }
}

static void
on_avatar_changed(TpAccount *account, gpointer user_data)
{
  tp_cli_dbus_properties_call_get(
    account, -1, TP_IFACE_ACCOUNT_INTERFACE_AVATAR, "Avatar",
    get_avatar_ready_cb, NULL, NULL, user_data);
}

static void
on_status_changed(TpAccount *account, guint old_status, guint new_status,
                  guint reason, gchar *dbus_error_name, GHashTable *details,
                  gpointer user_data)
{
  ACCOUNT_ITEM(user_data)->connected = new_status ==
    TP_CONNECTION_STATUS_CONNECTED;

  g_signal_emit(
    user_data, signals[CONNECTION_STATUS_CHANGED], 0, new_status, reason);
}

static void
on_properties_changed(TpAccount *proxy, GHashTable *properties,
                      gpointer user_data, GObject *weak_object)
{
  AccountItem *item;
  const GValue *v;

  g_return_if_fail(ACCOUNT_IS_ITEM(weak_object));

  item = ACCOUNT_ITEM(weak_object);
  v = g_hash_table_lookup(properties, "account");

  if (v)
  {
    const gchar *new_name = g_value_get_string(v);

    if (new_name)
    {
      if (!item->name || strcmp(item->name, new_name))
      {
        g_free(item->name);
        item->name = g_strdup(new_name);
        g_object_notify(G_OBJECT(item), "name");
      }
    }
  }
}

static void
change_name(AccountItem *item, const gchar *name)
{
  if (!name)
    return;

  g_free(item->display_name);
  ACCOUNT_ITEM(item)->display_name = g_strdup(name);
  g_object_notify(G_OBJECT(item), "display-name");
}

static void
on_display_name_changed(TpAccount *account, GParamSpec *pspec,
                        RtcomAccountItem *item)
{
  const gchar *name = tp_account_get_nickname(account);

  if (!name || !*name)
    name = tp_account_get_display_name(account);

  change_name(ACCOUNT_ITEM(item), name);
}

static void
on_nickname_changed(TpAccount *account, GParamSpec *pspec,
                    RtcomAccountItem *item)
{
  change_name(ACCOUNT_ITEM(item), tp_account_get_nickname(account));
}

static void
on_enabled_changed(TpAccount *account, GParamSpec *pspec,
                   RtcomAccountItem *item)
{
  ACCOUNT_ITEM(item)->enabled = tp_account_is_enabled(account);
  g_object_notify(G_OBJECT(item), "enabled");
}

static void
on_has_been_online_changed(TpAccount *account, GParamSpec *pspec,
                           RtcomAccountItem *item)
{
  ACCOUNT_ITEM(item)->draft = !tp_account_get_has_been_online(account);
  g_object_notify(G_OBJECT(item), "draft");
}

static void
tp_account_unref(RtcomAccountItem *item)
{
  if (!item || !item->account)
    return;

  g_signal_handlers_disconnect_matched(
    item->account, G_SIGNAL_MATCH_DATA | G_SIGNAL_MATCH_FUNC, 0, 0, NULL,
    on_properties_changed, item);
  g_signal_handlers_disconnect_matched(
    item->account, G_SIGNAL_MATCH_DATA | G_SIGNAL_MATCH_FUNC, 0, 0, NULL,
    on_display_name_changed, item);
  g_signal_handlers_disconnect_matched(
    item->account, G_SIGNAL_MATCH_DATA | G_SIGNAL_MATCH_FUNC, 0, 0, NULL,
    on_nickname_changed, item);
  g_signal_handlers_disconnect_matched(
    item->account, G_SIGNAL_MATCH_DATA | G_SIGNAL_MATCH_FUNC, 0, 0, NULL,
    on_enabled_changed, item);
  g_signal_handlers_disconnect_matched(
    item->account, G_SIGNAL_MATCH_DATA | G_SIGNAL_MATCH_FUNC, 0, 0, NULL,
    on_avatar_changed, item);

  g_signal_handlers_disconnect_matched(
    item->account, G_SIGNAL_MATCH_DATA | G_SIGNAL_MATCH_FUNC, 0, 0, NULL,
    on_status_changed, item);

  g_object_unref(item->account);
  item->account = NULL;
}

static void
rtcom_account_item_dispose(GObject *object)
{
  tp_account_unref(RTCOM_ACCOUNT_ITEM(object));

  G_OBJECT_CLASS(rtcom_account_item_parent_class)->dispose(object);
}

static void
free_store_data(RtcomAccountItem *item)
{
  if (item->new_params)
    g_hash_table_remove_all(item->new_params);

  g_free(item->avatar_data);
  item->avatar_data = NULL;

  g_free(item->avatar_mime);
  item->avatar_mime = NULL;

  g_free(item->display_name);
  item->display_name = NULL;

  g_free(item->nickname);
  item->nickname = NULL;

  g_strfreev(item->secondary_vcard_fields);
  item->secondary_vcard_fields = NULL;
}

static void
rtcom_account_item_finalize(GObject *object)
{
  RtcomAccountItem *item = RTCOM_ACCOUNT_ITEM(object);

  free_store_data(item);
  g_hash_table_destroy(item->new_params);

  G_OBJECT_CLASS(rtcom_account_item_parent_class)->finalize(object);
}

static void
ready_cb(GObject *object, GAsyncResult *res, gpointer user_data)
{
  AccountItem *item = user_data;
  TpAccount *account = (TpAccount *)object;
  GError *error = NULL;
  const GValue *v;
  gboolean bval;
  const gchar *display_name;

  if (!tp_proxy_prepare_finish(object, res, &error))
  {
    g_warning("%s: Could not prepare account %s: %s", __FUNCTION__,
              tp_account_get_path_suffix(account), error->message);
    g_clear_error(&error);
    return;
  }

  v = g_hash_table_lookup((GHashTable *)tp_account_get_parameters(account),
                          "account");

  if (v)
  {
    const gchar *name = g_value_get_string(v);

    if (name)
    {
      if (!item->name || strcmp(item->name, name))
      {
        g_free(item->name);
        item->name = g_strdup(name);
        g_object_notify(G_OBJECT(item), "name");
      }
    }
  }

  bval = tp_account_is_enabled(account);

  if (bval != item->enabled)
  {
    item->enabled = bval;
    g_object_notify(&item->parent_instance, "enabled");
  }

  item->connected = tp_account_get_connection_status(account, NULL) ==
    TP_CONNECTION_STATUS_CONNECTED;

  bval = tp_account_get_has_been_online(account);

  if (item->draft != !bval)
  {
    item->draft = !bval;
    g_object_notify(G_OBJECT(item), "draft");
  }

  display_name = tp_account_get_nickname(account);

  if (!display_name)
    display_name = tp_account_get_display_name(account);

  if (!display_name || !item->display_name ||
      strcmp(display_name, item->display_name))
  {
    g_free(item->display_name);
    item->display_name = g_strdup(display_name);
    g_object_notify(G_OBJECT(item), "display-name");
  }

  if (item->supports_avatar && !item->avatar)
  {
    tp_cli_dbus_properties_call_get(
      account, -1, TP_IFACE_ACCOUNT_INTERFACE_AVATAR, "Avatar",
      get_avatar_ready_cb, NULL, NULL, G_OBJECT(item));
  }
}

static void
rtcom_account_item_set_property(GObject *object, guint property_id,
                                const GValue *value, GParamSpec *pspec)
{
  g_return_if_fail(RTCOM_IS_ACCOUNT_ITEM(object));

  switch (property_id)
  {
    case PROP_ACCOUNT:
    {
      RtcomAccountItem *item = RTCOM_ACCOUNT_ITEM(object);

      g_return_if_fail(item->account == NULL);

      item->account = g_value_dup_object(value);

      if (item->account)
      {
        GQuark account_features[] =
        {
          TP_ACCOUNT_FEATURE_CORE,
          0
        };

        g_signal_connect(item->account, "avatar-changed",
                         G_CALLBACK(on_avatar_changed), item);

        tp_cli_account_connect_to_account_property_changed(
          item->account, on_properties_changed, NULL, NULL,
          G_OBJECT(item), NULL);

        g_signal_connect(item->account, "notify::display-name",
                         G_CALLBACK(on_display_name_changed), item);
        g_signal_connect(item->account, "notify::nickname",
                         G_CALLBACK(on_nickname_changed), item);
        g_signal_connect(item->account, "notify::enabled",
                         G_CALLBACK(on_enabled_changed), item);
        g_signal_connect(item->account, "notify::has-been-online",
                         G_CALLBACK(on_has_been_online_changed), item);
        g_signal_connect(item->account, "status-changed",
                         G_CALLBACK(on_status_changed), item);

        tp_proxy_prepare_async(
          TP_PROXY(item->account), account_features, ready_cb, item);
      }

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
rtcom_account_item_get_property(GObject *object, guint property_id,
                                GValue *value, GParamSpec *pspec)
{
  g_return_if_fail(RTCOM_IS_ACCOUNT_ITEM(object));

  switch (property_id)
  {
    case PROP_ACCOUNT:
    {
      g_value_set_object(value, RTCOM_ACCOUNT_ITEM(object)->account);
      break;
    }
    default:
    {
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
      break;
    }
  }
}

typedef struct
{
  GMainLoop *loop;
  gboolean enabled;
  GError **error;
}
set_enabled_data;

static void
set_enabled_cb(GObject *object, GAsyncResult *res, gpointer user_data)
{
  TpAccount *account = (TpAccount *)object;
  set_enabled_data *data = user_data;

  data->enabled = tp_account_set_enabled_finish(account, res, data->error);
  g_main_loop_quit(data->loop);
}

static gboolean
rtcom_account_item_set_enabled(AccountItem *account, gboolean enabled,
                               GError **error)
{
  RtcomAccountItem *item = RTCOM_ACCOUNT_ITEM(account);

  set_enabled_data data;

  g_return_val_if_fail(item->account, FALSE);

  data.loop = g_main_loop_new(NULL, FALSE);
  tp_account_set_enabled_async(item->account, enabled, set_enabled_cb, &data);

  GDK_THREADS_LEAVE();
  g_main_loop_run(data.loop);
  GDK_THREADS_ENTER();

  g_main_loop_unref(data.loop);

  return data.enabled;
}

static gboolean
stop_on_false_accumulator(GSignalInvocationHint *ihint, GValue *return_accu,
                          const GValue *handler_return, gpointer data)
{
  gboolean rv = g_value_get_boolean(handler_return);

  g_value_set_boolean(return_accu, rv);

  return rv;
}

static void
rtcom_account_item_class_init(RtcomAccountItemClass *
                              klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS(klass);

  object_class->dispose = rtcom_account_item_dispose;
  object_class->finalize = rtcom_account_item_finalize;
  object_class->set_property = rtcom_account_item_set_property;
  object_class->get_property = rtcom_account_item_get_property;
  ACCOUNT_ITEM_CLASS(klass)->set_enabled = rtcom_account_item_set_enabled;

  g_object_class_install_property(
    object_class, PROP_ACCOUNT,
    g_param_spec_object(
      "account",
      "TpAccount",
      "TpAccount",
      TP_TYPE_ACCOUNT,
      G_PARAM_WRITABLE | G_PARAM_READABLE));

  signals[STORE_SETTINGS] =
    g_signal_new(
      "store-settings", G_TYPE_FROM_CLASS(klass),
      G_SIGNAL_ACTION | G_SIGNAL_RUN_LAST, 0, stop_on_false_accumulator, NULL,
      rtcom_account_marshal_BOOLEAN__POINTER,
      G_TYPE_BOOLEAN, 1, G_TYPE_POINTER);

  signals[NAME_CHANGED] =
    g_signal_new(
      "name-changed", G_TYPE_FROM_CLASS(klass),
      G_SIGNAL_ACTION | G_SIGNAL_RUN_LAST, 0, NULL, NULL,
      g_cclosure_marshal_VOID__VOID,
      G_TYPE_NONE, 0);

  signals[CONNECTION_STATUS_CHANGED] =
    g_signal_new(
      "connection-status-changed", G_TYPE_FROM_CLASS(klass),
      G_SIGNAL_ACTION | G_SIGNAL_RUN_LAST, 0, NULL, NULL,
      rtcom_account_marshal_VOID__UINT_UINT,
      G_TYPE_NONE, 2, G_TYPE_UINT, G_TYPE_UINT);

  signals[UPDATED] =
    g_signal_new(
      "updated", G_TYPE_FROM_CLASS(klass),
      G_SIGNAL_ACTION | G_SIGNAL_RUN_LAST, 0, NULL, NULL,
      g_cclosure_marshal_VOID__BOOLEAN,
      G_TYPE_NONE, 1, G_TYPE_BOOLEAN);
}

static void
g_value_free(GValue *value)
{
  g_value_unset(value);
  g_free(value);
}

static void
rtcom_account_item_init(RtcomAccountItem *item)
{
  item->new_params = g_hash_table_new_full(
      (GHashFunc)&g_str_hash,
      (GEqualFunc)&g_str_equal,
      (GDestroyNotify)&g_free,
      (GDestroyNotify)g_value_free);
}

RtcomAccountItem *
rtcom_account_item_new(TpAccount *account, RtcomAccountService *service)
{
  RtcomAccountItem *item;

  g_return_val_if_fail(service != NULL, NULL);

  item = g_object_new(RTCOM_TYPE_ACCOUNT_ITEM,
                      "service", service,
                      NULL);
  g_object_set(item,
               "account", account,
               NULL);

  return item;
}

static gboolean
rtcom_account_item_verify_parameter(RtcomAccountItem *item, const gchar *name)
{
  AccountService *service = account_item_get_service(ACCOUNT_ITEM(item));

  if (tp_protocol_has_param(RTCOM_ACCOUNT_SERVICE(service)->protocol, name))
    return TRUE;

  g_warning("Parameter %s is not supported by service %s", name,
            service->name);

  return FALSE;
}

void
rtcom_account_item_store_param_boolean(RtcomAccountItem *item,
                                       const gchar *name, gboolean value)
{
  GValue *v;

  if (!rtcom_account_item_verify_parameter(item, name))
    return;

  v = g_new0(GValue, 1);
  g_value_init(v, G_TYPE_BOOLEAN);
  g_value_set_boolean(v, value);
  g_hash_table_insert(item->new_params, g_strdup(name), v);
}

void
rtcom_account_item_store_param_uint(RtcomAccountItem *item, const gchar *name,
                                    guint value)
{
  GValue *v;

  if (!rtcom_account_item_verify_parameter(item, name))
    return;

  v = g_new0(GValue, 1);
  g_value_init(v, G_TYPE_UINT);
  g_value_set_uint(v, value);
  g_hash_table_insert(item->new_params, g_strdup(name), v);
}

void
rtcom_account_item_store_param_int(RtcomAccountItem *item, const gchar *name,
                                   int value)
{
  GValue *v;

  if (!rtcom_account_item_verify_parameter(item, name))
    return;

  v = g_new0(GValue, 1);
  g_value_init(v, G_TYPE_INT);
  g_value_set_int(v, value);
  g_hash_table_insert(item->new_params, g_strdup(name), v);
}

void
rtcom_account_item_store_param_string(RtcomAccountItem *item, const gchar *name,
                                      const gchar *value)
{
  GValue *v;

  if (!rtcom_account_item_verify_parameter(item, name))
    return;

  v = g_new0(GValue, 1);
  g_value_init(v, G_TYPE_STRING);
  g_value_set_string(v, value);

  g_hash_table_insert(item->new_params, g_strdup(name), v);
}

void
rtcom_account_item_store_display_name(RtcomAccountItem *item, const gchar *name)
{
  item->set_mask |= DISPLAY_NAME_SET;
  item->display_name = g_strdup(name);
}

void
rtcom_account_item_store_nickname(RtcomAccountItem *item, const gchar *name)
{
  item->set_mask |= NICKNAME_SET;
  item->nickname = g_strdup(name);
}

void
rtcom_account_item_store_avatar(RtcomAccountItem *item, gchar *data, gsize len,
                                const gchar *mime_type)
{
  item->avatar_data = data;
  item->avatar_len = len;
  item->set_mask |= AVATAR_SET;
  item->avatar_mime = g_strdup(mime_type);
}

void
rtcom_account_item_store_secondary_vcard_fields(RtcomAccountItem *item,
                                                GList *fields)
{
  gchar **secondary_vcard_fields;
  int i = 0;

  secondary_vcard_fields = g_new(gchar *, g_list_length(fields) + 1);

  while (fields)
  {
    secondary_vcard_fields[i++] = g_strdup(fields->data);
    fields = fields->next;
  }

  secondary_vcard_fields[i] = NULL;

  if (item->secondary_vcard_fields)
    g_strfreev(item->secondary_vcard_fields);

  item->secondary_vcard_fields = secondary_vcard_fields;
  item->set_mask |= SVCF_SET;
}

void
rtcom_account_item_unset_param(RtcomAccountItem *item, const gchar *name)
{
  g_hash_table_remove(item->new_params, name);
}

gboolean
rtcom_account_item_store_settings(RtcomAccountItem *item, GError **error)
{
  TpProtocol *protocol = rtcom_account_item_get_tp_protocol(item);
  gboolean result;

  if (protocol && tp_protocol_has_param(protocol, "server"))
  {
    const TpConnectionManagerParam *server;

    server = tp_protocol_get_param(protocol, "server");

    if (server)
    {
      GValue v = G_VALUE_INIT;

      if (tp_connection_manager_param_get_default(server, &v) &&
          G_VALUE_HOLDS_STRING(&v))
      {
        rtcom_account_item_store_param_string(item,
                                              "server", g_value_get_string(&v));
      }
    }

    g_object_unref(protocol);
  }

  item->set_mask = 0;
  g_signal_emit(item, signals[STORE_SETTINGS], 0, error, &result);

  if (!result)
    free_store_data(item);

  return result;
}

TpProtocol *
rtcom_account_item_get_tp_protocol(RtcomAccountItem *item)
{
  RtcomAccountService *service;
  TpProtocol *protocol;

  g_return_val_if_fail(RTCOM_IS_ACCOUNT_ITEM(item), NULL);

  service = RTCOM_ACCOUNT_SERVICE(ACCOUNT_ITEM(item)->service);
  g_return_val_if_fail(service != NULL, NULL);

  protocol = rtcom_account_service_get_protocol(service);
  g_return_val_if_fail(protocol != NULL, NULL);

  return g_object_ref(protocol);
}

const gchar *
rtcom_account_item_get_unique_name(RtcomAccountItem *item)
{
  g_return_val_if_fail(RTCOM_IS_ACCOUNT_ITEM(item), NULL);

  if (item->account)
    return tp_account_get_path_suffix(item->account);

  return NULL;
}

void
rtcom_account_item_name_change(RtcomAccountItem *item)
{
  g_return_if_fail(RTCOM_IS_ACCOUNT_ITEM(item));

  g_signal_emit(item, signals[NAME_CHANGED], 0);
}

static void
service_connected(GObject *object, TpConnection *connection, GError *error,
                  gpointer user_data)
{
  g_signal_emit_by_name(object, "verified", error);
}

void
rtcom_account_item_verify(RtcomAccountItem *item, GError **error)
{
  g_return_if_fail(RTCOM_IS_ACCOUNT_ITEM(item));

  rtcom_account_service_connect(
    RTCOM_ACCOUNT_SERVICE(ACCOUNT_ITEM(item)->service), item->new_params,
    G_OBJECT(item), TRUE, service_connected, NULL);

  g_set_error(error, ACCOUNT_ERROR, ACCOUNT_ERROR_OPERATION_ASYNC, "%s", "");
}

void
rtcom_account_item_reconnect(RtcomAccountItem *item)
{
  g_return_if_fail(RTCOM_IS_ACCOUNT_ITEM(item));

  if (item->account)
  {
    tp_account_request_presence_async(item->account,
                                      TP_CONNECTION_PRESENCE_TYPE_OFFLINE,
                                      "offline", NULL, NULL, NULL);
    tp_account_request_presence_async(item->account,
                                      TP_CONNECTION_PRESENCE_TYPE_AVAILABLE,
                                      "available", NULL, NULL, NULL);
  }
}

void
rtcom_account_item_call_reconnect(RtcomAccountItem *item)
{
  g_return_if_fail(RTCOM_IS_ACCOUNT_ITEM(item));

  if (item->account)
    tp_account_reconnect_async(item->account, NULL, NULL);
}

void
rtcom_account_store_enabled_setting(RtcomAccountItem *item, gboolean enabled)
{
  g_return_if_fail(RTCOM_IS_ACCOUNT_ITEM(item));

  item->enabled_setting = enabled;
  item->set_mask |= ENABLED_SET;
}

gboolean
rtcom_account_get_enabled_setting(RtcomAccountItem *item)
{
  g_return_val_if_fail(RTCOM_IS_ACCOUNT_ITEM(item), FALSE);

  return item->enabled_setting;
}

gboolean
rtcom_account_item_delete(RtcomAccountItem *item)
{
  g_return_val_if_fail(RTCOM_IS_ACCOUNT_ITEM(item), FALSE);

  tp_cli_account_call_remove(item->account, -1, 0, 0, 0, 0);
  tp_account_unref(item);

  return TRUE;
}

gboolean
rtcom_account_item_is_online(RtcomAccountItem *item)
{
  g_return_val_if_fail(RTCOM_IS_ACCOUNT_ITEM(item), FALSE);

  return tp_account_get_connection_status(item->account, NULL) ==
         TP_CONNECTION_STATUS_CONNECTED;
}

static void
set_uri_schemes(RtcomAccountItem *item)
{
  gchar **scheme = item->secondary_vcard_fields;

  while (*scheme)
  {
    tp_account_set_uri_scheme_association_async(
      item->account, *scheme, TRUE, NULL, NULL);
    scheme++;
  }
}

static void
account_prepared_cb(GObject *object, GAsyncResult *res, gpointer user_data)
{
  RtcomAccountItem *item = user_data;
  GError *error = NULL;

  if (!tp_proxy_prepare_finish(object, res, &error))
  {
    g_warning("%s: Error preparing account: %s", __FUNCTION__, error->message);
    g_clear_error(&error);
  }

  g_signal_emit(item, signals[UPDATED], 0, TRUE);
  g_object_unref(item);
}

static void
create_account_cb(TpAccountManager *proxy, const gchar *out_Account,
                  const GError *error, gpointer user_data,
                  GObject *weak_object)
{
  RtcomAccountItem *item = RTCOM_ACCOUNT_ITEM(weak_object);

  if (error)
  {
    g_warning("%s: account could not be created: %s", __FUNCTION__,
              error->message);
  }
  else
  {
    GError *local_error = NULL;

    item->account = tp_simple_client_factory_ensure_account(
        tp_proxy_get_factory(proxy), out_Account, NULL, &local_error);

    if (item->account)
    {
      GArray *features;
      TpAccountManager *manager = RTCOM_ACCOUNT_PLUGIN(
          account_item_get_plugin(ACCOUNT_ITEM(item)))->manager;

      if (item->set_mask & SVCF_SET)
        set_uri_schemes(item);

      features = tp_simple_client_factory_dup_account_features(
          tp_proxy_get_factory(manager), item->account);

      tp_proxy_prepare_async(item->account, (GQuark *)features->data,
                             account_prepared_cb, g_object_ref(item));
      g_array_unref(features);

      g_signal_connect(item->account, "status-changed",
                       G_CALLBACK(on_status_changed), item);
    }
    else
    {
      g_warning("%s: unable to get account: %s",
                __FUNCTION__, local_error->message);
      g_error_free(local_error);
    }
  }
}

static void
create_account(RtcomAccountItem *item)
{
  TpProtocol *protocol = rtcom_account_item_get_tp_protocol(item);
  TpAccountManager *manager;
  GHashTable *properties;
  GHashTable *conditions;
  const gchar *service_name;
  const gchar *display_name;

  g_return_if_fail(protocol != NULL);

  service_name = ACCOUNT_ITEM(item)->service->service_name;
  manager = RTCOM_ACCOUNT_PLUGIN(
      account_item_get_plugin(ACCOUNT_ITEM(item)))->manager;

  properties = tp_asv_new(NULL, NULL);

  if (service_name)
  {
    gchar *icon_name = g_strdup_printf("im-%s", service_name);

    tp_asv_set_string(properties, TP_PROP_ACCOUNT_SERVICE, service_name);
    tp_asv_set_string(properties, TP_PROP_ACCOUNT_ICON, icon_name);
    g_free(icon_name);
  }

  tp_asv_set_boolean(properties, TP_PROP_ACCOUNT_ENABLED, TRUE);

#if 0
  tp_asv_set_string(properties,
                    "com.nokia.Account.Interface.Compat.Profile",
                    tp_protocol_get_name(protocol));
#endif

  conditions = g_hash_table_new((GHashFunc)&g_str_hash,
                                (GEqualFunc)&g_str_equal);
  g_hash_table_insert(conditions, "ip-route", "1");
  g_hash_table_insert(properties,
                      "com.nokia.Account.Interface.Conditions.Condition",
                      tp_g_value_slice_new_take_boxed(
                        TP_HASH_TYPE_STRING_STRING_MAP, conditions));

  if (item->set_mask & NICKNAME_SET)
  {
    tp_asv_set_static_string(properties, TP_PROP_ACCOUNT_NICKNAME,
                             item->nickname);
  }

  if (item->set_mask & AVATAR_SET)
  {
    GArray *data = g_array_new(FALSE, FALSE, sizeof(guchar));
    GValueArray *arr;

    g_array_append_vals(data, item->avatar_data, item->avatar_len);
    arr = tp_value_array_build(2,
                               TP_TYPE_UCHAR_ARRAY, data,
                               G_TYPE_STRING, item->avatar_mime,
                               G_TYPE_INVALID);
    g_array_unref(data);
    tp_asv_take_boxed(properties, TP_PROP_ACCOUNT_INTERFACE_AVATAR_AVATAR,
                      TP_STRUCT_TYPE_AVATAR, arr);
  }

  display_name = item->display_name;

  if (!display_name)
    display_name = ACCOUNT_ITEM(item)->name;

  if (!display_name)
  {
    display_name = g_value_get_string(g_hash_table_lookup(item->new_params,
                                                          "account"));
  }

  tp_cli_account_manager_call_create_account(
    manager,
    -1,
    tp_protocol_get_cm_name(protocol),
    tp_protocol_get_name(protocol),
    display_name,
    item->new_params,
    properties,
    create_account_cb,
    NULL,
    NULL,
    G_OBJECT(item));

  g_hash_table_destroy(properties);
  free_store_data(item);
  g_object_unref(protocol);
}

static void
check_param_removed(gpointer key, gpointer value, gpointer user_data)
{
  const GValue *val1 = value;
  gpointer *data = user_data;
  GHashTable *new_params = data[0];
  gchar **unset = data[1];
  const GValue *val2 = g_hash_table_lookup(new_params, key);
  gboolean remove;

  if (!val2)
  {
    gchar **p = unset;

    while (*p)
      p++;

    *p = g_strdup(key);
    return;
  }

  g_return_if_fail(val1 != NULL);

  if (G_VALUE_TYPE(val2) != G_VALUE_TYPE(val1))
    return;

  switch (G_VALUE_TYPE(val2))
  {
    case G_TYPE_CHAR:
    {
      remove = g_value_get_schar(val2) == g_value_get_schar(val1);
      break;
    }
    case G_TYPE_UCHAR:
    {
      remove = g_value_get_uchar(val2) == g_value_get_uchar(val1);
      break;
    }
    case G_TYPE_BOOLEAN:
    {
      remove = g_value_get_boolean(val2) == g_value_get_boolean(val1);
      break;
    }
    case G_TYPE_INT:
    {
      remove = g_value_get_int(val2) == g_value_get_int(val1);
      break;
    }
    case G_TYPE_UINT:
    {
      remove = g_value_get_uint(val2) == g_value_get_uint(val1);
      break;
    }
    case G_TYPE_LONG:
    {
      remove = g_value_get_long(val2) == g_value_get_long(val1);
      break;
    }
    case G_TYPE_ULONG:
    {
      remove = g_value_get_ulong(val2) == g_value_get_ulong(val1);
      break;
    }
    case G_TYPE_INT64:
    {
      remove = g_value_get_int64(val2) == g_value_get_int64(val1);
      break;
    }
    case G_TYPE_UINT64:
    {
      remove = g_value_get_uint64(val2) == g_value_get_uint64(val1);
      break;
    }
    case G_TYPE_STRING:
    {
      remove = !g_strcmp0(g_value_get_string(val2), g_value_get_string(val1));
      break;
    }
    default:
      return;
  }

  if (remove)
    g_hash_table_remove(new_params, key);
}

static void
update_parameters_cb(GObject *source_object, GAsyncResult *res,
                     gpointer user_data)
{
  RtcomAccountItem *item = user_data;
  TpAccount *account = TP_ACCOUNT(source_object);
  gchar **reconnect_required;
  GError *error = NULL;
  gboolean success = tp_account_update_parameters_finish(
      account, res, &reconnect_required, &error);

  if (!success || error || !reconnect_required)
    g_signal_emit(item, signals[UPDATED], 0, FALSE);
  else
    g_signal_emit(item, signals[UPDATED], 0, *reconnect_required);
}

void
rtcom_account_item_save_settings(RtcomAccountItem *item, GError **error)
{
  if (item->account)
  {
    GHashTable *params;
    gchar **unset = NULL;

    g_object_set_data_full(
      G_OBJECT(item), "user-id",
      g_value_dup_string(g_hash_table_lookup(item->new_params, "account")),
      (GDestroyNotify)&g_free);

    params = (GHashTable *)tp_account_get_parameters(item->account);

    if (params)
    {
      gpointer data[2];

      unset = g_new0(gchar *, g_hash_table_size(params) + 1);

      data[0] = item->new_params;
      data[1] = unset;
      g_hash_table_foreach(params, check_param_removed, data);
    }

    tp_account_update_parameters_async(
      item->account, item->new_params, (const gchar **)unset,
      update_parameters_cb, item);

    g_strfreev(unset);

    if (item->set_mask & DISPLAY_NAME_SET)
    {
      tp_account_set_display_name_async(
        item->account, item->display_name, NULL, NULL);
    }

    if (item->set_mask & NICKNAME_SET)
      tp_account_set_nickname_async(item->account, item->nickname, NULL, NULL);

    if (item->set_mask & AVATAR_SET)
    {
      tp_account_set_avatar_async(item->account, (guchar *)item->avatar_data,
                                  item->avatar_len, item->avatar_mime,
                                  NULL, NULL);
    }

    if (item->set_mask & SVCF_SET)
      set_uri_schemes(item);

    if (item->set_mask & ENABLED_SET)
    {
      tp_account_set_enabled_async(
        item->account, item->enabled_setting, NULL, NULL);
    }

    free_store_data(item);
  }
  else
    create_account(item);
}
