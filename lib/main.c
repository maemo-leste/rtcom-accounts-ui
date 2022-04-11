/*
 * main.c
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

#include <libosso.h>

#include <rtcom-accounts-ui-client/client.h>

static void
accounts_client_status_update(AuicClient *client)
{
  if (!auic_client_is_ui_open(client))
  {
    gtk_main_quit();
    g_object_unref(client);
  }
}

osso_return_t
execute(osso_context_t *osso, gpointer data, gboolean user_activated)
{
  AuicClient *auic = auic_client_new(GTK_WINDOW(data));

  g_signal_connect(auic, "status-update",
                   G_CALLBACK(accounts_client_status_update), NULL);
  auic_client_open_accounts_list(auic);
  gtk_main();

  return 0;
}

osso_return_t
save_state(osso_context_t *osso, gpointer user_data)
{
  return OSSO_ERROR;
}
