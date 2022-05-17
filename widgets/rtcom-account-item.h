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

#ifndef _RTCOM_ACCOUNT_ITEM_H_
#define _RTCOM_ACCOUNT_ITEM_H_

#include <libaccounts/account-item.h>
#include <telepathy-glib/account.h>

G_BEGIN_DECLS

#define RTCOM_TYPE_ACCOUNT_ITEM             (rtcom_account_item_get_type ())
#define RTCOM_ACCOUNT_ITEM(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), RTCOM_TYPE_ACCOUNT_ITEM, RtcomAccountItem))
#define RTCOM_ACCOUNT_ITEM_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), RTCOM_TYPE_ACCOUNT_ITEM, RtcomAccountItemClass))
#define RTCOM_IS_ACCOUNT_ITEM(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), RTCOM_TYPE_ACCOUNT_ITEM))
#define RTCOM_IS_ACCOUNT_ITEM_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), RTCOM_TYPE_ACCOUNT_ITEM))
#define RTCOM_ACCOUNT_ITEM_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), RTCOM_TYPE_ACCOUNT_ITEM, RtcomAccountItemClass))

typedef struct _RtcomAccountItemClass RtcomAccountItemClass;
typedef struct _RtcomAccountItem RtcomAccountItem;

#include "rtcom-account-service.h"

struct _RtcomAccountItemClass
{
    AccountItemClass parent_class;
};

struct _RtcomAccountItem
{
    AccountItem parent_instance;

    /* read-only, for plugin implementations (can be NULL) */
    TpAccount *account;

    /*< private >*/
    gint avatar_id;
    guint set_mask;
    GHashTable *new_params;
    gchar *display_name;
    gchar *nickname;
    gchar *avatar_data;
    gsize avatar_len;
    gchar *avatar_mime;
    gchar **secondary_vcard_fields;
    gboolean enabled_setting;
};

GType rtcom_account_item_get_type (void) G_GNUC_CONST;

RtcomAccountItem *rtcom_account_item_new (TpAccount *account,
                                          RtcomAccountService *service);

void rtcom_account_item_store_param_boolean (RtcomAccountItem *item,
                                             const gchar *name,
                                             gboolean value);
void rtcom_account_item_store_param_int (RtcomAccountItem *item,
                                         const gchar *name,
                                         gint value);
void rtcom_account_item_store_param_uint (RtcomAccountItem *item,
                                          const gchar *name,
                                          guint value);
void rtcom_account_item_store_param_string (RtcomAccountItem *item,
                                            const gchar *name,
                                            const gchar *value);
void rtcom_account_item_store_display_name (RtcomAccountItem *item,
                                            const gchar *name);
void rtcom_account_item_store_nickname (RtcomAccountItem *item,
                                        const gchar *name);
void rtcom_account_item_store_avatar (RtcomAccountItem *item,
                                      gchar *data, gsize len,
                                      const gchar *mime_type);
void rtcom_account_item_store_secondary_vcard_fields (RtcomAccountItem *item,
                                                      GList *fields);
void rtcom_account_item_unset_param (RtcomAccountItem *item,
                                     const gchar * name);

gboolean rtcom_account_item_store_settings (RtcomAccountItem *item,
                                            GError **error);

void rtcom_account_item_save_settings (RtcomAccountItem *item,
                                       GError **error);

gboolean rtcom_account_item_delete (RtcomAccountItem *item);

TpProtocol *rtcom_account_item_get_tp_protocol(RtcomAccountItem *item);
const gchar *rtcom_account_item_get_unique_name (RtcomAccountItem *item);

void rtcom_account_item_name_change (RtcomAccountItem *item);

void rtcom_account_item_verify (RtcomAccountItem *item, GError ** error);

void rtcom_account_item_reconnect (RtcomAccountItem *item);

void rtcom_account_item_call_reconnect (RtcomAccountItem *item);

/* store enabled setting as set in UI in contrast to the "enabled" property
 * in AccountItem representing actual TpAccount state */
void rtcom_account_store_enabled_setting (RtcomAccountItem *item, gboolean enabled);
gboolean rtcom_account_get_enabled_setting (RtcomAccountItem *item);

G_END_DECLS

#endif /* _RTCOM_ACCOUNT_ITEM_H_ */
