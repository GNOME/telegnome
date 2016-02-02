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

#include <cairo.h>

#include "pixpack.h"

struct _TgPixPackPrivate {
    GdkPixbuf *pixbuf;

    gboolean is_resize;
    gboolean autosize;
};

typedef struct _TgPixPackPrivate TgPixPackPrivate;



static void	tg_pixpack_class_init	(TgPixPackClass	*klass);
static void	tg_pixpack_init		(TgPixPack	*pixpack);
static void	tg_pixpack_destroy	(GtkObject	*object);
static void	tg_pixpack_realize	(GtkWidget	*widget);
static void	tg_pixpack_unrealize	(GtkWidget	*widget);
static void     tg_pixpack_paint	(TgPixPack	*pixpack,
					 GdkRectangle	*area);
static gint	tg_pixpack_expose	(GtkWidget	*widget,
					 GdkEventExpose	*event);
static void	tg_pixpack_size_request	(GtkWidget	*widget,
					 GtkRequisition	*allocation);

static GtkWidgetClass *parent_class = NULL;

GType
tg_pixpack_get_type(void)
{
    static GType pixpack_type = 0;

    if (!pixpack_type) {
	static const GTypeInfo pixpack_info = {
	    sizeof (TgPixPackClass),
	    NULL,
	    NULL,
	    (GClassInitFunc) tg_pixpack_class_init,
	    NULL,
	    NULL,
	    sizeof (TgPixPack),
	    0,
	    (GInstanceInitFunc) tg_pixpack_init,
	};
	pixpack_type = g_type_register_static(GTK_TYPE_WIDGET, "TgPixPack",
					      &pixpack_info, 0);
    }
    return pixpack_type;
}


static void
tg_pixpack_class_init(TgPixPackClass *klass)
{
    GtkObjectClass *object_class = (GtkObjectClass*) klass;
    GtkWidgetClass *widget_class = (GtkWidgetClass*) klass;

    parent_class = g_type_class_peek_parent(klass);

    object_class->destroy = tg_pixpack_destroy;

    widget_class->realize = tg_pixpack_realize;
    widget_class->unrealize = tg_pixpack_unrealize;
    widget_class->expose_event = tg_pixpack_expose;
    widget_class->size_request = tg_pixpack_size_request;
}


static void
tg_pixpack_init(TgPixPack *pixpack)
{
    TgPixPackPrivate *priv;
    priv = g_new0(TgPixPackPrivate, 1);

    priv->pixbuf = NULL;
    priv->is_resize = FALSE;
    priv->autosize = FALSE;
    pixpack->private_data = priv;

    gtk_widget_set_can_focus(GTK_WIDGET(pixpack), TRUE);
    gtk_widget_set_receives_default(GTK_WIDGET(pixpack), TRUE);
    gtk_widget_set_has_window(GTK_WIDGET(pixpack), TRUE);
}

GtkWidget*
tg_pixpack_new(void)
{
    return GTK_WIDGET(g_object_new(TG_TYPE_PIXPACK, NULL));
}



static void
tg_pixpack_destroy(GtkObject *object)
{
    TgPixPack *pixpack;
    TgPixPackPrivate *private;

    g_return_if_fail(object);
    g_return_if_fail(TG_IS_PIXPACK(object));

    pixpack = TG_PIXPACK(object);
    private = pixpack->private_data;

    if (private) {
	if (private->pixbuf)
	    g_clear_object(&private->pixbuf);

	g_clear_pointer(&pixpack->private_data, g_free);
    }

    if (GTK_OBJECT_CLASS(parent_class)->destroy)
	(*GTK_OBJECT_CLASS(parent_class)->destroy)(object);
}

static void
tg_pixpack_realize(GtkWidget *widget)
{
    GdkWindowAttr attributes;
    gint attributes_mask;
    TgPixPack *pixpack;
    GtkAllocation allocation;
    GdkWindow *window;

    g_return_if_fail(widget != NULL);
    g_return_if_fail(TG_IS_PIXPACK(widget));

    pixpack = TG_PIXPACK(widget);

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
tg_pixpack_unrealize(GtkWidget *widget)
{
    TgPixPack *pixpack;

    g_return_if_fail(widget != NULL);
    g_return_if_fail(TG_IS_PIXPACK(widget));

    pixpack = TG_PIXPACK(widget);

    if (gtk_widget_get_mapped(widget))
	gtk_widget_unmap(widget);

    gtk_widget_set_mapped(widget, FALSE);

    if (GTK_WIDGET_CLASS(parent_class)->unrealize)
	(*GTK_WIDGET_CLASS(parent_class)->unrealize)(widget);
}


static void
tg_pixpack_paint(TgPixPack* pixpack, GdkRectangle *area)
{
    GtkWidget *widget;
    TgPixPackPrivate *private;
    GdkWindow *window;
    cairo_t *cr;

    g_return_if_fail(pixpack != NULL);
    g_return_if_fail(TG_IS_PIXPACK(pixpack));
    g_return_if_fail(pixpack->private_data != NULL);

    private = pixpack->private_data;

    if (!private->is_resize) {
	area->height = gdk_pixbuf_get_height(private->pixbuf);
	area->width = gdk_pixbuf_get_width(private->pixbuf);
	area->x = 0;
	area->y = 0;
    }

    widget = GTK_WIDGET(pixpack);
    if (!gtk_widget_is_drawable(widget))
	return;

    window = gtk_widget_get_window(widget);
    gdk_window_clear_area(window, area->x, area->y, area->width, area->height);

    if (NULL == private->pixbuf)
	return;

    cr = gdk_cairo_create(GDK_DRAWABLE(window));
    gdk_cairo_set_source_pixbuf(cr, private->pixbuf, 0, 0);
    cairo_rectangle(cr, area->x, area->y, area->width, area->height);
    cairo_clip(cr);
    cairo_scale(cr,
		area->width / gdk_pixbuf_get_width(private->pixbuf),
		area->height / gdk_pixbuf_get_height(private->pixbuf));
    cairo_paint(cr);
    cairo_destroy(cr);

    private->is_resize = FALSE;
}


static gboolean
tg_pixpack_expose(GtkWidget *widget, GdkEventExpose *event)
{
    tg_pixpack_paint(TG_PIXPACK (widget), &event->area);
    return TRUE;
}

static void
tg_pixpack_size_request(GtkWidget *widget, GtkRequisition *req)
{
    TgPixPack *pixpack;
    TgPixPackPrivate *private;

    g_return_if_fail(TG_IS_PIXPACK(widget));

    pixpack = TG_PIXPACK(widget);
    private = pixpack->private_data;

    private->is_resize = TRUE;
}


void
tg_pixpack_load_image(TgPixPack *pixpack, GdkPixbuf *pixbuf)
{
    TgPixPackPrivate *private;

    g_return_if_fail(TG_IS_PIXPACK(pixpack));
    private = pixpack->private_data;

    if (private->pixbuf)
	g_object_unref(private->pixbuf);
    private->pixbuf = pixbuf;
    g_object_ref(private->pixbuf);

    /* this forces a repaint */

    if (private->autosize)
	gtk_widget_set_size_request(GTK_WIDGET(pixpack),
				    gdk_pixbuf_get_width(private->pixbuf),
				    gdk_pixbuf_get_height(private->pixbuf));
    gtk_widget_queue_draw(GTK_WIDGET(pixpack));
}

void
tg_pixpack_set_autosize(TgPixPack *pixpack, gboolean value)
{
	TgPixPackPrivate *private;
	g_return_if_fail(TG_IS_PIXPACK(pixpack));
	private = pixpack->private_data;

	private->autosize = value;
}

gboolean
tg_pixpack_get_autosize(TgPixPack *pixpack)
{
	TgPixPackPrivate *private;
	g_return_val_if_fail(TG_IS_PIXPACK(pixpack), FALSE);
	private = pixpack->private_data;

	return private->autosize;
}
