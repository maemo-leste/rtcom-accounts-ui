/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* vi: set et sw=4 ts=4 cino=t0,(0: */
/*
 * This file is part of osso-accounts
 *
 * Copyright (C) 2007 Nokia Corporation. All rights reserved.
 *
 * Contact: Lassi Syrjala <lassi.syrjala@nokia.com>
 *
 * This software, including documentation, is protected by copyright controlled
 * by Nokia Corporation. All rights are reserved.
 * Copying, including reproducing, storing, adapting or translating, any or all
 * of this material requires the prior written consent of Nokia Corporation.
 * This material also contains confidential information which may not be
 * disclosed to others without the prior written consent of Nokia.
 */
#ifndef _ACCOUNTS_WIZARD_DIALOG_H_
#define _ACCOUNTS_WIZARD_DIALOG_H_

#include <glib-object.h>
#include <gtk/gtk.h>
#include <libaccounts/account-item.h>
#include <libaccounts/account-service.h>
#include <libaccounts/account-plugin-manager.h>

G_BEGIN_DECLS

#define ACCOUNTS_TYPE_WIZARD_DIALOG            (accounts_wizard_dialog_get_type ())
#define ACCOUNTS_WIZARD_DIALOG(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), ACCOUNTS_TYPE_WIZARD_DIALOG, AccountsWizardDialog))
#define ACCOUNTS_WIZARD_DIALOG_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), ACCOUNTS_TYPE_WIZARD_DIALOG, AccountsWizardDialogClass))
#define ACCOUNTS_IS_WIZARD_DIALOG(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), ACCOUNTS_TYPE_WIZARD_DIALOG))
#define ACCOUNTS_IS_WIZARD_DIALOG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), ACCOUNTS_TYPE_WIZARD_DIALOG))
#define ACCOUNTS_WIZARD_DIALOG_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), ACCOUNTS_TYPE_WIZARD_DIALOG, AccountsWizardDialogClass))

typedef struct _AccountsWizardDialogClass AccountsWizardDialogClass;
typedef struct _AccountsWizardDialog AccountsWizardDialog;

struct _AccountsWizardDialogClass
{
    GtkDialogClass parent_class;

    /* signals */
    void (*delete_account) (AccountItem *item);
};

struct _AccountsWizardDialog
{
    GtkDialog parent_instance;
};

GType accounts_wizard_dialog_get_type (void) G_GNUC_CONST;

GtkWidget *accounts_wizard_dialog_new (GtkWindow *window,
                                       AccountPluginManager *manager,
                                       AccountItem *item,
                                       AccountService *service);

gboolean accounts_wizard_dialog_run (AccountsWizardDialog * dlg);

G_END_DECLS

#endif /* _ACCOUNTS_WIZARD_DIALOG_H_ */
