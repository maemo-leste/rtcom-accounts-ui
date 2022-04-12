/*
 * rtcom-avatar.c
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
#include <libosso-abook/osso-abook-avatar-chooser-dialog.h>
#include <libosso-abook/osso-abook-avatar-editor-dialog.h>
#include <libosso-abook/osso-abook-avatar-image.h>
#include <libosso-abook/osso-abook-icon-sizes.h>
#include <telepathy-glib/telepathy-glib.h>

#include <libintl.h>

#include "rtcom-avatar.h"

struct _RtcomAvatarPrivate
{
  gboolean check_size;
};

typedef struct _RtcomAvatarPrivate RtcomAvatarPrivate;

#define PRIVATE(avatar) \
  ((RtcomAvatarPrivate *) \
   rtcom_avatar_get_instance_private((RtcomAvatar *)(avatar)))

RTCOM_DEFINE_WIDGET_TYPE_WITH_PRIVATE(
  RtcomAvatar,
  rtcom_avatar,
  GTK_TYPE_EVENT_BOX
);

static void
rtcom_avatar_dispose(GObject *object)
{
  RtcomAvatar *avatar = RTCOM_AVATAR(object);

  if (avatar->src)
  {
    g_object_unref(avatar->src);
    avatar->src = NULL;
  }

  G_OBJECT_CLASS(rtcom_avatar_parent_class)->dispose(object);
}

static void
rtcom_avatar_class_init(RtcomAvatarClass *klass)
{
  G_OBJECT_CLASS(klass)->dispose = rtcom_avatar_dispose;
}

static void
_save_and_scale_avatar(RtcomAvatar *avatar, GdkPixbuf *pixbuf)
{
  GdkPixbuf *scaled; // r4

  g_return_if_fail(avatar);
  g_return_if_fail(pixbuf);

  if (avatar->src)
    g_object_unref(avatar->src);

  avatar->src = g_object_ref(pixbuf);
  scaled = gdk_pixbuf_scale_simple(pixbuf, HILDON_ICON_PIXEL_SIZE_FINGER,
                                   HILDON_ICON_PIXEL_SIZE_FINGER,
                                   GDK_INTERP_NEAREST);
  osso_abook_avatar_image_set_pixbuf(
    OSSO_ABOOK_AVATAR_IMAGE(avatar->image),
    scaled);
  g_object_unref(scaled);
}

static void
_avatar_editor_response_cb(OssoABookAvatarEditorDialog *editor,
                           gint response_id, RtcomAvatar *avatar)
{
  if (response_id == GTK_RESPONSE_OK)
  {
    GdkPixbuf *pixbuf =
      osso_abook_avatar_editor_dialog_get_scaled_pixbuf(editor);

    if (pixbuf)
    {
      PRIVATE(avatar)->check_size = TRUE;
      _save_and_scale_avatar(avatar, pixbuf);
    }
  }

  gtk_widget_destroy(GTK_WIDGET(editor));
}

static void
_avatar_chooser_response_cb(OssoABookAvatarChooserDialog *chooser,
                            gint response_id, RtcomAvatar *avatar)
{
  if (response_id == GTK_RESPONSE_OK)
  {
    GdkPixbuf *pixbuf = osso_abook_avatar_chooser_dialog_get_pixbuf(chooser);
    const char *icon_name =
      osso_abook_avatar_chooser_dialog_get_icon_name(chooser);

    if (icon_name)
    {
      if (!strcmp(icon_name, "general_default_avatar"))
      {
        osso_abook_avatar_image_set_pixbuf(
          OSSO_ABOOK_AVATAR_IMAGE(avatar->image), NULL);
        g_object_unref(avatar->src);
        avatar->src = NULL;
      }
      else if (pixbuf)
      {
        PRIVATE(avatar)->check_size = FALSE;
        _save_and_scale_avatar(avatar, pixbuf);
      }
    }
    else if (pixbuf)
    {
      GtkWidget *editor = osso_abook_avatar_editor_dialog_new(
          gtk_window_get_transient_for(GTK_WINDOW(chooser)), pixbuf);

      g_signal_connect(editor, "response",
                       G_CALLBACK(_avatar_editor_response_cb), avatar);
      gtk_widget_show(editor);
    }
  }

  gtk_widget_destroy(&chooser->parent.window.bin.container.widget);
}

static gboolean
_avatar_clicked_cb(GtkWidget *self, GdkEventButton *event, gpointer user_data)
{
  RtcomAvatar *avatar = user_data;
  GtkWidget *ancestor = gtk_widget_get_ancestor(avatar->image, GTK_TYPE_WINDOW);
  GtkWidget *chooser =
    osso_abook_avatar_chooser_dialog_new(GTK_WINDOW(ancestor));

  g_signal_connect(chooser, "response",
                   G_CALLBACK(_avatar_chooser_response_cb), avatar);
  gtk_widget_show(chooser);

  return TRUE;
}

static void
rtcom_avatar_init(RtcomAvatar *avatar)
{
  avatar->image = osso_abook_avatar_image_new();

  osso_abook_avatar_image_set_size(
    OSSO_ABOOK_AVATAR_IMAGE(avatar->image),
    hildon_get_icon_pixel_size(HILDON_ICON_SIZE_FINGER));
  osso_abook_avatar_image_set_maximum_zoom(
    OSSO_ABOOK_AVATAR_IMAGE(avatar->image), 1.0);
  gtk_container_add(GTK_CONTAINER(avatar), avatar->image);
  g_signal_connect(avatar, "button_press_event",
                   G_CALLBACK(_avatar_clicked_cb), avatar);
}

static gboolean
rtcom_avatar_store_settings(RtcomWidget *widget, GError **error,
                            RtcomAccountItem *item)
{
  RtcomAvatar *avatar = RTCOM_AVATAR(widget);
  RtcomAvatarPrivate *priv;
  AccountService *service;
  TpProtocol *protocol;
  const char *mime = NULL;
  const char *p;
  const char *type;
  GdkPixbuf *scaled = NULL;
  GError *local_error = NULL;
  gsize buffer_size;
  gchar *buffer;

  if (!avatar->src)
  {
    tp_account_set_avatar_async(item->account, NULL, 0, NULL, NULL, NULL);
    return TRUE;
  }

  priv = PRIVATE(avatar);

  service = account_item_get_service(ACCOUNT_ITEM(item));
  protocol = rtcom_account_service_get_protocol(RTCOM_ACCOUNT_SERVICE(service));

  if (protocol)
  {
    TpAvatarRequirements *req = tp_protocol_get_avatar_requirements(protocol);

    if (req && req->supported_mime_types)
      mime = req->supported_mime_types[0];
  }

  if (!protocol || !mime)
    mime = "image/png";

  p = strrchr(mime, '/');

  if (p)
    type = p + 1;
  else
  {
    g_warning("%s: Unexpected mime type: %s", __FUNCTION__, mime);
    type = mime;
  }

  if (priv->check_size)
  {
    if ((gdk_pixbuf_get_width(avatar->src) >
         OSSO_ABOOK_PIXEL_SIZE_AVATAR_MEDIUM) ||
        (gdk_pixbuf_get_height(avatar->src) >
         OSSO_ABOOK_PIXEL_SIZE_AVATAR_MEDIUM))
    {
      scaled = gdk_pixbuf_scale_simple(avatar->src,
                                       OSSO_ABOOK_PIXEL_SIZE_AVATAR_MEDIUM,
                                       OSSO_ABOOK_PIXEL_SIZE_AVATAR_MEDIUM,
                                       GDK_INTERP_BILINEAR);
    }
  }

  if (scaled)
  {
    gdk_pixbuf_save_to_buffer(
      scaled, &buffer, &buffer_size, type, &local_error, NULL);
    g_object_unref(scaled);
  }
  else
  {
    gdk_pixbuf_save_to_buffer(
      avatar->src, &buffer, &buffer_size, type, &local_error, NULL);
  }

  if (!local_error)
    rtcom_account_item_store_avatar(item, buffer, buffer_size, mime);
  else
  {
    g_warning("%s, Failed to save avatar: %s", __FUNCTION__,
              local_error->message);
    g_error_free(local_error);
  }

  return TRUE;
}

static void
_get_avatar_cb(TpProxy *proxy, const GValue *out_Value,
               const GError *error, gpointer user_data,
               GObject *weak_object)
{
  gpointer *data = user_data;

  if (error)
    g_warning("%s: Could not get avatar data %s", __FUNCTION__, error->message);
  else if (!G_VALUE_HOLDS(out_Value, TP_STRUCT_TYPE_AVATAR))
  {
    g_warning("%s: Avatar had wrong type: %s", __FUNCTION__,
              G_VALUE_TYPE_NAME(out_Value));
  }
  else
  {
    RtcomAvatar *avatar = RTCOM_AVATAR(data[1]);
    GValueArray *array = g_value_get_boxed(out_Value);
    const GArray *avatar_array;
    const gchar *mime_type;
    GdkPixbufLoader *loader = gdk_pixbuf_loader_new();

    tp_value_array_unpack(array, 2, &avatar_array, &mime_type);

    if (gdk_pixbuf_loader_write(loader, (guchar *)avatar_array->data,
                                avatar_array->len, NULL))
    {
      avatar->src = gdk_pixbuf_loader_get_pixbuf(loader);

      if (avatar->src)
      {
        g_object_ref(avatar->src);
        osso_abook_avatar_image_set_pixbuf(
              OSSO_ABOOK_AVATAR_IMAGE(avatar->image), avatar->src);
      }
    }

    gdk_pixbuf_loader_close(loader, NULL);
    g_object_unref(loader);
  }

  g_main_loop_quit(data[0]);
}

static void
rtcom_avatar_get_settings(RtcomWidget *widget, RtcomAccountItem *item)
{
  gpointer data[2];

  if (!item->account)
    return;

  data[0] = g_main_loop_new(NULL, FALSE);
  data[1] = g_object_ref(widget);

  tp_cli_dbus_properties_call_get(
    item->account, -1, TP_IFACE_ACCOUNT_INTERFACE_AVATAR, "Avatar",
    _get_avatar_cb, data, NULL, NULL);

  GDK_THREADS_LEAVE();
  g_main_loop_run(data[0]);
  GDK_THREADS_ENTER();

  g_main_loop_unref(data[0]);
  g_object_unref(data[1]);
}

static void
rtcom_widget_init(RtcomWidgetIface *iface)
{
  iface->store_settings = rtcom_avatar_store_settings;
  iface->get_settings = rtcom_avatar_get_settings;
}
