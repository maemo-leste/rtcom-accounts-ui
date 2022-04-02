/* vi: set et sw=4 ts=4 cino=t0,(0: */
/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of rtcom-accounts-ui
 *
 * Copyright (C) 2009 Nokia Corporation. All rights reserved.
 *
 * Contact: Salvatore Iovene <ext-salvatore.iovene@nokia.com>
 * Contact: Artem Garmash <artem.garmash@nokia.com>
 *
 * This software, including documentation, is protected by copyright controlled
 * by Nokia Corporation. All rights are reserved.
 * Copying, including reproducing, storing, adapting or translating, any or all
 * of this material requires the prior written consent of Nokia Corporation.
 * This material also contains confidential information which may not be
 * disclosed to others without the prior written consent of Nokia.
 */

#ifndef _RTCOM_AVATAR_H_
#define _RTCOM_AVATAR_H_

#include <gtk/gtkwidget.h>
#include <gtk/gtkeventbox.h>
#include "rtcom-widget.h"

G_BEGIN_DECLS

#define RTCOM_TYPE_AVATAR             (rtcom_avatar_get_type ())
#define RTCOM_AVATAR(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), RTCOM_TYPE_AVATAR, RtcomAvatar))
#define RTCOM_AVATAR_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), RTCOM_TYPE_AVATAR, RtcomAvatarClass))
#define RTCOM_IS_AVATAR(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), RTCOM_TYPE_AVATAR))
#define RTCOM_IS_AVATAR_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), RTCOM_TYPE_AVATAR))
#define RTCOM_AVATAR_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), RTCOM_TYPE_AVATAR, RtcomAvatarClass))

typedef struct _RtcomAvatarClass RtcomAvatarClass;
typedef struct _RtcomAvatar RtcomAvatar;

struct _RtcomAvatarClass
{
    GtkEventBoxClass parent_class;
};

struct _RtcomAvatar
{
    GtkEventBox parent_instance;

    /*< protected >*/
    GtkWidget *image;
    GdkPixbuf *src;
};

GType rtcom_avatar_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif /* _RTCOM_AVATAR_H_ */
