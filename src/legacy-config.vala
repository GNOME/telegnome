/*
 * Copyright (C) 2016 Colin Watson <cjwatson@debian.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 */

/* Compatibility with old (gnome-config) configuration files. */

namespace Tg {

/* There is no sensible conversion path from gnome-config to GSettings,
 * especially as we skipped GConf.  Fortunately, gnome-config files were
 * pretty much just GKeyFiles, so we can roll our own without too much
 * difficulty.
 */

private SList<KeyFile> read_files () {
	var names = new SList<string> ();
	names.append (Path.build_filename (get_sysconf_dir (), "gnome",
					   "config-override", "telegnome"));
	names.append (Path.build_filename (Environment.get_home_dir (),
					   ".gnome2", "telegnome"));
	names.append (Path.build_filename (get_sysconf_dir (), "gnome",
					   "config", "telegnome"));
	var keyfiles = new SList<KeyFile> ();
	foreach (var name in names) {
		var keyfile = new KeyFile ();
		try {
			keyfile.load_from_file (name, KeyFileFlags.NONE);
			keyfiles.append ((owned) keyfile);
		} catch (FileError e) {
		} catch (KeyFileError e) {
		}
	}
	return keyfiles;
}

private string? get_value (SList<KeyFile> keyfiles, string group, string key) {
	foreach (unowned KeyFile keyfile in keyfiles) {
		try {
			return keyfile.get_string (group, key);
		} catch (KeyFileError e) {
		}
	}

	return null;
}

public void legacy_convert (Settings settings) {
	if (settings.get_boolean ("legacy-migration-complete"))
		return;

	var keyfiles = read_files ();

	string val;
	var channel_count = 0;
	var current_channel = -1;

	val = get_value (keyfiles, "Channels", "count");
	if (val != null)
		channel_count = int.parse (val);

	val = get_value (keyfiles, "Default", "server");
	if (val != null)
		current_channel = int.parse (val);

	val = get_value (keyfiles, "Zooming", "factor");
	if (val != null)
		/* clip to 1 or 2 */
		settings.set_int ("zoom-factor", int.parse (val) == 1 ? 1 : 2);

	val = get_value (keyfiles, "Paging", "enabled");
	if (val != null) {
		bool paging_enabled;
		if (val[0].tolower () == 't' || val[0].tolower () == 'y' ||
		    int.parse (val) != 0)
			paging_enabled = true;
		else
			paging_enabled = false;
		settings.set_boolean ("paging-enabled", paging_enabled);
	}

	val = get_value (keyfiles, "Paging", "interval");
	if (val != null)
		settings.set_int ("paging-interval", int.parse (val));

	val = get_value (keyfiles, "Paging", "page_nr");
	if (val != null)
		settings.set_int ("current-page-number", int.parse (val));

	val = get_value (keyfiles, "Paging", "subpage_nr");
	if (val != null)
		settings.set_int ("current-subpage-number", int.parse (val));

	string[] children = new string[channel_count + 1];
	for (var i = 0; i < channel_count; ++i) {
		var channel_group = @"Channel$i";
		uint8[] uu = new uint8[16];
		UUID.generate (uu);
		char[] uuid_bytes = new char[37];
		UUID.unparse (uu, uuid_bytes);
		string uuid = (string) uuid_bytes;
		var child_path = @"/org/gnome/telegnome/channel/$uuid/";
		children[i] = uuid;
		var channel_settings = new Settings.with_path
			("org.gnome.telegnome.channel", child_path);

		val = get_value (keyfiles, channel_group, "name");
		if (val != null)
			channel_settings.set_string ("name", val);

		val = get_value (keyfiles, channel_group, "desc");
		if (val != null)
			channel_settings.set_string ("description", val);

		val = get_value (keyfiles, channel_group, "page_url");
		if (val != null)
			channel_settings.set_string ("page-url", val);

		val = get_value (keyfiles, channel_group, "subpage_url");
		if (val != null)
			channel_settings.set_string ("subpage-url", val);

		val = get_value (keyfiles, channel_group, "country");
		if (val != null)
			channel_settings.set_string ("country", val);

		if (i == current_channel)
			settings.set_string ("current-channel", uuid);
	}
	settings.set_strv ("channel-children", children);

	settings.set_boolean ("legacy-migration-complete", true);
	Settings.sync ();
}

}
