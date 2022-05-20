/*
 * rtcom-account-plugin.c
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

#include <gtk/gtk.h>
#include <libaccounts/account-plugin.h>

#include "rtcom-account-plugin.h"

struct _RtcomAccountPluginPrivate
{
  gboolean initialized;
};

typedef struct _RtcomAccountPluginPrivate RtcomAccountPluginPrivate;

G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE(
  RtcomAccountPlugin,
  rtcom_account_plugin,
  ACCOUNT_TYPE_PLUGIN
);

#define PRIVATE(plugin) \
  ((RtcomAccountPluginPrivate *) \
   rtcom_account_plugin_get_instance_private((RtcomAccountPlugin *)(plugin)))

enum
{
  PROP_INITIALIZED = 1
};

static RtcomAccountItem *
rtcom_account_plugin_get_account_by_name(RtcomAccountPlugin *plugin,
                                         const gchar *name)
{
  GList *accounts;
  GList *l;
  RtcomAccountItem *item = NULL;
  AccountsList *accounts_list = NULL;

  g_object_get(plugin, "accounts-list", &accounts_list, NULL);
  accounts = accounts_list_get_all(accounts_list);
  g_object_unref(accounts_list);

  for (l = accounts; l; l = l->next)
  {
    if (RTCOM_IS_ACCOUNT_ITEM(l->data) &&
        (account_item_get_plugin(ACCOUNT_ITEM(l->data)) ==
         ACCOUNT_PLUGIN(plugin)))
    {
      const gchar *item_name = rtcom_account_item_get_unique_name(l->data);

      if (item_name && !strcmp(item_name, name))
      {
        item = l->data;
        break;
      }
    }
  }

  g_list_free(accounts);

  return item;
}

static void
on_account_removed_cb(TpAccountManager *am, TpAccount *account,
                      RtcomAccountPlugin *plugin)
{
  RtcomAccountItem *item = rtcom_account_plugin_get_account_by_name(
      plugin, tp_account_get_path_suffix(account));

  if (item)
  {
    AccountsList *accounts_list = NULL;

    g_object_get(plugin, "accounts-list", &accounts_list, NULL);
    accounts_list_remove(accounts_list, ACCOUNT_ITEM(item));
    g_object_unref(accounts_list);
  }
}

static gchar *
get_service_id(TpAccount *account)
{
  const gchar *protocol_name = tp_account_get_protocol_name(account);
  const gchar *service = tp_account_get_service(account);

  if (!service || !*service || !strcmp(service, protocol_name))
  {
    return g_strdup_printf("%s/%s", tp_account_get_cm_name(account),
                           protocol_name);
  }
  else
  {
    return g_strdup_printf("%s/%s/%s", tp_account_get_cm_name(account),
                           protocol_name, service);
  }
}

static void
on_account_validity_changed_cb(TpAccountManager *am, TpAccount *account,
                               gboolean valid, gpointer user_data)
{
  if (valid)
  {
    RtcomAccountPlugin *plugin = user_data;

    if (!rtcom_account_plugin_get_account_by_name(
          plugin, tp_account_get_path_suffix(account)))
    {
      RtcomAccountService *service;
      AccountsList *accounts_list = NULL;
      gchar *service_id = get_service_id(account);

      service = g_hash_table_lookup(plugin->services, service_id);
      g_free(service_id);

      if (service)
      {
        RtcomAccountItem *item = rtcom_account_item_new(account, service);

        g_object_get(plugin, "accounts-list", &accounts_list, NULL);
        accounts_list_add(accounts_list, ACCOUNT_ITEM(item));
        g_object_unref(accounts_list);
        g_object_unref(item);
      }
    }
  }
  else
    on_account_removed_cb(am, account, user_data);
}

static void
rtcom_account_plugin_dispose(GObject *object)
{
  RtcomAccountPlugin *plugin = RTCOM_ACCOUNT_PLUGIN(object);

  if (plugin->services)
  {
    g_hash_table_destroy(plugin->services);
    plugin->services = NULL;
  }

  if (plugin->manager)
  {
    g_signal_handlers_disconnect_matched(
      plugin->manager, G_SIGNAL_MATCH_DATA | G_SIGNAL_MATCH_FUNC, 0, 0, NULL,
      on_account_validity_changed_cb, plugin);
    g_signal_handlers_disconnect_matched(
      plugin->manager, G_SIGNAL_MATCH_DATA | G_SIGNAL_MATCH_FUNC, 0, 0, NULL,
      on_account_removed_cb, plugin);
    plugin->manager = NULL;
  }

  G_OBJECT_CLASS(rtcom_account_plugin_parent_class)->dispose(object);
}

static void
rtcom_account_plugin_finalize(GObject *object)
{
  G_OBJECT_CLASS(rtcom_account_plugin_parent_class)->finalize(object);
}

static void
rtcom_account_plugin_get_property(GObject *object, guint property_id,
                                  GValue *value, GParamSpec *pspec)
{
  RtcomAccountPluginPrivate *priv = PRIVATE(object);

  switch (property_id)
  {
    case PROP_INITIALIZED:
    {
      g_value_set_boolean(value, priv->initialized);
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
_add_accounts(RtcomAccountPlugin *plugin)
{
  RtcomAccountPluginPrivate *priv = PRIVATE(plugin);
  AccountsList *accounts_list = NULL;
  GList *accounts;
  GList *l;

  priv->initialized = TRUE;

  g_object_get(plugin, "accounts-list", &accounts_list, NULL);

  accounts = tp_account_manager_dup_valid_accounts(plugin->manager);

  for (l = accounts; l; l = l->next)
  {
    gchar *service_id = get_service_id(l->data);
    GHashTableIter iter;
    const gchar *key;
    RtcomAccountService *svc;

    g_hash_table_iter_init(&iter, plugin->services);

    while (g_hash_table_iter_next(&iter, (gpointer *)&key, (gpointer *)&svc))
    {
      if (!strcmp(service_id, key))
      {
        accounts_list_add(accounts_list,
                          ACCOUNT_ITEM(rtcom_account_item_new(l->data, svc)));
      }
    }

    g_free(service_id);
  }

  g_list_free_full(accounts, g_object_unref);
  g_object_unref(accounts_list);
  g_object_notify(G_OBJECT(plugin), "initialized");
}

static gboolean
rtcom_account_plugin_setup(AccountPlugin *account_plugin,
                           AccountsList *accounts_list)
{
  RtcomAccountPlugin *plugin = RTCOM_ACCOUNT_PLUGIN(account_plugin);

  if (tp_proxy_is_prepared(plugin->manager, TP_ACCOUNT_MANAGER_FEATURE_CORE))
    _add_accounts(plugin);

  return TRUE;
}

static const gchar *
rtcom_account_plugin_get_name(AccountPlugin *account_plugin)
{
  return RTCOM_ACCOUNT_PLUGIN(account_plugin)->name;
}

static AccountEditContext *
rtcom_account_plugin_begin_new(AccountPlugin *account_plugin,
                               AccountService *service)
{
  RtcomAccountPlugin *plugin = RTCOM_ACCOUNT_PLUGIN(account_plugin);
  RtcomDialogContext *context = NULL;
  RtcomAccountItem *item;

  item = rtcom_account_item_new(NULL, RTCOM_ACCOUNT_SERVICE(service));

  if (item)
  {
    context = rtcom_dialog_context_new(plugin, item, FALSE);
    g_object_unref(item);
    RTCOM_ACCOUNT_PLUGIN_GET_CLASS(plugin)->context_init(plugin, context);
  }
  else
    g_warning("%s: RtcomAccountItem creation failed", __FUNCTION__);

  return ACCOUNT_EDIT_CONTEXT(context);
}

AccountEditContext *
rtcom_account_plugin_begin_edit(AccountPlugin *account_plugin,
                                AccountItem *account_item)
{
  RtcomAccountPlugin *plugin = RTCOM_ACCOUNT_PLUGIN(account_plugin);
  RtcomDialogContext *context;

  context = rtcom_dialog_context_new(plugin, RTCOM_ACCOUNT_ITEM(account_item),
                                     TRUE);
  RTCOM_ACCOUNT_PLUGIN_GET_CLASS(plugin)->context_init(plugin, context);

  return ACCOUNT_EDIT_CONTEXT(context);
}

static GList *
rtcom_account_plugin_list_services(AccountPlugin *account_plugin)
{
  RtcomAccountPlugin *plugin = RTCOM_ACCOUNT_PLUGIN(account_plugin);

  return g_hash_table_get_values(plugin->services);
}

static void
rtcom_account_plugin_deleted(AccountPlugin *plugin, AccountItem *account_item)
{
  rtcom_account_item_delete(RTCOM_ACCOUNT_ITEM(account_item));
}

static void
rtcom_account_plugin_class_init(RtcomAccountPluginClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS(klass);
  AccountPluginClass *plugin_class = ACCOUNT_PLUGIN_CLASS(klass);

  object_class->dispose = rtcom_account_plugin_dispose;
  object_class->finalize = rtcom_account_plugin_finalize;
  object_class->get_property = rtcom_account_plugin_get_property;

  plugin_class->setup = rtcom_account_plugin_setup;
  plugin_class->get_name = rtcom_account_plugin_get_name;
  plugin_class->begin_new = rtcom_account_plugin_begin_new;
  plugin_class->begin_edit = rtcom_account_plugin_begin_edit;
  plugin_class->list_services = rtcom_account_plugin_list_services;
  plugin_class->deleted = rtcom_account_plugin_deleted;

  g_object_class_override_property(
    object_class, PROP_INITIALIZED, "initialized");
}

void
on_manager_ready(GObject *source_object, GAsyncResult *res, gpointer user_data)
{
  GError *error = NULL;

  if (!tp_proxy_prepare_finish(source_object, res, &error))
  {
    if (error)
      g_warning("%s: got error: %s", __FUNCTION__, error->message);
    else
      g_warning("%s: got unknown error", __FUNCTION__);
  }
  else
    _add_accounts(user_data);
}

static void
rtcom_account_plugin_init(RtcomAccountPlugin *plugin)
{
  RtcomAccountPluginPrivate *priv = PRIVATE(plugin);

  plugin->manager = tp_account_manager_dup();

  tp_proxy_prepare_async(plugin->manager, NULL, on_manager_ready, plugin);

  g_signal_connect(plugin->manager, "account-validity-changed",
                   G_CALLBACK(on_account_validity_changed_cb), plugin);

  g_signal_connect(plugin->manager, "account-removed",
                   G_CALLBACK(on_account_removed_cb), plugin);

  plugin->services = g_hash_table_new_full(
      (GHashFunc)&g_str_hash,
      (GEqualFunc)&g_str_equal,
      (GDestroyNotify)&g_free,
      (GDestroyNotify)&g_object_unref);

  priv->initialized = FALSE;
}

RtcomAccountService *
rtcom_account_plugin_add_service(RtcomAccountPlugin *plugin,
                                 const gchar *service_id)
{
  RtcomAccountService *service = NULL;
  GStrv arr;
  guint len;

  g_return_val_if_fail(service_id != NULL, NULL);

  arr = g_strsplit(service_id, "/", 3);
  len = g_strv_length(arr);

  if (len == 3)
  {
    if (!arr[2] || !*arr[2])
      goto error;
  }
  else if (len != 2)
    goto error;

  if ((len == 2) || (len == 3))
  {
    if (!arr[0] || !*arr[0] || !arr[1] || !*arr[1])
      goto error;

    if ((len == 3) && !strcmp(arr[1], arr[2]))
      goto error;
  }

  service = rtcom_account_service_new(service_id, plugin);

  if (len == 3)
  {
    GdkPixbuf *icon;
    gchar *icon_name = g_strconcat("im-", arr[2], NULL);

    icon = gtk_icon_theme_load_icon(
        gtk_icon_theme_get_default(), icon_name, 48, 0, NULL);
    g_free(icon_name);

    if (icon)
    {
      g_object_set(G_OBJECT(service), "icon", icon, NULL);
      g_object_unref(icon);
    }

    g_object_set(G_OBJECT(service), "service-name", arr[2], NULL);
  }

  g_hash_table_insert(plugin->services, g_strdup(service_id), service);

error:
  g_strfreev(arr);

  if (!service)
    g_warning("Invalid service id, must be <cm_name/protocol_name>[/service]");

  return service;
}

TpDBusDaemon *
rtcom_account_plugin_get_dbus_daemon(RtcomAccountPlugin *plugin)
{
  return tp_proxy_get_dbus_daemon(plugin->manager);
}
