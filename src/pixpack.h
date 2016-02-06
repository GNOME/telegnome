/*
 * pixpack.h 
 */

/*
** Copyright (C) 2000 Dirk-Jan C. Binnema <djcb@dds.nl>
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
    

#ifndef _PIXPACK_H_
#define _PIXPACK_H_

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

G_BEGIN_DECLS

#define TG_TYPE_PIXPACK			(tg_pixpack_get_type ())
G_DECLARE_FINAL_TYPE (TgPixpack, tg_pixpack, TG, PIXPACK, GtkWidget)

GType		tg_pixpack_get_type	(void);
void		tg_pixpack_load_image	(TgPixpack* pixpack, GdkPixbuf* pixbuf);
GtkWidget*	tg_pixpack_new		(void);

G_END_DECLS

#endif /* _PIXPACK_H_ */
