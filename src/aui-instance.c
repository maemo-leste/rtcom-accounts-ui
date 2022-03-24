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

#include <dbus/dbus-glib-lowlevel.h>
#include <dbus/dbus.h>

#include "aui-instance.h"

AuiInstance *
aui_instance_new(DBusGConnection *dbus_gconnection, guint xid)
{
  return g_object_new(AUI_TYPE_INSTANCE,
                      "dbus-connection", dbus_gconnection,
                      "parent-xid", xid, NULL);
}
