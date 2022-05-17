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

#ifndef _RTCOM_EDIT_H_
#define _RTCOM_EDIT_H_

#include <gtk/gtk.h>

#include "rtcom-page.h"

G_BEGIN_DECLS

#define RTCOM_TYPE_EDIT             (rtcom_edit_get_type ())
#define RTCOM_EDIT(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), RTCOM_TYPE_EDIT, RtcomEdit))
#define RTCOM_EDIT_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), RTCOM_TYPE_EDIT, RtcomEditClass))
#define RTCOM_IS_EDIT(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), RTCOM_TYPE_EDIT))
#define RTCOM_IS_EDIT_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), RTCOM_TYPE_EDIT))
#define RTCOM_EDIT_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), RTCOM_TYPE_EDIT, RtcomEditClass))

typedef struct _RtcomEditClass RtcomEditClass;
typedef struct _RtcomEdit RtcomEdit;

struct _RtcomEditClass
{
    RtcomPageClass parent_class;
};

struct _RtcomEdit
{
    RtcomPage parent_instance;
};

GType rtcom_edit_get_type (void) G_GNUC_CONST;

void rtcom_edit_connect_on_advanced(RtcomEdit *edit, GCallback cb,
                                    gpointer user_data);

void rtcom_edit_append_widget(RtcomEdit *edit, GtkWidget *label,
                              GtkWidget *widget);

G_END_DECLS

#endif /* _RTCOM_EDIT_H_ */
