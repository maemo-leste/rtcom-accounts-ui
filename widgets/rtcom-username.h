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

#ifndef _RTCOM_USERNAME_H_
#define _RTCOM_USERNAME_H_

#include <gtk/gtkhbox.h>
#include "rtcom-widget.h"
#include "rtcom-entry-validation.h"

G_BEGIN_DECLS

#define RTCOM_TYPE_USERNAME             (rtcom_username_get_type ())
#define RTCOM_USERNAME(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), RTCOM_TYPE_USERNAME, RtcomUsername))
#define RTCOM_USERNAME_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), RTCOM_TYPE_USERNAME, RtcomUsernameClass))
#define RTCOM_IS_USERNAME(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), RTCOM_TYPE_USERNAME))
#define RTCOM_IS_USERNAME_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), RTCOM_TYPE_USERNAME))
#define RTCOM_USERNAME_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), RTCOM_TYPE_USERNAME, RtcomUsernameClass))

typedef struct _RtcomUsernameClass RtcomUsernameClass;
typedef struct _RtcomUsername RtcomUsername;

struct _RtcomUsernameClass
{
    GtkHBoxClass parent_class;
};

struct _RtcomUsername
{
    GtkHBox parent_instance;

    /*< protected >*/
    gchar *field;
    gchar *prefill;
    gboolean editable;
    gboolean check_uniqueness;
    gboolean show_server_name;

    GtkWidget *username_editor;
    GtkWidget *at_label;
    GtkWidget *server_editor;

    TpProtocol *protocol;

    guint filled_fields;

    RtcomEntryValidation *username_validation;
    RtcomEntryValidation *server_validation;

    gboolean must_have_at_separator;
    gchar *required_server;
    gchar *required_server_error;
};

GType rtcom_username_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif /* _RTCOM_USERNAME_H_ */
