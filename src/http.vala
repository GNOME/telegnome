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

/* HTTP fetches. */

namespace Tg {

public errordomain HttpError {
	/**
	 * Corrupt image.
	 */
	PIXBUF,
	/**
	 * Error from the VFS layer.
	 */
	VFS,
	/**
	 * Error constructing an HTTP query URL.
	 */
	HTTPQUERY
}

public Gdk.Pixbuf http_get_image (Gui gui) throws HttpError {
	var channel = gui.current_channel;
	string? url = null;
	if (gui.subpage_number > 0) {
		var subpage_url = channel.subpage_url;
		if (subpage_url != null && subpage_url != "")
			url = subpage_url.printf (gui.page_number,
						  gui.subpage_number);
	}
	if (url == null || url == "") {
		var page_url = channel.page_url;
		if (page_url != null && page_url != "")
			url = page_url.printf (gui.page_number);
	}
	if (url == null || url == "")
		throw new HttpError.HTTPQUERY ("");

	var http_file = File.new_for_uri (url);
	FileInputStream http_input;
	try {
		http_input = http_file.read ();
	} catch (Error e) {
		warning ("Unable to fetch '%s': %s", url, e.message);
		throw new HttpError.VFS (e.message);
	}

	var loader = new Gdk.PixbufLoader ();
	var buf = new uint8[4096];
	for (;;) {
		ssize_t bytes_read;
		try {
			bytes_read = http_input.read (buf);
			if (bytes_read == 0)
				break;
		} catch (IOError e) {
			warning ("Unable to read data from '%s': %s",
				 url, e.message);
			throw new HttpError.VFS (e.message);
		}
		try {
			loader.write (buf[0:bytes_read]);
		} catch (Error e) {
			warning ("Unable to parse image from '%s': %s",
				 url, e.message);
			throw new HttpError.PIXBUF (e.message);
		}
	}
	try {
		loader.close ();
	} catch (Error e) {
		warning ("Unable to parse image from '%s': %s",
			 url, e.message);
		throw new HttpError.PIXBUF (e.message);
	}

	var pixbuf = loader.get_pixbuf ();
	if (pixbuf == null) {
		warning ("Pixbuf loader did not create a pixbuf from '%s'",
			 url);
		throw new HttpError.PIXBUF ("");
	}
	return (owned) pixbuf;
}

}
