/*
 * accounts-ui.h
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

#ifndef ACCOUNTSUI_H
#define ACCOUNTSUI_H

G_BEGIN_DECLS

#define ACCOUNTS_TYPE_UI \
                (accounts_ui_get_type ())
#define ACCOUNTS_UI(obj) \
                (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
                 ACCOUNTS_TYPE_UI, \
                 AccountsUI))
#define ACCOUNTS_UI_CLASS(klass) \
                (G_TYPE_CHECK_CLASS_CAST ((klass), \
                 ACCOUNTS_TYPE_UI, \
                 AccountsUIClass))
#define ACCOUNTS_IS_UI(obj) \
                (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                 ACCOUNTS_TYPE_UI))
#define ACCOUNTS_IS_UI_CLASS(klass) \
                (G_TYPE_CHECK_CLASS_TYPE ((klass), \
                 ACCOUNTS_TYPE_UI))
#define ACCOUNTS_UI_GET_CLASS(obj) \
                (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                 ACCOUNTS_TYPE_UI, \
                 AccountsUIClass))

typedef struct _AccountsUIClass AccountsUIClass;
typedef struct _AccountsUI AccountsUI;

struct _AccountsUIClass
{
  GtkDialogClass parent_class;
};

struct _AccountsUI
{
  GtkDialog parent;
};

GType
accounts_ui_get_type(void) G_GNUC_CONST;

void
accounts_ui_set_parent(GtkWidget *accounts_ui,
                       GdkWindow *parent);

void
accounts_ui_show(GtkWidget *accounts_ui);

GtkWidget *
accounts_ui_dialogs_get_edit_account(GtkWidget *accounts_ui,
                                     const char *service_name,
                                     const char *user_name);

GtkWidget *
accounts_ui_dialogs_get_new_account(GtkWidget *accounts_ui,
                                    const char *service_name);

#endif /* ACCOUNTSUI_H */
