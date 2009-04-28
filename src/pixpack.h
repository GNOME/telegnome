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


#define TYPE_PIXPACK			(pixpack_get_type ())
#define PIXPACK(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_PIXPACK, PixPack))
#define PIXPACK_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_PIXPACK, PixPackClass))
#define IS_PIXPACK(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_PIXPACK ))
#define IS_PIXPACK_CLASS(klass)		(G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_PIXPACK))
	
       
typedef struct _PixPack		PixPack;
typedef struct _PixPackClass	PixPackClass;
	
struct _PixPack {

	GtkWidget widget;
	gpointer private_data; 
};


struct _PixPackClass {
	
	GtkWidgetClass parent_class; /* parent class */
};


GType		pixpack_get_type	( void );
void		pixpack_load_image	( PixPack* pixpack, GdkPixbuf* pixbuf );
GtkWidget*	pixpack_new		( void );
void		pixpack_set_autosize	( PixPack* pixpack, gboolean value );


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _PIXPACK_H_ */
