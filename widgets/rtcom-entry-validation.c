/*
 * rtcom-entry-validation.c
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
#include <libaccounts/account-error.h>

#include "rtcom-entry-validation.h"

G_DEFINE_TYPE(
  RtcomEntryValidation,
  rtcom_entry_validation,
  G_TYPE_OBJECT
);

enum
{
  PROP_INVALID_CHARS_RE = 1,
  PROP_MIN_LENGTH,
  PROP_MAX_LENGTH,
  PROP_MIN_LENGTH_MSG,
  PROP_MAX_LENGTH_MSG
};

static void
rtcom_entry_validation_set_property(GObject *object, guint property_id,
                                    const GValue *value, GParamSpec *pspec)
{
  RtcomEntryValidation *validation = RTCOM_ENTRY_VALIDATION(object);

  switch (property_id)
  {
    case PROP_INVALID_CHARS_RE:
    {
      rtcom_entry_validation_set_invalid_chars_re(validation,
                                                  g_value_get_string(value));
      break;
    }
    case PROP_MIN_LENGTH:
    {
      rtcom_entry_validation_set_min_length(validation,
                                            g_value_get_uint(value));
      break;
    }
    case PROP_MAX_LENGTH:
    {
      rtcom_entry_validation_set_max_length(validation,
                                            g_value_get_uint(value));
      break;
    }
    case PROP_MIN_LENGTH_MSG:
    {
      rtcom_entry_validation_set_min_length_msg(validation,
                                                g_value_get_string(value));
      break;
    }
    case PROP_MAX_LENGTH_MSG:
    {
      rtcom_entry_validation_set_max_length_msg(validation,
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
rtcom_entry_validation_get_property(GObject *object, guint property_id,
                                    GValue *value, GParamSpec *pspec)
{
  RtcomEntryValidation *validation = RTCOM_ENTRY_VALIDATION(object);

  switch (property_id)
  {
    case PROP_INVALID_CHARS_RE:
    {
      g_value_set_string(
        value, rtcom_entry_validation_get_invalid_chars_re(validation));
      break;
    }
    case PROP_MIN_LENGTH:
    {
      g_value_set_uint(
        value, rtcom_entry_validation_get_min_length(validation));
      break;
    }
    case PROP_MAX_LENGTH:
    {
      g_value_set_uint(
        value, rtcom_entry_validation_get_max_length(validation));
      break;
    }
    case PROP_MIN_LENGTH_MSG:
    {
      g_value_set_string(
        value, rtcom_entry_validation_get_min_length_msg(validation));
      break;
    }
    case PROP_MAX_LENGTH_MSG:
    {
      g_value_set_string(
        value, rtcom_entry_validation_get_max_length_msg(validation));
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
rtcom_entry_validation_dispose(GObject *object)
{
  RtcomEntryValidation *validation = RTCOM_ENTRY_VALIDATION(object);

  if (validation->validation_re)
  {
    g_regex_unref(validation->validation_re);
    validation->validation_re = NULL;
  }

  if (validation->min_length_msg)
  {
    g_free(validation->min_length_msg);
    validation->min_length_msg = NULL;
  }

  if (validation->max_length_msg)
  {
    g_free(validation->max_length_msg);
    validation->max_length_msg = NULL;
  }

  G_OBJECT_CLASS(rtcom_entry_validation_parent_class)->dispose(object);
}

static void
rtcom_entry_validation_class_init(RtcomEntryValidationClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS(klass);

  object_class->set_property = rtcom_entry_validation_set_property;
  object_class->get_property = rtcom_entry_validation_get_property;
  object_class->dispose = rtcom_entry_validation_dispose;

  g_object_class_install_property(
    object_class, PROP_INVALID_CHARS_RE,
    g_param_spec_string(
      "invalid-chars-re",
      "Validation RE",
      "Regular expression to detect invalid characters",
      NULL,
      GTK_PARAM_READWRITE));
  g_object_class_install_property(
    object_class, PROP_MIN_LENGTH,
    g_param_spec_uint(
      "min-length",
      "Minimal length",
      "Minimal text length",
      0,
      G_MAXUINT32,
      0,
      GTK_PARAM_READWRITE | G_PARAM_CONSTRUCT));
  g_object_class_install_property(
    object_class, PROP_MAX_LENGTH,
    g_param_spec_uint(
      "max-length",
      "Maximal length",
      "Maximal text length",
      0,
      G_MAXUINT32,
      G_MAXUINT32,
      GTK_PARAM_READWRITE | G_PARAM_CONSTRUCT));
  g_object_class_install_property(
    object_class, PROP_MIN_LENGTH_MSG,
    g_param_spec_string(
      "min-length-msg",
      "Minimal length warning",
      "Minimal text length warning",
      NULL,
      GTK_PARAM_READWRITE));
  g_object_class_install_property(
    object_class, PROP_MAX_LENGTH_MSG,
    g_param_spec_string(
      "max-length-msg",
      "Maximal length warning",
      "Maximal text length warning",
      0,
      GTK_PARAM_READWRITE));
}

static void
rtcom_entry_validation_init(RtcomEntryValidation *validation)
{}

void
rtcom_entry_validation_set_min_length(RtcomEntryValidation *self, guint length)
{
  self->min_length = length;
}

guint
rtcom_entry_validation_get_min_length(RtcomEntryValidation *self)
{
  return self->min_length;
}

void
rtcom_entry_validation_set_invalid_chars_re(RtcomEntryValidation *self,
                                            const gchar *pattern)
{
  GError *error = NULL;

  g_return_if_fail(pattern);

  self->validation_re = self->validation_re;

  if (self->validation_re)
    g_regex_unref(self->validation_re);

  self->validation_re = g_regex_new(pattern, 0, 0, &error);

  if (!self->validation_re)
  {
    g_warning("%s: failed to create regex %s", __FUNCTION__, error->message);
    g_error_free(error);
  }
}

const gchar *
rtcom_entry_validation_get_invalid_chars_re(RtcomEntryValidation *self)
{
  return g_regex_get_pattern(self->validation_re);
}

void
rtcom_entry_validation_set_max_length(RtcomEntryValidation *self, guint length)
{
  self->max_length = length;
}

guint
rtcom_entry_validation_get_max_length(RtcomEntryValidation *self)
{
  return self->max_length;
}

void
rtcom_entry_validation_set_min_length_msg(RtcomEntryValidation *self,
                                          const gchar *msg)
{
  if (self->min_length_msg)
    g_free(self->min_length_msg);

  self->min_length_msg = g_strdup(msg);
}

const gchar *
rtcom_entry_validation_get_min_length_msg(RtcomEntryValidation *self)
{
  return self->max_length_msg;
}

void
rtcom_entry_validation_set_max_length_msg(RtcomEntryValidation *self,
                                          const gchar *msg)
{
  if (self->max_length_msg)
    g_free(self->max_length_msg);

  self->max_length_msg = g_strdup(msg);
}

const gchar *
rtcom_entry_validation_get_max_length_msg(RtcomEntryValidation *self)
{
  return self->min_length_msg;
}

gboolean
rtcom_entry_validation_validate(RtcomEntryValidation *self,
                                const gchar *entry_text,
                                const gchar *msg_illegal, GError **error)
{
  glong len;
  gint start_pos = -1;
  gint end_pos;
  GMatchInfo *match_info;

  if (entry_text)
    len = g_utf8_strlen(entry_text, -1);
  else
    len = 0;

  if ((len < self->min_length) || !entry_text)
  {
    if (self->min_length_msg)
    {
      g_set_error(error, ACCOUNT_ERROR, ACCOUNT_ERROR_INVALID_VALUE,
                  "%s", self->min_length_msg);
      return FALSE;
    }

    if (entry_text)
    {
      g_warning("%s: min-length set without min-lenght-msg", __FUNCTION__);
      return FALSE;
    }

    return TRUE;
  }

  if (len > self->max_length)
  {
    if (self->max_length_msg)
    {
      g_set_error(error, ACCOUNT_ERROR, ACCOUNT_ERROR_INVALID_VALUE,
                  "%s", self->max_length_msg);
      return FALSE;
    }

    g_warning("%s: max-length set without max-lenght-msg", __FUNCTION__);
    return FALSE;
  }

  if (!self->validation_re)
    return TRUE;

  if (g_regex_match(self->validation_re, entry_text, 0, &match_info) &&
      g_match_info_fetch_pos(match_info, 0, &start_pos, &end_pos))
  {
    gchar buf[6 + 1];
    gunichar uch = g_utf8_get_char(&entry_text[start_pos]);
    gint bytes = g_unichar_to_utf8(uch, buf);
    const char *msgid;
    const gchar *fmt;

    if (g_unichar_isspace(uch) || g_unichar_iscntrl(uch))
      msgid = "accountwizard_ib_illegal_character_space";
    else if (msg_illegal)
      msgid = msg_illegal;
    else
      msgid = "accountwizard_ib_illegal_character";

    buf[bytes] = 0;
    fmt = _(msgid);
    g_set_error(
      error, ACCOUNT_ERROR, ACCOUNT_ERROR_INVALID_CHARACTER, fmt, buf);
  }

  g_match_info_free(match_info);

  return start_pos == -1;
}
