/*
 * rtcom-login.c
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
#include "rtcom-username.h"

#include "rtcom-login.h"

struct _RtcomLoginPrivate
{
  GtkWidget *vbox;
  gchar *service_icon;
  guint items_mask;
  GtkWidget *username;
  GtkWidget *new_account_button;
  GtkWidget *forgot_password_button;
  GtkWidget *advanced_settings_button;
  GtkWidget *define_username_button;
  AccountItem *account;
  gchar *username_field;
  gchar *username_invalid_chars_re;
  gchar *username_prefill;
  gchar *username_placeholder;
  gchar *username_label;
  gboolean username_show_server_name;
  gboolean username_must_have_at_separator;
  gchar *username_required_server;
  gchar *username_required_server_error;
  gchar *msg_empty;
  gchar *define_username;
  GtkTable *table;
};

typedef struct _RtcomLoginPrivate RtcomLoginPrivate;

#define PRIVATE(login) \
  ((RtcomLoginPrivate *) \
   rtcom_login_get_instance_private((RtcomLogin *)(login)))

G_DEFINE_TYPE_WITH_PRIVATE(
  RtcomLogin,
  rtcom_login,
  RTCOM_TYPE_PAGE
);

enum
{
  PROP_SERVICE_ICON = 1,
  PROP_ITEMS_MASK,
  PROP_ACCOUNT,
  PROP_USERNAME_FIELD,
  PROP_USERNAME_INVALID_CHARS_RE,
  PROP_USERNAME_PREFILL,
  PROP_USERNAME_PLACEHOLDER,
  PROP_USERNAME_LABEL,
  PROP_USERNAME_SHOW_SERVER_NAME,
  PROP_USERNAME_MUST_HAVE_AT_SEPARATOR,
  PROP_USERNAME_REQUIRED_SERVER,
  PROP_USERNAME_REQUIRED_SERVER_ERROR,
  PROP_MSG_EMPTY,
  PROP_DEFINE_USERNAME
};

static void
rtcom_login_finalize(GObject *object)
{
  RtcomLoginPrivate *priv = PRIVATE(object);

  g_free(priv->username_prefill);
  g_free(priv->username_placeholder);
  g_free(priv->username_label);
  g_free(priv->service_icon);
  g_free(priv->username_field);
  g_free(priv->username_invalid_chars_re);
  g_free(priv->msg_empty);
  g_free(priv->define_username);
  g_free(priv->username_required_server);
  g_free(priv->username_required_server_error);

  G_OBJECT_CLASS(rtcom_login_parent_class)->finalize(object);
}

static void
rtcom_login_set_property(GObject *object, guint property_id,
                         const GValue *value, GParamSpec *pspec)
{
  RtcomLoginPrivate *priv;

  g_return_if_fail(RTCOM_IS_LOGIN(object));

  priv = PRIVATE(object);

  switch (property_id)
  {
    case PROP_SERVICE_ICON:
    {
      g_free(priv->service_icon);
      priv->service_icon = g_value_dup_string(value);
      break;
    }
    case PROP_ITEMS_MASK:
    {
      priv->items_mask = g_value_get_uint(value);
      break;
    }
    case PROP_ACCOUNT:
    {
      priv->account = g_value_get_object(value);
      break;
    }
    case PROP_USERNAME_FIELD:
    {
      g_free(priv->username_field);
      priv->username_field = g_value_dup_string(value);
      break;
    }
    case PROP_USERNAME_INVALID_CHARS_RE:
    {
      g_free(priv->username_invalid_chars_re);
      priv->username_invalid_chars_re = g_value_dup_string(value);
      break;
    }
    case PROP_USERNAME_PREFILL:
    {
      g_free(priv->username_prefill);
      priv->username_prefill = g_value_dup_string(value);
      break;
    }
    case PROP_USERNAME_PLACEHOLDER:
    {
      g_free(priv->username_placeholder);
      priv->username_placeholder = g_value_dup_string(value);
      break;
    }
    case PROP_USERNAME_LABEL:
    {
      g_free(priv->username_label);
      priv->username_label = g_value_dup_string(value);
      break;
    }
    case PROP_USERNAME_SHOW_SERVER_NAME:
    {
      priv->username_show_server_name = g_value_get_boolean(value);
      break;
    }
    case PROP_USERNAME_MUST_HAVE_AT_SEPARATOR:
    {
      priv->username_must_have_at_separator = g_value_get_boolean(value);
      break;
    }
    case PROP_USERNAME_REQUIRED_SERVER:
    {
      g_free(priv->username_required_server);
      priv->username_required_server = g_value_dup_string(value);
      break;
    }
    case PROP_USERNAME_REQUIRED_SERVER_ERROR:
    {
      g_free(priv->username_required_server_error);
      priv->username_required_server_error = g_value_dup_string(value);
      break;
    }
    case PROP_MSG_EMPTY:
    {
      g_free(priv->msg_empty);
      priv->msg_empty = g_value_dup_string(value);
      break;
    }
    case PROP_DEFINE_USERNAME:
    {
      g_free(priv->define_username);
      priv->define_username = g_value_dup_string(value);
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
rtcom_login_get_property(GObject *object, guint property_id, GValue *value,
                         GParamSpec *pspec)
{
  RtcomLoginPrivate *priv;

  g_return_if_fail(RTCOM_IS_LOGIN(object));

  priv = PRIVATE(object);

  switch (property_id)
  {
    case PROP_SERVICE_ICON:
    {
      g_value_set_string(value, priv->service_icon);
      break;
    }
    case PROP_ITEMS_MASK:
    {
      g_value_set_uint(value, priv->items_mask);
      break;
    }
    case PROP_ACCOUNT:
    {
      g_value_set_object(value, priv->account);
      break;
    }
    case PROP_USERNAME_FIELD:
    {
      g_value_set_string(value, priv->username_field);
      break;
    }
    case PROP_USERNAME_INVALID_CHARS_RE:
    {
      g_value_set_string(value, priv->username_invalid_chars_re);
      break;
    }
    case PROP_USERNAME_PREFILL:
    {
      g_value_set_string(value, priv->username_prefill);
      break;
    }
    case PROP_USERNAME_PLACEHOLDER:
    {
      g_value_set_string(value, priv->username_placeholder);
      break;
    }
    case PROP_USERNAME_LABEL:
    {
      g_value_set_string(value, priv->username_label);
      break;
    }
    case PROP_USERNAME_SHOW_SERVER_NAME:
    {
      g_value_set_boolean(value, priv->username_show_server_name);
      break;
    }
    case PROP_USERNAME_MUST_HAVE_AT_SEPARATOR:
    {
      g_value_set_boolean(value, priv->username_must_have_at_separator);
      break;
    }
    case PROP_USERNAME_REQUIRED_SERVER:
    {
      g_value_set_string(value, priv->username_required_server);
      break;
    }
    case PROP_USERNAME_REQUIRED_SERVER_ERROR:
    {
      g_value_set_string(value, priv->username_required_server_error);
      break;
    }
    case PROP_MSG_EMPTY:
    {
      g_value_set_string(value, priv->msg_empty);
      break;
    }
    case PROP_DEFINE_USERNAME:
    {
      g_value_set_string(value, priv->define_username);
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
constructor(GType type, guint n_construct_properties,
            GObjectConstructParam *construct_properties)
{
  RtcomLoginPrivate *priv;
  const gchar *label;
  GtkWidget *password_field = NULL;
  GtkWidget *password_label = NULL;
  GObject *object;
  GtkWidget *username_label;
  GtkWidget *align;
  GtkWidget *area;
  guint top = 1;
  guint bottom = 2;
  guint rows = 1;
  gboolean check_uniqueness;

  object = G_OBJECT_CLASS(rtcom_login_parent_class)->constructor(
      type, n_construct_properties, construct_properties);

  priv = PRIVATE(object);
  area = g_object_new(HILDON_TYPE_PANNABLE_AREA, NULL);
  align = gtk_alignment_new(0.5, 0.5, 1.0, 1.0);
  priv->vbox = gtk_vbox_new(0, 0);

  if (priv->items_mask & RTCOM_PLUGIN_CAPABILITY_REGISTER)
  {
    priv->new_account_button = hildon_button_new_with_text(
        HILDON_SIZE_FINGER_HEIGHT, HILDON_BUTTON_ARRANGEMENT_HORIZONTAL,
        _("accounts_bd_create_new"), "");

    hildon_button_set_alignment(HILDON_BUTTON(priv->new_account_button),
                                0.0, 0.5, 1.0, 1.0);
  }

  if (priv->items_mask & RTCOM_PLUGIN_CAPABILITY_FORGOT_PWD)
  {
    priv->forgot_password_button = hildon_button_new_with_text(
        HILDON_SIZE_FINGER_HEIGHT, HILDON_BUTTON_ARRANGEMENT_HORIZONTAL,
        _("accounts_bd_forgot_password"), "");
    hildon_button_set_alignment(HILDON_BUTTON(priv->forgot_password_button),
                                0.0, 0.5, 1.0, 1.0);
    priv->items_mask = priv->items_mask;
  }

  if (priv->items_mask & RTCOM_PLUGIN_CAPABILITY_ADVANCED)
  {
    priv->advanced_settings_button = hildon_button_new_with_text(
        HILDON_SIZE_FINGER_HEIGHT, HILDON_BUTTON_ARRANGEMENT_HORIZONTAL,
        _("accounts_bd_advanced_settings"), "");
    priv->advanced_settings_button = priv->advanced_settings_button;
    hildon_button_set_alignment(HILDON_BUTTON(priv->advanced_settings_button),
                                0.0, 0.5, 1.0, 1.0);
  }

  label = priv->username_label;

  if (!label || !*label)
    label = _("accounts_fi_user_name");

  username_label = g_object_new(
      GTK_TYPE_LABEL,
      "label", label,
      "xalign", 0.0,
      NULL);

  check_uniqueness =
    (priv->items_mask & RTCOM_PLUGIN_CAPABILITY_ALLOW_MULTIPLE) ?
    TRUE : FALSE;
  priv->username = g_object_new(
      RTCOM_TYPE_USERNAME,
      "field", priv->username_field,
      "invalid-chars-re", priv->username_invalid_chars_re,
      "prefill", priv->username_prefill,
      "placeholder", priv->username_placeholder,
      "check-uniqueness", check_uniqueness,
      "show-server-name", priv->username_show_server_name,
      "must-have-at-separator", priv->username_must_have_at_separator,
      "required-server", priv->username_required_server,
      "required-server-error", priv->username_required_server_error,
      "msg-empty", priv->msg_empty,
      "required", TRUE,
      "can-next", FALSE,
      NULL);

  if (priv->items_mask & RTCOM_PLUGIN_CAPABILITY_PASSWORD)
  {
    password_label = g_object_new(
          GTK_TYPE_LABEL,
          "label", _("accounts_fi_password"),
          "xalign", 0.0,
          NULL);
    password_field = g_object_new(
          RTCOM_TYPE_PARAM_STRING,
          "field", "password",
          "can-next", FALSE,
          "visibility", FALSE,
          "required", TRUE,
          "max_length", 64,
          "msg-empty", priv->msg_empty,
          "hildon_input_mode", HILDON_GTK_INPUT_MODE_FULL |
          HILDON_GTK_INPUT_MODE_INVISIBLE,
          NULL);
    rows++;
  }

  if (priv->define_username)
  {
    priv->define_username_button = hildon_button_new_with_text(
        HILDON_SIZE_FINGER_HEIGHT, HILDON_BUTTON_ARRANGEMENT_HORIZONTAL,
        priv->define_username, "");
    top++;
    bottom++;
    rows++;
    hildon_button_set_alignment(HILDON_BUTTON(priv->define_username_button),
                                0.0, 0.5, 1.0, 1.0);
    priv->table = GTK_TABLE(gtk_table_new(rows, 2, FALSE));
    gtk_table_attach(priv->table, username_label, 0, 1, 0, 1,
                     GTK_FILL, GTK_SHRINK, 16, 0);
    gtk_table_attach(priv->table, priv->username, 1, 2, 0, 1,
                     GTK_FILL | GTK_EXPAND, GTK_SHRINK, 0, 0);
    gtk_table_attach(priv->table, priv->define_username_button, 0, 2, 1, 2,
                     GTK_FILL | GTK_EXPAND, GTK_SHRINK, 0, 0);
  }
  else
  {
    priv->table = GTK_TABLE(gtk_table_new(rows, 2, FALSE));
    gtk_table_attach(priv->table, username_label, 0, 1, 0, 1,
                     GTK_FILL, GTK_SHRINK, 16, 0);
    gtk_table_attach(priv->table, priv->username, 1, 2, 0, 1,
                     GTK_FILL | GTK_EXPAND, GTK_SHRINK, 0, 0);
  }

  if (priv->items_mask & RTCOM_PLUGIN_CAPABILITY_PASSWORD)
  {
    gtk_table_attach(priv->table, password_label, 0, 1, top, bottom,
                     GTK_FILL, GTK_SHRINK, 16, 0);
    gtk_table_attach(priv->table, password_field, 1, 2, top, bottom,
                     GTK_FILL | GTK_EXPAND, GTK_SHRINK, 0, 0);
  }

  if (priv->items_mask & RTCOM_PLUGIN_CAPABILITY_REGISTER)
  {
    gtk_box_pack_start(GTK_BOX(priv->vbox), priv->new_account_button,
                       FALSE, FALSE, 0);
  }

  gtk_box_pack_start(GTK_BOX(priv->vbox), GTK_WIDGET(priv->table),
                     FALSE, FALSE, 0);

  if (priv->items_mask & RTCOM_PLUGIN_CAPABILITY_FORGOT_PWD)
  {
    gtk_box_pack_start(GTK_BOX(priv->vbox), priv->forgot_password_button,
                       FALSE, FALSE, 0);
  }

  if (priv->items_mask & RTCOM_PLUGIN_CAPABILITY_ADVANCED)
  {
    gtk_box_pack_start(GTK_BOX(priv->vbox), priv->advanced_settings_button,
                       FALSE, FALSE, 0);
  }

  gtk_container_add(GTK_CONTAINER(align), priv->vbox);
  hildon_pannable_area_add_with_viewport(HILDON_PANNABLE_AREA(area), align);
  g_signal_connect_swapped(priv->vbox, "size-request",
                           G_CALLBACK(gtk_widget_queue_resize), object);
  gtk_container_add(GTK_CONTAINER(object), area);

  return object;
}

static void
rtcom_login_size_request(GtkWidget *widget, GtkRequisition *requisition)
{
  RtcomLoginPrivate *priv = PRIVATE(widget);

  if (priv->vbox)
  {
    GtkWidget *ancestor;
    GtkRequisition r;
    gint height_request;

    gtk_widget_size_request(priv->vbox, &r);

    ancestor = gtk_widget_get_ancestor(priv->vbox, HILDON_TYPE_PANNABLE_AREA);

    height_request = r.height;

    if (r.height >= 350)
      height_request = 350;

    r.height = height_request;
    g_object_set(ancestor, "height-request", height_request, NULL);
  }

  GTK_WIDGET_CLASS(rtcom_login_parent_class)->size_request(widget, requisition);
}

static void
rtcom_login_map(GtkWidget *widget)
{
  GTK_WIDGET_CLASS(rtcom_login_parent_class)->map(widget);

  gtk_widget_grab_focus(PRIVATE(widget)->username);
}

static void
rtcom_login_set_account(RtcomPage *page, RtcomAccountItem *account)
{
#if 0 /* FIXME - reconsider if disabling register button is an option */
  AccountService *service = account_item_get_service(ACCOUNT_ITEM(account));

  if (rtcom_account_service_get_param_type(RTCOM_ACCOUNT_SERVICE(service),
                                           "register") == G_TYPE_INVALID)
  {
    RtcomLoginPrivate *priv = PRIVATE(page);

    gtk_widget_set_sensitive(priv->new_account_button, FALSE);
  }
#endif
  RTCOM_PAGE_CLASS(rtcom_login_parent_class)->set_account(page, account);
}

static void
rtcom_login_class_init(RtcomLoginClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS(klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

  object_class->finalize = rtcom_login_finalize;
  object_class->set_property = rtcom_login_set_property;
  object_class->get_property = rtcom_login_get_property;
  object_class->constructor = constructor;

  widget_class->map = rtcom_login_map;
  widget_class->size_request = rtcom_login_size_request;

  RTCOM_PAGE_CLASS(klass)->set_account = rtcom_login_set_account;

  g_object_class_install_property(
    object_class, PROP_USERNAME_PREFILL,
    g_param_spec_string(
      "username-prefill",
      "Login prefill",
      "username prefill",
      "",
      G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE));
  g_object_class_install_property(
    object_class, PROP_USERNAME_PLACEHOLDER,
    g_param_spec_string(
      "username-placeholder",
      "Login placeholder",
      "username placeholder",
      "",
      G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE));
  g_object_class_install_property(
    object_class, PROP_USERNAME_LABEL,
    g_param_spec_string(
      "username-label",
      "Login label",
      "username label",
      "",
      G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE));
  g_object_class_install_property(
    object_class, PROP_SERVICE_ICON,
    g_param_spec_string(
      "service-icon",
      "Service icon",
      "service icon",
      "",
      G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE));
  g_object_class_install_property(
    object_class, PROP_ITEMS_MASK,
    g_param_spec_uint(
      "items-mask",
      NULL,
      NULL,
      0, RTCOM_PLUGIN_CAPABILITY_ALL, 0,
      G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE));
  g_object_class_install_property(
    object_class, PROP_ACCOUNT,
    g_param_spec_object(
      "account",
      "The RtcomAccountItem",
      "the RtcomAccountItem",
      RTCOM_TYPE_ACCOUNT_ITEM,
      G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE));
  g_object_class_install_property(
    object_class, PROP_USERNAME_FIELD,
    g_param_spec_string(
      "username-field",
      "Login field name",
      "TpAccount field name for the username",
      NULL,
      G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE));
  g_object_class_install_property(
    object_class, PROP_USERNAME_INVALID_CHARS_RE,
    g_param_spec_string(
      "username-invalid-chars-re",
      "Invalid chars regex for the username",
      "Regex to find forbidden characters in theusername",
      NULL,
      G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE));
  g_object_class_install_property(
    object_class, PROP_USERNAME_SHOW_SERVER_NAME,
    g_param_spec_boolean(
      "username-show-server-name",
      "Show servername",
      "Shows the @ sign and the server name part",
      TRUE,
      G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE));
  g_object_class_install_property(
    object_class, PROP_USERNAME_MUST_HAVE_AT_SEPARATOR,
    g_param_spec_boolean(
      "username-must-have-at-separator",
      "The @ sign is mandatory",
      "The @ sign is mandatory",
      FALSE,
      G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE));
  g_object_class_install_property(
    object_class, PROP_USERNAME_REQUIRED_SERVER,
    g_param_spec_string(
      "username-required-server",
      "The required server",
      "The required server part in username",
      NULL,
      G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE));
  g_object_class_install_property(
    object_class, PROP_USERNAME_REQUIRED_SERVER_ERROR,
    g_param_spec_string(
      "username-required-server-error",
      "The required server error",
      "The error for required server part in username",
      NULL,
      G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE));
  g_object_class_install_property(
    object_class, PROP_MSG_EMPTY,
    g_param_spec_string(
      "msg-empty",
      "Message empty",
      "Message to be printed if fields are empty",
      _("accounts_fi_enter_fields_first"),
      G_PARAM_CONSTRUCT_ONLY | GTK_PARAM_READWRITE));
  g_object_class_install_property(
    object_class, PROP_DEFINE_USERNAME,
    g_param_spec_string(
      "define-username",
      "Define Login User Label",
      "define username user label",
      NULL,
      G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE));
}

static void
rtcom_login_init(RtcomLogin *login)
{}

void
rtcom_login_connect_on_define(RtcomLogin *login, GCallback cb,
                              gpointer user_data)
{
  g_signal_connect_swapped(PRIVATE(login)->define_username_button, "clicked",
                           cb, user_data);
}

void
rtcom_login_connect_on_advanced(RtcomLogin *login, GCallback cb,
                                gpointer user_data)
{
  g_signal_connect_swapped(PRIVATE(login)->advanced_settings_button, "clicked",
                           cb, user_data);
}

void
rtcom_login_connect_on_forgot_password(RtcomLogin *login, GCallback cb,
                                       gpointer user_data)
{
  g_signal_connect_swapped(PRIVATE(login)->forgot_password_button, "clicked",
                           cb, user_data);
}

void
rtcom_login_connect_on_register(RtcomLogin *login, GCallback cb,
                                gpointer user_data)
{
  g_signal_connect_swapped(PRIVATE(login)->new_account_button, "clicked",
                           cb, user_data);
}

void
rtcom_login_append_widget(RtcomLogin *login, GtkWidget *label,
                          GtkWidget *widget)
{
  RtcomLoginPrivate *priv = PRIVATE(login);

  if (!label)
  {
    gtk_box_pack_start(GTK_BOX(priv->vbox), widget, FALSE, FALSE, 0);
    gtk_box_reorder_child(GTK_BOX(priv->vbox), widget, 1);
  }
  else
  {
    guint rows;

    g_object_get(priv->table, "n-rows", &rows, NULL);
    rows++;
    gtk_table_resize(GTK_TABLE(priv->table), rows, 2);

    gtk_table_attach(priv->table, label, 0, 1, rows, rows + 1,
                     GTK_FILL, GTK_SHRINK, 16, 0);
    gtk_table_attach(priv->table, widget, 1, 2, rows, rows + 1,
                     GTK_FILL | GTK_EXPAND, GTK_SHRINK, 0, 0);
    gtk_widget_show(label);
  }

  gtk_widget_show(widget);
}
