/*
 * main.c
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

#include <dbus/dbus-glib.h>
#include <hildon/hildon.h>
#include <libosso.h>

#include "aui-service.h"

static void
aui_service_num_instances_changed_cb(AuiService *service)
{
  static guint timeout_id = 0;

  if (aui_service_has_instances(service))
  {
    if (timeout_id)
    {
      g_source_remove(timeout_id);
      timeout_id = 0;
    }
  }
  else
    timeout_id = g_timeout_add_seconds(5, (GSourceFunc)gtk_main_quit, NULL);
}

int
main(int argc, char **argv, char **envp)
{
  int rv = 0;
  osso_context_t *osso;
  DBusGConnection *dbus;
  AuiService *service;
  GError *error = NULL;

#if !GLIB_CHECK_VERSION(2, 32, 0)
  g_thread_init(NULL);
#endif
  g_set_application_name("");
  osso = osso_initialize("RtcomAccounts", "1.0", FALSE, NULL);

  if (!osso)
  {
    g_warning("Failed to initialize OSSO");
    exit(1);
  }

  hildon_gtk_init(&argc, &argv);
  dbus = dbus_g_bus_get(DBUS_BUS_SESSION, &error);

  if (dbus)
  {
    service = aui_service_new(dbus);

    if (service)
    {
      g_signal_connect(service, "num-instances-changed",
                       G_CALLBACK(aui_service_num_instances_changed_cb), NULL);
      gtk_main();
      g_object_unref(service);
    }
    else
      rv = TRUE;

    dbus_g_connection_unref(dbus);
  }
  else
    rv = 1;

  if (error)
  {
    g_warning("Failed to get session bus: %s", error->message);
    g_clear_error(&error);
    rv = 1;
  }

  osso_deinitialize(osso);

  return rv;
}
