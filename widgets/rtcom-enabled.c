/*
 * rtcom-enabled.c
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

#include "rtcom-enabled.h"

RTCOM_DEFINE_WIDGET_TYPE(
  RtcomEnabled,
  rtcom_enabled,
  HILDON_TYPE_CHECK_BUTTON
);

static GObject *
rtcom_enabled_constructor(GType type, guint n_construct_properties,
                          GObjectConstructParam *construct_properties)
{
  GObject *object = G_OBJECT_CLASS(rtcom_enabled_parent_class)->constructor(
      type, n_construct_properties, construct_properties);

  g_object_set(object, "xalign", 0.0, NULL);
  hildon_gtk_widget_set_theme_size(GTK_WIDGET(object),
                                   HILDON_SIZE_FINGER_HEIGHT);
  gtk_widget_set_name(GTK_WIDGET(object), "GtkButton-finger");
  gtk_button_set_label(GTK_BUTTON(object), _("accounts_fi_enabled"));

  return object;
}

static void
rtcom_enabled_class_init(RtcomEnabledClass *klass)
{
  G_OBJECT_CLASS(klass)->constructor = rtcom_enabled_constructor;
}

static void
rtcom_enabled_init(RtcomEnabled *self)
{
}

static gboolean
rtcom_enabled_store_settings(RtcomWidget *widget, GError **error,
                             RtcomAccountItem *item)
{
  rtcom_account_store_enabled_setting(
        item, hildon_check_button_get_active(HILDON_CHECK_BUTTON(widget)));

  return TRUE;
}

static void
rtcom_enabled_get_settings(RtcomWidget *widget, RtcomAccountItem *item)
{
  gboolean enabled;

  g_object_get(item, "enabled", &enabled, NULL);
  hildon_check_button_set_active(HILDON_CHECK_BUTTON(widget), enabled);
}

static void
rtcom_widget_init(RtcomWidgetIface *iface)
{
  iface->store_settings = rtcom_enabled_store_settings;
  iface->get_settings = rtcom_enabled_get_settings;
}

