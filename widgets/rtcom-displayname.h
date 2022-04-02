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

#ifndef _RTCOM_DISPLAYNAME_H_
#define _RTCOM_DISPLAYNAME_H_

#include <gtk/gtkentry.h>
#include "rtcom-widget.h"

G_BEGIN_DECLS

#define RTCOM_TYPE_DISPLAYNAME             (rtcom_displayname_get_type ())
#define RTCOM_DISPLAYNAME(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), RTCOM_TYPE_DISPLAYNAME, RtcomDisplayname))
#define RTCOM_DISPLAYNAME_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), RTCOM_TYPE_DISPLAYNAME, RtcomDisplaynameClass))
#define RTCOM_IS_DISPLAYNAME(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), RTCOM_TYPE_DISPLAYNAME))
#define RTCOM_IS_DISPLAYNAME_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), RTCOM_TYPE_DISPLAYNAME))
#define RTCOM_DISPLAYNAME_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), RTCOM_TYPE_DISPLAYNAME, RtcomDisplaynameClass))

typedef struct _RtcomDisplaynameClass RtcomDisplaynameClass;
typedef struct _RtcomDisplayname RtcomDisplayname;

struct _RtcomDisplaynameClass
{
    GtkEntryClass parent_class;
};

struct _RtcomDisplayname
{
    GtkEntry parent_instance;

    /*< protected >*/
    gboolean filled;
};

GType rtcom_displayname_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif /* _RTCOM_DISPLAYNAME_H_ */
