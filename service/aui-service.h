/*
 * aui-service.h
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

#ifndef AUISERVICE_H
#define AUISERVICE_H

G_BEGIN_DECLS

#define AUI_TYPE_SERVICE \
                (aui_service_get_type ())
#define AUI_SERVICE(obj) \
                (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
                 AUI_TYPE_SERVICE, \
                 AuiService))
#define AUI_SERVICE_CLASS(klass) \
                (G_TYPE_CHECK_CLASS_CAST ((klass), \
                 AUI_TYPE_SERVICE, \
                 AuiServiceClass))
#define AUI_IS_SERVICE(obj) \
                (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                 AUI_TYPE_SERVICE))
#define AUI_IS_SERVICE_CLASS(klass) \
                (G_TYPE_CHECK_CLASS_TYPE ((klass), \
                 AUI_TYPE_SERVICE))
#define AUI_SERVICE_GET_CLASS(obj) \
                (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                 AUI_TYPE_SERVICE, \
                 AuiServiceClass))

typedef struct _AuiServiceClass AuiServiceClass;
typedef struct _AuiService AuiService;

struct _AuiServiceClass
{
  GObjectClass parent_class;
};

struct _AuiService
{
  GObject parent;
};

GType
aui_service_get_type(void) G_GNUC_CONST;

gboolean
aui_service_has_instances(AuiService *service);

AuiService *
aui_service_new(DBusGConnection *dbus_gconnection);

#define AUI_SERVICE_DBUS_NAME "com.nokia.AccountsUI"
#define AUI_SERVICE_DBUS_PATH "/com/nokia/AccountsUI"

G_END_DECLS

#endif /* AUISERVICE_H */
