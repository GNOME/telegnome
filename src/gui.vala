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

/* The main user interface. */

namespace Tg {

private const string notfound_pixmap =
	"/org/gnome/telegnome/pixmaps/testbeeld.png";
private const string logo_pixmap =
	"/org/gnome/telegnome/pixmaps/telegnome-logo.png";
private const string main_ui = "/org/gnome/telegnome/telegnome.ui";

/* The timeout before the input field resets. */
private const int kb_timeout = 2500;
/* The time for which the logo gets displayed. */
private const int logo_timeout = 7500;

private enum InputStatus {
	NEW,
	CONTINUED
}

public class Gui : Object {

	[Description (nick = "Channel children", blurb = "List of relative settings paths at which channels are stored")]
	public string[] channel_children { get; set; }

	private string? _current_channel_uuid;
	[Description (nick = "Current channel", blurb = "Current channel")]
	public string? current_channel_uuid {
		get { return _current_channel_uuid; }
		set {
			_current_channel_uuid = value;
			var channel = channel_find_by_uuid (value);
			if (channel != null)
				channel_select (channel);
		}
	}

	[Description (nick = "Zoom factor", blurb = "Page zoom factor.  Larger numbers produce larger text.")]
	public int zoom_factor { get; set; default = 1; }
	[Description (nick = "Paging enabled", blurb = "Automatically switch page at periodic intervals")]
	public bool paging_enabled { get; set; default = false; }
	[Description (nick = "Paging interval", blurb = "Specifies the interval for the auto-pager, in milliseconds.")]
	public int paging_interval { get; set; default = 12000; }
	[Description (nick = "Current page number", blurb = "Current page number")]
	public int page_number { get; set; default = -1; }
	[Description (nick = "Current subpage number", blurb = "Current subpage number")]
	public int subpage_number { get; set; default = -1; }

	private Channel? _current_channel;
	public Channel? current_channel { get { return _current_channel; } }

	private Gtk.Builder builder;
	public Gtk.Window window { get; private set; }
	private Gtk.Entry entry;
	private Pixpack pixpack;
	private Gtk.ProgressBar progress_bar;
	private Gtk.Statusbar status_bar;
	private Menu channel_menu;
	private Settings settings;

	/* For timer input. */
	private uint? logo_timer_id;
	private uint? kb_timer_id;
	private InputStatus kb_status;

	private uint? page_timer_id;
	private int page_progress;

	private SList<Channel> channels;

	public Gui (Gtk.Application app, Settings settings) {
		base ();

		/* Register custom type. */
		assert (typeof (Pixpack).name() != "");

		var theme = Gtk.IconTheme.get_default ();
		theme.add_resource_path ("/org/gnome/telegnome");

		builder = new Gtk.Builder.from_resource (main_ui);
		window = builder.get_object ("main_window") as Gtk.Window;
		app.add_window (window);
		entry = builder.get_object ("page_entry") as Gtk.Entry;
		pixpack = builder.get_object ("pixpack") as Pixpack;
		progress_bar = builder.get_object ("progress_bar")
			as Gtk.ProgressBar;
		status_bar = builder.get_object ("status_bar")
			as Gtk.Statusbar;

		window.key_press_event.connect
			((event) => on_keypress (event));
		entry.activate.connect (() => on_goto_page ());
		var goto_button = builder.get_object ("goto_button")
			as Gtk.ToolButton;
		goto_button.clicked.connect (() => on_goto_page ());
		var prev_button = builder.get_object ("prev_button")
			as Gtk.ToolButton;
		prev_button.clicked.connect (() => on_prev_page ());
		var next_button = builder.get_object ("next_button")
			as Gtk.ToolButton;
		next_button.clicked.connect (() => on_next_page ());
		var home_button = builder.get_object ("home_button")
			as Gtk.ToolButton;
		home_button.clicked.connect (() => on_home ());
		var paging_button = builder.get_object("paging_button")
			as Gtk.ToggleToolButton;
		paging_button.clicked.connect (() => on_toggle_paging ());

		channel_menu = app.get_menu_by_id ("channels");

		this.settings = settings;
		settings.bind ("channel-children", this, "channel_children",
			       SettingsBindFlags.DEFAULT);
		settings.bind ("current-channel", this, "current_channel_uuid",
			       SettingsBindFlags.DEFAULT);
		settings.bind ("zoom-factor", this, "zoom_factor",
			       SettingsBindFlags.DEFAULT);
		settings.bind ("paging-enabled", this, "paging_enabled",
			       SettingsBindFlags.DEFAULT);
		settings.bind ("paging-interval", this, "paging_interval",
			       SettingsBindFlags.DEFAULT);
		settings.bind ("current-page-number", this, "page_number",
			       SettingsBindFlags.DEFAULT);
		settings.bind ("current-subpage-number",
			       this, "subpage_number",
			       SettingsBindFlags.DEFAULT);

		window.show_all ();

		kb_timer_id = null;
		kb_status = InputStatus.NEW;

		channels = new SList<Channel> ();
		refresh_channel_menu ();
		_current_channel = channel_find_by_uuid (current_channel_uuid);
		update_title_bar ();

		update_entry (0, 0);
		pixpack.load_image_from_resource (logo_pixmap);

		if (page_number > 0)
			logo_timer_id = Timeout.add (logo_timeout, logo_timer);
		else
			logo_timer_id = null;
	}

	private void update_title_bar () {
		if (current_channel != null) {
			var name = current_channel.name;
			var desc = current_channel.description;
			if (name != null && desc != null) {
				var title = _("TeleGNOME: %s (%s)").printf
					(name, desc);
				window.set_title (title);
			}
		}
	}

	/**
	 * logo_timer:
	 *
	 * Remove the logo from the screen and go to a page.
	 */
	private bool logo_timer () {
		get_the_page ();
		logo_timer_id = null;
		return false;
	}

	private Channel? channel_find_by_uuid (string uuid) {
		foreach (var channel in channels) {
			if (uuid == channel.uuid)
				return channel;
		}
		return null;
	}

	private void channel_select (Channel channel) {
		_current_channel = channel;
		update_title_bar ();
	}

	public void change_state_set_channel (SimpleAction action,
					      Variant? value) {
		current_channel_uuid = value.get_string ();
		page_number = 100;
		subpage_number = 0;
		get_the_page ();
	}

	private void populate_channel_menu () {
		channel_menu.remove_all ();

		foreach (var channel in channels) {
			var action = @"app.set-channel::$(channel.uuid)";
			channel_menu.append (channel.name, action);
		}
	}

	private string? get_default_channel_value (KeyFile file,
						   string group, string key,
						   bool missing_ok = false) {
		try {
			return file.get_string (group, key);
		} catch (KeyFileError e) {
			if (missing_ok)
				return null;
			else
				assert_not_reached ();
		}
	}

	/**
	 * reload_channels:
	 *
	 * Load all the channels from settings and store them in the
	 * "channels" property.
	 */
	private void reload_channels () {
		var current_uuid = current_channel_uuid;

		channels = new SList<Channel> ();
		foreach (var uuid in channel_children)
			channels.append (new Channel (uuid));

		if (channels == null) {
			/* Nothing set up yet; fill in some defaults. */
			string[] children = {};
			Bytes bytes;
			try {
				bytes = resources_lookup_data
					("/org/gnome/telegnome/default-channels.cfg",
					 ResourceLookupFlags.NONE);
			} catch (Error e) {
				assert_not_reached ();
			}
			var file = new KeyFile ();
			try {
				file.load_from_data
					((string) bytes.get_data (), bytes.length,
					 KeyFileFlags.NONE);
			} catch (Error e) {
				assert_not_reached ();
			}
			foreach (unowned string group in file.get_groups ()) {
				var name = get_default_channel_value
					(file, group, "Name");
				var description = get_default_channel_value
					(file, group, "Description");
				var page_url = get_default_channel_value
					(file, group, "PageURL");
				var subpage_url = get_default_channel_value
					(file, group, "SubpageURL", true);
				var country = get_default_channel_value
					(file, group, "Country");
				var channel = new Channel.with_parameters
					(name, description,
					 page_url, subpage_url, country);
				channels.append (channel);
				children += channel.uuid;
			}
			settings.set_strv ("channel-children", children);
		}

		if (current_uuid != null)
			current_channel_uuid = current_uuid;
		if (current_channel == null) {
			current_channel_uuid = channels.data.uuid;
			page_number = 100;
			subpage_number = 0;
			if (current_uuid != null)
				get_the_page ();
		}
	}

	private void refresh_channel_menu () {
		/* load the channels from settings */
		reload_channels ();

		/* re-populate the menu */
		populate_channel_menu ();
	}

	/**
	 * print_in_statusbar:
	 * @status: A status string.
	 *
	 * Print a string in the status bar.
	 */
	private void print_in_statusbar (string? status) {
		var context_id = status_bar.get_context_id ("errors");
		status_bar.remove_all (context_id);
		if (status != null)
			status_bar.push (context_id, status);
	}

	private bool pager_timer () {
		page_progress += paging_interval / 100;
		progress_bar.set_fraction
			(page_progress / (double) paging_interval);

		if (page_progress >= paging_interval) {
			page_progress = 0;
			progress_bar.set_fraction (0.0);
			on_next_page ();
		}
		return true;
	}

	private void on_toggle_paging () {
		progress_bar.set_fraction (0.0);
		if (paging_enabled) {
			if (page_timer_id != null)
				Source.remove (page_timer_id);
			page_timer_id = null;
			paging_enabled = false;
		} else {
			page_timer_id = Timeout.add (paging_interval / 100,
						     pager_timer);
			paging_enabled = true;
		}
		page_progress = 0;
	}

	private void update_page () {
		/* Save these and restore them, if necessary. */
		int old_page = page_number;
		int old_subpage = subpage_number;

		try {
			var pixbuf = http_get_image (this);
			pixpack.load_image (pixbuf);
		} catch (HttpError e) {
			if (e is HttpError.PIXBUF) {
				/* We got an error from the web page.  Maybe
				 * we forgot the subpage number, or used it
				 * when we shouldn't.
				 */
				if (subpage_number == 0)
					subpage_number = 1;
				else
					subpage_number = 0;
				try {
					var pixbuf = http_get_image (this);
					pixpack.load_image (pixbuf);
				} catch (HttpError e2) {
					if (subpage_number != 1) {
						/* Maybe we've run out of
						 * subpages.  Go to the next
						 * main page.
						 */
						++page_number;
						subpage_number = 1;
						update_entry
							(page_number,
							 subpage_number);
						get_the_page ();
					} else {
						print_in_statusbar
							(_("Web server error: Wrong page number?"));
						/* Restore. */
						page_number = old_page;
						subpage_number = old_subpage;
						update_entry
							(page_number,
							 subpage_number);
						pixpack.load_image_from_resource
							(notfound_pixmap);
					}
				}
			} else if (e is HttpError.VFS)
				print_in_statusbar
					(_("Error making HTTP connection"));
			else if (e is HttpError.HTTPQUERY)
				print_in_statusbar
					(_("Internal error in HTTP query code"));
			else
				assert_not_reached ();
		}
	}

	/**
	 * update_entry:
	 *
	 * Update the entry box with the current values of page and subpage.
	 */
	private void update_entry (int page_nr, int subpage_nr) {
		string full_num;
		if (subpage_nr > 0)
			full_num = @"$page_nr/$subpage_nr";
		else
			full_num = @"$page_nr";
		entry.set_text (full_num);
	}

	/**
	 * get_the_page:
	 *
	 * Try to get the page, and do something smart if it fails.
	 */
	private void get_the_page () {
		if (logo_timer_id != null)
			Source.remove (logo_timer_id);
		logo_timer_id = null;

		if (current_channel != null)
			update_page ();

		update_entry (page_number, subpage_number);
		print_in_statusbar (null);
	}

	public void activate_quit (SimpleAction action, Variant? parameter) {
		Application.get_default ().quit ();
	}

	public void activate_help_contents (SimpleAction action,
					    Variant? parameter) {
		try {
			Gtk.show_uri_on_window (window, "help:telegnome",
						Gdk.CURRENT_TIME);
		} catch (Error e) {
			warning ("Error displaying help: %s", e.message);
		}
	}

	public void activate_about (SimpleAction action, Variant? parameter) {
		Gtk.Dialog about = builder.get_object ("about_dialog")
			as Gtk.Dialog;
		about.run ();
		about.hide ();
	}

	private void refresh_timer () {
		var perc = progress_bar.get_fraction ();
		progress_bar.set_fraction (perc);
		if (paging_enabled) {
			Source.remove (page_timer_id);
			page_timer_id = Timeout.add (paging_interval / 100,
						     pager_timer);
		}
		page_progress = (int) ((paging_interval / 100) * perc);
	}

	public void activate_preferences (SimpleAction action,
					  Variant? parameter) {
		new Prefs (this).show ();
		refresh_channel_menu ();
		refresh_timer ();
	}

	private void on_next_page () {
		if (subpage_number == 0)
			++page_number;
		else
			++subpage_number;

		update_entry (page_number, subpage_number);
		get_the_page ();
	}

	private void on_prev_page () {
		if (subpage_number > 0)
			--subpage_number;
		if (subpage_number == 0)
			--page_number;

		update_entry (page_number, subpage_number);
		get_the_page ();
	}

	private void on_home () {
		page_number = 100;
		subpage_number = 0;

		update_entry (page_number, subpage_number);
		get_the_page ();
	}

	private void on_goto_page () {
		kb_status = InputStatus.NEW;
		var entry_text = entry.get_text ();
		var tokens = entry_text.split ("/", 2);
		int64? new_page = null;
		int64 new_subpage = 0;
		if (tokens.length >= 1) {
			if (!int64.try_parse (tokens[0], out new_page))
				new_page = null;
		}
		if (tokens.length >= 2) {
			if (!int64.try_parse (tokens[1], out new_subpage))
				new_page = null;
		}
		if (new_page == null)
			print_in_statusbar (_("Error in page entry"));
		else {
			page_number = (int) new_page;
			subpage_number = (int) new_subpage;
			get_the_page ();
		}
	}

	private bool keyboard_timer () {
		kb_status = InputStatus.NEW;
		kb_timer_id = null;
		return false;
	}

	private bool on_keypress (Gdk.EventKey event) {
		if (event.keyval == Gdk.Key.KP_Enter) {
			on_goto_page ();
			update_entry (page_number, subpage_number);
			return false;
		}

		if (!entry.is_focus)
			entry.grab_focus ();

		if (kb_status == InputStatus.NEW)
			entry.set_text ("");

		if (kb_timer_id != null)
			Source.remove (kb_timer_id);
		kb_timer_id = Timeout.add (kb_timeout, keyboard_timer);
		kb_status = InputStatus.CONTINUED;
		return false;
	}

}

}
