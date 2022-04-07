/*
 * rtcom-username.c
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

#include "rtcom-username.h"

struct _RtcomUsernamePrivate
{
  gchar *placeholder;
  gchar *msg_empty;
};

typedef struct _RtcomUsernamePrivate RtcomUsernamePrivate;

#define PRIVATE(username) \
  ((RtcomUsernamePrivate *) \
   rtcom_username_get_instance_private((RtcomUsername *)(username)))

RTCOM_DEFINE_WIDGET_TYPE_WITH_PRIVATE(
  RtcomUsername,
  rtcom_username,
  GTK_TYPE_HBOX
);

enum
{
  PROP_FIELD = 1,
  PROP_INVALID_CHARS_RE,
  PROP_PREFILL,
  PROP_PLACEHOLDER,
  PROP_EDITABLE,
  PROP_CHECK_UNIQUENESS,
  PROP_SHOW_SERVER_NAME,
  PROP_V_MIN_LENGTH,
  PROP_V_MIN_LENGTH_MSG,
  PROP_V_MAX_LENGTH,
  PROP_V_MAX_LENGTH_MSG,
  PROP_MUST_HAVE_AT_SEPARATOR,
  PROP_REQUIRED_SERVER,
  PROP_REQUIRED_SERVER_ERROR,
  PROP_MSG_EMPTY
};

enum
{
  FIELD_USERNAME = 1,
  FIELD_SERVER = 2
};

static void
rtcom_username_dispose(GObject *object)
{
  RtcomUsername *self = RTCOM_USERNAME(object);

  if (self->protocol)
  {
    g_object_unref(self->protocol);
    self->protocol = NULL;
  }

  if (self->username_validation)
  {
    g_object_unref(self->username_validation);
    self->username_validation = NULL;
  }

  if (self->server_validation)
  {
    g_object_unref(self->server_validation);
    self->server_validation = NULL;
  }

  G_OBJECT_CLASS(rtcom_username_parent_class)->dispose(object);
}

static void
rtcom_username_finalize(GObject *object)
{
  RtcomUsername *self = RTCOM_USERNAME(object);
  RtcomUsernamePrivate *priv = PRIVATE(self);

  g_free(self->field);
  g_free(self->prefill);
  g_free(self->required_server);
  g_free(self->required_server_error);

  g_free(priv->placeholder);
  g_free(priv->msg_empty);

  G_OBJECT_CLASS(rtcom_username_parent_class)->finalize(object);
}

static void
rtcom_username_set_property(GObject *object, guint property_id,
                            const GValue *value, GParamSpec *pspec)
{
  RtcomUsername *self = RTCOM_USERNAME(object);
  RtcomUsernamePrivate *priv = PRIVATE(self);

  switch (property_id)
  {
    case PROP_FIELD:
    {
      g_free(self->field);
      self->field = g_value_dup_string(value);
      break;
    }
    case PROP_INVALID_CHARS_RE:
    {
      rtcom_entry_validation_set_invalid_chars_re(
        self->username_validation, g_value_get_string(value));
      break;
    }
    case PROP_PREFILL:
    {
      g_free(self->prefill);
      self->prefill = g_value_dup_string(value);
      break;
    }
    case PROP_PLACEHOLDER:
    {
      g_free(priv->placeholder);
      priv->placeholder = g_value_dup_string(value);
      break;
    }
    case PROP_EDITABLE:
    {
      self->editable = g_value_get_boolean(value);
      break;
    }
    case PROP_CHECK_UNIQUENESS:
    {
      self->check_uniqueness = g_value_get_boolean(value);
      break;
    }
    case PROP_SHOW_SERVER_NAME:
    {
      self->show_server_name = g_value_get_boolean(value);
      break;
    }
    case PROP_V_MIN_LENGTH:
    {
      rtcom_entry_validation_set_min_length(
        self->username_validation, g_value_get_uint(value));
      break;
    }
    case PROP_V_MIN_LENGTH_MSG:
    {
      rtcom_entry_validation_set_min_length_msg(
        self->username_validation, g_value_get_string(value));
      break;
    }
    case PROP_V_MAX_LENGTH:
    {
      rtcom_entry_validation_set_max_length(
        self->username_validation, g_value_get_uint(value));
      break;
    }
    case PROP_V_MAX_LENGTH_MSG:
    {
      rtcom_entry_validation_set_max_length_msg(
        self->username_validation, g_value_get_string(value));
      break;
    }
    case PROP_MUST_HAVE_AT_SEPARATOR:
    {
      self->must_have_at_separator = g_value_get_boolean(value);
      break;
    }
    case PROP_REQUIRED_SERVER:
    {
      g_free(self->required_server);
      self->required_server = g_value_dup_string(value);
      break;
    }
    case PROP_REQUIRED_SERVER_ERROR:
    {
      g_free(self->required_server_error);
      self->required_server_error = g_value_dup_string(value);
      break;
    }
    case PROP_MSG_EMPTY:
    {
      g_free(priv->msg_empty);
      priv->msg_empty = g_value_dup_string(value);
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
rtcom_username_get_property(GObject *object,
                            guint property_id,
                            GValue *value,
                            GParamSpec *pspec)
{
  RtcomUsername *self = RTCOM_USERNAME(object);
  RtcomUsernamePrivate *priv = PRIVATE(self);

  switch (property_id)
  {
    case PROP_FIELD:
    {
      g_value_set_string(value, self->field);
      break;
    }
    case PROP_INVALID_CHARS_RE:
    {
      g_value_set_string(value,
                         rtcom_entry_validation_get_invalid_chars_re(
                           self->username_validation));
      break;
    }
    case PROP_PREFILL:
    {
      g_value_set_string(value, self->prefill);
      break;
    }
    case PROP_PLACEHOLDER:
    {
      g_value_set_string(value, priv->placeholder);
      break;
    }
    case PROP_EDITABLE:
    {
      g_value_set_boolean(value, self->editable);
      break;
    }
    case PROP_CHECK_UNIQUENESS:
    {
      g_value_set_boolean(value, self->check_uniqueness);
      break;
    }
    case PROP_SHOW_SERVER_NAME:
    {
      g_value_set_boolean(value, self->show_server_name);
      break;
    }
    case PROP_V_MIN_LENGTH:
    {
      g_value_set_uint(value,
                       rtcom_entry_validation_get_min_length(
                         self->username_validation));
      break;
    }
    case PROP_V_MIN_LENGTH_MSG:
    {
      g_value_set_string(value,
                         rtcom_entry_validation_get_min_length_msg(
                           self->username_validation));
      break;
    }
    case PROP_V_MAX_LENGTH:
    {
      g_value_set_uint(value,
                       rtcom_entry_validation_get_max_length(
                         self->username_validation));
      break;
    }
    case PROP_V_MAX_LENGTH_MSG:
    {
      g_value_set_string(value,
                         rtcom_entry_validation_get_max_length_msg(
                           self->username_validation));
      break;
    }
    case PROP_MUST_HAVE_AT_SEPARATOR:
    {
      g_value_set_boolean(value, self->must_have_at_separator);
      break;
    }
    case PROP_REQUIRED_SERVER:
    {
      g_value_set_string(value, self->required_server);
      break;
    }
    case PROP_REQUIRED_SERVER_ERROR:
    {
      g_value_set_string(value, self->required_server_error);
      break;
    }
    case PROP_MSG_EMPTY:
    {
      g_value_set_string(value, priv->msg_empty);
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
rtcom_username_constructed(GObject *object)
{
  rtcom_widget_set_msg_next(RTCOM_WIDGET(object), PRIVATE(object)->msg_empty);
}

static void
rtcom_username_grab_focus(GtkWidget *widget)
{
  RtcomUsername *self = RTCOM_USERNAME(widget);
  gchar *placeholder = PRIVATE(self)->placeholder;

  if (!placeholder || !*placeholder)
    gtk_widget_grab_focus(self->username_editor);
}

static void
rtcom_username_class_init(RtcomUsernameClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS(klass);

  object_class->dispose = rtcom_username_dispose;
  object_class->finalize = rtcom_username_finalize;
  object_class->constructed = rtcom_username_constructed;
  object_class->set_property = rtcom_username_set_property;
  object_class->get_property = rtcom_username_get_property;

  GTK_WIDGET_CLASS(klass)->grab_focus = rtcom_username_grab_focus;

  g_object_class_install_property(
    object_class,
    PROP_FIELD,
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
    object_class, PROP_PREFILL,
    g_param_spec_string(
      "prefill",
      "Prefill",
      "prefill",
      NULL,
      G_PARAM_READWRITE));
  g_object_class_install_property(
    object_class, PROP_PLACEHOLDER,
    g_param_spec_string(
      "placeholder",
      "Placeholder",
      "placeholder",
      NULL,
      G_PARAM_READWRITE));
  g_object_class_install_property(
    object_class, PROP_EDITABLE,
    g_param_spec_boolean(
      "editable",
      "editable",
      "Whether the entry is editable",
      TRUE,
      G_PARAM_READWRITE));
  g_object_class_install_property(
    object_class, PROP_CHECK_UNIQUENESS,
    g_param_spec_boolean(
      "check-uniqueness",
      "Check uniqueness",
      "Check if the account exists already",
      FALSE,
      G_PARAM_READWRITE));
  g_object_class_install_property(
    object_class, PROP_SHOW_SERVER_NAME,
    g_param_spec_boolean(
      "show-server-name",
      "Show servername",
      "Shows the @ sign and the server name part",
      TRUE,
      G_PARAM_READWRITE));
  g_object_class_install_property(
    object_class, PROP_V_MIN_LENGTH,
    g_param_spec_uint(
      "v-min-length",
      "Minimal length",
      "Minimal text length for validation",
      0, G_MAXUINT32, 0,
      GTK_PARAM_READWRITE));
  g_object_class_install_property(
    object_class, PROP_V_MAX_LENGTH,
    g_param_spec_uint(
      "v-max-length",
      "Maximal length",
      "Maximal text length for validation",
      0, G_MAXUINT32, G_MAXUINT32,
      GTK_PARAM_READWRITE));
  g_object_class_install_property(
    object_class, PROP_V_MIN_LENGTH_MSG,
    g_param_spec_string(
      "v-min-length-msg",
      "Minimal length warning",
      "Minimal text length warning for validation",
      NULL,
      GTK_PARAM_READWRITE));
  g_object_class_install_property(
    object_class, PROP_V_MAX_LENGTH_MSG,
    g_param_spec_string(
      "v-max-length-msg",
      "Maximal length warning",
      "Maximal text length warning for validation",
      NULL,
      GTK_PARAM_READWRITE));
  g_object_class_install_property(
    object_class, PROP_MUST_HAVE_AT_SEPARATOR,
    g_param_spec_boolean(
      "must-have-at-separator",
      "Must have @ sign",
      "Whether the @ sign is mandatory",
      TRUE,
      G_PARAM_READWRITE));
  g_object_class_install_property(
    object_class, PROP_REQUIRED_SERVER,
    g_param_spec_string(
      "required-server",
      "The required server",
      "The required server part in username",
      NULL,
      G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE));
  g_object_class_install_property(
    object_class, PROP_REQUIRED_SERVER_ERROR,
    g_param_spec_string(
      "required-server-error",
      "The required server error",
      "The error for required server part in username",
      NULL,
      G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE));
  g_object_class_install_property(
    object_class,
    PROP_MSG_EMPTY,
    g_param_spec_string(
      "msg-empty",
      "Message empty",
      "Message to be printed if fields are empty",
      _("accounts_fi_enter_fields_first"),
      G_PARAM_CONSTRUCT_ONLY | GTK_PARAM_READWRITE));
}

static void
emptiness_changed(RtcomUsername *self)
{
  if (self->filled_fields == (FIELD_USERNAME | FIELD_SERVER))
    g_object_set(self, "can-next", 1, NULL);
  else
  {
    RtcomUsernamePrivate *priv = PRIVATE(self);
    GtkWidget *error_widget;

    if (self->filled_fields & FIELD_USERNAME)
      error_widget = self->server_editor;
    else
      error_widget = self->username_editor;

    g_object_set(self, "can-next", FALSE, NULL);
    rtcom_widget_set_error_widget(RTCOM_WIDGET(self), error_widget);
    rtcom_widget_set_msg_next(RTCOM_WIDGET(self), priv->msg_empty);
  }
}

static void
on_username_changed(GtkEditable *editable, RtcomUsername *self)
{
  const gchar *text = gtk_entry_get_text(GTK_ENTRY(editable));

  if (text && *text)
  {
    if (!(self->filled_fields & FIELD_USERNAME))
    {
      self->filled_fields |= FIELD_USERNAME;
      emptiness_changed(self);
    }
  }
  else
  {
    gboolean required;

    g_object_get(self, "required", &required, NULL);

    if (required)
    {
      self->filled_fields &= ~FIELD_USERNAME;
      emptiness_changed(self);
    }
  }

  rtcom_widget_value_changed(RTCOM_WIDGET(self));
}

static void
rtcom_username_init(RtcomUsername *self)
{
  self->editable = TRUE;
  self->username_editor =
    g_object_new(HILDON_TYPE_ENTRY,
                 "hildon-input-mode", HILDON_GTK_INPUT_MODE_FULL,
                 NULL);
  g_signal_connect(self->username_editor, "changed",
                   G_CALLBACK(on_username_changed), self);
  self->at_label =
    g_object_new(GTK_TYPE_LABEL,
                 "label", "@",
                 "no-show-all", TRUE,
                 NULL);
  gtk_box_pack_start(GTK_BOX(self), self->username_editor,
                     TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(self), self->at_label,
                     FALSE, FALSE, 8);
  gtk_widget_show(self->username_editor);
  rtcom_widget_set_error_widget(RTCOM_WIDGET(self),
                                self->username_editor);
  self->username_validation =
    g_object_new(
      RTCOM_TYPE_ENTRY_VALIDATION,
      NULL);
  self->server_validation =
    g_object_new(
      RTCOM_TYPE_ENTRY_VALIDATION,
      "invalid-chars-re", "[^A-Za-z0-9\\-\\.]",
      "min-length", 4,
      "min-length-msg", _("accountwizard_ib_illegal_server_address"),
      NULL);
}

static gboolean
rtcom_username_store_settings(RtcomWidget *widget, GError **error,
                              RtcomAccountItem *item)
{
  RtcomUsername *self = RTCOM_USERNAME(widget);
  const gchar *username = gtk_entry_get_text(GTK_ENTRY(self->username_editor));

  if (self->server_editor)
  {
    GtkWidget *server_entry;
    const gchar *server;
    gchar *text;

    if (GTK_IS_ENTRY(self->server_editor))
      server_entry = self->server_editor;
    else
      server_entry = GTK_BIN(self->server_editor)->child;

    server = gtk_entry_get_text(GTK_ENTRY(server_entry));
    text = g_strconcat(username, "@", server, NULL);
    rtcom_account_item_store_param_string(item, self->field, text);
    g_free(text);
  }
  else
    rtcom_account_item_store_param_string(item, self->field, username);

  return TRUE;
}

static gboolean
rtcom_username_get_value(RtcomWidget *widget, GValue *value)
{
  RtcomUsername *self = RTCOM_USERNAME(widget);

  if (G_VALUE_HOLDS(value, G_TYPE_INVALID) || G_VALUE_HOLDS(value, G_TYPE_NONE))
    g_value_init(value, G_TYPE_STRING);

  if (G_VALUE_HOLDS(value, G_TYPE_STRING))
  {
    const gchar *username_text =
      gtk_entry_get_text(GTK_ENTRY(self->username_editor));

    if (self->server_editor)
    {
      GtkWidget *server_entry;
      const gchar *server_text;

      if (GTK_IS_ENTRY(self->server_editor))
        server_entry = self->server_editor;
      else
        server_entry = GTK_BIN(self->server_editor)->child;

      server_text = gtk_entry_get_text(GTK_ENTRY(server_entry));
      g_value_take_string(value,
                          g_strconcat(username_text, "@", server_text, NULL));
    }
    else
      g_value_set_static_string(value, username_text);

    return TRUE;
  }
  else
    return FALSE;
}

static void
rtcom_username_get_settings(RtcomWidget *widget, RtcomAccountItem *item)
{
  RtcomUsername *self = RTCOM_USERNAME(widget);
  GHashTable *parameters;
  const GValue *v;
  gchar *text;

  if (!self->field)
    return;

  parameters = (GHashTable *)tp_account_get_parameters(item->account);

  if (!parameters)
    return;

  v = g_hash_table_lookup(parameters, self->field);

  if (!v)
    return;

  text = g_value_dup_string(v);

  if (self->server_editor && *text)
  {
    gchar *p = text;
    gchar *server_text = NULL;

    while (*p)
    {
      if (*p == '@')
      {
        server_text = p + 1;
        *p = 0;
      }

      p++;
    }

    gtk_entry_set_text(GTK_ENTRY(self->username_editor), text);

    if (server_text)
    {
      GtkWidget *server_entry;

      if (GTK_IS_ENTRY(self->server_editor))
        server_entry = self->server_editor;
      else
        server_entry = GTK_BIN(self->server_editor)->child;

      gtk_entry_set_text(GTK_ENTRY(server_entry), server_text);
    }
  }
  else
    gtk_entry_set_text(GTK_ENTRY(self->username_editor), text);

  g_free(text);
}

static void
on_domain_changed(GtkEditable *editable, RtcomUsername *self)
{
  const gchar *text = gtk_entry_get_text(GTK_ENTRY(editable));

  if (text && *text)
  {
    if (!(self->filled_fields & FIELD_SERVER))
    {
      self->filled_fields |= FIELD_SERVER;
      emptiness_changed(self);
    }
  }
  else
  {
    gboolean required;

    g_object_get(self, "required", &required, NULL);

    if (required)
    {
      self->filled_fields &= ~FIELD_SERVER;
      emptiness_changed(self);
    }
  }

  rtcom_widget_value_changed(RTCOM_WIDGET(self));
}

static void
rtcom_username_set_account(RtcomWidget *widget, RtcomAccountItem *account)
{
  RtcomUsername *self = RTCOM_USERNAME(widget);
  RtcomAccountService *service =
    RTCOM_ACCOUNT_SERVICE(ACCOUNT_ITEM(account)->service);

  g_return_if_fail(self->protocol == NULL);

  self->protocol = rtcom_account_item_get_tp_protocol(account);

  g_return_if_fail(TP_IS_PROTOCOL(self->protocol));

  gtk_editable_set_editable(GTK_EDITABLE(self->username_editor),
                            self->editable);

  if (service->account_domains)
  {
    gchar **domains;

    if (self->show_server_name)
      gtk_widget_show(self->at_label);

    if ((domains = g_strsplit(service->account_domains, ",", 0)))
    {
      if (domains[0] && domains[1])
      {
        self->server_editor = gtk_combo_box_entry_new_text();
        gtk_widget_set_sensitive(self->server_editor, self->editable);
        g_object_set(
          GTK_BIN(self->server_editor)->child,
          "hildon-input-mode", HILDON_GTK_INPUT_MODE_FULL,
          "no-show-all", TRUE,
          NULL);
        g_signal_connect(GTK_BIN(self->server_editor)->child, "changed",
                         G_CALLBACK(on_domain_changed), self);
      }
      else
      {
        self->server_editor = g_object_new(
            HILDON_TYPE_ENTRY,
            "hildon-input-mode", HILDON_GTK_INPUT_MODE_FULL,
            "editable", self->editable,
            "no-show-all", TRUE,
            NULL);
        g_signal_connect(self->server_editor, "changed",
                         G_CALLBACK(on_domain_changed), self);
      }

      gtk_box_pack_start(GTK_BOX(self), self->server_editor, TRUE, TRUE, 0);

      if (domains[0] && domains[1])
      {
        gchar **domain;

        for (domain = domains; *domain; domain++)
        {
          gtk_combo_box_append_text(GTK_COMBO_BOX(self->server_editor),
                                    g_strstrip(*domain));
        }

        gtk_combo_box_set_active(GTK_COMBO_BOX(self->server_editor), FALSE);
      }
      else
        gtk_entry_set_text(GTK_ENTRY(self->server_editor), domains[0]);

      g_strfreev(domains);
    }
    else
    {
      self->server_editor = g_object_new(
          HILDON_TYPE_ENTRY,
          "hildon-input-mode", HILDON_GTK_INPUT_MODE_FULL,
          "editable", self->editable,
          "no-show-all", TRUE,
          NULL);

      g_signal_connect(self->server_editor, "changed",
                       G_CALLBACK(on_domain_changed), self);
      gtk_box_pack_start(GTK_BOX(self), self->server_editor, TRUE, TRUE, 0);
    }

    if (self->show_server_name)
      gtk_widget_show(self->server_editor);
  }
  else
  {
    self->filled_fields |= FIELD_SERVER;

    if (self->prefill && *self->prefill)
      hildon_entry_set_text(HILDON_ENTRY(self->username_editor), self->prefill);
    else
    {
      const gchar *placeholder = PRIVATE(self)->placeholder;

      if (placeholder)
      {
        hildon_entry_set_placeholder(HILDON_ENTRY(self->username_editor),
                                     placeholder);
      }
    }
  }
}

static gboolean
rtcom_username_validate(RtcomWidget *widget, GError **error)
{
  RtcomUsername *self = RTCOM_USERNAME(widget);
  const gchar *username_text;
  const gchar *server_text = NULL;
  const gchar *username;

  username_text = gtk_entry_get_text(GTK_ENTRY(self->username_editor));

  if (self->server_editor)
  {
    GtkWidget *server_editor;

    if (GTK_IS_ENTRY(self->server_editor))
      server_editor = self->server_editor;
    else
      server_editor = GTK_BIN(self->server_editor)->child;

    server_text = gtk_entry_get_text(GTK_ENTRY(server_editor));

    if (!rtcom_entry_validation_validate(
          self->server_validation, server_text, NULL, error))
    {
      goto error_server;
    }

    if (!rtcom_entry_validation_validate(self->username_validation,
                                         username_text, NULL, error))
    {
      goto error_username;
    }

    username = username_text;
  }
  else
  {
    gchar *first_at = g_utf8_strchr(username_text, -1, '@');
    gchar *last_at = g_utf8_strrchr(username_text, -1, '@');
    gboolean has_at_char = !!first_at;
    gboolean bad_at_char;

    if (first_at == last_at)
      bad_at_char = FALSE;
    else
      bad_at_char = has_at_char;

    if (bad_at_char)
    {
      const gchar *fmt = _("accountwizard_ib_illegal_character");

      g_set_error(
        error, ACCOUNT_ERROR, ACCOUNT_ERROR_INVALID_CHARACTER, fmt, "@");
      rtcom_widget_set_error_widget(RTCOM_WIDGET(self), self->username_editor);
      return FALSE;
    }

    if (!has_at_char)
    {
      if (self->must_have_at_separator)
      {
        g_set_error(error, ACCOUNT_ERROR, ACCOUNT_ERROR_INVALID_VALUE,
                    "%s", _("accountwizard_ib_illegal_server_address"));
        rtcom_widget_set_error_widget(RTCOM_WIDGET(self),
                                      self->username_editor);
        return FALSE;
      }

      if (!rtcom_entry_validation_validate(self->username_validation,
                                           username_text, NULL, error))
      {
        rtcom_widget_set_error_widget(RTCOM_WIDGET(self),
                                      self->username_editor);
        return FALSE;
      }

      username = username_text;
    }
    else
    {
      gchar **split = g_strsplit(username_text, "@", 0);

      g_assert(split);

      if ((username_text == last_at) || (username_text == first_at))
      {
        g_set_error(error, ACCOUNT_ERROR, ACCOUNT_ERROR_INVALID_VALUE, "%s",
                    PRIVATE(self)->msg_empty);
        g_strfreev(split);
        rtcom_widget_set_error_widget(RTCOM_WIDGET(self),
                                      self->username_editor);
        return FALSE;
      }

      username = split[0];
      server_text = split[1];

      if (!rtcom_entry_validation_validate(self->username_validation, username,
                                           NULL, error))
      {
        g_strfreev(split);
        goto error_username;
      }

      if (self->required_server &&
          g_strcmp0(server_text, self->required_server))
      {
        g_set_error(error, ACCOUNT_ERROR, ACCOUNT_ERROR_INVALID_VALUE,
                    "%s", self->required_server_error);
        rtcom_widget_set_error_widget(RTCOM_WIDGET(self),
                                      self->username_editor);
        g_strfreev(split);
        return FALSE;
      }

      if (!rtcom_entry_validation_validate(self->server_validation, server_text,
                                           NULL, error))
      {
        g_strfreev(split);
        goto error_server;
      }

      g_strfreev(split);
    }
  }

  if (self->check_uniqueness)
  {
    RtcomAccountItem *account = rtcom_widget_get_account(RTCOM_WIDGET(self));
    AccountPlugin *plugin = account_item_get_plugin(ACCOUNT_ITEM(account));
    gboolean unique = TRUE;
    AccountsList *accounts_list = NULL;

    g_object_get(plugin, "accounts-list", &accounts_list, NULL);

    if (accounts_list)
    {
      AccountService *service = account_item_get_service(ACCOUNT_ITEM(account));
      gchar *tmp;
      GList *all;
      GList *l;

      if (self->server_editor)
        tmp = g_strconcat(username_text, "@", server_text, NULL);
      else
        tmp = g_strdup(username_text);

      all = accounts_list_get_all(accounts_list);
      g_object_unref(accounts_list);

      for (l = all; l; l = l->next)
      {
        RtcomAccountItem *ai = l->data;

        if (RTCOM_IS_ACCOUNT_ITEM(l->data) && (account != ai) && ai->account &&
            (service == account_item_get_service(ACCOUNT_ITEM(ai))))
        {
          GHashTable *parameters;

          parameters = (GHashTable *)tp_account_get_parameters(ai->account);

          if (parameters)
          {
            const GValue *v = g_hash_table_lookup(parameters, self->field);

            if (v && !strcmp(tmp, g_value_get_string(v)))
            {
              unique = FALSE;
              break;
            }
          }
        }
      }

      g_free(tmp);
    }

    if (!accounts_list || !unique)
    {
      const gchar *fmt = _("accounts_fi_username_exists");
      AccountService *service = account_item_get_service(
          ACCOUNT_ITEM(rtcom_widget_get_account(RTCOM_WIDGET(self))));

      g_set_error(error, ACCOUNT_ERROR, ACCOUNT_ERROR_ALREADY_EXISTS,
                  fmt, username, account_service_get_display_name(service));
      rtcom_widget_set_error_widget(RTCOM_WIDGET(self), self->username_editor);
      return FALSE;
    }
  }

  return TRUE;

error_server:
  rtcom_widget_set_error_widget(RTCOM_WIDGET(self), self->server_editor);

  return FALSE;

error_username:
  rtcom_widget_set_error_widget(RTCOM_WIDGET(self), self->username_editor);

  return FALSE;
}

static void
rtcom_widget_init(RtcomWidgetIface *iface)
{
  iface->store_settings = rtcom_username_store_settings;
  iface->get_value = rtcom_username_get_value;
  iface->get_settings = rtcom_username_get_settings;
  iface->set_account = rtcom_username_set_account;
  iface->validate = rtcom_username_validate;
}
