/*
 * aui-instance.h
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

#ifndef AUIINSTANCE_H
#define AUIINSTANCE_H

G_BEGIN_DECLS

#define AUI_TYPE_INSTANCE \
                (aui_instance_get_type ())
#define AUI_INSTANCE(obj) \
                (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
                 AUI_TYPE_INSTANCE, \
                 AuiInstance))
#define AUI_INSTANCE_CLASS(klass) \
                (G_TYPE_CHECK_CLASS_CAST ((klass), \
                 AUI_TYPE_INSTANCE, \
                 AuiInstanceClass))
#define AUI_IS_INSTANCE(obj) \
                (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                 AUI_TYPE_INSTANCE))
#define AUI_IS_INSTANCE_CLASS(klass) \
                (G_TYPE_CHECK_CLASS_TYPE ((klass), \
                 AUI_TYPE_INSTANCE))
#define AUI_INSTANCE_GET_CLASS(obj) \
                (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                 AUI_TYPE_INSTANCE, \
                 AuiInstanceClass))

typedef struct _AuiInstanceClass AuiInstanceClass;
typedef struct _AuiInstance AuiInstance;

struct _AuiInstanceClass
{
  GObjectClass parent_class;
};

struct _AuiInstance
{
  GObject parent;
};

GType
aui_instance_get_type(void) G_GNUC_CONST;

AuiInstance *
aui_instance_new(DBusGConnection *dbus_gconnection,
                 guint xid);

const gchar *
aui_instance_get_object_path(AuiInstance *instance);

void
aui_instance_action_open_accounts_list(AuiInstance *instance,
                                       GError **error);

GHashTable *
aui_instance_get_properties(AuiInstance *instance);

gboolean
aui_instance_action_new_account(AuiInstance *instance,
                                const gchar *service_name,
                                const gchar *on_finish,
                                DBusGMethodInvocation *ctx);

gboolean
aui_instance_action_edit_account(AuiInstance *instance,
                                 const gchar *account_name,
                                 const char *on_finish,
                                 DBusGMethodInvocation *context);

G_END_DECLS

#endif /* AUIINSTANCE_H */
