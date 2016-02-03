/* app.h
 * Part of TeleGNOME, a GNOME app to view Teletext.
 * This file declares the main TeleGNOME application.
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

#ifndef _APP_H_
#define _APP_H_

#include <glib-object.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define TG_TYPE_APP		    (tg_app_get_type ())
G_DECLARE_FINAL_TYPE (TgApp, tg_app, TG, APP, GtkApplication)

GType tg_app_get_type (void);

TgApp *tg_app_new (void);

G_END_DECLS

#endif