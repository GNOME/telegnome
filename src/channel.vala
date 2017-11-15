/*
 *    Copyright (C) 1999, 2000,
 *    Dirk-Jan C. Binnema <djcb@dds.nl>,
 *    Arjan Scherpenisse <acscherp@wins.uva.nl>
 *    Copyright (C) 2016 Colin Watson <cjwatson@debian.org>
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
 */

/* A channel. */

namespace Tg {

public class Channel : Object {
	public Settings settings { get; private set; }

	[Description (nick = "UUID", blurb = "UUID")]
	public string uuid { get; construct; }
	[Description (nick = "Name", blurb = "Name")]
	public string? name { get; set; }
	[Description (nick = "Description", blurb = "Description")]
	public string? description { get; set; }
	[Description (nick = "Page URL", blurb = "Page URL")]
	public string? page_url { get; set; }
	[Description (nick = "Subpage URL", blurb = "Subpage URL")]
	public string? subpage_url { get; set; }
	[Description (nick = "Country", blurb = "Country")]
	public string? country { get; set; }

	construct {
		if (uuid == null) {
			uint8[] uu = new uint8[16];
			UUID.generate (uu);
			char[] real_uuid = new char[37];
			UUID.unparse (uu, real_uuid);
			uuid = (string) real_uuid;
		}
		bind_settings ();
	}

	public Channel (string? uuid = null) {
		Object (uuid: uuid);
	}

	public Channel.with_parameters (string? name, string? description,
					string? page_url, string? subpage_url,
					string? country) {
		Object (name: name, description: description,
			page_url: page_url, subpage_url: subpage_url,
			country: country);
	}

	private void bind_settings () {
		var path = @"/org/gnome/telegnome/channel/$uuid/";
		settings = new Settings.with_path
			("org.gnome.telegnome.channel", path);
		settings.bind ("name", this, "name",
			       SettingsBindFlags.DEFAULT);
		settings.bind ("description", this, "description",
			       SettingsBindFlags.DEFAULT);
		settings.bind ("page-url", this, "page_url",
			       SettingsBindFlags.DEFAULT);
		settings.bind ("subpage-url", this, "subpage_url",
			       SettingsBindFlags.DEFAULT);
		settings.bind ("country", this, "country",
			       SettingsBindFlags.DEFAULT);
	}
}

}
