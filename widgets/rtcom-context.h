/* vi: set et sw=4 ts=4 cino=t0,(0: */
/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of osso-accounts
 *
 * Copyright (C) 2007 Nokia Corporation. All rights reserved.
 *
 * Contact: Alberto Mardegan <alberto.mardegan@nokia.com>
 * Contact: Lassi Syrjala <lassi.syrjala@nokia.com>
 * Contact: Salvatore Iovene <ext-salvatore.iovene@nokia.com>
 *
 * This software, including documentation, is protected by copyright controlled
 * by Nokia Corporation. All rights are reserved.
 * Copying, including reproducing, storing, adapting or translating, any or all
 * of this material requires the prior written consent of Nokia Corporation.
 * This material also contains confidential information which may not be
 * disclosed to others without the prior written consent of Nokia.
 */

#ifndef _RTCOM_CONTEXT_H_
#define _RTCOM_CONTEXT_H_

#include <libaccounts/account-edit-context.h>
#include <libaccounts/account-wizard-context.h>


G_BEGIN_DECLS

#define RTCOM_TYPE_CONTEXT             (rtcom_context_get_type ())
#define RTCOM_CONTEXT(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), RTCOM_TYPE_CONTEXT, RtcomContext))
#define RTCOM_CONTEXT_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), RTCOM_TYPE_CONTEXT, RtcomContextClass))
#define RTCOM_IS_CONTEXT(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), RTCOM_TYPE_CONTEXT))
#define RTCOM_IS_CONTEXT_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), RTCOM_TYPE_CONTEXT))
#define RTCOM_CONTEXT_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), RTCOM_TYPE_CONTEXT, RtcomContextClass))

typedef struct _RtcomContextClass RtcomContextClass;
typedef struct _RtcomContext RtcomContext;

#include "rtcom-page.h"
#include "rtcom-account-plugin.h"

struct _RtcomContextClass
{
    AccountEditContextClass parent_class;
};

struct _RtcomContext
{
    AccountEditContext parent_instance;

    /*< private >*/
    GList *pages;
    GList *objects;
    RtcomPage *current_page;
    gboolean can_next;
    gboolean can_back;
    gboolean can_finish;
};

GType rtcom_context_get_type (void) G_GNUC_CONST;

RtcomContext *rtcom_context_new (RtcomAccountPlugin *plugin,
                                 RtcomAccountItem *item,
                                 gboolean editing_existing);

void rtcom_context_take_obj (RtcomContext *context, GObject *object);
void rtcom_context_remove_obj (RtcomContext *context, GObject *object);

void rtcom_context_append_page (RtcomContext *context, RtcomPage *page);
void rtcom_context_truncate (RtcomContext *context);
gboolean rtcom_context_current_is_last (RtcomContext *context);
RtcomPage *rtcom_context_get_current_page (RtcomContext *context);

G_END_DECLS

#endif /* _RTCOM_CONTEXT_H_ */
