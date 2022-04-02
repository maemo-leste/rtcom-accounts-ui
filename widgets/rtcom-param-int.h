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

#ifndef _RTCOM_PARAM_INT_H_
#define _RTCOM_PARAM_INT_H_

#include <hildon/hildon-entry.h>
#include "rtcom-widget.h"

G_BEGIN_DECLS

#define RTCOM_TYPE_PARAM_INT             (rtcom_param_int_get_type ())
#define RTCOM_PARAM_INT(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), RTCOM_TYPE_PARAM_INT, RtcomParamInt))
#define RTCOM_PARAM_INT_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), RTCOM_TYPE_PARAM_INT, RtcomParamIntClass))
#define RTCOM_IS_PARAM_INT(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), RTCOM_TYPE_PARAM_INT))
#define RTCOM_IS_PARAM_INT_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), RTCOM_TYPE_PARAM_INT))
#define RTCOM_PARAM_INT_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), RTCOM_TYPE_PARAM_INT, RtcomParamIntClass))

typedef struct _RtcomParamIntClass RtcomParamIntClass;
typedef struct _RtcomParamInt RtcomParamInt;

struct _RtcomParamIntClass
{
    HildonEntryClass parent_class;
};

struct _RtcomParamInt
{
    HildonEntry parent_instance;

    /*< protected >*/
    gchar *field;
    gint min, max;
};

GType rtcom_param_int_get_type (void) G_GNUC_CONST;

const gchar *rtcom_param_int_get_field (RtcomParamInt *param);
gint rtcom_param_int_get_value (RtcomParamInt *param);
void rtcom_param_int_set_value (RtcomParamInt *param, gint value);

G_END_DECLS

#endif /* _RTCOM_PARAM_INT_H_ */
