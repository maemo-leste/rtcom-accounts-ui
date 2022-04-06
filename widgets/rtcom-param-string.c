/*
 * rtcom-param-string.c
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

#include <gtk/gtkprivate.h>
#include <hildon/hildon.h>

#include "rtcom-param-string.h"

static void
rtcom_widget_init(RtcomWidgetIface *iface);

RTCOM_DEFINE_WIDGET_TYPE(
  RtcomParamString,
  rtcom_param_string,
  HILDON_TYPE_ENTRY
);

enum
{
  PROP_FIELD = 1,
  PROP_INVALID_CHARS_RE,
  PROP_MSG_EMPTY,
  PROP_MSG_ILLEGAL,
  PROP_V_MIN_LENGTH,
  PROP_V_MIN_LENGTH_MSG,
  PROP_V_MAX_LENGTH,
  PROP_V_MAX_LENGTH_MSG
};

static gboolean
rtcom_param_string_store_settings(RtcomWidget *widget, GError **error,
                                  RtcomAccountItem *item)
{
  RtcomParamString *string = RTCOM_PARAM_STRING(widget);
  const gchar *text;

  if (!string->field)
    return FALSE;

  if (!gtk_widget_get_sensitive(GTK_WIDGET(string)))
  {
    rtcom_account_item_unset_param(item, string->field);
    return FALSE;
  }

  text = gtk_entry_get_text(GTK_ENTRY(string));

  if (!text || !*text)
    return FALSE;

  rtcom_account_item_store_param_string(item, string->field, text);

  return TRUE;
}

static gboolean
rtcom_param_string_validate(RtcomWidget *widget, GError **error)
{
  RtcomParamString *string = RTCOM_PARAM_STRING(widget);
  const gchar *text = gtk_entry_get_text(GTK_ENTRY(widget));

  return rtcom_entry_validation_validate(string->validation, text,
                                         string->msg_illegal, error);
}

static void
rtcom_param_string_set_account(RtcomWidget *widget, RtcomAccountItem *account)
{
  RtcomParamString *string = RTCOM_PARAM_STRING(widget);
  TpProtocol *protocol = rtcom_account_item_get_tp_protocol(account);

  if (!protocol)
    return;

  if (string->field)
  {
    const TpConnectionManagerParam *param;
    GValue v;

    param = tp_protocol_get_param(protocol, string->field);

    if (param &&
        tp_connection_manager_param_get_default(param, &v) &&
        G_VALUE_HOLDS_STRING(&v))
    {
      gtk_entry_set_text(GTK_ENTRY(string), g_value_get_string(&v));
    }
  }

  g_object_unref(protocol);
}

static void
rtcom_param_string_get_settings(RtcomWidget *widget, RtcomAccountItem *item)
{
  RtcomParamString *string = RTCOM_PARAM_STRING(widget);
  GHashTable *parameters;

  if (!string->field)
    return;

  parameters = (GHashTable *)tp_account_get_parameters(item->account);

  if (parameters)
  {
    const GValue *v = g_hash_table_lookup(parameters, string->field);

    if (v && G_VALUE_HOLDS_STRING(v))
      gtk_entry_set_text(GTK_ENTRY(string), g_value_get_string(v));
  }
}

static void
rtcom_widget_init(RtcomWidgetIface *iface)
{
  iface->store_settings = rtcom_param_string_store_settings;
  iface->validate = rtcom_param_string_validate;
  iface->set_account = rtcom_param_string_set_account;
  iface->get_settings = rtcom_param_string_get_settings;
}

static void
rtcom_param_string_dispose(GObject *object)
{
  RtcomParamString *string = RTCOM_PARAM_STRING(object);

  if (string->validation)
  {
    g_object_unref(string->validation);
    string->validation = NULL;
  }

  G_OBJECT_CLASS(rtcom_param_string_parent_class)->dispose(object);
}

static void
rtcom_param_string_finalize(GObject *object)
{
  RtcomParamString *string = RTCOM_PARAM_STRING(object);

  g_free(string->field);
  g_free(string->msg_empty);
  g_free(string->msg_illegal);

  G_OBJECT_CLASS(rtcom_param_string_parent_class)->finalize(object);
}

static void
rtcom_param_string_set_property(GObject *object, guint property_id,
                                const GValue *value, GParamSpec *pspec)
{
  RtcomParamString *string = RTCOM_PARAM_STRING(object);

  g_return_if_fail(RTCOM_IS_PARAM_STRING(object));

  switch (property_id)
  {
    case PROP_FIELD:
    {
      g_free(string->field);
      string->field = g_value_dup_string(value);
      break;
    }
    case PROP_INVALID_CHARS_RE:
    {
      rtcom_entry_validation_set_invalid_chars_re(string->validation,
                                                  g_value_get_string(value));
      break;
    }
    case PROP_MSG_EMPTY:
    {
      g_free(string->msg_empty);
      string->msg_empty = g_value_dup_string(value);

      if (!string->filled)
      {
        if (string->msg_empty)
          rtcom_widget_set_msg_next(RTCOM_WIDGET(string), string->msg_empty);
      }

      break;
    }
    case PROP_MSG_ILLEGAL:
    {
      g_free(string->msg_illegal);
      string->msg_illegal = g_value_dup_string(value);
      break;
    }
    case PROP_V_MIN_LENGTH:
    {
      rtcom_entry_validation_set_min_length(string->validation,
                                            g_value_get_uint(value));
      break;
    }
    case PROP_V_MIN_LENGTH_MSG:
    {
      rtcom_entry_validation_set_min_length_msg(string->validation,
                                                g_value_get_string(value));
      break;
    }
    case PROP_V_MAX_LENGTH:
    {
      rtcom_entry_validation_set_max_length(string->validation,
                                            g_value_get_uint(value));
      break;
    }
    case PROP_V_MAX_LENGTH_MSG:
    {
      rtcom_entry_validation_set_max_length_msg(string->validation,
                                                g_value_get_string(value));
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
rtcom_param_string_get_property(GObject *object, guint property_id,
                                GValue *value, GParamSpec *pspec)
{
  RtcomParamString *string = RTCOM_PARAM_STRING(object);

  g_return_if_fail(RTCOM_IS_PARAM_STRING(object));

  switch (property_id)
  {
    case PROP_FIELD:
    {
      g_value_set_string(value, string->field);
      break;
    }
    case PROP_INVALID_CHARS_RE:
    {
      g_value_set_string(
        value,
        rtcom_entry_validation_get_invalid_chars_re(string->validation));
      break;
    }
    case PROP_MSG_EMPTY:
    {
      g_value_set_string(value, string->msg_empty);
      break;
    }
    case PROP_MSG_ILLEGAL:
    {
      g_value_set_string(value, string->msg_illegal);
      break;
    }
    case PROP_V_MIN_LENGTH:
    {
      g_value_set_uint(
        value, rtcom_entry_validation_get_min_length(string->validation));
      break;
    }
    case PROP_V_MIN_LENGTH_MSG:
    {
      g_value_set_string(
        value,
        rtcom_entry_validation_get_min_length_msg(string->validation));
      break;
    }
    case PROP_V_MAX_LENGTH:
    {
      g_value_set_uint(
        value, rtcom_entry_validation_get_max_length(string->validation));
      break;
    }
    case PROP_V_MAX_LENGTH_MSG:
    {
      g_value_set_string(
        value,
        rtcom_entry_validation_get_max_length_msg(string->validation));
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
rtcom_param_string_class_init(RtcomParamStringClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS(klass);

  object_class->dispose = rtcom_param_string_dispose;
  object_class->finalize = rtcom_param_string_finalize;
  object_class->set_property = rtcom_param_string_set_property;
  object_class->get_property = rtcom_param_string_get_property;

  g_object_class_install_property(
    object_class, PROP_FIELD,
    g_param_spec_string(
      "field",
      "Field name",
      "TpAccount field name",
      NULL,
      G_PARAM_READWRITE));
  g_object_class_install_property(
    object_class, PROP_INVALID_CHARS_RE,
    g_param_spec_string(
      "invalid-chars-re",
      "Invalid chars regex",
      "Regular expression to find forbidden characters",
      NULL,
      G_PARAM_READWRITE));
  g_object_class_install_property(
    object_class, PROP_MSG_EMPTY,
    g_param_spec_string(
      "msg-empty",
      "Message if empty",
      "Message when empty",
      NULL,
      G_PARAM_READWRITE));
  g_object_class_install_property(
    object_class, PROP_MSG_ILLEGAL,
    g_param_spec_string(
      "msg-illegal",
      "",
      "",
      NULL,
      G_PARAM_READWRITE));
  g_object_class_install_property(
    object_class, PROP_V_MIN_LENGTH,
    g_param_spec_uint(
      "v-min-length",
      "Minimal length",
      "Minimal text length for validation",
      0,
      G_MAXUINT32,
      0,
      GTK_PARAM_WRITABLE));
  g_object_class_install_property(
    object_class, PROP_V_MAX_LENGTH,
    g_param_spec_uint(
      "v-max-length",
      "Maximal length",
      "Maximal text length for validation",
      0,
      G_MAXUINT32,
      G_MAXUINT32,
      GTK_PARAM_WRITABLE));
  g_object_class_install_property(
    object_class, PROP_V_MIN_LENGTH_MSG,
    g_param_spec_string(
      "v-min-length-msg",
      "Minimal length warning",
      "Minimal text length warning for validation",
      NULL,
      GTK_PARAM_WRITABLE));
  g_object_class_install_property(
    object_class, PROP_V_MAX_LENGTH_MSG,
    g_param_spec_string(
      "v-max-length-msg",
      "Maximal length warning",
      "Maximal text length warning for validation",
      NULL,
      GTK_PARAM_WRITABLE));
}

static void
_changed(RtcomParamString *string)
{
  GtkWidget *area;
  const gchar *text;

  area = gtk_widget_get_ancestor(GTK_WIDGET(string),
                                 HILDON_TYPE_PANNABLE_AREA);

  if (area && gtk_widget_get_realized(area))
  {
    hildon_pannable_area_jump_to_child(HILDON_PANNABLE_AREA(area),
                                       GTK_WIDGET(string));
  }

  text = gtk_entry_get_text(&string->parent_instance.parent);

  if (text && *text)
  {
    if (!string->filled)
    {
      string->filled = TRUE;
      g_object_set(string, "can-next", TRUE, NULL);
    }
  }
  else
  {
    gboolean required;

    g_object_get(string, "required", &required, NULL);

    if (required)
    {
      g_object_set(string, "can-next", FALSE, NULL);
      string->filled = FALSE;

      if (string->msg_empty)
        rtcom_widget_set_msg_next(RTCOM_WIDGET(string), string->msg_empty);
    }
  }

  rtcom_widget_value_changed(RTCOM_WIDGET(string));
}

static void
rtcom_param_string_init(RtcomParamString *string)
{
  g_signal_connect(string, "changed", G_CALLBACK(_changed), NULL);
  string->validation = g_object_new(RTCOM_TYPE_ENTRY_VALIDATION, NULL);
}

const gchar *
rtcom_param_string_get_field(RtcomParamString *param)
{
  return param->field;
}
