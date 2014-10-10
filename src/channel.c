/*
 * channel.c
 * A channel
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

#include <glib.h>
#include <libgnome/libgnome.h>

#include "channel.h"

TgChannel *
tg_channel_new(int id, const char *name, const char *desc, const char *page_url, const char *subpage_url, const char *country)
{
    TgChannel *t;
    t = g_malloc(sizeof(TgChannel));
    
    t->id = id;
    t->name = g_string_new(name);
    t->desc = g_string_new(desc);
    t->page_url = g_string_new(page_url);
    t->subpage_url = g_string_new(subpage_url);
    t->country = g_string_new(country);
    
    return t;
}

TgChannel *
tg_channel_new_from_config(int id)
{
    TgChannel *t;
    GString *prefix;
    t = g_malloc(sizeof(TgChannel));

    prefix = g_string_new(NULL);
    g_string_printf(prefix, "/telegnome/Channel%d/", id);
    gnome_config_push_prefix(prefix->str);

    t->id = id;
    t->name = g_string_new(gnome_config_get_string("name"));
    t->desc = g_string_new(gnome_config_get_string("desc"));
    t->page_url = g_string_new(gnome_config_get_string("page_url"));
    t->subpage_url = g_string_new(gnome_config_get_string("subpage_url"));
    t->country = g_string_new(gnome_config_get_string("country"));

    gnome_config_pop_prefix();

    return t;
}

void
tg_channel_save_to_config(TgChannel *channel)
{
    GString *prefix;

    g_assert(channel != NULL);

    prefix = g_string_new("a");
    g_string_printf(prefix, "/telegnome/Channel%d/", channel->id);
    gnome_config_push_prefix(prefix->str);

    gnome_config_set_string("name", channel->name->str);
    gnome_config_set_string("desc", channel->desc->str);
    gnome_config_set_string("page_url", channel->page_url->str);
    gnome_config_set_string("subpage_url", channel->subpage_url->str);
    gnome_config_set_string("country", channel->country->str);
    gnome_config_sync();
    gnome_config_pop_prefix();
}
    
    

void 
tg_channel_free(TgChannel *channel)
{
    g_assert(channel != NULL);
    g_string_free(channel->name, TRUE);
    g_string_free(channel->page_url, TRUE);
    g_string_free(channel->subpage_url, TRUE);
    g_string_free(channel->country, TRUE);
    g_free(channel);
}
