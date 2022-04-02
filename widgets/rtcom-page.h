/* vi: set et sw=4 ts=4 cino=t0,(0: */
/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of osso-accounts
 *
 * Copyright (C) 2007 Nokia Corporation. All rights reserved.
 *
 * Contact: Alberto Mardegan <alberto.mardegan@nokia.com>
 * Contact: Lassi Syrjala <lassi.syrjala@nokia.com>
 *
 * This software, including documentation, is protected by copyright controlled
 * by Nokia Corporation. All rights are reserved.
 * Copying, including reproducing, storing, adapting or translating, any or all
 * of this material requires the prior written consent of Nokia Corporation.
 * This material also contains confidential information which may not be
 * disclosed to others without the prior written consent of Nokia.
 */

#ifndef _RTCOM_PAGE_H_
#define _RTCOM_PAGE_H_

#include <gtk/gtkbin.h>
#include <libaccounts/account-error.h>

G_BEGIN_DECLS

#define RTCOM_TYPE_PAGE             (rtcom_page_get_type ())
#define RTCOM_PAGE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), RTCOM_TYPE_PAGE, RtcomPage))
#define RTCOM_PAGE_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), RTCOM_TYPE_PAGE, RtcomPageClass))
#define RTCOM_IS_PAGE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), RTCOM_TYPE_PAGE))
#define RTCOM_IS_PAGE_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), RTCOM_TYPE_PAGE))
#define RTCOM_PAGE_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), RTCOM_TYPE_PAGE, RtcomPageClass))

typedef struct _RtcomPageClass RtcomPageClass;
typedef struct _RtcomPage RtcomPage;

#include "rtcom-account-item.h"
#include "rtcom-widget.h"

struct _RtcomPageClass
{
    GtkBinClass parent_class;

    /* signals */
    gboolean (*validate) (RtcomPage *page, GError **error);
    void (*set_account) (RtcomPage *page, RtcomAccountItem *account);
};

struct _RtcomPage
{
    GtkBin parent_instance;
    
    /*< private >*/
    GHashTable *widgets;
    gint flags;
    gboolean last;
    gchar *title;
};

GType rtcom_page_get_type (void) G_GNUC_CONST;

void rtcom_page_set_account (RtcomPage *page, RtcomAccountItem *account);

gboolean rtcom_page_validate (RtcomPage *page, GError **error);

const gchar *rtcom_page_get_title (RtcomPage *page);

void rtcom_page_set_object_can_next (RtcomPage *page, GObject *object,
                                     gboolean can_next);

G_END_DECLS

#endif /* _RTCOM_PAGE_H_ */
