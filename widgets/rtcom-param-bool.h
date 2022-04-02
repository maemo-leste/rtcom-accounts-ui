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

#ifndef _RTCOM_PARAM_BOOL_H_
#define _RTCOM_PARAM_BOOL_H_

#include <hildon/hildon-check-button.h>
#include "rtcom-widget.h"

G_BEGIN_DECLS

#define RTCOM_TYPE_PARAM_BOOL             (rtcom_param_bool_get_type ())
#define RTCOM_PARAM_BOOL(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), RTCOM_TYPE_PARAM_BOOL, RtcomParamBool))
#define RTCOM_PARAM_BOOL_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), RTCOM_TYPE_PARAM_BOOL, RtcomParamBoolClass))
#define RTCOM_IS_PARAM_BOOL(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), RTCOM_TYPE_PARAM_BOOL))
#define RTCOM_IS_PARAM_BOOL_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), RTCOM_TYPE_PARAM_BOOL))
#define RTCOM_PARAM_BOOL_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), RTCOM_TYPE_PARAM_BOOL, RtcomParamBoolClass))

typedef struct _RtcomParamBoolClass RtcomParamBoolClass;
typedef struct _RtcomParamBool RtcomParamBool;

struct _RtcomParamBoolClass
{
    HildonCheckButtonClass parent_class;
};

struct _RtcomParamBool
{
    HildonCheckButton parent_instance;

    /*< protected >*/
    gchar *field;
};

GType rtcom_param_bool_get_type (void) G_GNUC_CONST;

const gchar *rtcom_param_bool_get_field (RtcomParamBool *param);

G_END_DECLS

#endif /* _RTCOM_PARAM_BOOL_H_ */
