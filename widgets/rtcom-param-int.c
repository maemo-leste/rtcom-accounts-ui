/*
 * rtcom-param-int.c
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

enum
{
  PROP_FIELD = 1,
  PROP_RANGE
};

#if 0
static void
rtcom_param_int_class_init(RtcomParamIntClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS(klass);

  object_class->dispose = rtcom_param_int_dispose;
  object_class->finalize = rtcom_param_int_finalize;
  object_class->set_property = rtcom_param_int_set_property;
  object_class->get_property = rtcom_param_int_get_property;
  object_class->constructor = rtcom_param_int_constructor;

  GTK_WIDGET_CLASS(klass)->focus_out_event = rtcom_param_int_focus_out_event;

  g_object_class_install_property(
    object_class, PROP_FIELD,
    g_param_spec_string(
          "field",
          "Field name",
          "TpAccount field name",
          NULL,
          G_PARAM_READWRITE));
  g_object_class_install_property(
    object_class, PROP_RANGE,
    g_param_spec_string(
          "range",
          "Range",
          "Valid range",
          NULL,
          G_PARAM_WRITABLE));
}
#endif
