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

/* Vala bindings for configured paths. */

[CCode (cheader_filename = "paths.h", lower_case_cprefix = "tg_")]
namespace Tg {
	public static unowned string get_sysconf_dir ();
	public static unowned string get_locale_dir ();
}
