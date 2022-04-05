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

#ifndef _RTCOM_ACCOUNT_SERVICE_H_
#define _RTCOM_ACCOUNT_SERVICE_H_

#include <libaccounts/account-service.h>
#include <telepathy-glib/connection-manager.h>
#include <telepathy-glib/connection.h>

G_BEGIN_DECLS

#define RTCOM_TYPE_ACCOUNT_SERVICE             (rtcom_account_service_get_type ())
#define RTCOM_ACCOUNT_SERVICE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), RTCOM_TYPE_ACCOUNT_SERVICE, RtcomAccountService))
#define RTCOM_ACCOUNT_SERVICE_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), RTCOM_TYPE_ACCOUNT_SERVICE, RtcomAccountServiceClass))
#define RTCOM_IS_ACCOUNT_SERVICE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), RTCOM_TYPE_ACCOUNT_SERVICE))
#define RTCOM_IS_ACCOUNT_SERVICE_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), RTCOM_TYPE_ACCOUNT_SERVICE))
#define RTCOM_ACCOUNT_SERVICE_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), RTCOM_TYPE_ACCOUNT_SERVICE, RtcomAccountServiceClass))

typedef struct _RtcomAccountServiceClass RtcomAccountServiceClass;
typedef struct _RtcomAccountService RtcomAccountService;

#include "rtcom-account-plugin.h"

/* Return TpConnection proxy to account plugin for ,e.g.
 * Setting profile information on this connection
 * proxy will be NULL if there is error */
typedef void (*RtcomAccountServiceConnectionCb) (GObject *, TpConnection *, GError *, gpointer);

struct _RtcomAccountServiceClass
{
    AccountServiceClass parent_class;
};

struct _RtcomAccountService
{
    AccountService parent_instance;

    /*< private >*/
    TpProtocol *protocol;
    gchar *successful_msg;
};

GType rtcom_account_service_get_type (void) G_GNUC_CONST;

RtcomAccountService *rtcom_account_service_new (const gchar *name,
                                                RtcomAccountPlugin *plugin);

TpProtocol *rtcom_account_service_get_protocol (RtcomAccountService *service);
GType rtcom_account_service_get_param_type (RtcomAccountService *service,
                                            const gchar *name);

void rtcom_account_service_connect (RtcomAccountService *service,
                                    GHashTable *params,
                                    GObject * requester,
                                    gboolean disconnect,
                                    RtcomAccountServiceConnectionCb cb,
                                    gpointer user_data);

void rtcom_account_service_set_successful_message (RtcomAccountService *service,
                                                   const gchar *msg);

G_END_DECLS

#endif /* _RTCOM_ACCOUNT_SERVICE_H_ */
