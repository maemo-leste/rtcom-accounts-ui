/*
 * rtcom-dialog-context.c
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

#include "rtcom-dialog-context.h"

typedef struct
{
  RtcomDialogContext *context;
  RtcomAccountItem *item;
}
response_data;

struct _RtcomDialogContextPrivate
{
  GtkWidget *start_page;
  GtkWidget *dialog;
  guint connect_timeout;
  gulong updated_id;
};

typedef struct _RtcomDialogContextPrivate RtcomDialogContextPrivate;

#define PRIVATE(context) \
  ((RtcomDialogContextPrivate *) \
   rtcom_dialog_context_get_instance_private((RtcomDialogContext *)(context)))

enum
{
  PROP_START_PAGE = 1
};

static void
rtcom_dialog_context_dialog_iface_init(AccountDialogContextIface *result);

G_DEFINE_TYPE_WITH_CODE(
  RtcomDialogContext,
  rtcom_dialog_context,
  ACCOUNT_TYPE_EDIT_CONTEXT,
  G_IMPLEMENT_INTERFACE(
    ACCOUNT_TYPE_DIALOG_CONTEXT,
    rtcom_dialog_context_dialog_iface_init);
  G_ADD_PRIVATE(RtcomDialogContext);
)

static void
connection_status_changed_cb(RtcomAccountItem *item, TpConnectionStatus status,
                             TpConnectionStatusReason reason,
                             RtcomDialogContext *context);
static void
_account_failed(RtcomDialogContext *context, AccountErrorCode error_code);

static RtcomAccountItem *
_get_item_from_ctx(RtcomDialogContext *context)
{
  return RTCOM_ACCOUNT_ITEM(account_edit_context_get_account(
                              ACCOUNT_EDIT_CONTEXT(context)));
}

static void
_destroy_object(gpointer object)
{
  if (GTK_IS_DIALOG(object))
    gtk_widget_destroy(object);
  else
    g_object_unref(object);
}

static void
rtcom_dialog_context_dispose(GObject *object)
{
  RtcomDialogContext *context = RTCOM_DIALOG_CONTEXT(object);
  RtcomDialogContextPrivate *priv = PRIVATE(context);
  RtcomAccountItem *item = _get_item_from_ctx(context);

  if (priv->updated_id)
  {
    g_signal_handler_disconnect(item, priv->updated_id);
    priv->updated_id = 0;
  }

  g_list_free_full(context->objects, _destroy_object);
  context->objects = NULL;

  if (priv->connect_timeout)
  {
    g_source_remove(priv->connect_timeout);
    priv->connect_timeout = 0;
  }

  G_OBJECT_CLASS(rtcom_dialog_context_parent_class)->dispose(object);
}

static void
rtcom_dialog_context_finalize(GObject *object)
{
  G_OBJECT_CLASS(rtcom_dialog_context_parent_class)->finalize(object);
}

static void
rtcom_dialog_context_set_property(GObject *object, guint property_id,
                                  const GValue *value, GParamSpec *pspec)
{
  RtcomDialogContextPrivate *priv = PRIVATE(object);

  switch (property_id)
  {
    case PROP_START_PAGE:
    {
      priv->start_page = g_value_get_pointer(value);
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
rtcom_dialog_context_get_property(GObject *object,
                                  guint property_id,
                                  GValue *value,
                                  GParamSpec *pspec)
{
  RtcomDialogContextPrivate *priv = PRIVATE(object);

  switch (property_id)
  {
    case PROP_START_PAGE:
    {
      g_value_set_pointer(value, priv->start_page);
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
rtcom_dialog_context_class_init(RtcomDialogContextClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS(klass);

  object_class->dispose = rtcom_dialog_context_dispose;
  object_class->finalize = rtcom_dialog_context_finalize;
  object_class->set_property = rtcom_dialog_context_set_property;
  object_class->get_property = rtcom_dialog_context_get_property;

  g_object_class_install_property(
    object_class, PROP_START_PAGE,
    g_param_spec_pointer(
      "start-page",
      "Start page",
      "Start page",
      G_PARAM_WRITABLE | G_PARAM_READABLE));
}

static void
rtcom_dialog_context_init(RtcomDialogContext *context)
{}

static GtkWidget *
rtcom_dialog_context_start(AccountDialogContext *context, GError **error)
{
  RtcomAccountItem *item = _get_item_from_ctx(RTCOM_DIALOG_CONTEXT(context));
  GtkWidget *page;

  page = rtcom_dialog_context_get_start_page(RTCOM_DIALOG_CONTEXT(context));

  if (item)
    rtcom_page_set_account(RTCOM_PAGE(page), item);

  return page;
}

static void
rtcom_dialog_context_cancel(AccountDialogContext *context)
{}

static const gchar *
rtcom_dialog_context_get_page_title(AccountDialogContext *context)
{
  GtkWidget *start_page = PRIVATE(context)->start_page;

  if (start_page)
    return rtcom_page_get_title(RTCOM_PAGE(start_page));

  return NULL;
}

static void
rtcom_dialog_context_dialog_iface_init(AccountDialogContextIface *result)
{
  result->start = rtcom_dialog_context_start;
  result->cancel = rtcom_dialog_context_cancel;
  result->finish = rtcom_dialog_context_finish;
  result->get_page_title = rtcom_dialog_context_get_page_title;
}

RtcomDialogContext *
rtcom_dialog_context_new(RtcomAccountPlugin *plugin, RtcomAccountItem *item,
                         gboolean editing_existing)
{
  return g_object_new(RTCOM_TYPE_DIALOG_CONTEXT,
                      "account", item,
                      "plugin", plugin,
                      "editing", editing_existing,
                      NULL);
}

void
rtcom_dialog_context_set_start_page(RtcomDialogContext *dialog_context,
                                    GtkWidget *page)
{
  PRIVATE(dialog_context)->start_page = page;
}

void
rtcom_dialog_context_take_obj(RtcomDialogContext *dialog_context,
                              GObject *object)
{
  dialog_context->objects = g_list_prepend(dialog_context->objects, object);
}

void
rtcom_dialog_context_remove_obj(RtcomDialogContext *dialog_context,
                                GObject *object)
{
  dialog_context->objects = g_list_remove(dialog_context->objects, object);
}

static gboolean
_account_created_dialog_closed(GtkWidget *self, GdkEvent *event,
                               RtcomDialogContext *context)
{
  g_signal_emit_by_name(context, "operation-async", NULL);

  return FALSE;
}

static gboolean
_verifying_dialog_cb(GtkWidget *self, GdkEvent *event,
                     RtcomDialogContext *context)
{
  _account_failed(context, ACCOUNT_ERROR_USER_CANCELLED);

  return FALSE;
}

static void
_account_created(RtcomDialogContext *context)
{
  RtcomDialogContextPrivate *priv = PRIVATE(context);
  RtcomAccountItem *item = _get_item_from_ctx(context);
  const GHashTable *parameters;
  const gchar *user_id;
  GtkWidget *toplevel;
  RtcomAccountService *service;

  g_return_if_fail(ACCOUNT_IS_ITEM(item));

  parameters = tp_account_get_parameters(item->account);
  user_id = tp_asv_get_string(parameters, "account");
  toplevel = gtk_widget_get_toplevel(priv->start_page);
  g_object_get(item, "service", &service, NULL);

  g_return_if_fail(service != NULL);

  g_signal_handlers_disconnect_matched(
    item, G_SIGNAL_MATCH_DATA | G_SIGNAL_MATCH_FUNC, 0, 0, NULL,
    connection_status_changed_cb, context);

  if (priv->dialog)
  {
    gtk_widget_destroy(priv->dialog);
    priv->dialog = NULL;
  }

  if (priv->connect_timeout)
  {
    g_source_remove(priv->connect_timeout);
    priv->connect_timeout = 0;
  }

  if (service->successful_msg)
  {
    GdkPixbuf *service_icon;
    const gchar *msgid = _("accounts_ti_completed_with_conn");
    gchar *title;
    GtkDialog *dialog;
    GtkWidget *hbox;
    GtkWidget *align;

    g_object_get(item, "service-icon", &service_icon, NULL);
    title = g_strdup_printf(msgid, user_id);
    dialog = g_object_new(GTK_TYPE_DIALOG,
                          "title", title,
                          "transient-for", toplevel,
                          "has-separator", FALSE,
                          "destroy-with-parent", TRUE,
                          "modal", FALSE,
                          NULL);
    hbox = gtk_hbox_new(FALSE, 0);
    align = gtk_alignment_new(0.5, 0.5, 1.0, 1.0);
    gtk_alignment_set_padding(GTK_ALIGNMENT(align), 0, 16, 0, 0);
    gtk_container_add(GTK_CONTAINER(align), hbox);
    gtk_box_pack_start(
      GTK_BOX(GTK_DIALOG(dialog)->vbox), align, FALSE, FALSE, 0);

    if (service_icon)
    {
      gtk_box_pack_start(GTK_BOX(hbox), gtk_image_new_from_pixbuf(service_icon),
                         FALSE, FALSE, 16);
    }

    gtk_box_pack_start(GTK_BOX(hbox), gtk_label_new(service->successful_msg),
                       FALSE, FALSE, 0);
    g_signal_connect(dialog, "delete-event",
                     G_CALLBACK(_account_created_dialog_closed), context);
    gtk_widget_show_all(GTK_WIDGET(dialog));
    g_free(title);
  }
  else
  {
    const gchar *msgid = _("accounts_ia_account_setup_successful_with_conn");
    gchar *description = g_strdup_printf(msgid, user_id);

    g_signal_emit_by_name(context, "operation-async", NULL);
    hildon_banner_show_information(toplevel, NULL, description);
    g_free(description);
  }

  g_object_unref(service);
}

static void
_account_failed(RtcomDialogContext *context, AccountErrorCode error_code)
{
  RtcomDialogContextPrivate *priv = PRIVATE(context);
  RtcomAccountItem *item = _get_item_from_ctx(context);
  GError error;

  error.code = error_code;
  error.message = "";
  error.domain = ACCOUNT_ERROR;

  if (priv->dialog)
  {
    gtk_widget_destroy(priv->dialog);
    priv->dialog = NULL;
  }

  if (priv->connect_timeout)
  {
    g_source_remove(priv->connect_timeout);
    priv->connect_timeout = 0;
  }

  g_signal_handlers_disconnect_matched(
    item, G_SIGNAL_MATCH_DATA | G_SIGNAL_MATCH_FUNC, 0, 0, NULL,
    connection_status_changed_cb, context);

  if (((error_code == ACCOUNT_ERROR_AUTHENTICATION_FAILED) ||
       (error_code == ACCOUNT_ERROR_USER_CANCELLED)) &&
      !account_edit_context_get_editing(&context->parent_instance))
  {
    rtcom_account_item_delete(item);
  }

  if (error_code == ACCOUNT_ERROR_AUTHENTICATION_FAILED)
  {
    hildon_banner_show_information(
      gtk_widget_get_toplevel(priv->start_page), NULL,
      _("accounts_ti_auth_failed"));
  }
  else if (error_code == ACCOUNT_ERROR_CONNECTION_FAILED)
  {
    GtkWidget *toplevel = gtk_widget_get_toplevel(priv->start_page);
    GtkWindow* window = gtk_window_get_transient_for(GTK_WINDOW(toplevel));

    hildon_banner_show_information(
          GTK_WIDGET(window), NULL,
          _("accounts_fi_problems_service_connection"));
  }

  g_signal_emit_by_name(context, "operation-async", &error);
}

static gboolean
connection_timeout(gpointer user_data)
{
  _account_failed(user_data, ACCOUNT_ERROR_CONNECTION_FAILED);

  return FALSE;
}

static void
connection_status_changed_cb(RtcomAccountItem *item, TpConnectionStatus status,
                             TpConnectionStatusReason reason,
                             RtcomDialogContext *context)
{
  RtcomDialogContextPrivate *priv = PRIVATE(context);

  switch (status)
  {
    case TP_CONNECTION_STATUS_CONNECTING:
    {
      if (priv->connect_timeout)
        g_source_remove(priv->connect_timeout);

      priv->connect_timeout = g_timeout_add(30000, connection_timeout, context);
      break;
    }
    case TP_CONNECTION_STATUS_CONNECTED:
    {
      _account_created(context);
      break;
    }
    case TP_CONNECTION_STATUS_DISCONNECTED:
    {
      AccountErrorCode error_code;

      switch (reason)
      {
        case TP_CONNECTION_STATUS_REASON_AUTHENTICATION_FAILED:
        {
          error_code = ACCOUNT_ERROR_AUTHENTICATION_FAILED;
          break;
        }
        case TP_CONNECTION_STATUS_REASON_NETWORK_ERROR:
        /* fall-through */
        case TP_CONNECTION_STATUS_REASON_REQUESTED:
        {
          error_code = ACCOUNT_ERROR_CONNECTION_FAILED;
          break;
        }
        default:
        {
          error_code = ACCOUNT_ERROR_UNKNOWN;
          break;
        }
      }

      _account_failed(context, error_code);
      break;
    }
  }
}

static void
sign_in_dialog(RtcomDialogContext *context, RtcomAccountItem *item)
{
  RtcomDialogContextPrivate *priv = PRIVATE(context);
  const gchar *user_id = g_object_get_data(G_OBJECT(item), "user-id");
  GtkWidget *toplevel;
  gchar *title;
  GdkPixbuf *service_icon;
  GtkWidget *dialog;
  const char *msgid;

  if (!user_id)
  {
    const GHashTable *parameters = tp_account_get_parameters(item->account);

    user_id = tp_asv_get_string(parameters, "account");
  }

  if (priv->connect_timeout)
  {
    g_source_remove(priv->connect_timeout);
    priv->connect_timeout = 0;
  }

  priv->connect_timeout = g_timeout_add(30000, connection_timeout, context);

  if (priv->dialog)
  {
    gtk_widget_destroy(priv->dialog);
    priv->dialog = NULL;
  }

  toplevel = gtk_widget_get_toplevel(priv->start_page);
  msgid = _("accounts_ti_signing_in");
  title = g_strdup_printf(msgid, user_id ? user_id : "");
  service_icon = ACCOUNT_ITEM(item)->service_icon;
  dialog = gtk_dialog_new();
  gtk_window_set_title(GTK_WINDOW(dialog), title);
  gtk_dialog_set_has_separator(GTK_DIALOG(dialog), FALSE);
  g_free(title);
  hildon_gtk_window_set_progress_indicator(GTK_WINDOW(dialog), 1);

  if (service_icon)
  {
    gtk_box_pack_start(
      GTK_BOX(GTK_DIALOG(dialog)->vbox),
      gtk_image_new_from_pixbuf(service_icon), FALSE, FALSE, 16);
  }

  if (toplevel)
  {
    gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(toplevel));
    gtk_window_set_destroy_with_parent(GTK_WINDOW(dialog), FALSE);
  }

  gtk_window_set_modal(GTK_WINDOW(dialog), FALSE);
  g_signal_connect(dialog, "delete-event",
                   G_CALLBACK(_verifying_dialog_cb), context);
  priv->dialog = dialog;
  gtk_widget_show_all(dialog);
  g_signal_connect(item, "connection-status-changed",
                   G_CALLBACK(connection_status_changed_cb), context);
  rtcom_account_item_reconnect(item);
}

static void
reconnect_confirmation_response(GtkWidget *dialog, gint response_id,
                                response_data *data)
{
  gtk_widget_destroy(dialog);

  if (response_id == GTK_RESPONSE_OK)
    rtcom_account_item_call_reconnect(data->item);

  g_signal_emit_by_name(data->context, "operation-async", NULL);
  g_slice_free(response_data, data);
}

static void
reconnect_dialog(RtcomDialogContext *context, RtcomAccountItem *item)
{
  RtcomDialogContextPrivate *priv = PRIVATE(context);
  GtkWidget *toplevel = gtk_widget_get_toplevel(priv->start_page);

  if (toplevel)
  {
    const gchar *msgid = _("accounts_fi_reconnect_account");
    gchar *description = g_strdup_printf(msgid, ACCOUNT_ITEM(item)->name);
    GtkWidget *note = hildon_note_new_confirmation(GTK_WINDOW(toplevel),
                                                   description);
    response_data *data;

    gtk_window_set_destroy_with_parent(GTK_WINDOW(note), TRUE);
    data = g_slice_new0(response_data);
    data->context = context;
    data->item = item;
    g_signal_connect(note, "response",
                     G_CALLBACK(reconnect_confirmation_response), data);
    gtk_widget_show(note);
    g_free(description);
  }
}

static void
account_item_updated_cb(RtcomAccountItem *item, gboolean reconnect,
                        RtcomDialogContext *context)
{
  gboolean sign_in;

  sign_in = ACCOUNT_ITEM(item)->draft;

  if (reconnect || sign_in)
  {
    if (!account_edit_context_get_editing(ACCOUNT_EDIT_CONTEXT(context)))
      sign_in = TRUE;

    if (sign_in)
      sign_in_dialog(context, item);
    else
      reconnect_dialog(context, item);
  }
  else
    g_signal_emit_by_name(context, "operation-async", NULL);
}

static void
save_account_item(RtcomDialogContext *context, GError **error)
{
  RtcomDialogContextPrivate *priv = PRIVATE(context);
  RtcomAccountItem *item = _get_item_from_ctx(context);

  if (!priv->updated_id)
  {
    priv->updated_id =
      g_signal_connect(item, "updated",
                       G_CALLBACK(account_item_updated_cb), context);
  }

  rtcom_account_item_save_settings(item, error);
  g_set_error(error, ACCOUNT_ERROR, ACCOUNT_ERROR_OPERATION_ASYNC, "%s", "");
}

static void
account_disable_cb(GObject *source_object, GAsyncResult *res,
                   gpointer user_data)
{
  GError *error = NULL;

  tp_account_set_enabled_finish(TP_ACCOUNT(source_object), res, &error);

  if (error)
  {
    g_signal_emit_by_name(user_data, "operation-async", error);
    g_clear_error(&error);
  }

  save_account_item(user_data, &error);

  if (!error || (error->domain != ACCOUNT_ERROR) ||
      (error->code != ACCOUNT_ERROR_OPERATION_ASYNC))
  {
    g_signal_emit_by_name(user_data, "operation-async", error);
  }

  if (error)
    g_error_free(error);
}

static void
disabling_confirmation_response(GtkWidget *dialog, gint response_id,
                                response_data *data)
{
  gtk_widget_destroy(dialog);

  if (response_id == GTK_RESPONSE_OK)
  {
    tp_account_set_enabled_async(data->item->account, FALSE,
                                 account_disable_cb,
                                 g_object_ref(data->context));
  }
  else
  {
    GError *error = NULL;
    RtcomAccountItem *item = _get_item_from_ctx(data->context);

    if (item->account && (!ACCOUNT_ITEM(item)->draft))
      rtcom_account_store_enabled_setting(item, FALSE);

    rtcom_account_item_save_settings(item, &error);
    g_signal_emit_by_name(data->context, "operation-async", error);

    if (error)
      g_error_free(error);
  }

  g_slice_free(response_data, data);
}

static void
confirm_activate(RtcomDialogContext *context, RtcomAccountItem *old_item,
                 GError **error)
{
  RtcomDialogContextPrivate *priv = PRIVATE(context);
  AccountService *service = account_item_get_service(ACCOUNT_ITEM(old_item));
  GtkWidget *toplevel = gtk_widget_get_toplevel(priv->start_page);
  const GHashTable *parameters = tp_account_get_parameters(old_item->account);
  const gchar *account = tp_asv_get_string(parameters, "account");
  const gchar *display_name = account_service_get_display_name(service);
  const gchar *msg_id = _("accounts_nc_activate_account");
  gchar *confirmation = g_strdup_printf(msg_id, display_name, account);
  GtkWidget *note = hildon_note_new_confirmation(GTK_WINDOW(toplevel),
                                                 confirmation);
  response_data *data;

  gtk_window_set_destroy_with_parent(GTK_WINDOW(note), TRUE);

  data = g_slice_new0(response_data);
  data->context = context;
  data->item = old_item;

  g_signal_connect(note, "response",
                   G_CALLBACK(disabling_confirmation_response), data);
  gtk_widget_show(note);
  g_free(confirmation);
  g_set_error(error, ACCOUNT_ERROR, ACCOUNT_ERROR_OPERATION_ASYNC, "%s", "");
}

static gboolean
enable_single_account_only(RtcomDialogContext *context, GError **error)
{
  RtcomAccountItem *item = _get_item_from_ctx(context);
  TpProtocol *protocol = rtcom_account_item_get_tp_protocol(item);
  gboolean rv = TRUE;

  if (!protocol)
    return TRUE;

  /* FIXME: I was not able to find anything similar in telepathy :( */
  if (FALSE /*mc_profile_get_single_enable(protocol)*/)
  {
    AccountService *service = account_item_get_service(ACCOUNT_ITEM(item));
    AccountPlugin *plugin = account_item_get_plugin(ACCOUNT_ITEM(item));
    AccountsList *accounts_list = NULL;

    g_object_get(plugin, "accounts-list", &accounts_list, NULL);

    if (accounts_list)
    {
      GList *accounts = accounts_list_get_all(accounts_list);
      GList *l;
      RtcomAccountItem *old_item = NULL;

      for (l = accounts; l; l = l->next)
      {
        RtcomAccountItem *it = l->data;

        if ((item != it) && it->account &&
            (service == account_item_get_service(ACCOUNT_ITEM(it))) &&
            tp_account_get_has_been_online(it->account) &&
            tp_account_is_enabled(it->account))
        {
          old_item = it;
          break;
        }
      }

      g_list_free(accounts);

      if (old_item)
      {
        confirm_activate(context, old_item, error);
        rv = FALSE;
      }
    }
  }

  g_object_unref(protocol);

  return rv;
}

gboolean
rtcom_dialog_context_finish(AccountDialogContext *context, GError **error)
{
  RtcomDialogContext *ctx = RTCOM_DIALOG_CONTEXT(context);
  GtkWidget *start_page = rtcom_dialog_context_get_start_page(ctx);
  RtcomAccountItem *item = _get_item_from_ctx(ctx);
  gboolean enabled;

  g_return_val_if_fail(item != NULL, FALSE);

  if (start_page && !rtcom_page_validate(RTCOM_PAGE(start_page), error))
  {
    if (error && *error)
    {
      g_warning("%s: error \"%s\"", __FUNCTION__, (*error)->message);
      hildon_banner_show_information(gtk_widget_get_toplevel(start_page), NULL,
                                     (*error)->message);
    }

    return FALSE;
  }

  if (!rtcom_account_item_store_settings(item, error))
    return FALSE;

  enabled = rtcom_account_get_enabled_setting(item);

  if (!item->account || enabled || ACCOUNT_ITEM(item)->draft)
  {
    if (!enable_single_account_only(ctx, error))
      return FALSE;
  }

  save_account_item(ctx, error);

  if (error && *error)
    return FALSE;

  return TRUE;
}

GtkWidget *
rtcom_dialog_context_get_start_page(RtcomDialogContext *dialog_context)
{
  return PRIVATE(dialog_context)->start_page;
}
