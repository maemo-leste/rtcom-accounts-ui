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

#ifndef _RTCOM_ALIAS_H_
#define _RTCOM_ALIAS_H_

#include <hildon/hildon-entry.h>
#include "rtcom-widget.h"

G_BEGIN_DECLS

#define RTCOM_TYPE_ALIAS             (rtcom_alias_get_type ())
#define RTCOM_ALIAS(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), RTCOM_TYPE_ALIAS, RtcomAlias))
#define RTCOM_ALIAS_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), RTCOM_TYPE_ALIAS, RtcomAliasClass))
#define RTCOM_IS_ALIAS(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), RTCOM_TYPE_ALIAS))
#define RTCOM_IS_ALIAS_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), RTCOM_TYPE_ALIAS))
#define RTCOM_ALIAS_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), RTCOM_TYPE_ALIAS, RtcomAliasClass))

typedef struct _RtcomAliasClass RtcomAliasClass;
typedef struct _RtcomAlias RtcomAlias;

struct _RtcomAliasClass
{
    HildonEntryClass parent_class;
};

struct _RtcomAlias
{
    HildonEntry parent_instance;
};

GType rtcom_alias_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif /* _RTCOM_ALIAS_H_ */
