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

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <cairo.h>

#include "pixpack.h"

struct _TgPixpack {
    GtkWidget parent_instance;

    GdkPixbuf *pixbuf;

    gboolean autosize;
};

enum {
    PROP_0,

    PROP_AUTOSIZE
};

G_DEFINE_TYPE (TgPixpack, tg_pixpack, GTK_TYPE_WIDGET)


static void	tg_pixpack_init		(TgPixpack	*pixpack);
static void	tg_pixpack_get_property	(GObject	*object,
					 guint		 property_id,
					 GValue		*value,
					 GParamSpec	*pspec);
static void	tg_pixpack_set_property	(GObject	*object,
					 guint		 property_id,
					 const GValue	*value,
					 GParamSpec	*pspec);
static void	tg_pixpack_destroy	(GtkWidget	*widget);
static void	tg_pixpack_realize	(GtkWidget	*widget);
static void	tg_pixpack_unrealize	(GtkWidget	*widget);
static gboolean	tg_pixpack_draw		(GtkWidget	*widget,
					 cairo_t	*cr);

static void
tg_pixpack_class_init(TgPixpackClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

    gobject_class->get_property = tg_pixpack_get_property;
    gobject_class->set_property = tg_pixpack_set_property;

    g_object_class_install_property
	(gobject_class, PROP_AUTOSIZE,
	 g_param_spec_boolean("autosize",
			      "Autosize",
			      "If true, set the widget's size request to the "
			      "size of the image.",
			      FALSE,
			      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

    widget_class->destroy = tg_pixpack_destroy;
    widget_class->realize = tg_pixpack_realize;
    widget_class->unrealize = tg_pixpack_unrealize;
    widget_class->draw = tg_pixpack_draw;
}


static void
tg_pixpack_init(TgPixpack *pixpack)
{
    pixpack->pixbuf = NULL;
    pixpack->autosize = FALSE;

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
tg_pixpack_get_property(GObject *object, guint property_id,
			GValue *value, GParamSpec *pspec)
{
    TgPixpack *self = TG_PIXPACK(object);

    switch (property_id) {
	case PROP_AUTOSIZE:
	    g_value_set_boolean(value, self->autosize);
	    break;

	default:
	    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
	    break;
    }
}

static void
tg_pixpack_set_property(GObject *object, guint property_id,
			const GValue *value, GParamSpec *pspec)
{
    TgPixpack *self = TG_PIXPACK(object);

    switch (property_id) {
	case PROP_AUTOSIZE:
	    self->autosize = g_value_get_boolean(value);
	    break;

	default:
	    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
	    break;
    }
}

static void
tg_pixpack_destroy(GtkWidget *widget)
{
    TgPixpack *pixpack;

    g_return_if_fail(widget);
    g_return_if_fail(TG_IS_PIXPACK(widget));

    pixpack = TG_PIXPACK(widget);

    g_clear_object(&pixpack->pixbuf);

    if (GTK_WIDGET_CLASS(tg_pixpack_parent_class)->destroy)
	(*GTK_WIDGET_CLASS(tg_pixpack_parent_class)->destroy)(widget);
}

static void
tg_pixpack_realize(GtkWidget *widget)
{
    GdkWindowAttr attributes;
    gint attributes_mask;
    TgPixpack *pixpack;
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
    attributes.event_mask = gtk_widget_get_events(widget) | GDK_EXPOSURE_MASK;

    attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL;

    window = gdk_window_new(gtk_widget_get_parent_window(widget),
			    &attributes, attributes_mask);
    gtk_widget_set_window(widget, window);
    gdk_window_set_user_data(window, widget);
}


static void
tg_pixpack_unrealize(GtkWidget *widget)
{
    TgPixpack *pixpack;

    g_return_if_fail(widget != NULL);
    g_return_if_fail(TG_IS_PIXPACK(widget));

    pixpack = TG_PIXPACK(widget);

    if (gtk_widget_get_mapped(widget))
	gtk_widget_unmap(widget);

    gtk_widget_set_mapped(widget, FALSE);

    if (GTK_WIDGET_CLASS(tg_pixpack_parent_class)->unrealize)
	(*GTK_WIDGET_CLASS(tg_pixpack_parent_class)->unrealize)(widget);
}


static gboolean
tg_pixpack_draw(GtkWidget *widget, cairo_t *cr)
{
    TgPixpack *pixpack;
    gint width, height;

    g_return_val_if_fail(widget != NULL, FALSE);
    g_return_val_if_fail(TG_IS_PIXPACK(widget), FALSE);

    pixpack = TG_PIXPACK(widget);

    if (NULL == pixpack->pixbuf)
	return FALSE;

    width = gtk_widget_get_allocated_width(widget);
    height = gtk_widget_get_allocated_height(widget);

    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
    cairo_fill(cr);
    gdk_cairo_set_source_pixbuf(cr, pixpack->pixbuf, 0, 0);
    cairo_scale(cr,
		width / gdk_pixbuf_get_width(pixpack->pixbuf),
		height / gdk_pixbuf_get_height(pixpack->pixbuf));
    cairo_paint(cr);

    return TRUE;
}

void
tg_pixpack_load_image(TgPixpack *pixpack, GdkPixbuf *pixbuf)
{
    g_return_if_fail(TG_IS_PIXPACK(pixpack));

    if (pixpack->pixbuf)
	g_object_unref(pixpack->pixbuf);
    pixpack->pixbuf = pixbuf;
    g_object_ref(pixpack->pixbuf);

    /* this forces a repaint */

    if (pixpack->autosize)
	gtk_widget_set_size_request(GTK_WIDGET(pixpack),
				    gdk_pixbuf_get_width(pixpack->pixbuf),
				    gdk_pixbuf_get_height(pixpack->pixbuf));
    gtk_widget_queue_draw(GTK_WIDGET(pixpack));
}
