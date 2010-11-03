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



static void	pixpack_class_init	(PixPackClass	*klass);
static void	pixpack_init		(PixPack	*pixpack);
static void	pixpack_destroy		(GtkObject	*object);
static void	pixpack_realize		(GtkWidget	*widget);
static void	pixpack_unrealize	(GtkWidget	*widget);
static void     pixpack_paint		(PixPack	*pixpack,
					 GdkRectangle	*area);
static gint	pixpack_expose		(GtkWidget	*widget,
					 GdkEventExpose	*event);
static void	pixpack_size_request	(GtkWidget	*widget,
					 GtkRequisition	*allocation);

static GtkWidgetClass *parent_class = NULL;

GType
pixpack_get_type(void)
{
    static GType pixpack_type = 0;

    if (!pixpack_type) {
	static const GTypeInfo pixpack_info = {
	    sizeof (PixPackClass),
	    NULL,
	    NULL,
	    (GClassInitFunc) pixpack_class_init,
	    NULL,
	    NULL,
	    sizeof (PixPack),
	    0,
	    (GInstanceInitFunc) pixpack_init,
	};
	pixpack_type = g_type_register_static(GTK_TYPE_WIDGET, "PixPack",
					      &pixpack_info, 0);
    }
    return pixpack_type;
}


static void
pixpack_class_init(PixPackClass *klass)
{
    GtkObjectClass *object_class = (GtkObjectClass*) klass;
    GtkWidgetClass *widget_class = (GtkWidgetClass*) klass;

    parent_class = g_type_class_peek_parent(klass);

    object_class->destroy = pixpack_destroy;

    widget_class->realize = pixpack_realize;
    widget_class->unrealize = pixpack_unrealize;
    widget_class->expose_event = pixpack_expose;
    widget_class->size_request = pixpack_size_request;
}


static void
pixpack_init(PixPack *pixpack)
{
    PixPackPrivate *priv;
    priv = g_new0(PixPackPrivate, 1);

    priv->pixbuf = NULL;
    priv->scaled_pixbuf = NULL;
    priv->is_resize = FALSE;
    priv->autosize = FALSE;
    pixpack->private_data = priv;

    gtk_widget_set_can_focus(GTK_WIDGET(pixpack), TRUE);
    gtk_widget_set_receives_default(GTK_WIDGET(pixpack), TRUE);
    gtk_widget_set_has_window(GTK_WIDGET(pixpack), TRUE);
}

GtkWidget*
pixpack_new(void)
{
    PixPack *pixpack = gtk_type_new(pixpack_get_type());
    gdk_rgb_set_verbose(TRUE);
    gdk_rgb_init();
    return GTK_WIDGET(pixpack);
}



static void
pixpack_destroy(GtkObject *object)
{
    PixPack *pixpack;
    PixPackPrivate *private;

    g_return_if_fail(object);
    g_return_if_fail(IS_PIXPACK(object));

    pixpack = PIXPACK(object);
    private = pixpack->private_data;

    if (private) {
	if (private->pixbuf)
	    g_object_unref(private->pixbuf);
	if (private->scaled_pixbuf)
	    g_object_unref(private->scaled_pixbuf);

	g_free(pixpack->private_data);
	pixpack->private_data = NULL;
    }

    if (GTK_OBJECT_CLASS(parent_class)->destroy)
	(*GTK_OBJECT_CLASS(parent_class)->destroy)(object);
}

static void
pixpack_realize(GtkWidget *widget)
{
    GdkWindowAttr attributes;
    gint attributes_mask;
    PixPack *pixpack;
    GtkAllocation allocation;
    GdkWindow *window;

    g_return_if_fail(widget != NULL);
    g_return_if_fail(IS_PIXPACK(widget));

    pixpack = PIXPACK(widget);

    gtk_widget_set_realized(widget, TRUE);

    gtk_widget_get_allocation(widget, &allocation);

    attributes.window_type = GDK_WINDOW_CHILD;
    attributes.x = allocation.x;
    attributes.y = allocation.y;
    attributes.height = allocation.height;
    attributes.width = allocation.width;
    attributes.wclass = GDK_INPUT_OUTPUT;
    attributes.visual = gtk_widget_get_visual(widget);
    attributes.colormap = gtk_widget_get_colormap(widget);
    attributes.event_mask = gtk_widget_get_events(widget) | GDK_EXPOSURE_MASK;

    attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;

    window = gdk_window_new(gtk_widget_get_parent_window(widget),
			    &attributes, attributes_mask);
    gtk_widget_set_window(widget, window);
    gdk_window_set_user_data(window, widget);

    gtk_widget_style_attach(widget);
    gtk_style_set_background(gtk_widget_get_style(widget), window,
			     GTK_STATE_NORMAL);
}


static void
pixpack_unrealize(GtkWidget *widget)
{
    PixPack *pixpack;

    g_return_if_fail(widget != NULL);
    g_return_if_fail(IS_PIXPACK(widget));

    pixpack = PIXPACK(widget);

    if (gtk_widget_get_mapped(widget))
	gtk_widget_unmap(widget);

    gtk_widget_set_mapped(widget, FALSE);

    if (GTK_WIDGET_CLASS(parent_class)->unrealize)
	(*GTK_WIDGET_CLASS(parent_class)->unrealize)(widget);
}


static void
pixpack_paint(PixPack* pixpack, GdkRectangle *area)
{
    GtkWidget *widget;
    PixPackPrivate *private;
    GdkWindow *window;

    g_return_if_fail(pixpack != NULL);
    g_return_if_fail(IS_PIXPACK(pixpack));
    g_return_if_fail(pixpack->private_data != NULL);

    private = pixpack->private_data;

    if (!private->is_resize) {
	area->height = gdk_pixbuf_get_height(private->scaled_pixbuf);
	area->width = gdk_pixbuf_get_width(private->scaled_pixbuf);
	area->x = 0;
	area->y = 0;
    }

    widget = GTK_WIDGET(pixpack);
    if (!gtk_widget_is_drawable(widget))
	return;

    window = gtk_widget_get_window(widget);
    gdk_window_clear_area(window, area->x, area->y, area->width, area->height);
    gdk_gc_set_clip_rectangle(gtk_widget_get_style(widget)->black_gc, area);

    if (NULL == private->pixbuf)
	return;

    if (private->scaled_pixbuf)
	g_object_unref(private->scaled_pixbuf);

    private->scaled_pixbuf = gdk_pixbuf_scale_simple(private->pixbuf,
						     area->width, area->height,
						     GDK_INTERP_BILINEAR);
    gdk_pixbuf_render_to_drawable(private->scaled_pixbuf, window,
				  gtk_widget_get_style(widget)->black_gc, 0, 0,
				  area->x, area->y, area->width, area->height,
				  GDK_RGB_DITHER_MAX, 1, 1);

    private->is_resize = FALSE;
}


static gboolean
pixpack_expose(GtkWidget *widget, GdkEventExpose *event)
{
    pixpack_paint(PIXPACK (widget), &event->area);
    return TRUE;
}

static void
pixpack_size_request(GtkWidget *widget, GtkRequisition *req)
{
    PixPack *pixpack;
    PixPackPrivate *private;

    g_return_if_fail(IS_PIXPACK(widget));

    pixpack = PIXPACK(widget);
    private = pixpack->private_data;

    private->is_resize = TRUE;
}


void
pixpack_load_image(PixPack *pixpack, GdkPixbuf *pixbuf)
{
    PixPackPrivate *private;

    g_return_if_fail(IS_PIXPACK(pixpack));
    private = pixpack->private_data;

    if (private->pixbuf)
	g_object_unref(private->pixbuf);
    private->pixbuf = pixbuf;
    g_object_ref(private->pixbuf);

    /* this forces a repaint */

    if (private->autosize)
	gtk_widget_set_usize(GTK_WIDGET(pixpack),
			     gdk_pixbuf_get_width(private->pixbuf),
			     gdk_pixbuf_get_height(private->pixbuf));
    gtk_widget_queue_draw(GTK_WIDGET(pixpack));
}

void
pixpack_set_autosize(PixPack *pixpack, gboolean value)
{
	PixPackPrivate *private;
	g_return_if_fail(IS_PIXPACK(pixpack));
	private = pixpack->private_data;

	private->autosize = value;
}

gboolean
pixpack_get_autosize(PixPack *pixpack)
{
	PixPackPrivate *private;
	g_return_val_if_fail(IS_PIXPACK(pixpack), FALSE);
	private = pixpack->private_data;

	return private->autosize;
}
