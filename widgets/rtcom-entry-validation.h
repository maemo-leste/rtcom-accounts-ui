/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of osso-accounts
 *
 * Copyright (C) 2009 Nokia Corporation. All rights reserved.
 *
 * Contact: Artem Garmash <artem.garmash@nokia.com>
 *
 * This software, including documentation, is protected by copyright controlled
 * by Nokia Corporation. All rights are reserved.
 * Copying, including reproducing, storing, adapting or translating, any or all
 * of this material requires the prior written consent of Nokia Corporation.
 * This material also contains confidential information which may not be
 * disclosed to others without the prior written consent of Nokia.
 */

#ifndef _RTCOM_ENTRY_VALIDATION_H_
#define _RTCOM_ENTRY_VALIDATION_H_

#include <glib-object.h>
#include <glib/gregex.h>

G_BEGIN_DECLS

#define RTCOM_TYPE_ENTRY_VALIDATION             (rtcom_entry_validation_get_type ())
#define RTCOM_ENTRY_VALIDATION(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), RTCOM_TYPE_ENTRY_VALIDATION, RtcomEntryValidation))
#define RTCOM_ENTRY_VALIDATION_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), RTCOM_TYPE_ENTRY_VALIDATION, RtcomEntryValidationClass))
#define RTCOM_IS_ENTRY_VALIDATION(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), RTCOM_TYPE_ENTRY_VALIDATION))
#define RTCOM_IS_ENTRY_VALIDATION_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), RTCOM_TYPE_ENTRY_VALIDATION))
#define RTCOM_ENTRY_VALIDATION_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), RTCOM_TYPE_ENTRY_VALIDATION, RtcomEntryValidationClass))

typedef struct _RtcomEntryValidationClass RtcomEntryValidationClass;
typedef struct _RtcomEntryValidation      RtcomEntryValidation;

struct _RtcomEntryValidationClass
{
    GObjectClass parent_class;
};

struct _RtcomEntryValidation
{
    GObject parent_instance;

    /* <private> */
    GRegex *validation_re;
    guint min_length;
    guint max_length;
    gchar *min_length_msg;
    gchar *max_length_msg;
};

GType rtcom_entry_validation_get_type (void) G_GNUC_CONST;

gboolean
rtcom_entry_validation_validate (RtcomEntryValidation *self,
                                 const gchar *entry_text,
                                 const gchar *msg_illegal,
                                 GError **error);

void
rtcom_entry_validation_set_invalid_chars_re (RtcomEntryValidation *self,
                                             const gchar *pattern);
const gchar*
rtcom_entry_validation_get_invalid_chars_re (RtcomEntryValidation *self);

void
rtcom_entry_validation_set_min_length (RtcomEntryValidation *self,
                                       guint length);
guint
rtcom_entry_validation_get_min_length (RtcomEntryValidation *self);

void
rtcom_entry_validation_set_max_length (RtcomEntryValidation *self,
                                       guint length);
guint
rtcom_entry_validation_get_max_length (RtcomEntryValidation *self);

void
rtcom_entry_validation_set_min_length_msg (RtcomEntryValidation *self,
                                           const gchar *msg);
const gchar*
rtcom_entry_validation_get_min_length_msg (RtcomEntryValidation *self);

void
rtcom_entry_validation_set_max_length_msg (RtcomEntryValidation *self,
                                           const gchar *msg);
const gchar*
rtcom_entry_validation_get_max_length_msg (RtcomEntryValidation *self);

G_END_DECLS

#endif /* _RTCOM_ENTRY_VALIDATION_H_ */
