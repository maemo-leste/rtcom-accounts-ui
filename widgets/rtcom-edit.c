/*
 * rtcom-edit.c
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
#include <hildon-uri.h>
#include <hildon/hildon.h>

#include "rtcom-alias.h"
#include "rtcom-avatar.h"
#include "rtcom-enabled.h"
#include "rtcom-param-string.h"
#include "rtcom-username.h"

#include "rtcom-edit.h"

struct _RtcomEditPrivate
{
  GtkWidget *vbox;
  guint items_mask;
  gchar *edit_info_uri;
  GtkWidget *advanced_button;
  RtcomAccountItem *account;
  GtkWidget *screen_widget;
  gchar *username_field;
  gchar *username_label;
  gchar *username_invalid_chars_re;
  gboolean username_must_have_at;
  gboolean username_show_server_name;
  gchar *username_required_server;
  gchar *username_required_server_error;
  gchar *msg_empty;
  gchar *at;
  GtkTable *table;
  GtkWidget *avatar_align;
  GtkWidget *avatar_label;
};

typedef struct _RtcomEditPrivate RtcomEditPrivate;

#define PRIVATE(edit) \
  ((RtcomEditPrivate *) \
   rtcom_edit_get_instance_private((RtcomEdit *)(edit)))

G_DEFINE_TYPE_WITH_PRIVATE(
  RtcomEdit,
  rtcom_edit,
  RTCOM_TYPE_PAGE
);

enum
{
  PROP_ITEMS_MASK = 1,
  PROP_ACCOUNT,
  PROP_EDIT_INFO_URI,
  PROP_USERNAME_FIELD,
  PROP_USERNAME_LABEL,
  PROP_USERNAME_INVALID_CHARS_RE,
  PROP_USERNAME_SHOW_SERVER_NAME,
  PROP_USERNAME_MUST_HAVE_AT_SEPARATOR,
  PROP_USERNAME_REQUIRED_SERVER,
  PROP_USERNAME_REQUIRED_SERVER_ERROR,
  PROP_MSG_EMPTY,
  PROP_USER_SERVER_SEPARATOR
};

static void
rtcom_edit_dispose(GObject *object)
{
  RtcomEditPrivate *priv = PRIVATE(object);

  if (priv->account)
  {
    g_object_unref(priv->account);
    priv->account = NULL;
  }

  G_OBJECT_CLASS(rtcom_edit_parent_class)->dispose(object);
}

static void
rtcom_edit_finalize(GObject *object)
{
  RtcomEditPrivate *priv = PRIVATE(object);

  g_free(priv->edit_info_uri);
  g_free(priv->username_field);
  g_free(priv->username_label);
  g_free(priv->username_invalid_chars_re);
  g_free(priv->msg_empty);
  g_free(priv->at);
  g_free(priv->username_required_server);
  g_free(priv->username_required_server_error);

  G_OBJECT_CLASS(rtcom_edit_parent_class)->finalize(object);
}

static void
rtcom_edit_set_property(GObject *object, guint property_id, const GValue *value,
                        GParamSpec *pspec)
{
  RtcomEditPrivate *priv = PRIVATE(object);

  switch (property_id)
  {
    case PROP_ITEMS_MASK:
    {
      priv->items_mask = g_value_get_uint(value);
      break;
    }
    case PROP_ACCOUNT:
    {
      g_assert(priv->account == NULL);
      priv->account = g_value_dup_object(value);
      break;
    }
    case PROP_EDIT_INFO_URI:
    {
      g_free(priv->edit_info_uri);
      priv->edit_info_uri = g_value_dup_string(value);
      break;
    }
    case PROP_USERNAME_FIELD:
    {
      g_free(priv->username_field);
      priv->username_field = g_value_dup_string(value);
      break;
    }
    case PROP_USERNAME_LABEL:
    {
      g_free(priv->username_label);
      priv->username_label = g_value_dup_string(value);
      break;
    }
    case PROP_USERNAME_INVALID_CHARS_RE:
    {
      g_free(priv->username_invalid_chars_re);
      priv->username_invalid_chars_re = g_value_dup_string(value);
      break;
    }
    case PROP_USERNAME_MUST_HAVE_AT_SEPARATOR:
    {
      priv->username_must_have_at = g_value_get_boolean(value);
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
    case PROP_USER_SERVER_SEPARATOR:
    {
      g_free(priv->at);
      priv->at = g_value_dup_string(value);
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
rtcom_edit_get_property(GObject *object, guint property_id, GValue *value,
                        GParamSpec *pspec)
{
  RtcomEditPrivate *priv = PRIVATE(object);

  switch (property_id)
  {
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
    case PROP_EDIT_INFO_URI:
    {
      g_value_set_string(value, priv->edit_info_uri);
      break;
    }
    case PROP_USERNAME_FIELD:
    {
      g_value_set_string(value, priv->username_field);
      break;
    }
    case PROP_USERNAME_LABEL:
    {
      g_value_set_string(value, priv->username_label);
      break;
    }
    case PROP_USERNAME_INVALID_CHARS_RE:
    {
      g_value_set_string(value, priv->username_invalid_chars_re);
      break;
    }
    case PROP_USERNAME_MUST_HAVE_AT_SEPARATOR:
    {
      g_value_set_boolean(value, priv->username_must_have_at);
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
    case PROP_USER_SERVER_SEPARATOR:
    {
      g_value_set_string(value, priv->at);
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
_edit_info_cb(GtkButton *button, RtcomEdit *self)
{
  RtcomEditPrivate *priv = PRIVATE(self);
  GError *error = NULL;

  g_return_if_fail(priv->edit_info_uri != NULL);

  if (!hildon_uri_open(priv->edit_info_uri, NULL, &error))
  {
    g_warning("%s: Failed to open browser: %s", __FUNCTION__, error->message);
    g_error_free(error);
  }
}

static void
on_content_resize(GtkWidget *widget, GtkRequisition *requisition,
                  RtcomEdit *self)
{
  gtk_widget_queue_resize(GTK_WIDGET(self));
}

static GObject *
rtcom_edit_constructor(GType type, guint n_construct_properties,
                       GObjectConstructParam *construct_properties)
{
  GObject *object = G_OBJECT_CLASS(rtcom_edit_parent_class)->constructor(
      type, n_construct_properties, construct_properties);
  RtcomEditPrivate *priv = PRIVATE(object);
  gboolean draft = FALSE;
  gboolean supports_avatar = FALSE;
  GtkWidget *edit_info_button = NULL;
  GtkWidget *enabled = NULL;
  GtkWidget *screen_label = NULL;
  const gchar *username_label_label;
  GtkWidget *username_label;
  GtkWidget *username_widget;
  GtkWidget *password_widget = NULL;
  GtkWidget *password_label = NULL;
  GtkWidget *align;
  GtkWidget *area;
  GtkWidget *vbox;
  guint rows = 3;
  guint top = 1;
  guint bottom = 2;

  priv->avatar_align = NULL;
  priv->avatar_label = NULL;

  area = g_object_new(HILDON_TYPE_PANNABLE_AREA, NULL);
  align = gtk_alignment_new(0.0, 0.5, 1.0, 1.0);
  vbox = gtk_vbox_new(FALSE, 0);
  priv->vbox = vbox;

  if (priv->account)
    draft = ACCOUNT_ITEM(priv->account)->draft;

  if (!draft)
  {
    if (priv->edit_info_uri)
    {
      edit_info_button = hildon_button_new_with_text(
          HILDON_SIZE_FINGER_HEIGHT, HILDON_BUTTON_ARRANGEMENT_HORIZONTAL,
          _("accounts_fi_edit_personal_info"), "");
      hildon_button_set_alignment(HILDON_BUTTON(edit_info_button),
                                  0.0, 0.5, 1.0, 1.0);
      g_signal_connect(edit_info_button, "clicked",
                       G_CALLBACK(_edit_info_cb), object);
    }

    enabled = (GtkWidget *)g_object_new(RTCOM_TYPE_ENABLED, NULL);

    if (priv->account)
    {
      const gchar *fmt = _("accounts_fi_edit_account_title");
      gchar *title =
        g_strdup_printf(fmt, ACCOUNT_ITEM(priv->account)->service_name);

      g_object_set(object, "title", title, NULL);
      g_free(title);
    }
  }

  if ((priv->items_mask & RTCOM_PLUGIN_CAPABILITY_ADVANCED))
  {
    priv->advanced_button = hildon_button_new_with_text(
        HILDON_SIZE_FINGER_HEIGHT, HILDON_BUTTON_ARRANGEMENT_HORIZONTAL,
        _("accounts_fi_advanced_settings"), "");
    priv->advanced_button = priv->advanced_button;
    hildon_button_set_alignment(HILDON_BUTTON(priv->advanced_button),
                                0.0, 0.5, 1.0, 1.0);
  }

  username_label_label = priv->username_label;

  if (!username_label_label || !*username_label_label)
    username_label_label = _("accounts_fi_user_name");

  username_label = g_object_new(
      GTK_TYPE_LABEL,
      "label", username_label_label,
      "xalign", 0.0,
      NULL);
  username_widget = g_object_new(
      RTCOM_TYPE_USERNAME,
      "field", priv->username_field,
      "sensitive", draft,
      "invalid-chars-re", priv->username_invalid_chars_re,
      "must-have-at-separator", priv->username_must_have_at,
      "required-server", priv->username_required_server,
      "required-server-error", priv->username_required_server_error,
      "msg-empty", priv->msg_empty,
      "user-server-separator", priv->at,
      NULL);

  if (priv->items_mask & RTCOM_PLUGIN_CAPABILITY_PASSWORD)
  {
    password_label = g_object_new(
          GTK_TYPE_LABEL,
          "label", _("accounts_fi_password"),
          "xalign", 0.0,
          NULL);
    password_widget = g_object_new(
          RTCOM_TYPE_PARAM_STRING,
          "field", "password",
          "can-next", FALSE,
          "visibility", FALSE,
          "required", TRUE,
          "max_length", 64,
          "msg_empty", priv->msg_empty,
          "hildon_input_mode", HILDON_GTK_INPUT_MODE_FULL |
          HILDON_GTK_INPUT_MODE_INVISIBLE,
          NULL);
    rows++;
  }

  if (priv->items_mask & RTCOM_PLUGIN_CAPABILITY_SCREEN_NAME)
  {
    screen_label = g_object_new(
        GTK_TYPE_LABEL,
        "label", _("accounts_fi_screen_name"),
        "xalign", 0.0,
        NULL);
    priv->screen_widget = g_object_new(RTCOM_TYPE_ALIAS, 0);
  }

  if (priv->account)
  {
    g_object_get(priv->account, "supports-avatar", &supports_avatar, NULL);

    if (supports_avatar)
    {
      priv->avatar_label = g_object_new(
          GTK_TYPE_LABEL,
          "label", _("accounts_fi_avatar"),
          "xalign", 0.0,
          NULL);
      priv->avatar_align = gtk_alignment_new(0.0, 0.5, 0.0, 1.0);
      gtk_container_add(GTK_CONTAINER(priv->avatar_align),
                        g_object_new(RTCOM_TYPE_AVATAR, NULL));
    }
  }

  priv->table = GTK_TABLE(gtk_table_new(rows, 2, FALSE));
  gtk_table_set_col_spacings(priv->table, 16);
  gtk_table_attach(priv->table, username_label, 0, 1, 0, 1,
                   GTK_FILL, GTK_SHRINK, 16, 0);
  gtk_table_attach(priv->table, username_widget, 1, 2, 0, 1,
                   GTK_FILL | GTK_EXPAND, GTK_SHRINK, 0, 0);

  if (priv->items_mask & RTCOM_PLUGIN_CAPABILITY_PASSWORD)
  {
    gtk_table_attach(priv->table, password_label, 0, 1, top, bottom,
                     GTK_FILL, GTK_SHRINK, 16, 0);
    gtk_table_attach(priv->table, password_widget, 1, 2, top++, bottom++,
                     GTK_FILL | GTK_EXPAND, GTK_SHRINK, 0, 0);
  }

  if (priv->items_mask & RTCOM_PLUGIN_CAPABILITY_SCREEN_NAME)
  {
    gtk_table_attach(priv->table, screen_label, 0, 1, top, bottom,
                     GTK_FILL, GTK_SHRINK, 16, 0);
    gtk_table_attach(priv->table, priv->screen_widget, 1, 2, top++, bottom++,
                     GTK_FILL | GTK_EXPAND, GTK_SHRINK, 0, 0);
  }

  if (supports_avatar)
  {
    gtk_table_attach(priv->table, priv->avatar_label, 0, 1, top, bottom,
                     GTK_FILL, GTK_SHRINK, 16, 0);
    gtk_table_attach(priv->table, priv->avatar_align, 1, 2, top++, bottom++,
                     GTK_FILL | GTK_EXPAND, GTK_SHRINK, 0, 0);
  }

  gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(priv->table), FALSE, FALSE, 0);

  if (!draft)
  {
    if (priv->edit_info_uri)
      gtk_box_pack_start(GTK_BOX(vbox), edit_info_button, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(vbox), enabled, FALSE, FALSE, 0);
  }

  if (priv->items_mask & RTCOM_PLUGIN_CAPABILITY_ADVANCED)
    gtk_box_pack_start(GTK_BOX(vbox), priv->advanced_button, FALSE, FALSE, 0);

  gtk_container_add(GTK_CONTAINER(align), vbox);
  hildon_pannable_area_add_with_viewport(HILDON_PANNABLE_AREA(area), align);
  g_signal_connect(vbox, "size-request",
                   G_CALLBACK(on_content_resize), object);
  gtk_container_add(GTK_CONTAINER(object), area);
  gtk_widget_show_all(GTK_WIDGET(object));

  return object;
}

static void
rtcom_edit_size_request(GtkWidget *widget, GtkRequisition *requisition)
{
  RtcomEditPrivate *priv = PRIVATE(widget);

  if (priv->vbox)
  {
    GtkRequisition r;
    GtkWidget *area;

    gtk_widget_size_request(priv->vbox, &r);
    area = gtk_widget_get_ancestor(priv->vbox, HILDON_TYPE_PANNABLE_AREA);

    if (r.height >= 350)
      r.height = 350;

    g_object_set(area, "height-request", r.height, NULL);
  }

  GTK_WIDGET_CLASS(rtcom_edit_parent_class)->size_request(widget, requisition);
}

static void
rtcom_edit_class_init(RtcomEditClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS(klass);

  object_class->dispose = rtcom_edit_dispose;
  object_class->finalize = rtcom_edit_finalize;
  object_class->set_property = rtcom_edit_set_property;
  object_class->get_property = rtcom_edit_get_property;
  object_class->constructor = rtcom_edit_constructor;

  GTK_WIDGET_CLASS(klass)->size_request = rtcom_edit_size_request;

  g_object_class_install_property(
    object_class, PROP_ITEMS_MASK,
    g_param_spec_uint(
      "items-mask",
      NULL, NULL,
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
    object_class, PROP_EDIT_INFO_URI,
    g_param_spec_string(
      "edit-info-uri",
      "URI for editing personal info",
      "edit info uri",
      NULL,
      G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE));
  g_object_class_install_property(
    object_class, PROP_USERNAME_FIELD,
    g_param_spec_string(
      "username-field",
      "Username field name",
      "TpAccount field name for the username",
      NULL,
      G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE));
  g_object_class_install_property(
    object_class, PROP_USERNAME_LABEL,
    g_param_spec_string(
      "username-label",
      "Username label",
      "username label",
      "",
      G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE));
  g_object_class_install_property(
    object_class, PROP_USERNAME_INVALID_CHARS_RE,
    g_param_spec_string(
      "username-invalid-chars-re",
      "Invalid chars reges for the username",
      "Regex to find forbidden characters in the username",
      NULL,
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
    object_class, PROP_USER_SERVER_SEPARATOR,
    g_param_spec_string(
      "user-server-separator",
      "User/server separator",
      "Separator between username and server in account string",
      "@",
      G_PARAM_CONSTRUCT_ONLY | GTK_PARAM_READWRITE));
}

static void
rtcom_edit_init(RtcomEdit *self)
{}

void
rtcom_edit_connect_on_advanced(RtcomEdit *edit,
                               GCallback cb,
                               gpointer user_data)
{
  g_signal_connect_swapped(PRIVATE(edit)->advanced_button, "clicked",
                           cb, user_data);
}

void
rtcom_edit_append_widget(RtcomEdit *edit, GtkWidget *label, GtkWidget *widget)
{
  RtcomEditPrivate *priv = PRIVATE(edit);

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

    if (priv->avatar_label)
      rows--;

    gtk_table_attach(priv->table, label, 0, 1, rows, rows + 1,
                     GTK_FILL, GTK_SHRINK, 16, 0);
    gtk_table_attach(priv->table, widget, 1, 2, rows, rows + 1,
                     GTK_FILL | GTK_EXPAND, GTK_SHRINK, 0, 0);

    /* keep avatar always the last element in the table */
    if (priv->avatar_label)
    {
      g_object_ref(priv->avatar_label);
      g_object_ref(priv->avatar_align);
      gtk_container_remove(GTK_CONTAINER(priv->table), priv->avatar_label);
      gtk_container_remove(GTK_CONTAINER(priv->table), priv->avatar_align);
      rows++;
      gtk_table_attach(priv->table, priv->avatar_label, 0, 1, rows, rows + 1,
                       GTK_FILL, GTK_SHRINK, 16, 0);
      gtk_table_attach(priv->table, priv->avatar_align, 1, 2, rows, rows + 1,
                       GTK_FILL | GTK_EXPAND, GTK_SHRINK, 0, 0);
      g_object_unref(priv->avatar_label);
      g_object_unref(priv->avatar_align);
    }

    gtk_widget_show(label);
  }

  gtk_widget_show(widget);
}
