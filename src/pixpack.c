/*
 *  pixpack.c
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
#include "pixpack.h"

struct _PixPackPrivate {

	GdkPixbuf *pixbuf;
	GdkPixbuf *scaled_pixbuf;
	
	gboolean is_resize;
	gboolean autosize;
};

typedef struct _PixPackPrivate PixPackPrivate;



static void	pixpack_class_init	( PixPackClass	*klass );
static void	pixpack_init		( PixPack	*pixpack );
static void	pixpack_destroy		( GtkObject	*object );
static void	pixpack_realize		( GtkWidget *widget );
static void	pixpack_unrealize	( GtkWidget *widget );
static void	pixpack_draw		( GtkWidget *widget, GdkRectangle *area );
static void     pixpack_paint		( PixPack* pixpack, GdkRectangle *area );
static gint	pixpack_expose		( GtkWidget *widget, GdkEventExpose *event );
static void	pixpack_size_request	( GtkWidget *widget, GtkRequisition *allocation );

static GtkWidgetClass	*parent_class = NULL;

guint
pixpack_get_type (void)
{
	static guint pixpack_type = 0;

	if (!pixpack_type) {
		
		static const GtkTypeInfo pixpack_info = {
			"PixPack",
			sizeof (PixPack),
			sizeof (PixPackClass),
			(GtkClassInitFunc)  pixpack_class_init,
			(GtkObjectInitFunc) pixpack_init,
			NULL, NULL,
			(GtkClassInitFunc) NULL,
		};
		pixpack_type = gtk_type_unique ( gtk_widget_get_type (), &pixpack_info );
	}
	return pixpack_type;
}


static void
pixpack_class_init ( PixPackClass* klass )
{
	GtkObjectClass *object_class		= (GtkObjectClass*) klass;
	GtkWidgetClass *widget_class		= (GtkWidgetClass*) klass;
	
	parent_class = gtk_type_class ( gtk_widget_get_type() );	

	object_class->destroy		= pixpack_destroy;


	widget_class->realize		= pixpack_realize;
	widget_class->unrealize		= pixpack_unrealize;
	widget_class->draw		= pixpack_draw;
	widget_class->expose_event	= pixpack_expose;
	widget_class->size_request	= pixpack_size_request;
}


static void
pixpack_init ( PixPack *pixpack )
{
	PixPackPrivate *priv;
	priv = g_new0 ( PixPackPrivate, 1 );

	pixpack->private_data = priv;
	((PixPackPrivate*)pixpack -> private_data) -> pixbuf = NULL;
	((PixPackPrivate*)pixpack -> private_data) -> scaled_pixbuf = NULL;
	((PixPackPrivate*)pixpack -> private_data) -> is_resize = FALSE;
	((PixPackPrivate*)pixpack -> private_data) -> autosize = FALSE;

	GTK_WIDGET_SET_FLAGS   ( GTK_WIDGET (pixpack), GTK_CAN_FOCUS | GTK_RECEIVES_DEFAULT);
	GTK_WIDGET_UNSET_FLAGS ( GTK_WIDGET (pixpack), GTK_NO_WINDOW);
}

GtkWidget*
pixpack_new (void)
{
	PixPack *pixpack = gtk_type_new ( pixpack_get_type ());
	gdk_rgb_set_verbose(TRUE);
	gdk_rgb_init();
	return GTK_WIDGET(pixpack);
}



static void 
pixpack_destroy ( GtkObject *object )
{
	PixPack *pixpack;
	PixPackPrivate* private;
	
	g_return_if_fail ( object );
	g_return_if_fail ( IS_PIXPACK (object));

	pixpack = PIXPACK (object);
	private = pixpack -> private_data;

	if ( private -> pixbuf )
		gdk_pixbuf_unref ( private -> pixbuf );
	if ( private -> scaled_pixbuf )
		gdk_pixbuf_unref ( private -> scaled_pixbuf );

	g_free ( pixpack->private_data );

	if ( GTK_OBJECT_CLASS(parent_class)->destroy)
		(*GTK_OBJECT_CLASS(parent_class)->destroy) (object);
	
}

static void
pixpack_realize ( GtkWidget *widget )
{
	GdkWindowAttr attributes;
	gint attributes_mask;
	PixPack *pixpack;

	g_return_if_fail ( widget != NULL );
	g_return_if_fail ( IS_PIXPACK (widget));
	
	pixpack = PIXPACK ( widget );

	GTK_WIDGET_SET_FLAGS ( widget, GTK_REALIZED );

	attributes.window_type	= GDK_WINDOW_CHILD;
	attributes.x		= widget -> allocation.x;
	attributes.y		= widget -> allocation.y;
	attributes.height	= widget -> allocation.height;
	attributes.width	= widget -> allocation.width;
	attributes.wclass	= GDK_INPUT_OUTPUT;
	attributes.visual	= gtk_widget_get_visual ( widget );
	attributes.colormap	= gtk_widget_get_colormap ( widget );
	attributes.event_mask	= gtk_widget_get_events ( widget ) | GDK_EXPOSURE_MASK;

	attributes_mask		= GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;

	widget -> window = gdk_window_new ( gtk_widget_get_parent_window ( widget), &attributes, attributes_mask );
	gdk_window_set_user_data ( widget -> window, widget );
	
	widget -> style = gtk_style_attach ( widget->style, widget->window );
	gtk_style_set_background ( widget -> style, widget->window, GTK_STATE_NORMAL );
}


static void
pixpack_unrealize ( GtkWidget *widget )
{
	
	PixPack *pixpack;
	
	g_return_if_fail ( widget != NULL );
	g_return_if_fail ( IS_PIXPACK ( widget ));

	pixpack = PIXPACK ( widget );
	
	if ( GTK_WIDGET_MAPPED (widget))
		gtk_widget_unmap (widget);

	GTK_WIDGET_UNSET_FLAGS ( widget, GTK_MAPPED );

	if ( GTK_WIDGET_CLASS ( parent_class ) -> unrealize)
		(* GTK_WIDGET_CLASS (parent_class)->unrealize) ( widget );
	
}


static void
pixpack_draw ( GtkWidget *widget, GdkRectangle *area )
{
	PixPack *pixpack;
	
	g_return_if_fail ( widget != NULL );
	g_return_if_fail ( IS_PIXPACK (widget));

	pixpack = PIXPACK(widget);
	
	pixpack_paint ( pixpack, area );
}



static void
pixpack_paint ( PixPack* pixpack, GdkRectangle *area )
{
	GtkWidget *widget;
	PixPackPrivate* private;
	gint height, width, x, y;

	g_return_if_fail ( pixpack != NULL );
	g_return_if_fail ( IS_PIXPACK ( pixpack ));
	g_return_if_fail ( pixpack -> private_data != NULL );

	private = pixpack -> private_data;

	if ( !private -> is_resize ) {
	
		area -> height  =  gdk_pixbuf_get_height   ( private -> scaled_pixbuf );
		area -> width  =  gdk_pixbuf_get_width    ( private -> scaled_pixbuf );
		area -> x      =  0;
		area -> y      =  0;
	}

	widget = GTK_WIDGET ( pixpack );
	if ( ! GTK_WIDGET_DRAWABLE (widget))
		return;

	gdk_window_clear_area ( widget -> window, x, y, width, height);
	gdk_gc_set_clip_rectangle ( widget->style->black_gc, area );

	if ( NULL == private -> pixbuf ) 
		return;

	if ( private -> scaled_pixbuf )
		gdk_pixbuf_unref ( private -> scaled_pixbuf );
	
	private  -> scaled_pixbuf = gdk_pixbuf_scale_simple (private -> pixbuf, 
							     area->width, area->height, 
							     GDK_INTERP_BILINEAR );	
	gdk_pixbuf_render_to_drawable ( private -> scaled_pixbuf, widget->window, widget->style->black_gc, 
					0, 0, area->x, area->y, area->width, area->height, GDK_RGB_DITHER_MAX, 1, 1);

	private -> is_resize = FALSE;
}


static gint
pixpack_expose ( GtkWidget *widget, GdkEventExpose *event )
{
	pixpack_paint ( PIXPACK (widget), &event->area );
}

static void
pixpack_size_request ( GtkWidget *widget, GtkRequisition* req )
{
	PixPack* pixpack;
	PixPackPrivate* private;

	g_return_if_fail (IS_PIXPACK(widget));

	pixpack = PIXPACK (widget);
	private = pixpack -> private_data;

	private -> is_resize = TRUE;
}


void		
pixpack_load_image_file ( PixPack* pixpack, gchar* filename )
{
	PixPackPrivate* private;
	g_return_if_fail ( IS_PIXPACK (pixpack ));
	private = pixpack -> private_data;

	private -> pixbuf = gdk_pixbuf_new_from_file ( filename );
	
	/* this forces a repaint */

	if ( private -> autosize ) 
		gtk_widget_set_usize( GTK_WIDGET(pixpack),
			              gdk_pixbuf_get_width( private -> pixbuf ),
			              gdk_pixbuf_get_height( private -> pixbuf ));
	else
		gtk_widget_queue_draw( GTK_WIDGET(pixpack) );
}

void 
pixpack_set_autosize( PixPack *pixpack, gboolean value )
{
	PixPackPrivate* private;
	g_return_if_fail ( IS_PIXPACK (pixpack ));
	private = pixpack -> private_data;
    
	private->autosize = value;
}

gboolean
pixpack_get_autosize( PixPack *pixpack )
{
	PixPackPrivate* private;
	g_return_if_fail ( IS_PIXPACK (pixpack ));
	private = pixpack -> private_data;
    
	return private -> autosize;
}
