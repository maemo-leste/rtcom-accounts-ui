/*
 * rtcom-account-service.c
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

#include <conic/conic.h>
#include <hildon/hildon.h>
#include <telepathy-glib/simple-client-factory.h>

#include "rtcom-account-service.h"

G_DEFINE_TYPE(
  RtcomAccountService,
  rtcom_account_service,
  ACCOUNT_TYPE_SERVICE
);

static GQuark connection_data_quark = 0;

static void
rtcom_account_service_dispose(GObject *object)
{
  RtcomAccountService *service = RTCOM_ACCOUNT_SERVICE(object);

  if (service->protocol)
  {
    g_object_unref(service->protocol);
    service->protocol = NULL;
  }

  G_OBJECT_CLASS(rtcom_account_service_parent_class)->dispose(object);
}

static void
rtcom_account_service_finalize(GObject *object)
{
  RtcomAccountService *service = RTCOM_ACCOUNT_SERVICE(object);

  g_free(service->successful_msg);
  g_free(service->account_domains);

  G_OBJECT_CLASS(rtcom_account_service_parent_class)->finalize(object);
}

static void
cm_prepared_cb(GObject *object, GAsyncResult *res, gpointer user_data)
{
  gpointer *data = user_data;
  TpConnectionManager *cm = (TpConnectionManager *)object;
  GError *error = NULL;

  if (!tp_proxy_prepare_finish(object, res, &error))
  {
    g_warning("Error preparing connection manager: %s\n", error->message);
    g_error_free(error);
  }
  else
    data[2] = tp_connection_manager_get_protocol_object(cm, data[0]);

  g_main_loop_quit(data[1]);
}

static TpProtocol *
get_protocol(AccountService *service)
{
  GStrv arr = g_strsplit(service->name, "/", 2);
  gchar *cm_name = NULL;
  gchar *protocol_name = NULL;
  TpDBusDaemon *dbus;
  TpConnectionManager *cm;
  GError *error = NULL;
  TpProtocol *protocol = NULL;
  gpointer data[3];
  GMainLoop *loop;

  if (arr && arr[0] && arr[1])
  {
    cm_name = arr[0];
    protocol_name = arr[1];
  }

  g_return_val_if_fail(cm_name != NULL, NULL);
  g_return_val_if_fail(protocol_name != NULL, NULL);

  dbus = tp_dbus_daemon_dup(NULL);
  cm = tp_connection_manager_new(dbus, cm_name, NULL, &error);
  g_object_unref(dbus);

  if (error)
  {
    g_warning("%s: Failed to create connection manager for %s: [%s]",
              __FUNCTION__, cm_name, error->message);
    g_strfreev(arr);
    g_error_free(error);
    return NULL;
  }

  loop = g_main_loop_new(NULL, FALSE);
  data[0] = protocol_name;
  data[1] = loop;
  data[2] = NULL; /* out */
  tp_proxy_prepare_async(cm, NULL, cm_prepared_cb, data);

  GDK_THREADS_LEAVE();
  g_main_loop_run(loop);
  GDK_THREADS_ENTER();

  g_object_unref(cm);
  g_main_loop_unref(loop);
  g_strfreev(arr);

  if (data[2])
    protocol = g_object_ref(data[2]);

  return protocol;
}

static GObject *
rtcom_account_service_constructor(GType type, guint n_construct_properties,
                                  GObjectConstructParam *construct_properties)
{
  GObject *object;
  AccountService *service;
  TpProtocol *protocol;
  const gchar *icon_name;

  object = G_OBJECT_CLASS(rtcom_account_service_parent_class)->
    constructor(type, n_construct_properties, construct_properties);

  service = ACCOUNT_SERVICE(object);

  protocol = get_protocol(service);
  RTCOM_ACCOUNT_SERVICE(service)->protocol = protocol;

  g_return_val_if_fail(protocol != NULL, object);

  service->display_name = g_strdup(tp_protocol_get_english_name(protocol));
  icon_name = tp_protocol_get_icon_name(protocol);

  if (icon_name)
  {
    service->icon = gtk_icon_theme_load_icon(
        gtk_icon_theme_get_default(), icon_name, 48, 0, NULL);
  }

  RTCOM_ACCOUNT_SERVICE(service)->successful_msg = NULL;
  RTCOM_ACCOUNT_SERVICE(service)->account_domains = NULL;

  /* we don't have those in telepathy */
  service->supports_avatar = TRUE;
  service->priority = 0;

  return object;
}

static void
rtcom_account_service_class_init(RtcomAccountServiceClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS(klass);

  object_class->dispose = rtcom_account_service_dispose;
  object_class->finalize = rtcom_account_service_finalize;
  object_class->constructor = rtcom_account_service_constructor;
}

static void
rtcom_account_service_init(RtcomAccountService *service)
{
  connection_data_quark = g_quark_from_static_string("connection-data");
}

RtcomAccountService *
rtcom_account_service_new(const gchar *name, RtcomAccountPlugin *plugin)
{
  g_return_val_if_fail(name != NULL, NULL);

  return g_object_new(RTCOM_TYPE_ACCOUNT_SERVICE,
                      "plugin", plugin,
                      "name", name,
                      NULL);
}

TpProtocol *
rtcom_account_service_get_protocol (RtcomAccountService *service)
{
  g_return_val_if_fail(RTCOM_IS_ACCOUNT_SERVICE(service), NULL);

  return service->protocol;
}

typedef struct
{
  RtcomAccountService *service;
  TpConnection *tp_conn;
  TpConnectionManager *cm;
  TpProxyPendingCall *request_connection;
  TpProxySignalConnection *status_changed;
  TpProxySignalConnection *connection_error;
  GHashTable *params;
  gboolean disconnect;
  RtcomAccountServiceConnectionCb cb;
  gpointer user_data;
  guint connection_timeout_id;
  GError *error;
  ConIcConnection *conn_ic;
  gulong connection_event_id;
  GSList *iaps;
}
connection_data;

static void
_connection_data_free(connection_data *cd)
{
  g_return_if_fail(cd != NULL);

  if (cd->tp_conn)
    g_object_unref(cd->tp_conn);

  if (cd->cm)
  {
    if (cd->request_connection)
    {
      tp_proxy_pending_call_cancel(cd->request_connection);
      cd->cm = cd->cm;
    }

    g_object_unref(cd->cm);
  }

  if (cd->connection_error)
    tp_proxy_signal_connection_disconnect(cd->connection_error);

  if (cd->status_changed)
    tp_proxy_signal_connection_disconnect(cd->status_changed);

  if (cd->connection_timeout_id)
    g_source_remove(cd->connection_timeout_id);

  if (cd->error)
    g_error_free(cd->error);

  if (cd->conn_ic)
  {
    if (cd->connection_event_id)
    {
      g_signal_handler_disconnect(cd->conn_ic, cd->connection_event_id);
      cd->connection_event_id = 0;
    }

    g_object_unref(cd->conn_ic);
    cd->conn_ic = NULL;
  }

  g_slist_free_full(cd->iaps, g_free);

  if (cd->params)
    g_hash_table_unref(cd->params);

  g_free(cd);
}

static void
_tp_disconnect_cb(TpConnection *proxy, const GError *error, gpointer user_data,
                  GObject *requester)
{
  connection_data *cd = g_object_get_qdata(requester, connection_data_quark);

  g_return_if_fail(cd != NULL);

  cd->cb(requester, proxy, cd->error, cd->user_data);
  g_object_set_qdata(requester, connection_data_quark, NULL);
}

static void
_tp_disconnect(GObject *requester)
{
  connection_data *cd = g_object_get_qdata(requester, connection_data_quark);

  if (!cd->disconnect || !cd->tp_conn ||
      (tp_connection_get_status(cd->tp_conn, 0) ==
       TP_CONNECTION_STATUS_DISCONNECTED))
  {
    cd->cb(requester, cd->tp_conn, cd->error, cd->user_data);
    g_object_set_qdata(requester, connection_data_quark, NULL);
    return;
  }

  if (cd->connection_error)
  {
    tp_proxy_signal_connection_disconnect(cd->connection_error);
    cd->connection_error = NULL;
  }

  if (cd->status_changed)
  {
    tp_proxy_signal_connection_disconnect(cd->status_changed);
    cd->status_changed = NULL;
  }

  tp_cli_connection_call_disconnect(
    cd->tp_conn, -1, _tp_disconnect_cb, NULL, NULL, requester);
}

static void
_connection_finished(GObject *requester, AccountErrorCode error_code)
{
  connection_data *cd = g_object_get_qdata(requester, connection_data_quark);

  if (error_code == ACCOUNT_ERROR_AUTHENTICATION_FAILED)
  {
    if (!cd->error)
    {
      const gchar *msgid = _("accounts_ti_auth_failed");

      cd->error = g_error_new(
          ACCOUNT_ERROR, ACCOUNT_ERROR_AUTHENTICATION_FAILED, msgid,
          g_value_get_string(g_hash_table_lookup(cd->params, "account")));
    }
  }
  else if (error_code == ACCOUNT_ERROR_NAME_IN_USE)
  {
    const gchar *msgid = _("accounts_fi_username_exists");

    cd->error = g_error_new(
        ACCOUNT_ERROR, ACCOUNT_ERROR_NAME_IN_USE, msgid,
        g_value_get_string(g_hash_table_lookup(cd->params, "account")),
        ACCOUNT_SERVICE(cd->service)->display_name);
  }
  else if (error_code != -1)
  {
    cd->error = g_error_new(ACCOUNT_ERROR, error_code, "%s",
                            _("accounts_fi_problems_service_connection"));
  }

  _tp_disconnect(requester);
}

static void
_connection_connect_cb(TpConnection *proxy, const GError *error,
                       gpointer user_data, GObject *weak_object)
{
  if (g_object_get_qdata(weak_object, connection_data_quark))
  {
    if (error)
    {
      g_warning("%s carries error: %s", __FUNCTION__, error->message);
      _connection_finished(weak_object, ACCOUNT_ERROR_CONNECTION_FAILED);
    }
  }
}

static void
_connection_error_cb(TpConnection *proxy, const gchar *arg_Error,
                     GHashTable *arg_Details, gpointer user_data,
                     GObject *weak_object)
{
  connection_data *cd = g_object_get_qdata(weak_object, connection_data_quark);

  if (!cd)
    return;

  if (arg_Details)
  {
    /* threre was some skype-specific error processing here, but I kept the
     * callback in case we want some detailed error messages
     */
  }
}

static void
_status_changed_cb(TpConnection *proxy, guint arg_Status, guint arg_Reason,
                   gpointer user_data, GObject *weak_object)
{
  connection_data *cd = g_object_get_qdata(weak_object, connection_data_quark);

  if (cd && (arg_Status != TP_CONNECTION_STATUS_CONNECTING))
  {
    if (arg_Status)
    {
      AccountErrorCode error_code;

      if (arg_Reason == TP_CONNECTION_STATUS_REASON_AUTHENTICATION_FAILED)
        error_code = ACCOUNT_ERROR_AUTHENTICATION_FAILED;
      else if (arg_Reason == TP_CONNECTION_STATUS_REASON_NAME_IN_USE)
        error_code = ACCOUNT_ERROR_NAME_IN_USE;
      else
        error_code = ACCOUNT_ERROR_CONNECTION_FAILED;

      _connection_finished(weak_object, error_code);
    }
    else
    {
      tp_proxy_signal_connection_disconnect(cd->connection_error);
      cd->connection_error = NULL;
      tp_proxy_signal_connection_disconnect(cd->status_changed);
      cd->status_changed = NULL;
      _connection_finished(weak_object, -1);
    }
  }
}

static void
_request_connection_cb(TpConnectionManager *proxy, const gchar *out_Bus_Name,
                       const gchar *out_Object_Path, const GError *error,
                       gpointer user_data, GObject *requester)
{
  connection_data *cd = g_object_get_qdata(requester, connection_data_quark);

  g_return_if_fail(cd != NULL);

  cd->request_connection = NULL;

  if (error)
  {
    g_warning("%s carries error: %s", __FUNCTION__, error->message);
    _connection_finished(requester, ACCOUNT_ERROR_CONNECTION_FAILED);
  }
  else
  {
    GError *local_error = NULL;

    cd->tp_conn =
      tp_simple_client_factory_ensure_connection(
        tp_proxy_get_factory(proxy), out_Object_Path, NULL, &local_error);

    if (cd->tp_conn)
    {
      tp_cli_connection_call_connect(
        cd->tp_conn, -1, _connection_connect_cb, NULL, NULL, requester);

      g_assert(cd->connection_error == NULL);

      cd->connection_error =
        tp_cli_connection_connect_to_connection_error(
          cd->tp_conn, _connection_error_cb, NULL, NULL, requester, NULL);

      g_assert(cd->status_changed == NULL);

      cd->status_changed = tp_cli_connection_connect_to_status_changed(
          cd->tp_conn, _status_changed_cb, NULL, NULL, requester, NULL);
    }
    else
    {
      g_warning("%s, tp_conn_new returned NULL (%s)",
                __FUNCTION__, local_error->message);
      g_error_free(local_error);
      _connection_finished(requester, ACCOUNT_ERROR_CONNECTION_FAILED);
    }
  }
}

static void
_connectivity_ready(gpointer requester)
{
  connection_data *cd = g_object_get_qdata(requester, connection_data_quark);

  g_return_if_fail(cd != NULL);

  if (cd->iaps)
  {
    TpDBusDaemon *dbus;
    GError *error = NULL;

    if (cd->cm)
      return;

    dbus = rtcom_account_plugin_get_dbus_daemon(
        RTCOM_ACCOUNT_PLUGIN(ACCOUNT_SERVICE(cd->service)->plugin));
    cd->cm = tp_connection_manager_new(
        dbus, tp_protocol_get_cm_name(cd->service->protocol), 0, &error);

    if (error)
    {
      g_warning("%s: %s", __FUNCTION__, error->message);
      g_error_free(error);
    }
    else
    {
      const gchar *protocol_name = tp_protocol_get_name(cd->service->protocol);

      cd->request_connection =
        tp_cli_connection_manager_call_request_connection(
          cd->cm, -1, protocol_name, cd->params, _request_connection_cb, NULL,
          NULL, requester);
      return;
    }
  }

  _connection_finished(requester, ACCOUNT_ERROR_CONNECTION_FAILED);
}

static gboolean
_connection_timeout(gpointer requester)
{
  g_return_val_if_fail(G_IS_OBJECT(requester), G_SOURCE_REMOVE);

  _connection_finished(requester, ACCOUNT_ERROR_UNKNOWN);

  return G_SOURCE_REMOVE;
}

static void
_connection_event(ConIcConnection *connection, ConIcConnectionEvent *event,
                  gpointer requester)
{
  connection_data *cd = g_object_get_qdata(requester, connection_data_quark);
  ConIcConnectionStatus status = con_ic_connection_event_get_status(event);
  const gchar *iap_id = con_ic_event_get_iap_id(CON_IC_EVENT(event));

  if (cd->connection_timeout_id)
    g_source_remove(cd->connection_timeout_id);

  cd->connection_timeout_id =
    g_timeout_add(30000, _connection_timeout, requester);

  if (status == CON_IC_STATUS_CONNECTED)
  {
    if (!g_slist_find_custom(cd->iaps, iap_id, (GCompareFunc)&strcmp))
      cd->iaps = g_slist_prepend(cd->iaps, g_strdup(iap_id));

    _connectivity_ready(requester);
  }
  else if (status == CON_IC_STATUS_DISCONNECTED)
  {
    GSList *iap = g_slist_find_custom(cd->iaps, iap_id, (GCompareFunc)&strcmp);

    if (iap)
    {
      g_free(iap->data);
      cd->iaps = g_slist_delete_link(cd->iaps, iap);
    }
  }
}

void
rtcom_account_service_connect(RtcomAccountService *service, GHashTable *params,
                              GObject *requester, gboolean disconnect,
                              RtcomAccountServiceConnectionCb cb,
                              gpointer user_data)
{
  g_return_if_fail(G_IS_OBJECT(requester));
  g_return_if_fail(RTCOM_IS_ACCOUNT_SERVICE(service));

  if (!g_object_get_qdata(requester, connection_data_quark))
  {
    connection_data *cd = g_new0(connection_data, 1);

    cd->user_data = user_data;
    cd->cb = cb;
    cd->service = service;
    cd->disconnect = disconnect;
    cd->params = params;
    g_hash_table_ref(params);
    g_object_set_qdata_full(requester, connection_data_quark, cd,
                            (GDestroyNotify)_connection_data_free);
    cd->conn_ic = con_ic_connection_new();

    if (cd->conn_ic)
    {
      g_object_set(cd->conn_ic,
                   "automatic-connection-events", TRUE,
                   NULL);
      cd->connection_event_id =
        g_signal_connect(cd->conn_ic, "connection-event",
                         G_CALLBACK(_connection_event), requester);
    }

    if (!cd->conn_ic ||
        !con_ic_connection_connect(cd->conn_ic, CON_IC_CONNECT_FLAG_NONE))
    {
      GError error;

      error.code = ACCOUNT_ERROR_CONNECTION_FAILED;
      error.domain = ACCOUNT_ERROR;
      error.message = (gchar *)_("accounts_fi_problems_service_connection");
      g_object_set_qdata(requester, connection_data_quark, NULL);
      cb(requester, NULL, &error, user_data);
    }
  }
}

void
rtcom_account_service_set_successful_message(RtcomAccountService *service,
                                             const gchar *msg)
{
  g_return_if_fail(RTCOM_IS_ACCOUNT_SERVICE(service));

  g_free(service->successful_msg);

  service->successful_msg = g_strdup(msg);
}

void
rtcom_account_service_set_account_domains (RtcomAccountService *service,
                                           const gchar *domains)
{
  g_return_if_fail(RTCOM_IS_ACCOUNT_SERVICE(service));

  g_free(service->account_domains);

  service->account_domains = g_strdup(domains);
}
