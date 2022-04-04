/*
 * rtcom-widget.c
 *
 * Copyright (C) 2022 Ivaylo Dimitrov <ivo.g.dimitrov.75@gmail.com>
 *
 * This library is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "config.h"

#include <hildon/hildon.h>

#include "rtcom-widget.h"

struct _RtcomWidgetPrivate
{
  gboolean required : 1;
  gboolean valid : 1;
  gboolean can_next : 1;
  gboolean name_change : 1;
  RtcomAccountItem *account;
  gchar *msg_next;
  GtkWidget *error_widget;
};

typedef struct _RtcomWidgetPrivate RtcomWidgetPrivate;

enum
{
  PROP_REQUIRED = 'e',
  PROP_VALID,
  PROP_CAN_NEXT,
  PROP_NAME_CHANGE
};

GType
rtcom_widget_get_type(void)
{
  static volatile gsize g_define_type_id__volatile = 0;

  if (g_once_init_enter(&g_define_type_id__volatile))
  {
    static const GTypeInfo info =
    {
      sizeof(RtcomWidgetIface),
      NULL,
      NULL,
      NULL,
      NULL,
      NULL,
      0,
      0,
      NULL,
      NULL,
    };

    GType g_define_type_id =
      g_type_register_static(G_TYPE_INTERFACE,
                             g_intern_static_string("RtcomWidget"),
                             &info,
                             0);

    g_once_init_leave(&g_define_type_id__volatile, g_define_type_id);
  }

  return g_define_type_id__volatile;
}

static void
rtcom_widget_set_property(GObject *object, guint property_id,
                          const GValue *value, GParamSpec *pspec)
{
  RtcomWidgetPrivate *priv = g_object_get_data(object, "rtcom");

  switch (property_id)
  {
    case PROP_REQUIRED:
    {
      priv->required = g_value_get_boolean(value);
      break;
    }
    case PROP_VALID:
    {
      priv->valid = g_value_get_boolean(value);
      break;
    }
    case PROP_CAN_NEXT:
    {
      priv->can_next = g_value_get_boolean(value);
      break;
    }
    case PROP_NAME_CHANGE:
    {
      priv->name_change = g_value_get_boolean(value);
      break;
    }
    default:
    {
      RtcomWidgetIface *iface = RTCOM_WIDGET_GET_IFACE(object);

      iface->class_set_property(object, property_id, value, pspec);
      break;
    }
  }
}

static void
rtcom_widget_dispose(GObject *object)
{
  RtcomWidgetPrivate *priv = g_object_get_data(object, "rtcom");
  RtcomWidgetIface *iface = RTCOM_WIDGET_GET_IFACE(object);

  if (priv->account)
  {
    g_signal_handlers_disconnect_matched(
      priv->account, G_SIGNAL_MATCH_DATA | G_SIGNAL_MATCH_FUNC, 0, 0, NULL,
      iface->store_settings, object);
    g_object_unref(priv->account);
    priv->account = 0;
  }

  iface->class_dispose(object);
}

static void
rtcom_widget_get_property(GObject *object, guint property_id,
                          GValue *value, GParamSpec *pspec)
{
  RtcomWidgetPrivate *priv = g_object_get_data(object, "rtcom");

  switch (property_id)
  {
    case PROP_REQUIRED:
    {
      g_value_set_boolean(value, priv->required);
      break;
    }
    case PROP_VALID:
    {
      g_value_set_boolean(value, priv->valid);
      break;
    }
    case PROP_CAN_NEXT:
    {
      g_value_set_boolean(value, priv->can_next);
      break;
    }
    case PROP_NAME_CHANGE:
    {
      g_value_set_boolean(value, priv->name_change);
      break;
    }
    default:
    {
      RtcomWidgetIface *iface = RTCOM_WIDGET_GET_IFACE(object);

      iface->class_get_property(object, property_id, value, pspec);
      break;
    }
  }
}

void
rtcom_widget_class_init(GObjectClass *object_class)
{
  RtcomWidgetIface *iface = g_type_interface_peek(object_class,
                                                  RTCOM_TYPE_WIDGET);

  iface->class_dispose = object_class->dispose;
  iface->class_set_property = object_class->set_property;
  iface->class_get_property = object_class->get_property;

  object_class->set_property = rtcom_widget_set_property;
  object_class->get_property = rtcom_widget_get_property;
  object_class->dispose = rtcom_widget_dispose;

  g_object_class_install_property(
    object_class, PROP_REQUIRED,
    g_param_spec_boolean(
      "required",
      "Required",
      "Required", FALSE,
      G_PARAM_READWRITE));
  g_object_class_install_property(
    object_class, PROP_VALID,
    g_param_spec_boolean(
      "valid",
      "Valid",
      "Valid",
      FALSE,
      G_PARAM_READWRITE));
  g_object_class_install_property(
    object_class, PROP_CAN_NEXT,
    g_param_spec_boolean(
      "can-next",
      "Can next",
      "Allow page `Next' button",
      TRUE,
      G_PARAM_READWRITE));
  g_object_class_install_property(
    object_class, PROP_NAME_CHANGE,
    g_param_spec_boolean(
      "name-change",
      "Name change",
      "Name change", FALSE,
      G_PARAM_READWRITE));
}

static void
rtcom_widget_private_free(gpointer data)
{
  RtcomWidgetPrivate *priv = data;

  g_free(priv->msg_next);
  g_free(priv);
}

void
rtcom_widget_instance_init(RtcomWidget *widget)
{
  RtcomWidgetPrivate *priv = g_new0(RtcomWidgetPrivate, 1);

  g_object_set_data_full(G_OBJECT(widget), "rtcom", priv,
                         rtcom_widget_private_free);
  priv->can_next = TRUE;
}

gboolean
rtcom_widget_validate(RtcomWidget *widget, GError **error)
{
  RtcomWidgetIface *iface;
  GtkWidget *child;
  GtkWidget *parent;

  g_return_val_if_fail(RTCOM_IS_WIDGET(widget), FALSE);

  iface = RTCOM_WIDGET_GET_IFACE(widget);

  if (!iface->validate || iface->validate(widget, error))
    return TRUE;

  child = GTK_WIDGET(widget);

  do
  {
    if (RTCOM_IS_PAGE(child))
      break;

    parent = gtk_widget_get_parent(child);

    if (GTK_IS_NOTEBOOK(parent))
    {
      gtk_notebook_set_current_page(
        GTK_NOTEBOOK(parent),
        gtk_notebook_page_num(GTK_NOTEBOOK(parent), child));
    }

    child = parent;
  }
  while (parent);

  rtcom_widget_focus_error(widget);

  return FALSE;
}

void
rtcom_widget_set_account(RtcomWidget *widget, RtcomAccountItem *account)
{
  RtcomWidgetPrivate *priv;

  g_return_if_fail(RTCOM_IS_WIDGET(widget));
  g_return_if_fail(RTCOM_IS_ACCOUNT_ITEM(account));

  priv = g_object_get_data(G_OBJECT(widget), "rtcom");

  if (priv->account != account)
  {
    RtcomWidgetIface *iface = RTCOM_WIDGET_GET_IFACE(widget);

    if (priv->account)
    {
      g_signal_handlers_disconnect_matched(
        priv->account, G_SIGNAL_MATCH_DATA | G_SIGNAL_MATCH_FUNC, 0, 0, NULL,
        iface->store_settings, widget);
      g_object_unref(priv->account);
    }

    priv->account = g_object_ref(account);

    if (iface->set_account)
      iface->set_account(widget, account);

    if (account->account)
      iface->get_settings(widget, account);

    g_signal_connect_swapped(account, "store-settings",
                             G_CALLBACK(iface->store_settings), widget);
  }
}

RtcomAccountItem *
rtcom_widget_get_account(RtcomWidget *widget)
{
  RtcomWidgetPrivate *priv;

  g_return_val_if_fail(RTCOM_IS_WIDGET(widget), NULL);

  priv = g_object_get_data(G_OBJECT(widget), "rtcom");

  return priv->account;
}

void
rtcom_widget_set_msg_next(RtcomWidget *widget, const gchar *message)
{
  RtcomWidgetPrivate *priv;

  g_return_if_fail(RTCOM_IS_WIDGET(widget));

  priv = g_object_get_data(G_OBJECT(widget), "rtcom");
  g_free(priv->msg_next);
  priv->msg_next = g_strdup(message);
}

const gchar *
rtcom_widget_get_msg_next(RtcomWidget *widget)
{
  RtcomWidgetPrivate *priv;

  g_return_val_if_fail(RTCOM_IS_WIDGET(widget), NULL);

  priv = g_object_get_data(G_OBJECT(widget), "rtcom");

  return priv->msg_next;
}

void
rtcom_widget_set_error_widget(RtcomWidget *widget, GtkWidget *error_widget)
{
  RtcomWidgetPrivate *priv;

  g_return_if_fail(RTCOM_IS_WIDGET(widget));

  priv = g_object_get_data(G_OBJECT(widget), "rtcom");
  priv->error_widget = error_widget;
}

void
rtcom_widget_focus_error(RtcomWidget *widget)
{
  RtcomWidgetPrivate *priv;
  GtkWidget *error_widget;

  g_return_if_fail(RTCOM_IS_WIDGET(widget));

  priv = g_object_get_data(G_OBJECT(widget), "rtcom");

  error_widget = priv->error_widget;

  if (!error_widget)
    error_widget = GTK_WIDGET(widget);

  gtk_widget_grab_focus(error_widget);
}

gboolean
rtcom_widget_get_value(RtcomWidget *widget, GValue *value)
{
  RtcomWidgetIface *iface;

  g_return_val_if_fail(RTCOM_IS_WIDGET(widget), FALSE);

  iface = RTCOM_WIDGET_GET_IFACE(widget);

  if (iface->get_value)
    return iface->get_value(widget, value);

  g_warning("RtcomWidget::get_value not implemented for `%s'",
            g_type_name(G_TYPE_FROM_INSTANCE(widget)));

  return FALSE;
}

void
rtcom_widget_value_changed(RtcomWidget *widget)
{
  RtcomWidgetPrivate *priv;

  g_return_if_fail(RTCOM_IS_WIDGET(widget));

  priv = g_object_get_data(G_OBJECT(widget), "rtcom");

  if ( priv->account && priv->name_change)
    rtcom_account_item_name_change(priv->account);
}
