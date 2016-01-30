/* legacy-config.c
 * Part of TeleGNOME, a GNOME app to view Teletext.
 * This file deals with compatibility with old (gnome-config) configuration
 * files.
 */

/*
** Copyright (C) 2016 Colin Watson <cjwatson@debian.org>
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

#include <stdlib.h>

#include <glib.h>
#include <gio/gio.h>
#include <uuid.h>

#include "legacy-config.h"

/* There is no sensible conversion path from gnome-config to GSettings,
 * especially as we skipped GConf.  Fortunately, gnome-config files were
 * pretty much just GKeyFiles, so we can roll our own without too much
 * difficulty.
 */

static void
legacy_read_one_file (gpointer data, gpointer user_data)
{
    const gchar *name = data;
    GSList **keyfiles = user_data;
    GKeyFile *keyfile;

    keyfile = g_key_file_new ();
    if (g_key_file_load_from_file (keyfile, name, G_KEY_FILE_NONE, NULL))
	*keyfiles = g_slist_append (*keyfiles, keyfile);
    else
	g_key_file_unref (keyfile);
}

static GSList *
legacy_read_files (void)
{
    GSList *names = NULL;
    GSList *keyfiles = NULL;

    names = g_slist_append (names, g_build_filename (
	SYSCONFDIR, "gnome", "config-override", "telegnome", NULL));
    names = g_slist_append (names, g_build_filename (
	g_get_home_dir (), ".gnome2", "telegnome", NULL));
    names = g_slist_append (names, g_build_filename (
	SYSCONFDIR, "gnome", "config", "telegnome", NULL));

    g_slist_foreach (names, legacy_read_one_file, &keyfiles);
    g_slist_free_full (names, g_free);

    return keyfiles;
}

static gchar *
legacy_get_value (GSList *keyfiles, const gchar *group, const gchar *key)
{
    GSList *iter;

    for (iter = keyfiles; iter; iter = g_slist_next (iter)) {
	gchar *ret = g_key_file_get_string (iter->data, group, key, NULL);
	if (ret)
	    return ret;
    }

    return NULL;
}

void
legacy_convert (GSettings *settings)
{
    GSList *keyfiles;
    gchar *value;
    gchar **children;
    gint channel_count = 0, current_channel = -1, i;

    if (g_settings_get_boolean (settings, "legacy-migration-complete"))
	return;

    keyfiles = legacy_read_files ();

    value = legacy_get_value (keyfiles, "Channels", "count");
    if (value)
	channel_count = atoi (value);
    g_free (value);

    value = legacy_get_value (keyfiles, "Default", "server");
    if (value)
	current_channel = atoi (value);
    g_free (value);

    value = legacy_get_value (keyfiles, "Zooming", "factor");
    if (value) {
	gint zoom_factor = atoi (value);
	/* clip to 1 or 2 */
	g_settings_set_int (settings, "zoom-factor", zoom_factor == 1 ? 1 : 2);
    }
    g_free (value);

    value = legacy_get_value (keyfiles, "Paging", "enabled");
    if (value) {
	gboolean paging_enabled;
	if (g_ascii_tolower (*value) == 't' ||
	    g_ascii_tolower (*value) == 'y' ||
	    atoi (value))
	    paging_enabled = TRUE;
	else
	    paging_enabled = FALSE;
	g_settings_set_boolean (settings, "paging-enabled", paging_enabled);
    }
    g_free (value);

    value = legacy_get_value (keyfiles, "Paging", "interval");
    if (value) {
	gint paging_interval = atoi (value);
	g_settings_set_int (settings, "paging-interval", paging_interval);
    }
    g_free (value);

    value = legacy_get_value (keyfiles, "Paging", "page_nr");
    if (value) {
	gint current_page_number = atoi (value);
	g_settings_set_int (settings,
			    "current-page-number", current_page_number);
    }
    g_free (value);

    value = legacy_get_value (keyfiles, "Paging", "subpage_nr");
    if (value) {
	gint current_subpage_number = atoi (value);
	g_settings_set_int (settings,
			    "current-subpage-number", current_subpage_number);
    }
    g_free (value);

    children = g_new0 (gchar *, channel_count + 1);
    for (i = 0; i < channel_count; ++i) {
	gchar *channel_group = g_strdup_printf ("Channel%d", i);
	GSettings *channel_settings;
	uuid_t uu;
	gchar uuid[37];
	gchar *child_path;

	uuid_generate (uu);
	uuid_unparse (uu, uuid);
	child_path = g_strdup_printf
	    ("/org/gnome/telegnome/channel/%s/", uuid);
	children[i] = g_strdup (uuid);
	channel_settings = g_settings_new_with_path
	    ("org.gnome.telegnome.channel", child_path);

	value = legacy_get_value (keyfiles, channel_group, "name");
	if (value)
	    g_settings_set_string (channel_settings, "name", value);
	g_free (value);

	value = legacy_get_value (keyfiles, channel_group, "desc");
	if (value)
	    g_settings_set_string (channel_settings, "description", value);
	g_free (value);

	value = legacy_get_value (keyfiles, channel_group, "page_url");
	if (value)
	    g_settings_set_string (channel_settings, "page-url", value);
	g_free (value);

	value = legacy_get_value (keyfiles, channel_group, "subpage_url");
	if (value)
	    g_settings_set_string (channel_settings, "subpage-url", value);
	g_free (value);

	value = legacy_get_value (keyfiles, channel_group, "country");
	if (value)
	    g_settings_set_string (channel_settings, "country", value);
	g_free (value);

	if (i == current_channel)
	    g_settings_set_string (settings, "current-channel", uuid);

	g_object_unref (channel_settings);
	g_free (channel_group);
    }
    g_settings_set_strv (settings, "channel-children",
			 (const gchar * const *) children);
    g_strfreev (children);

    g_slist_free_full (keyfiles, (GDestroyNotify) g_key_file_unref);

    g_settings_set_boolean (settings, "legacy-migration-complete", TRUE);
    g_settings_sync ();
}
