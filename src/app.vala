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

/* The main TeleGNOME application. */

namespace Tg {

[CCode (cname = "GETTEXT_PACKAGE")]
extern const string GETTEXT_PACKAGE;

public class App : Gtk.Application {

	private Gui _gui;
	public Gui gui { get { return _gui; } }

	private const ActionEntry[] app_entries = {
		{ "quit", activate_quit },
		{ "preferences", activate_preferences },
		{ "help-contents", activate_help_contents },
		{ "about", activate_about },
		{ "set-channel", null, "s", "''", change_state_set_channel }
	};

	public App () {
		Object (application_id: "org.gnome.telegnome",
			flags: ApplicationFlags.NON_UNIQUE);

		this.startup.connect_after (on_startup);
	}

	/* These are bodges since we can't pass the Gui object as user data.
	 * It may be better to simply turn Gui into a subclass of
	 * Gtk.Application.
	 */

	public void activate_quit (SimpleAction action, Variant? parameter) {
		gui.activate_quit (action, parameter);
	}

	public void activate_preferences (SimpleAction action,
					  Variant? parameter) {
		gui.activate_preferences (action, parameter);
	}

	public void activate_help_contents (SimpleAction action,
					    Variant? parameter) {
		gui.activate_help_contents (action, parameter);
	}

	public void activate_about (SimpleAction action, Variant? parameter) {
		gui.activate_about (action, parameter);
	}

	public void change_state_set_channel (SimpleAction action,
					      Variant? value) {
		gui.change_state_set_channel (action, value);
	}

	private void on_startup () {
		var settings = new Settings (get_application_id ());
		legacy_convert (settings);

		_gui = new Gui (this, settings);
		add_action_entries (app_entries, this);
	}

	public override void activate () {
	 	gui.window.present ();
	}

}

}
