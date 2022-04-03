/* vi: set et sw=4 ts=4 cino=t0,(0: */
/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of osso-accounts
 *
 * Copyright (C) 2008 Nokia Corporation. All rights reserved.
 *
 * Contact: Salvatore Iovene <ext-salvatore.iovene@nokia.com>
 *
 * This software, including documentation, is protected by copyright controlled
 * by Nokia Corporation. All rights are reserved.
 * Copying, including reproducing, storing, adapting or translating, any or all
 * of this material requires the prior written consent of Nokia Corporation.
 * This material also contains confidential information which may not be
 * disclosed to others without the prior written consent of Nokia.
 */

#ifndef _RTCOM_DIALOG_CONTEXT_H_
#define _RTCOM_DIALOG_CONTEXT_H_

#include <libaccounts/account-edit-context.h>
#include <libaccounts/account-dialog-context.h>


G_BEGIN_DECLS

#define RTCOM_TYPE_DIALOG_CONTEXT             (rtcom_dialog_context_get_type ())
#define RTCOM_DIALOG_CONTEXT(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), RTCOM_TYPE_DIALOG_CONTEXT, RtcomDialogContext))
#define RTCOM_DIALOG_CONTEXT_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), RTCOM_TYPE_DIALOG_CONTEXT, RtcomDialogContextClass))
#define RTCOM_IS_DIALOG_CONTEXT(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), RTCOM_TYPE_DIALOG_CONTEXT))
#define RTCOM_IS_DIALOG_CONTEXT_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), RTCOM_TYPE_DIALOG_CONTEXT))
#define RTCOM_DIALOG_CONTEXT_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), RTCOM_TYPE_DIALOG_CONTEXT, RtcomDialogContextClass))

typedef struct _RtcomDialogContextClass RtcomDialogContextClass;
typedef struct _RtcomDialogContext RtcomDialogContext;

#include "rtcom-page.h"
#include "rtcom-account-plugin.h"

struct _RtcomDialogContextClass
{
    AccountEditContextClass parent_class;
};

struct _RtcomDialogContext
{
    AccountEditContext parent_instance;

    /*< private >*/
    GList *objects;
};

GType rtcom_dialog_context_get_type (void) G_GNUC_CONST;

RtcomDialogContext *rtcom_dialog_context_new (RtcomAccountPlugin *plugin,
                                              RtcomAccountItem *item,
                                              gboolean editing_existing);

GtkWidget * rtcom_dialog_context_get_start_page(RtcomDialogContext * dialog_context);
void rtcom_dialog_context_set_start_page(RtcomDialogContext * dialog_context, GtkWidget * page);
void rtcom_dialog_context_take_obj (RtcomDialogContext *dialog_context, GObject *object);
void rtcom_dialog_context_remove_obj (RtcomDialogContext *dialog_context, GObject *object);

/*
void rtcom_dialog_context_truncate (RtcomDialogContext *dialog_context);
*/
gboolean rtcom_dialog_context_finish (AccountDialogContext *dialog_context, GError **error);

G_END_DECLS

#endif /* _RTCOM_DIALOG_CONTEXT_H_ */
