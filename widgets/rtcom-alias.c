/*
 * rtcom-alias.c
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

#include "rtcom-alias.h"

RTCOM_DEFINE_WIDGET_TYPE(
  RtcomAlias,
  rtcom_alias,
  HILDON_TYPE_ENTRY
);

static GObject *
rtcom_alias_constructor(GType type, guint n_construct_properties,
                        GObjectConstructParam *construct_properties)
{
  GObject *object = G_OBJECT_CLASS(rtcom_alias_parent_class)->constructor(
      type, n_construct_properties, construct_properties);

  g_object_set(object, "hildon-input-mode",
               HILDON_GTK_INPUT_MODE_FULL | HILDON_GTK_INPUT_MODE_AUTOCAP,
               NULL);

  return object;
}

static void
rtcom_alias_invalid_input(GtkEntry *entry,
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
rtcom_alias_class_init(RtcomAliasClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS(klass);

  object_class->constructor = rtcom_alias_constructor;

  GTK_ENTRY_CLASS(klass)->invalid_input = rtcom_alias_invalid_input;
}

static void
rtcom_alias_init(RtcomAlias *alias)
{}

static gboolean
rtcom_alias_store_settings(RtcomWidget *widget, GError **error,
                           RtcomAccountItem *item)
{
  const gchar *text = hildon_entry_get_text(HILDON_ENTRY(widget));

  if (text && !*text)
    text = NULL;

  rtcom_account_item_store_nickname(item, text);

  return TRUE;
}

static void
rtcom_alias_get_settings(RtcomWidget *widget, RtcomAccountItem *item)
{
  const gchar *nickname = tp_account_get_nickname(item->account);

  if (!nickname)
    nickname = "";

  hildon_entry_set_text(HILDON_ENTRY(widget), nickname);
}

static void
rtcom_widget_init(RtcomWidgetIface *iface)
{
  iface->store_settings = rtcom_alias_store_settings;
  iface->get_settings = rtcom_alias_get_settings;
}
