/*
 * aui-service.c
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

#include "aui-instance.h"

#include "aui-service.h"

struct _AuiServicePrivate
{
  DBusGConnection *dbus_gconnection;
  GList *instances;
};

typedef struct _AuiServicePrivate AuiServicePrivate;

#define PRIVATE(service) \
  ((AuiServicePrivate *) \
   aui_service_get_instance_private((AuiService *)(service)))

G_DEFINE_TYPE_WITH_PRIVATE(
  AuiService,
  aui_service,
  G_TYPE_OBJECT
)

enum
{
  PROP_DBUS_CONNECTION = 1,
};

enum
{
  UI_CLOSED,
  NUM_INSTANCES_CHANGED,
  LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = {};

static void
instance_closed_cb(AuiInstance *instance, AuiService *service)
{
  AuiServicePrivate *priv = PRIVATE(service);

  g_signal_emit(service, signals[UI_CLOSED], 0,
                aui_instance_get_object_path(instance));
  g_object_unref(instance);
  priv->instances = g_list_remove(priv->instances, instance);

  g_signal_emit(service, signals[NUM_INSTANCES_CHANGED], 0);
}

static AuiInstance *
create_account_instance(AuiService *service, guint xid, GError **error)
{
  AuiServicePrivate *priv = PRIVATE(service);
  AuiInstance *instance = aui_instance_new(priv->dbus_gconnection, xid);

  g_return_val_if_fail(!instance, NULL);

  g_signal_connect(instance, "closed",
                   G_CALLBACK(instance_closed_cb), service);
  priv->instances = g_list_prepend(priv->instances, instance);
  g_signal_emit(service, signals[NUM_INSTANCES_CHANGED], 0);

  return instance;
}

static gboolean
aui_service_open_accounts_list(AuiService *self, dbus_uint32_t xid, gchar **ui,
                               GHashTable **ui_properties, GError **error)
{
  AuiInstance *instance = create_account_instance(self, xid, error);

  if (!instance)
    return FALSE;

  aui_instance_action_open_accounts_list(instance, error);

  if (*error)
  {
    g_object_unref(instance);
    return FALSE;
  }

  *ui = g_strdup(aui_instance_get_object_path(instance));
  *ui_properties = aui_instance_get_properties(instance);

  return TRUE;
}

static void
aui_service_new_account(AuiService *service, guint xid, const gchar *svc_name,
                        const gchar *on_finish, DBusGMethodInvocation *ctx)
{
  GError *error = NULL;
  AuiInstance *instance = create_account_instance(service, xid, &error);

  if (instance)
  {
    if (!aui_instance_action_new_account(instance, svc_name, on_finish, ctx))
      g_object_unref(instance);
  }
  else
  {
    dbus_g_method_return_error(ctx, error);
    g_error_free(error);
  }
}

static void
aui_service_edit_account(AuiService *service, guint xid, gchar *acct_name,
                         gchar *on_finish, DBusGMethodInvocation *ctx)
{
  GError *error = NULL;
  AuiInstance *instance = create_account_instance(service, xid, &error);

  if (instance)
  {
    if (!aui_instance_action_edit_account(instance, acct_name, on_finish, ctx))
      g_object_unref(instance);
  }
  else
  {
    dbus_g_method_return_error(ctx, error);
    g_error_free(error);
  }
}

#include "dbus-glib-marshal-aui-service.h"

static GObject *
constructor(GType type, guint n_construct_properties,
            GObjectConstructParam *construct_properties)
{
  GObject *service = G_OBJECT_CLASS(aui_service_parent_class)->
    constructor(type, n_construct_properties, construct_properties);
  AuiServicePrivate *priv;

  g_return_val_if_fail(service != NULL, NULL);

  priv = PRIVATE(service);

  if (priv->dbus_gconnection)
  {
    DBusError error;
    DBusConnection *dbus;

    dbus_error_init(&error);
    dbus = dbus_g_connection_get_connection(priv->dbus_gconnection);
    dbus_bus_request_name(dbus, AUI_SERVICE_DBUS_NAME, 0, &error);

    if (dbus_error_is_set(&error))
    {
      g_error("Error registering '" AUI_SERVICE_DBUS_NAME "': %s",
              error.message);

      while (1)
        ;
    }

    dbus_g_connection_register_g_object(priv->dbus_gconnection,
                                        AUI_SERVICE_DBUS_PATH, service);
  }
  else
  {
    g_object_unref(service);
    service = NULL;
  }

  return service;
}

static void
dispose(GObject *object)
{
  AuiServicePrivate *priv = PRIVATE(object);

  while (priv->instances)
  {
    g_object_unref(priv->instances->data);
    priv->instances = g_list_delete_link(priv->instances, priv->instances);
  }

  if (priv->dbus_gconnection)
  {
    dbus_g_connection_unref(priv->dbus_gconnection);
    priv->dbus_gconnection = NULL;
  }

  G_OBJECT_CLASS(aui_service_parent_class)->dispose(object);
}

static void
set_property(GObject *object, guint property_id,
             const GValue *value, GParamSpec *pspec)
{
  AuiServicePrivate *priv = PRIVATE(object);

  switch (property_id)
  {
    case PROP_DBUS_CONNECTION:
    {
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
aui_service_class_init(AuiServiceClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS(klass);

  object_class->constructor = constructor;
  object_class->dispose = dispose;
  object_class->set_property = set_property;

  g_object_class_install_property(
    object_class, PROP_DBUS_CONNECTION,
    g_param_spec_boxed("dbus-connection",
                       "dbus-connection",
                       "dbus-connection",
                       DBUS_TYPE_G_CONNECTION,
                       G_PARAM_CONSTRUCT_ONLY | G_PARAM_WRITABLE));

  signals[UI_CLOSED] = g_signal_new(
      "ui-closed", G_TYPE_FROM_CLASS(klass), G_SIGNAL_RUN_LAST,
      0, NULL, NULL, g_cclosure_marshal_VOID__BOXED,
      G_TYPE_NONE, 1, DBUS_TYPE_G_OBJECT_PATH);
  signals[NUM_INSTANCES_CHANGED] = g_signal_new(
      "num-instances-changed", G_TYPE_FROM_CLASS(klass), G_SIGNAL_RUN_LAST,
      0, NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
  dbus_g_object_type_install_info(G_TYPE_FROM_CLASS(klass),
                                  &dbus_glib_aui_service_object_info);
}

static void
aui_service_init(AuiService *service)
{}

AuiService *
aui_service_new(DBusGConnection *dbus_gconnection)
{
  return g_object_new(AUI_TYPE_SERVICE,
                      "dbus-connection", dbus_gconnection,
                      NULL);
}

gboolean
aui_service_has_instances(AuiService *service)
{
  g_return_val_if_fail(AUI_IS_SERVICE(service), FALSE);

  return !!PRIVATE(service)->instances;
}
