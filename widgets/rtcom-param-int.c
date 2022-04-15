/*
 * rtcom-param-int.c
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

#include "rtcom-param-int.h"

RTCOM_DEFINE_WIDGET_TYPE(
  RtcomParamInt,
  rtcom_param_int,
  HILDON_TYPE_ENTRY
);

enum
{
  PROP_FIELD = 1,
  PROP_RANGE
};

static void
rtcom_param_int_finalize(GObject *object)
{
  RtcomParamInt *self = RTCOM_PARAM_INT(object);

  g_free(self->field);

  G_OBJECT_CLASS(rtcom_param_int_parent_class)->finalize(object);
}

static void
rtcom_param_int_set_property(GObject *object, guint property_id,
                             const GValue *value, GParamSpec *pspec)
{
  RtcomParamInt *self = RTCOM_PARAM_INT(object);

  switch (property_id)
  {
    case PROP_FIELD:
    {
      self->field = g_value_dup_string(value);
      break;
    }
    case PROP_RANGE:
    {
      const gchar *range = g_value_get_string(value);
      gint current_value;
      gint new_value;
      gchar *endptr;

      if (range)
      {
        self->min = strtol(range, &endptr, 10);
        self->min = self->min;

        if (endptr && *endptr && (range != endptr))
          self->max = strtol(endptr + 1, NULL, 10);
      }

      current_value = rtcom_param_int_get_value(self);
      new_value = self->max;

      if (current_value <= new_value)
      {
        if (current_value < self->min)
          new_value = self->min;
        else
          new_value = current_value;
      }

      rtcom_param_int_set_value(self, new_value);
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
rtcom_param_int_get_property(GObject *object, guint property_id, GValue *value,
                             GParamSpec *pspec)
{
  RtcomParamInt *self = RTCOM_PARAM_INT(object);

  switch (property_id)
  {
    case PROP_FIELD:
    {
      g_value_set_string(value, self->field);
      break;
    }
    default:
    {
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
      break;
    }
  }
}

GObject *
rtcom_param_int_constructor(GType type, guint n_construct_properties,
                            GObjectConstructParam *construct_properties)
{
  GObject *object = G_OBJECT_CLASS(rtcom_param_int_parent_class)->constructor(
      type, n_construct_properties, construct_properties);

  hildon_gtk_entry_set_input_mode(GTK_ENTRY(object),
                                  HILDON_GTK_INPUT_MODE_NUMERIC);

  return object;
}

static gboolean
rtcom_param_int_focus_out_event(GtkWidget *widget, GdkEventFocus *event)
{
  RtcomParamInt *self = RTCOM_PARAM_INT(widget);
  gint value;

  GTK_WIDGET_CLASS(rtcom_param_int_parent_class)->
  focus_out_event(widget, event);

  value = rtcom_param_int_get_value(self);

  if ((self->min || self->max) && (value != G_MININT) &&
      ((value < self->min) || (value > self->max)))
  {
    hildon_banner_show_information(gtk_widget_get_toplevel(widget),
                                   NULL, _("accounts_fi_port_out_of_range"));
    gtk_widget_grab_focus(widget);
  }

  return FALSE;
}

static void
rtcom_param_int_class_init(RtcomParamIntClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS(klass);

  object_class->finalize = rtcom_param_int_finalize;
  object_class->set_property = rtcom_param_int_set_property;
  object_class->get_property = rtcom_param_int_get_property;
  object_class->constructor = rtcom_param_int_constructor;

  GTK_WIDGET_CLASS(klass)->focus_out_event = rtcom_param_int_focus_out_event;

  g_object_class_install_property(
    object_class, PROP_FIELD,
    g_param_spec_string(
      "field",
      "Field name",
      "TpAccount field name",
      NULL,
      G_PARAM_READWRITE));
  g_object_class_install_property(
    object_class, PROP_RANGE,
    g_param_spec_string(
      "range",
      "Range",
      "Valid range",
      NULL,
      G_PARAM_WRITABLE));
}

static void
rtcom_param_int_init(RtcomParamInt *self)
{
  g_signal_connect(self, "notify::value",
                   G_CALLBACK(rtcom_widget_value_changed), NULL);
}

static gboolean
rtcom_param_int_store_settings(RtcomWidget *widget, GError **error,
                               RtcomAccountItem *item)
{
  RtcomParamInt *self = RTCOM_PARAM_INT(widget);
  gint value;
  RtcomAccountService *service;

  if (!gtk_widget_get_sensitive(GTK_WIDGET(self)))
  {
    rtcom_account_item_unset_param(item, self->field);
    return TRUE;
  }

  value = rtcom_param_int_get_value(self);

  if (value == G_MININT)
  {
    rtcom_account_item_unset_param(item, self->field);
    return TRUE;
  }

  service = RTCOM_ACCOUNT_SERVICE(account_item_get_service(ACCOUNT_ITEM(item)));

  if (rtcom_account_service_get_param_type(service, self->field) == G_TYPE_INT)
    rtcom_account_item_store_param_int(item, self->field, value);
  else
    rtcom_account_item_store_param_uint(item, self->field, value);

  return TRUE;
}

static gboolean
rtcom_param_int_validate(RtcomWidget *widget, GError **error)
{
  RtcomParamInt *self = RTCOM_PARAM_INT(widget);

  gint value;

  if (!self->min && !self->max)
    return TRUE;

  value = rtcom_param_int_get_value(self);

  if ((value == G_MININT) || ((value >= self->min) && (value <= self->max)))
    return TRUE;

  g_set_error(error, ACCOUNT_ERROR, ACCOUNT_ERROR_INVALID_VALUE, "%s",
              _("accounts_fi_port_out_of_range"));

  return FALSE;
}

static void
rtcom_param_int_account(RtcomWidget *widget, RtcomAccountItem *account)
{
  RtcomParamInt *self = RTCOM_PARAM_INT(widget);
  TpProtocol *protocol = rtcom_account_item_get_tp_protocol(account);

  if (protocol)
  {
    const TpConnectionManagerParam *param;
    GValue v = G_VALUE_INIT;

    param = tp_protocol_get_param(protocol, self->field);

    if (param && tp_connection_manager_param_get_default(param, &v))
    {
      gchar *text = NULL;

      if (G_VALUE_HOLDS_STRING(&v))
        text = g_strdup(g_value_get_string(&v));
      else if (G_VALUE_HOLDS_INT(&v))
        text = g_strdup_printf("%d", g_value_get_int(&v));
      else if (G_VALUE_HOLDS_UINT(&v))
        text = g_strdup_printf("%u", g_value_get_uint(&v));
      else
        g_warn_if_reached();

      hildon_entry_set_text(HILDON_ENTRY(self), text);

      g_free(text);
    }

    g_object_unref(protocol);
  }
}

static void
rtcom_param_int_settings(RtcomWidget *widget, RtcomAccountItem *item)
{
  RtcomParamInt *self = RTCOM_PARAM_INT(widget);
  GHashTable *parameters;
  const GValue *value;

  if (!self->field)
    return;

  parameters = (GHashTable *)tp_account_get_parameters(item->account);

  if (parameters && (value = g_hash_table_lookup(parameters, self->field)))
  {
    gchar *text;

    if (G_VALUE_HOLDS_INT(value))
      text = g_strdup_printf("%d", g_value_get_int(value));
    else
      text = g_strdup_printf("%u", g_value_get_uint(value));

    hildon_entry_set_text(HILDON_ENTRY(self), text);
    g_free(text);
  }
  else
    hildon_entry_set_text(HILDON_ENTRY(self), "");
}

static void
rtcom_widget_init(RtcomWidgetIface *iface)
{
  iface->store_settings = rtcom_param_int_store_settings;
  iface->validate = rtcom_param_int_validate;
  iface->set_account = rtcom_param_int_account;
  iface->get_settings = rtcom_param_int_settings;
}

const gchar *
rtcom_param_int_get_field(RtcomParamInt *param)
{
  return param->field;
}

gint
rtcom_param_int_get_value(RtcomParamInt *param)
{
  const gchar *text;

  g_return_val_if_fail(RTCOM_IS_PARAM_INT(param), G_MININT);

  text = hildon_entry_get_text(&param->parent_instance);

  if (text && *text)
    return strtol(text, NULL, 10);

  return G_MININT;
}

void
rtcom_param_int_set_value(RtcomParamInt *param, gint value)
{
  gchar text[15];

  g_return_if_fail(RTCOM_IS_PARAM_INT(param));

  g_snprintf(text, sizeof(text), "%d", value);
  hildon_entry_set_text(HILDON_ENTRY(param), text);
}
