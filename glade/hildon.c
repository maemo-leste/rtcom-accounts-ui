/*
 * hildon.c
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
#include <hildon/hildon.h>

static GtkWidget *
set_theme_size(GladeXML *xml, GType widget_type, GladeWidgetInfo *info)
{
  GtkWidget *widget = glade_standard_build_widget(xml, widget_type, info);

  hildon_gtk_widget_set_theme_size(widget, HILDON_SIZE_FINGER_HEIGHT);

  return widget;
}

GLADE_MODULE_CHECK_INIT void
glade_module_register_widgets(void)
{
  glade_register_widget(HILDON_TYPE_CAPTION, glade_standard_build_widget,
                        glade_standard_build_children, NULL);
  glade_register_widget(HILDON_TYPE_FIND_TOOLBAR, glade_standard_build_widget,
                        glade_standard_build_children, NULL);
  glade_register_widget(HILDON_TYPE_WINDOW, glade_standard_build_widget,
                        glade_standard_build_children, NULL);
  glade_register_widget(HILDON_TYPE_COLOR_BUTTON, glade_standard_build_widget,
                        NULL, NULL);
  glade_register_widget(HILDON_TYPE_CHECK_BUTTON, set_theme_size,
                        NULL, NULL);
  glade_register_widget(HILDON_TYPE_PANNABLE_AREA, glade_standard_build_widget,
                        glade_standard_build_children, NULL);
  glade_register_widget(HILDON_TYPE_BUTTON, glade_standard_build_widget,
                        glade_standard_build_children, NULL);
  glade_register_widget(HILDON_TYPE_DIALOG, glade_standard_build_widget,
                        glade_standard_build_children, NULL);
  glade_register_widget(HILDON_TYPE_PICKER_BUTTON, set_theme_size,
                        NULL, NULL);
  glade_register_widget(HILDON_TYPE_TIME_EDITOR, glade_standard_build_widget,
                        NULL, NULL);
  glade_register_widget(HILDON_TYPE_NUMBER_EDITOR, glade_standard_build_widget,
                        NULL, NULL);
  glade_register_widget(HILDON_TYPE_CONTROLBAR, glade_standard_build_widget,
                        NULL, NULL);
  glade_register_widget(HILDON_TYPE_SEEKBAR, glade_standard_build_widget,
                        NULL, NULL);
  glade_register_widget(HILDON_TYPE_HVOLUMEBAR, glade_standard_build_widget,
                        NULL, NULL);
  glade_register_widget(HILDON_TYPE_VVOLUMEBAR, glade_standard_build_widget,
                        NULL, NULL);
  glade_register_widget(HILDON_TYPE_FONT_SELECTION_DIALOG,
                        glade_standard_build_widget, NULL, NULL);
  glade_register_widget(HILDON_TYPE_GET_PASSWORD_DIALOG,
                        glade_standard_build_widget, NULL, NULL);
  glade_register_widget(HILDON_TYPE_SET_PASSWORD_DIALOG,
                        glade_standard_build_widget, NULL, NULL);
  glade_register_widget(HILDON_TYPE_NOTE, glade_standard_build_widget,
                        NULL, NULL);
  glade_register_widget(HILDON_TYPE_SORT_DIALOG, glade_standard_build_widget,
                        NULL, NULL);
  glade_register_widget(HILDON_TYPE_WIZARD_DIALOG, glade_standard_build_widget,
                        NULL, NULL);
  glade_provide("rtcom-hildon");
}
