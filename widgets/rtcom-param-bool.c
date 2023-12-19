/*
 * rtcom-param-bool.c
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

#include "rtcom-param-bool.h"

RTCOM_DEFINE_WIDGET_TYPE(
  RtcomParamBool,
  rtcom_param_bool,
  HILDON_TYPE_CHECK_BUTTON
);

enum
{
  PROP_FIELD = 1
};

static void
rtcom_param_bool_finalize(GObject *object)
{
  g_free(RTCOM_PARAM_BOOL(object)->field);

  G_OBJECT_CLASS(rtcom_param_bool_parent_class)->finalize(object);
}

static void
rtcom_param_bool_set_property(GObject *object, guint property_id,
                              const GValue *value, GParamSpec *pspec)
{
  switch (property_id)
  {
    case PROP_FIELD:
    {
      RTCOM_PARAM_BOOL(object)->field = g_value_dup_string(value);
      break;
    }
    default:
    {
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
      break;
    }
  }
}

static void
rtcom_param_bool_get_property(GObject *object, guint property_id, GValue *value,
                              GParamSpec *pspec)
{
  switch (property_id)
  {
    case PROP_FIELD:
    {
      g_value_set_string(value, RTCOM_PARAM_BOOL(object)->field);
      break;
    }
    default:
    {
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
      break;
    }
  }
}

static GObject *
rtcom_param_bool_constructor(GType type, guint n_construct_properties,
                             GObjectConstructParam *construct_properties)
{
  GObject *object = G_OBJECT_CLASS(rtcom_param_bool_parent_class)->
    constructor(type, n_construct_properties, construct_properties);

  g_object_set(object, "size", HILDON_SIZE_FINGER_HEIGHT, NULL);
  gtk_widget_set_name(GTK_WIDGET(object), "GtkButton-finger");

  return object;
}

static void
rtcom_param_bool_toggled(HildonCheckButton *button)
{
  HildonCheckButtonClass *parent_class =
    HILDON_CHECK_BUTTON_CLASS(rtcom_param_bool_parent_class);

  rtcom_widget_value_changed(RTCOM_WIDGET(button));

  if (parent_class->toggled)
    parent_class->toggled(button);
}

static void
rtcom_param_bool_class_init(RtcomParamBoolClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS(klass);

  object_class->finalize = rtcom_param_bool_finalize;
  object_class->set_property = rtcom_param_bool_set_property;
  object_class->get_property = rtcom_param_bool_get_property;
  object_class->constructor = rtcom_param_bool_constructor;

  HILDON_CHECK_BUTTON_CLASS(klass)->toggled = rtcom_param_bool_toggled;

  g_object_class_install_property(
    object_class, PROP_FIELD,
    g_param_spec_string(
      "field",
      "Field name",
      "TpAccount field name",
      NULL,
      G_PARAM_WRITABLE | G_PARAM_READABLE));
}

static void
rtcom_param_bool_init(RtcomParamBool *self)
{}

static gboolean
rtcom_param_bool_store_settings(RtcomWidget *widget, GError **error,
                                RtcomAccountItem *item)
{
  RtcomParamBool *self = RTCOM_PARAM_BOOL(widget);

  if (gtk_widget_get_sensitive(GTK_WIDGET(self)))
  {
    rtcom_account_item_store_param_boolean(
      item, self->field,
      hildon_check_button_get_active(HILDON_CHECK_BUTTON(self)));
  }
  else
    rtcom_account_item_unset_param(item, self->field);

  return TRUE;
}

static void
rtcom_param_bool_get_settings(RtcomWidget *widget, RtcomAccountItem *item)
{
  RtcomParamBool *self = RTCOM_PARAM_BOOL(widget);
  GHashTable *parameters;
  const GValue *v;

  if (!self->field)
    return;

  parameters = (GHashTable *)tp_account_get_parameters(item->account);

  if (!parameters)
    return;

  v = g_hash_table_lookup(parameters, self->field);

  if (v)
  {
    hildon_check_button_set_active(&self->parent_instance,
                                   g_value_get_boolean(v));
  }
}

static void
rtcom_param_bool_set_account(RtcomWidget *widget, RtcomAccountItem *account)
{
  RtcomParamBool *self = RTCOM_PARAM_BOOL(widget);
  TpProtocol *protocol = rtcom_account_item_get_tp_protocol(account);

  if (self->field)
  {
    const TpConnectionManagerParam *param;
    GValue v = G_VALUE_INIT;

    param = tp_protocol_get_param(protocol, self->field);

    if (param && tp_connection_manager_param_get_default(param, &v))
    {
      if (G_VALUE_HOLDS_STRING(&v))
      {
        const gchar *s = g_value_get_string(&v);

        if (s && *s)
        {
          if (!strcmp(s, "true") || !strcmp(s, "1"))
            hildon_check_button_set_active(HILDON_CHECK_BUTTON(self), TRUE);
          else if (!strcmp(s, "false") || !strcmp(s, "0"))
            hildon_check_button_set_active(HILDON_CHECK_BUTTON(self), FALSE);
          else
          {
            g_warning("%s: Unrecognized value for boolean param: %s",
                      __FUNCTION__, s);
          }
        }
      }
      else if (G_VALUE_HOLDS_BOOLEAN(&v))
        hildon_check_button_set_active(HILDON_CHECK_BUTTON(self),
                                       g_value_get_boolean(&v));
    }
  }

  g_object_unref(protocol);
}

static void
rtcom_widget_init(RtcomWidgetIface *iface)
{
  iface->store_settings = rtcom_param_bool_store_settings;
  iface->get_settings = rtcom_param_bool_get_settings;
  iface->set_account = rtcom_param_bool_set_account;
}

const gchar *
rtcom_param_bool_get_field(RtcomParamBool *param)
{
  return param->field;
}
