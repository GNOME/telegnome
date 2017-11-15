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

public static int main (string[] args) {
	Intl.bindtextdomain ("telegnome", Tg.get_locale_dir ());
	Intl.bind_textdomain_codeset ("telegnome", "UTF-8");
	Intl.textdomain ("telegnome");

	var app = new Tg.App ();
	return app.run (args);
}
