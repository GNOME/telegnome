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

#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#ifdef __cplusplus 
extern "C" {
#endif /* __cplusplus */


#define TG_TYPE_PIXPACK			(tg_pixpack_get_type ())
#define TG_PIXPACK(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), TG_TYPE_PIXPACK, TgPixPack))
#define TG_PIXPACK_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), TG_TYPE_PIXPACK, TgPixPackClass))
#define TG_IS_PIXPACK(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), TG_TYPE_PIXPACK ))
#define TG_IS_PIXPACK_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), TG_TYPE_PIXPACK))
	
       
typedef struct _TgPixPack	TgPixPack;
typedef struct _TgPixPackClass	TgPixPackClass;
	
struct _TgPixPack {

	GtkWidget widget;
	gpointer private_data; 
};


struct _TgPixPackClass {
	
	GtkWidgetClass parent_class; /* parent class */
};


GType		tg_pixpack_get_type	( void );
void		tg_pixpack_load_image	( TgPixPack* pixpack, GdkPixbuf* pixbuf );
GtkWidget*	tg_pixpack_new		( void );
void		tg_pixpack_set_autosize	( TgPixPack* pixpack, gboolean value );


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _PIXPACK_H_ */
