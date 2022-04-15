/*
 * rtcom-displayname.c
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

#include <libintl.h>

#include "rtcom-displayname.h"

RTCOM_DEFINE_WIDGET_TYPE(
  RtcomDisplayname,
  rtcom_displayname,
  GTK_TYPE_ENTRY
);

static void
rtcom_displayname_invalid_input(GtkEntry *entry,
                                GtkInvalidInputType invalid_input_type)
{
  if (invalid_input_type == GTK_INVALID_INPUT_MAX_CHARS_REACHED)
  {
    hildon_banner_show_information(
      gtk_widget_get_toplevel(GTK_WIDGET(entry)), NULL,
      dgettext("hildon-common-strings",
               "ckdg_ib_maximum_characters_reached"));
  }
}

static void
rtcom_displayname_class_init(RtcomDisplaynameClass *klass)
{
  GTK_ENTRY_CLASS(klass)->invalid_input = rtcom_displayname_invalid_input;
}

static void
rtcom_displayname_init(RtcomDisplayname *displayname)
{}

static gboolean
rtcom_displayname_store_settings(RtcomWidget *widget, GError **error,
                                 RtcomAccountItem *item)
{
  const gchar *text;

  text = gtk_entry_get_text(GTK_ENTRY(widget));

  if (text && !*text)
    text = NULL;

  rtcom_account_item_store_display_name(item, text);

  return TRUE;
}

static void
rtcom_displayname_get_settings(RtcomWidget *widget, RtcomAccountItem *item)
{
  const gchar *name = tp_account_get_display_name(item->account);

  if (!name)
    name = "";

  gtk_entry_set_text(GTK_ENTRY(widget), name);
}

static void
rtcom_widget_init(RtcomWidgetIface *iface)
{
  iface->store_settings = rtcom_displayname_store_settings;
  iface->get_settings = rtcom_displayname_get_settings;
}
