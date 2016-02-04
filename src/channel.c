/*
 * channel.c
 * A channel
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

#include <stdarg.h>

#include <glib.h>
#include <glib/gi18n.h>
#include <glib-object.h>
#include <gio/gio.h>
#include <uuid.h>

#include "channel.h"

struct _TgChannel {
    GObject parent_instance;

    GSettings *settings;

    gchar *uuid;
    gchar *name;
    gchar *desc;
    gchar *page_url;
    gchar *subpage_url;
    gchar *country;
};

enum {
    PROP_0,

    PROP_UUID,
    PROP_NAME,
    PROP_DESC,
    PROP_PAGE_URL,
    PROP_SUBPAGE_URL,
    PROP_COUNTRY
};

G_DEFINE_TYPE (TgChannel, tg_channel, G_TYPE_OBJECT)

static void
tg_channel_init (TgChannel *self)
{
}

static void
tg_channel_bind_settings (TgChannel *self)
{
    gchar *path;

    path = g_strdup_printf ("/org/gnome/telegnome/channel/%s/", self->uuid);
    self->settings = g_settings_new_with_path
	("org.gnome.telegnome.channel", path);
    g_settings_bind (self->settings, "name", self, "name",
		     G_SETTINGS_BIND_DEFAULT);
    g_settings_bind (self->settings, "description", self, "description",
		     G_SETTINGS_BIND_DEFAULT);
    g_settings_bind (self->settings, "page-url", self, "page-url",
		     G_SETTINGS_BIND_DEFAULT);
    g_settings_bind (self->settings, "subpage-url", self, "subpage-url",
		     G_SETTINGS_BIND_DEFAULT);
    g_settings_bind (self->settings, "country", self, "country",
		     G_SETTINGS_BIND_DEFAULT);
}

static void
tg_channel_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
    TgChannel *self = TG_CHANNEL (object);

    switch (property_id) {
	case PROP_UUID:
	    g_value_set_string (value, self->uuid);
	    break;

	case PROP_NAME:
	    g_value_set_string (value, self->name);
	    break;

	case PROP_DESC:
	    g_value_set_string (value, self->desc);
	    break;

	case PROP_PAGE_URL:
	    g_value_set_string (value, self->page_url);
	    break;

	case PROP_SUBPAGE_URL:
	    g_value_set_string (value, self->subpage_url);
	    break;

	case PROP_COUNTRY:
	    g_value_set_string (value, self->country);
	    break;

	default:
	    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	    break;
    }
}

static void
tg_channel_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
    TgChannel *self = TG_CHANNEL (object);

    switch (property_id) {
	case PROP_UUID:
	    g_free (self->uuid);
	    self->uuid = g_value_dup_string (value);
	    break;

	case PROP_NAME:
	    g_free (self->name);
	    self->name = g_value_dup_string (value);
	    break;

	case PROP_DESC:
	    g_free (self->desc);
	    self->desc = g_value_dup_string (value);
	    break;

	case PROP_PAGE_URL:
	    g_free (self->page_url);
	    self->page_url = g_value_dup_string (value);
	    break;

	case PROP_SUBPAGE_URL:
	    g_free (self->subpage_url);
	    self->subpage_url = g_value_dup_string (value);
	    break;

	case PROP_COUNTRY:
	    g_free (self->country);
	    self->country = g_value_dup_string (value);
	    break;

	default:
	    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	    break;
    }
}

static void
tg_channel_dispose (GObject *object)
{
    TgChannel *self = TG_CHANNEL (object);

    g_clear_object (&self->settings);

    G_OBJECT_CLASS (tg_channel_parent_class)->dispose (object);
}

static void
tg_channel_finalize (GObject *object)
{
    TgChannel *self = TG_CHANNEL (object);

    g_clear_pointer (&self->name, g_free);
    g_clear_pointer (&self->desc, g_free);
    g_clear_pointer (&self->page_url, g_free);
    g_clear_pointer (&self->subpage_url, g_free);
    g_clear_pointer (&self->country, g_free);

    G_OBJECT_CLASS (tg_channel_parent_class)->finalize (object);
}

static void
tg_channel_class_init (TgChannelClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->get_property = tg_channel_get_property;
    gobject_class->set_property = tg_channel_set_property;
    gobject_class->dispose = tg_channel_dispose;
    gobject_class->finalize = tg_channel_finalize;

    g_object_class_install_property
	(gobject_class, PROP_UUID,
	 g_param_spec_string ("uuid",
			      "UUID", "UUID",
			      NULL,
			      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS |
			      G_PARAM_CONSTRUCT_ONLY));

    g_object_class_install_property
	(gobject_class, PROP_NAME,
	 g_param_spec_string ("name",
			      "Name", "Name",
			      NULL,
			      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

    g_object_class_install_property
	(gobject_class, PROP_DESC,
	 g_param_spec_string ("description",
			      "Description", "Description",
			      NULL,
			      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

    g_object_class_install_property
	(gobject_class, PROP_PAGE_URL,
	 g_param_spec_string ("page-url",
			      "Page URL", "Page URL",
			      NULL,
			      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

    g_object_class_install_property
	(gobject_class, PROP_SUBPAGE_URL,
	 g_param_spec_string ("subpage-url",
			      "Subpage URL", "Subpage URL",
			      NULL,
			      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

    g_object_class_install_property
	(gobject_class, PROP_COUNTRY,
	 g_param_spec_string ("country",
			      "Country", "Country",
			      NULL,
			      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
}

TgChannel *
tg_channel_new (const gchar *uuid, const gchar *first_property_name, ...)
{
    va_list argv;
    uuid_t uu;
    gchar real_uuid[37];
    TgChannel *channel;

    va_start (argv, first_property_name);
    if (!uuid) {
	uuid_generate (uu);
	uuid_unparse (uu, real_uuid);
	uuid = real_uuid;
    }
    channel = TG_CHANNEL (g_object_new (TG_TYPE_CHANNEL, "uuid", uuid, NULL));
    tg_channel_bind_settings (channel);
    g_object_set_valist (G_OBJECT (channel), first_property_name, argv);
    va_end (argv);
    return channel;
}

GSettings *
tg_channel_get_settings(TgChannel *channel)
{
    return channel->settings;
}
