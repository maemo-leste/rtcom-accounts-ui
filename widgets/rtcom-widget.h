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

#ifndef _RTCOM_WIDGET_H_
#define _RTCOM_WIDGET_H_

#include <gtk/gtkwidget.h>

#include "rtcom-account-item.h"

G_BEGIN_DECLS

#define RTCOM_TYPE_WIDGET             (rtcom_widget_get_type ())
#define RTCOM_WIDGET(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), RTCOM_TYPE_WIDGET, RtcomWidget))
#define RTCOM_IS_WIDGET(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), RTCOM_TYPE_WIDGET))
#define RTCOM_WIDGET_GET_IFACE(obj)   (G_TYPE_INSTANCE_GET_INTERFACE ((obj), RTCOM_TYPE_WIDGET, RtcomWidgetIface))

typedef struct _RtcomWidgetIface RtcomWidgetIface;
typedef struct _RtcomWidget RtcomWidget;

struct _RtcomWidgetIface
{
    GTypeInterface g_iface;

    /* methods */
    gboolean (* store_settings) (RtcomWidget *rtcom_widget, GError **error,
                                 RtcomAccountItem  *item);
    void (* get_settings) (RtcomWidget *rtcom_widget, RtcomAccountItem  *item);
    void (* set_account) (RtcomWidget *widget, RtcomAccountItem *account);
    gboolean (* validate) (RtcomWidget *widget, GError **error);
    gboolean (* get_value) (RtcomWidget *widget, GValue *value);

    /*< private >*/
    void (* class_set_property) (GObject *object, guint prop_id,
                                 const GValue *value, GParamSpec *pspec);
    void (* class_get_property) (GObject *object, guint property_id,
                                 GValue *value, GParamSpec *pspec);
    void (* class_dispose) (GObject *object);
};

/* Type creation macros */
#define RTCOM_DEFINE_WIDGET_TYPE(TN, t_n, T_P) \
    _RTCOM_DEFINE_TYPE_EXTENDED_BEGIN (TN, t_n, T_P, 0) \
    G_IMPLEMENT_INTERFACE (RTCOM_TYPE_WIDGET, rtcom_widget_init) \
    _RTCOM_DEFINE_TYPE_EXTENDED_END()

#define RTCOM_DEFINE_WIDGET_TYPE_WITH_PRIVATE(TN, t_n, T_P) \
    _RTCOM_DEFINE_TYPE_EXTENDED_BEGIN (TN, t_n, T_P, 0) \
    G_IMPLEMENT_INTERFACE (RTCOM_TYPE_WIDGET, rtcom_widget_init) \
    G_ADD_PRIVATE (TN) \
    _RTCOM_DEFINE_TYPE_EXTENDED_END()

/* this is the GType _G_DEFINE_TYPE_EXTENDED_BEGIN with the following changes:
 *  - the class_intern_init() also calls rtcom_widget_class_init at the end
 *  - instead of calling the type_name##_init() directly, this has been
 *  interned too; the intern function calls rtcom_widget_instance_init() too.
 */
#define _RTCOM_DEFINE_TYPE_EXTENDED_BEGIN(TypeName, type_name, TYPE_PARENT, flags) \
\
static void     type_name##_init              (TypeName        *self); \
static void     type_name##_class_init        (TypeName##Class *klass); \
static gpointer type_name##_parent_class = NULL; \
static gint     TypeName##_private_offset; \
static void     rtcom_widget_init(RtcomWidgetIface *iface); \
static void     type_name##_intern_init (TypeName *self) \
{ \
  rtcom_widget_instance_init ((RtcomWidget*) self); \
  type_name##_init (self); \
} \
\
static void     type_name##_class_intern_init (gpointer klass) \
{ \
  type_name##_parent_class = g_type_class_peek_parent (klass); \
  if (TypeName##_private_offset != 0) \
    g_type_class_adjust_private_offset (klass, &TypeName##_private_offset); \
  type_name##_class_init ((TypeName##Class*) klass); \
  rtcom_widget_class_init ((GObjectClass*) klass); \
} \
\
G_GNUC_UNUSED \
static inline gpointer \
type_name##_get_instance_private (TypeName *self) \
{ \
  return (G_STRUCT_MEMBER_P (self, TypeName##_private_offset)); \
} \
\
GType \
type_name##_get_type (void) \
{ \
  static gsize g_define_type_id_fence = 0; \
  if (g_once_init_enter (&g_define_type_id_fence))  \
    { \
      GType g_define_type_id = \
        g_type_register_static_simple (TYPE_PARENT, \
                                       g_intern_static_string (#TypeName), \
                                       sizeof (TypeName##Class), \
                                       (GClassInitFunc)type_name##_class_intern_init, \
                                       sizeof (TypeName), \
                                       (GInstanceInitFunc)type_name##_intern_init, \
                                       (GTypeFlags) flags); \
      { /* custom code follows */
#define _RTCOM_DEFINE_TYPE_EXTENDED_END() \
        /* following custom code */ \
      } \
      g_once_init_leave (&g_define_type_id_fence, g_define_type_id); \
    } \
  return g_define_type_id_fence; \
} /* closes type_name##_get_type() */

GType  rtcom_widget_get_type (void) G_GNUC_CONST;

void rtcom_widget_instance_init (RtcomWidget *widget);
void rtcom_widget_class_init (GObjectClass *object_class);

void rtcom_widget_set_account (RtcomWidget *widget, RtcomAccountItem *account);
RtcomAccountItem *rtcom_widget_get_account (RtcomWidget *widget);

void rtcom_widget_set_msg_next (RtcomWidget *widget, const gchar *message);
const gchar *rtcom_widget_get_msg_next (RtcomWidget *widget);
void rtcom_widget_set_error_widget (RtcomWidget *widget,
                                    GtkWidget *error_widget);
void rtcom_widget_focus_error (RtcomWidget *widget);
gboolean rtcom_widget_validate (RtcomWidget *widget, GError **error);

gboolean rtcom_widget_get_value (RtcomWidget *widget, GValue *value);
void rtcom_widget_value_changed (RtcomWidget *widget);

G_END_DECLS

#endif /* _RTCOM_WIDGET_H_ */
