/*
 * gui.h
 */

/*
**    Copyright (C) 1999, 2000,
**    Dirk-Jan C. Binnema <djcb@dds.nl>,
**    Arjan Scherpenisse <acscherp@wins.uva.nl>
**    Copyright (C) 2016 Colin Watson <cjwatson@debian.org>
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

#ifndef _GUI_H_
#define _GUI_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <glib-object.h>
#include <gio/gio.h>
#include <gtk/gtk.h>

#define TG_MAX_CHANNELS 100

int tg_gui_update_entry (gint page_nr, gint subpage_nr);
void tg_gui_get_the_page (gboolean redraw);


/* event handler callbacks */
void tg_gui_activate_quit (GSimpleAction *action, GVariant *parameter,
			   gpointer data);
void tg_gui_activate_help_contents (GSimpleAction *action, GVariant *parameter,
				    gpointer data);
void tg_gui_activate_about (GSimpleAction *action, GVariant *parameter,
			    gpointer data);
void tg_gui_activate_preferences (GSimpleAction *action, GVariant *parameter,
				  gpointer data);
void tg_gui_change_state_set_channel (GSimpleAction *action, GVariant *value,
				      gpointer data);
void tg_gui_cb_next_page (GtkWidget* widget, gpointer data);
void tg_gui_cb_prev_page (GtkWidget* widget, gpointer data);
void tg_gui_cb_home (GtkWidget* widget, gpointer data);
void tg_gui_cb_goto_page (GtkWidget* widget, gpointer data);
void tg_gui_cb_zoom (GtkWidget *widget, gpointer data);
gint tg_cb_keypress (GtkWidget *widget, GdkEventKey *event);

#define TG_TYPE_GUI             (tg_gui_get_type ())
G_DECLARE_FINAL_TYPE (TgGui, tg_gui, TG, GUI, GObject)

GType tg_gui_get_type (void);

TgGui *tg_gui_new (GtkApplication *app, GSettings *settings);

GtkWidget *tg_gui_get_window (TgGui *gui);

/* DnD target types */
enum {
	TARGET_GIF=0,
	TARGET_TXT,
	TARGET_FILE,
	TARGET_ROOTWIN
};

enum {
    INPUT_NEW=0,
    INPUT_CONTINUED
};

#endif /* _GUI_H_ */
