/*
 * widgets.c
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

#include <glade/glade-build.h>
#include <glade/glade-init.h>

#include "rtcom-alias.h"
#include "rtcom-displayname.h"
#include "rtcom-page.h"
#include "rtcom-param-bool.h"
#include "rtcom-param-int.h"
#include "rtcom-param-string.h"
#include "rtcom-username.h"

static GtkSizeGroup *
create_sizegroup(gchar mode_char)
{
  GtkSizeGroupMode mode;

  switch (mode_char)
  {
    case 'h':
    {
      mode = GTK_SIZE_GROUP_HORIZONTAL;
      break;
    }
    case 'v':
    {
      mode = GTK_SIZE_GROUP_VERTICAL;
      break;
    }
    case 'b':
    {
      mode = GTK_SIZE_GROUP_BOTH;
      break;
    }
    default:
      g_warning("%s: Invalid mode '%c'", __FUNCTION__, mode_char);
      return NULL;
  }

  return gtk_size_group_new(mode);
}

static void
sizegroup_handler(GladeXML *xml, GtkWidget *widget, const gchar *propname,
                  const gchar *value)
{
  GObject *object = G_OBJECT(xml);
  GHashTable *groups = g_object_get_data(object, "sizegroups");
  GtkSizeGroup *group;

  if (!groups)
  {
    groups = g_hash_table_new_full((GHashFunc)&g_str_hash,
                                   (GEqualFunc)&g_str_equal,
                                   (GDestroyNotify)&g_free,
                                   (GDestroyNotify)&g_object_unref);
    g_return_if_fail(groups != NULL);
    g_object_set_data_full(object, "sizegroups", groups,
                           (GDestroyNotify)&g_hash_table_destroy);
  }

  group = g_hash_table_lookup(groups, value);

  if (!group)
  {
    group = create_sizegroup(*value);

    g_return_if_fail(group != NULL);

    g_hash_table_insert(groups, g_strdup(value), group);
    g_signal_connect(widget, "map",
                     G_CALLBACK(gtk_widget_queue_resize), NULL);
  }

  g_object_set(widget, "size-group", group, NULL);
}


static GtkWidget *
rtcom_param_bool_build_children(GladeXML *xml, GType widget_type,
                                GladeWidgetInfo *info)
{
  GtkWidget *widget = glade_standard_build_widget(xml, widget_type, info);

  hildon_gtk_widget_set_theme_size(widget, HILDON_SIZE_FINGER_HEIGHT);

  return widget;
}

GLADE_MODULE_CHECK_INIT void
glade_module_register_widgets(void)
{
  glade_require("gtk");

  glade_register_widget(RTCOM_TYPE_PAGE,
                        glade_standard_build_widget,
                        glade_standard_build_children,
                        NULL);
  glade_register_widget(RTCOM_TYPE_PARAM_STRING,
                        glade_standard_build_widget,
                        NULL,
                        NULL);
  glade_register_widget(RTCOM_TYPE_PARAM_BOOL,
                        rtcom_param_bool_build_children,
                        NULL,
                        NULL);
  glade_register_widget(RTCOM_TYPE_PARAM_INT,
                        glade_standard_build_widget,
                        NULL,
                        NULL);
  glade_register_widget(RTCOM_TYPE_USERNAME,
                        glade_standard_build_widget,
                        NULL,
                        NULL);
  glade_register_widget(RTCOM_TYPE_ALIAS,
                        glade_standard_build_widget,
                        NULL,
                        NULL);
  glade_register_widget(RTCOM_TYPE_DISPLAYNAME,
                        glade_standard_build_widget,
                        NULL,
                        NULL);

  glade_register_custom_prop(GTK_TYPE_WIDGET,
                             "sizegroup",
                             sizegroup_handler);

  glade_provide("osso-rtcom-accounts");
}
