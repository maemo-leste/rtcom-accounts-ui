/*
 * rtcom-page.c
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

#include "rtcom-account-marshal.h"

#include "rtcom-page.h"

G_DEFINE_TYPE(
  RtcomPage,
  rtcom_page,
  GTK_TYPE_BIN
)

enum
{
  PROP_VALID = 1,
  PROP_CAN_NEXT,
  PROP_LAST,
  PROP_TITLE
};

enum
{
  VALIDATE,
  SET_ACCOUNT,
  LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

#define FLAG_VALID 1
#define FLAG_CAN_NEXT 2

static gint global_flags = FLAG_CAN_NEXT;

static gboolean
stop_on_false_accumulator(GSignalInvocationHint *ihint, GValue *return_accu,
                          const GValue *handler_return, gpointer data)
{
  gboolean rv = g_value_get_boolean(handler_return);

  g_value_set_boolean(return_accu, rv);

  return rv;
}

static void
rtcom_page_set_property(GObject *object, guint property_id, const GValue *value,
                        GParamSpec *pspec)
{
  RtcomPage *page;

  g_return_if_fail(RTCOM_IS_PAGE(object));

  page = RTCOM_PAGE(object);

  switch (property_id)
  {
    case PROP_LAST:
    {
      page->last = g_value_get_boolean(value);
      break;
    }
    case PROP_TITLE:
    {
      g_free(page->title);
      page->title = g_value_dup_string(value);
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
rtcom_page_get_property(GObject *object,
                        guint property_id,
                        GValue *value,
                        GParamSpec *pspec)
{
  RtcomPage *page;

  g_return_if_fail(RTCOM_IS_PAGE(object));

  page = RTCOM_PAGE(object);

  switch (property_id)
  {
    case PROP_VALID:
    {
      g_value_set_boolean(value, page->flags & FLAG_VALID);
      break;
    }
    case PROP_CAN_NEXT:
    {
      g_value_set_boolean(value, page->flags & FLAG_CAN_NEXT);
      break;
    }
    case PROP_LAST:
    {
      g_value_set_boolean(value, page->last);
      break;
    }
    case PROP_TITLE:
    {
      g_value_set_string(value, page->title);
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
rtcom_page_finalize(GObject *object)
{
  RtcomPage *page = RTCOM_PAGE(object);

  g_free(page->title);
  g_hash_table_destroy(page->widgets);

  G_OBJECT_CLASS(rtcom_page_parent_class)->finalize(object);
}

static void
rtcom_page_size_request(GtkWidget *widget, GtkRequisition *requisition)
{
  GtkWidget *child = GTK_BIN(widget)->child;

  if (child)
    gtk_widget_size_request(child, requisition);
}

static void
rtcom_page_size_allocate(GtkWidget *widget, GtkAllocation *allocation)
{
  GtkWidget *child = GTK_BIN(widget)->child;

  widget->allocation = *allocation;

  if (child)
    gtk_widget_size_allocate(child, allocation);
}

static gboolean
_rtcom_page_validate(RtcomPage *page, GError **error)
{
  return TRUE;
}

static gboolean
widget_has_not_flag(gpointer key, gpointer value, gpointer user_data)
{
  return !(GPOINTER_TO_INT(user_data) & GPOINTER_TO_INT(value));
}

static gboolean
widgets_have_flag(RtcomPage *page, gint flag)
{
  return !g_hash_table_find(page->widgets, widget_has_not_flag,
                            GINT_TO_POINTER(flag));
}

static void
widget_can_next_changed(GtkWidget *widget, GParamSpec *pspec, RtcomPage *page)
{
  gint old_flags = GPOINTER_TO_INT(g_hash_table_lookup(page->widgets, widget));
  gint new_flags;
  gboolean can_next;

  g_object_get(G_OBJECT(widget), "can-next", &can_next, NULL);

  if (can_next)
    new_flags = old_flags | global_flags;
  else
    new_flags = old_flags & ~global_flags;

  g_hash_table_replace(page->widgets, widget, GINT_TO_POINTER(new_flags));

  if (global_flags & page->flags)
  {
    if (!can_next)
    {
      page->flags = ~global_flags & page->flags;
      g_object_notify(G_OBJECT(page), "can-next");
    }
  }
  else if (can_next && widgets_have_flag(page, global_flags))
  {
    page->flags |= global_flags;
    g_object_notify(G_OBJECT(page), "can-next");
  }
}

static void
rtcom_page_set_account_intern(GtkWidget *widget, gpointer user_data)
{
  gpointer *data = user_data;

  if (RTCOM_IS_WIDGET(widget))
  {
    RtcomPage *page = data[1];
    gboolean can_next;

    rtcom_widget_set_account(RTCOM_WIDGET(widget), data[0]);
    g_signal_connect(widget, "notify::can-next",
                     G_CALLBACK(widget_can_next_changed), page);
    g_object_get(widget, "can-next", &can_next, NULL);

    if (global_flags & page->flags)
    {
      if (!can_next)
      {
        page->flags = ~global_flags & page->flags;
        g_object_notify(G_OBJECT(page), "can-next");
      }
    }

    g_hash_table_insert(
      page->widgets, widget,
      GINT_TO_POINTER(0x8000 | (can_next ? PROP_CAN_NEXT : 0)));
    g_signal_connect_swapped(page, "validate",
                             G_CALLBACK(rtcom_widget_validate), widget);
  }
  else if (GTK_IS_CONTAINER(widget))
  {
    gtk_container_foreach(GTK_CONTAINER(widget),
                          rtcom_page_set_account_intern, data);
  }
}

static void
_rtcom_page_set_account(RtcomPage *page, RtcomAccountItem *account)
{
  gpointer data[2];

  data[0] = account;
  data[1] = page;
  gtk_container_foreach(&(GTK_BIN(page)->container),
                        rtcom_page_set_account_intern, data);
}

static void
rtcom_page_class_init(RtcomPageClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS(klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

  object_class->finalize = rtcom_page_finalize;
  object_class->set_property = rtcom_page_set_property;
  object_class->get_property = rtcom_page_get_property;

  widget_class->size_request = rtcom_page_size_request;
  widget_class->size_allocate = rtcom_page_size_allocate;

  klass->validate = _rtcom_page_validate;
  klass->set_account = _rtcom_page_set_account;

  g_object_class_install_property(
    object_class, PROP_VALID,
    g_param_spec_boolean(
      "valid",
      "Valid",
      "Valid", FALSE,
      G_PARAM_READABLE));
  g_object_class_install_property(
    object_class, PROP_CAN_NEXT,
    g_param_spec_boolean(
      "can-next",
      "Can next",
      "Allow page `Next' button",
      FALSE,
      G_PARAM_READABLE));
  g_object_class_install_property(
    object_class, PROP_LAST,
    g_param_spec_boolean(
      "last",
      "Is last",
      "Last page in the wizard",
      FALSE,
      G_PARAM_READWRITE));
  g_object_class_install_property(
    object_class, PROP_TITLE,
    g_param_spec_string(
      "title",
      "Page title",
      "Title of the page",
      NULL,
      G_PARAM_READWRITE));

  signals[VALIDATE] =
    g_signal_new("validate",
                 G_TYPE_FROM_CLASS(klass),
                 G_SIGNAL_ACTION | G_SIGNAL_RUN_LAST,
                 G_STRUCT_OFFSET(RtcomPageClass, validate),
                 stop_on_false_accumulator, NULL,
                 rtcom_account_marshal_BOOLEAN__POINTER,
                 G_TYPE_BOOLEAN,
                 1, G_TYPE_POINTER);
  signals[SET_ACCOUNT] =
    g_signal_new("set-account",
                 G_TYPE_FROM_CLASS(klass),
                 G_SIGNAL_ACTION | G_SIGNAL_RUN_LAST,
                 G_STRUCT_OFFSET(RtcomPageClass, set_account),
                 NULL, NULL,
                 g_cclosure_marshal_VOID__OBJECT,
                 G_TYPE_NONE,
                 1, RTCOM_TYPE_ACCOUNT_ITEM);
}

static void
rtcom_page_init(RtcomPage *page)
{
  page->flags = FLAG_CAN_NEXT;
  page->widgets = g_hash_table_new(NULL, NULL);
}

void
rtcom_page_set_account(RtcomPage *page, RtcomAccountItem *account)
{
  g_signal_emit(page, signals[SET_ACCOUNT], 0, account);
}

static void
get_flagged_widget(gpointer key, gpointer value, gpointer user_data)
{
  gpointer *data = user_data;

  if (!(GPOINTER_TO_INT(value) & GPOINTER_TO_INT(data[1])))
  {
    gint y;
    gint x;

    if (data[0])
      gtk_widget_translate_coordinates(data[0], key, 0, 0, &x, &y);

    if (!data[0] || (y > 0) || ((y == 0) && (x >= 0)))
      *data = key;
  }
}

gboolean
rtcom_page_validate(RtcomPage *page, GError **error)
{
  RtcomWidget *widget;
  gpointer data[2];
  gboolean can_next;

  g_return_val_if_fail(RTCOM_IS_PAGE(page), FALSE);

  g_object_get(page, "can-next", &can_next, NULL);

  if (can_next)
  {
    gboolean rv = TRUE;
    g_signal_emit(page, signals[VALIDATE], 0, error, &rv);
    return rv;
  }

  data[0] = NULL;
  data[1] = GINT_TO_POINTER(FLAG_CAN_NEXT);
  g_hash_table_foreach(page->widgets, get_flagged_widget, data);
  widget = data[0];

  if (!widget)
    return FALSE;

  if (error)
  {
    g_set_error(error, ACCOUNT_ERROR, ACCOUNT_ERROR_CANNOT_NEXT,
                "%s", rtcom_widget_get_msg_next(widget));
  }

  rtcom_widget_focus_error(widget);

  return FALSE;
}

const gchar *
rtcom_page_get_title(RtcomPage *page)
{
  return page->title;
}

void
rtcom_page_set_object_can_next(RtcomPage *page, GObject *object,
                               gboolean can_next)
{
  gint flags = 0x8000;

  g_return_if_fail(RTCOM_IS_PAGE (page));
  g_return_if_fail(G_IS_OBJECT (object));

  if (can_next)
    flags |= FLAG_CAN_NEXT;

  g_hash_table_replace(page->widgets, object, GINT_TO_POINTER(flags));

  if (page->flags & FLAG_CAN_NEXT)
  {
    if (!can_next)
    {
      page->flags &= ~FLAG_CAN_NEXT;
      g_object_notify(G_OBJECT(page), "can-next");
    }
  }
  else if (can_next && widgets_have_flag(page, FLAG_CAN_NEXT))
  {
    page->flags |= FLAG_CAN_NEXT;
    g_object_notify(G_OBJECT(page), "can-next");
  }
}
