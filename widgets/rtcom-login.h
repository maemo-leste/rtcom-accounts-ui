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

#ifndef _RTCOM_LOGIN_H_
#define _RTCOM_LOGIN_H_

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define RTCOM_TYPE_LOGIN             (rtcom_login_get_type ())
#define RTCOM_LOGIN(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), RTCOM_TYPE_LOGIN, RtcomLogin))
#define RTCOM_LOGIN_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), RTCOM_TYPE_LOGIN, RtcomLoginClass))
#define RTCOM_IS_LOGIN(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), RTCOM_TYPE_LOGIN))
#define RTCOM_IS_LOGIN_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), RTCOM_TYPE_LOGIN))
#define RTCOM_LOGIN_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), RTCOM_TYPE_LOGIN, RtcomLoginClass))

typedef struct _RtcomLoginClass RtcomLoginClass;
typedef struct _RtcomLogin RtcomLogin;

#include "rtcom-account-item.h"
#include "rtcom-widget.h"
#include "rtcom-page.h"

struct _RtcomLoginClass
{
    RtcomPageClass parent_class;
};

struct _RtcomLogin
{
    RtcomPage parent_instance;
};

GType rtcom_login_get_type (void) G_GNUC_CONST;
void rtcom_login_connect_on_register(RtcomLogin *, GCallback, gpointer);
void rtcom_login_connect_on_forgot_password(RtcomLogin *, GCallback, gpointer);
void rtcom_login_connect_on_advanced(RtcomLogin *, GCallback, gpointer);
void rtcom_login_connect_on_define(RtcomLogin *, GCallback, gpointer);

G_END_DECLS

#endif /* _RTCOM_LOGIN_H_ */
