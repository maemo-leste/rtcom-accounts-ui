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

#ifndef _RTCOM_ACCOUNT_PLUGIN_H_
#define _RTCOM_ACCOUNT_PLUGIN_H_

#include <libaccounts/account-plugin.h>
#include <telepathy-glib/account-manager.h>

G_BEGIN_DECLS

#define RTCOM_TYPE_ACCOUNT_PLUGIN             (rtcom_account_plugin_get_type ())
#define RTCOM_ACCOUNT_PLUGIN(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), RTCOM_TYPE_ACCOUNT_PLUGIN, RtcomAccountPlugin))
#define RTCOM_ACCOUNT_PLUGIN_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), RTCOM_TYPE_ACCOUNT_PLUGIN, RtcomAccountPluginClass))
#define RTCOM_IS_ACCOUNT_PLUGIN(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), RTCOM_TYPE_ACCOUNT_PLUGIN))
#define RTCOM_IS_ACCOUNT_PLUGIN_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), RTCOM_TYPE_ACCOUNT_PLUGIN))
#define RTCOM_ACCOUNT_PLUGIN_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), RTCOM_TYPE_ACCOUNT_PLUGIN, RtcomAccountPluginClass))

enum
{
    RTCOM_PLUGIN_CAPABILITY_REGISTER       = 1 << 0,
    RTCOM_PLUGIN_CAPABILITY_FORGOT_PWD     = 1 << 1,
    RTCOM_PLUGIN_CAPABILITY_ADVANCED       = 1 << 2,
    RTCOM_PLUGIN_CAPABILITY_SCREEN_NAME    = 1 << 4,
    RTCOM_PLUGIN_CAPABILITY_ALLOW_MULTIPLE = 1 << 5,

    RTCOM_PLUGIN_CAPABILITY_ALL =
        RTCOM_PLUGIN_CAPABILITY_REGISTER      |
        RTCOM_PLUGIN_CAPABILITY_FORGOT_PWD    |
        RTCOM_PLUGIN_CAPABILITY_ADVANCED      |
        RTCOM_PLUGIN_CAPABILITY_SCREEN_NAME   |
        RTCOM_PLUGIN_CAPABILITY_ALLOW_MULTIPLE
};

typedef struct _RtcomAccountPluginClass RtcomAccountPluginClass;
typedef struct _RtcomAccountPlugin RtcomAccountPlugin;

#include "rtcom-dialog-context.h"

struct _RtcomAccountPluginClass
{
    AccountPluginClass parent_class;
    void (*context_init) (RtcomAccountPlugin *plugin, RtcomDialogContext *context);
};

struct _RtcomAccountPlugin
{
    AccountPlugin parent_instance;

    /*< protected >*/
    GHashTable *profiles;
    /* The name must be set in the instance init function of the derived class */
    gchar *name;

    /* Properties to be set in the instance init function of the derived class */
    gchar * username_prefill;
    guint capabilities;

    TpAccountManager *manager;
};

GType rtcom_account_plugin_get_type (void) G_GNUC_CONST;

RtcomAccountService *
rtcom_account_plugin_add_service (RtcomAccountPlugin *plugin,
                                  const gchar *name);

TpDBusDaemon *rtcom_account_plugin_get_dbus_daemon (RtcomAccountPlugin *plugin);

G_END_DECLS

#endif /* _RTCOM_ACCOUNT_PLUGIN_H_ */
