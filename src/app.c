/* app.c
 * Part of TeleGNOME, a GNOME app to view Teletext.
 * This file defines the main TeleGNOME application.
 */

/*
** Copyright (C) 2016 Colin Watson <cjwatson@debian.org>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
**
*/

#include <string.h>

#include <glib-object.h>
#include <gio/gio.h>
#include <gtk/gtk.h>

#include "app.h"
#include "gui.h"
#include "legacy-config.h"

struct _TgApp {
    GtkApplication parent;
};

G_DEFINE_TYPE (TgApp, tg_app, GTK_TYPE_APPLICATION)

static void
tg_app_init (TgApp *app)
{
}

static void
tg_app_activate (GApplication *app)
{
    GSettings *settings;
    TgGui *gui;

    settings = g_settings_new (g_application_get_application_id (app));
    legacy_convert (settings);

    gui = tg_gui_new (GTK_APPLICATION (app), settings);
    gtk_window_present (GTK_WINDOW (tg_gui_get_window (gui)));
}

static void
tg_app_class_init (TgAppClass *klass)
{
    G_APPLICATION_CLASS (klass)->activate = tg_app_activate;
}

TgApp *
tg_app_new (void)
{
    return g_object_new (TG_TYPE_APP,
			 "application-id", "org.gnome.telegnome",
			 "flags", G_APPLICATION_NON_UNIQUE,
			 NULL);
}
