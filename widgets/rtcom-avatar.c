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

#include <libintl.h>

#include "rtcom-avatar.h"

struct _RtcomAvatarPrivate
{
  gint flags;
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
      PRIVATE(avatar)->flags |= 1u;
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
        PRIVATE(avatar)->flags &= ~1u;
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

static void
rtcom_widget_init(RtcomWidgetIface *iface)
{
  /*iface->store_settings = rtcom_avatar_store_settings;
  iface->get_settings = rtcom_avatar_get_settings;*/
}
