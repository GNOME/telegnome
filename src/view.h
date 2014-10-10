/*
 * view.h
 * The viewer component of TeleGNOME
 */

/*
**    Copyright (C) 1999, 2000,
**    Dirk-Jan C. Binnema <djcb@dds.nl>,
**    Arjan Scherpenisse <acscherp@wins.uva.nl>
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

#ifndef _VIEW_H_
#define _VIEW_H_

#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "channel.h"

typedef struct _TgView {
    gfloat zoom_factor;
    void (*error_handler)(const char *);

    GtkWidget *pixpack;
    
    TgChannel *channel;

    int page_nr;
    int subpage_nr;

    /* the box */
    GtkWidget *box;
} TgView;
    
TgView *tg_view_new();
void tg_view_set_error_handler(TgView *view, void (*e)(const char *));
void tg_view_error(TgView *view, const char *c);
gint tg_view_update_pixmap(TgView *view, GdkPixbuf *pixbuf);
gint tg_view_update_page(TgView *view, int *major_nr, int *minor_nr);
GtkWidget *tg_view_get_widget(TgView *view);
void tg_view_free();
#endif

