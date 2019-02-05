/*
 * Copyright (C) 2000 Arjan Scherpenisse <acscherp@wins.uva.nl>
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
 */

/* The preferences dialog. */

namespace Tg {

private const string prefs_ui = "/org/gnome/telegnome/prefs.ui";

private enum Column {
	COUNTRY,
	NAME,
	CHANNEL
}

public class Prefs : Object {

	private Settings settings;
	private Gtk.Builder builder;
	private Gtk.Dialog dialog;
	private Gtk.ListStore channel_store;
	private Gtk.TreeView channel_view;
	private Gtk.Label channel_label;
	private Gtk.Dialog channel_dialog;
	private Gtk.Entry name_entry;
	private Gtk.Entry description_entry;
	private Gtk.Entry page_url_entry;
	private Gtk.Entry subpage_url_entry;
	private Gtk.Entry country_entry;

	public Prefs (Gui gui) {
		settings = new Settings ("org.gnome.telegnome");

		builder = new Gtk.Builder ();
		builder.expose_object ("main_window", gui.window);
		try {
			builder.add_from_resource (prefs_ui);
		} catch (Error e) {
			error ("failed to add UI: %s", e.message);
		}
		dialog = builder.get_object ("prefs_dialog") as Gtk.Dialog;
		channel_store = builder.get_object ("prefs_channel_store")
			as Gtk.ListStore;
		channel_view = builder.get_object ("prefs_channel_view")
			as Gtk.TreeView;
		channel_label = builder.get_object ("prefs_channel_label")
			as Gtk.Label;
		channel_dialog = builder.get_object ("channel_dialog")
			as Gtk.Dialog;
		name_entry = builder.get_object ("name_entry") as Gtk.Entry;
		description_entry = builder.get_object ("description_entry")
			as Gtk.Entry;
		page_url_entry = builder.get_object ("page_url_entry")
			as Gtk.Entry;
		subpage_url_entry = builder.get_object ("subpage_url_entry")
			as Gtk.Entry;
		country_entry = builder.get_object ("country_entry")
			as Gtk.Entry;

		channel_view.get_selection ().changed.connect
			((selection) =>
			 on_channel_selection_changed (selection));
		var move_up_button = builder.get_object ("move_up_button")
			as Gtk.Button;
		move_up_button.clicked.connect (() => on_channel_move_up ());
		var move_down_button = builder.get_object ("move_down_button")
			as Gtk.Button;
		move_down_button.clicked.connect
			(() => on_channel_move_down ());
		var add_button = builder.get_object ("add_button")
			as Gtk.Button;
		add_button.clicked.connect (() => on_channel_add ());
		var delete_button = builder.get_object ("delete_button")
			as Gtk.Button;
		delete_button.clicked.connect (() => on_channel_delete ());
		var edit_button = builder.get_object ("edit_button")
			as Gtk.Button;
		edit_button.clicked.connect (() => on_channel_edit ());
	}

	private void append_channel (Channel channel) {
		Gtk.TreeIter iter;
		channel_store.append (out iter);
		channel_store.set (iter,
				   Column.COUNTRY, channel.country,
				   Column.NAME, channel.name,
				   Column.CHANNEL, channel,
				   -1);
	}

	private void fill_channel_list () {
		channel_store.clear ();
		var children = settings.get_strv ("channel-children");
		foreach (var child in children)
			append_channel (new Channel (child));
	}

	private bool edit_channel (Channel orig) {
		var settings = orig.settings;
		settings.delay ();
		settings.bind ("name", name_entry, "text",
			       SettingsBindFlags.DEFAULT);
		settings.bind ("description", description_entry, "text",
			       SettingsBindFlags.DEFAULT);
		settings.bind ("page-url", page_url_entry, "text",
			       SettingsBindFlags.DEFAULT);
		settings.bind ("subpage-url", subpage_url_entry, "text",
			       SettingsBindFlags.DEFAULT);
		settings.bind ("country", country_entry, "text",
			       SettingsBindFlags.DEFAULT);

		channel_dialog.show_all ();
		var reply = channel_dialog.run ();
		bool changed;
		switch (reply) {
			case Gtk.ResponseType.OK:
				settings.apply ();
				changed = true;
				break;
			default:
				settings.revert ();
				changed = false;
				break;
		}
		channel_dialog.hide ();

		return changed;
	}

	private void on_channel_selection_changed
			(Gtk.TreeSelection selection) {
		Gtk.TreeIter iter;
		string description;
		if (selection.get_selected (null, out iter)) {
			Channel channel;
			channel_store.get (iter, Column.CHANNEL, out channel,
					   -1);
			description = channel.description;
		} else
			description = "";
		channel_label.set_text (description);
	}

	private void sync_channel_children () {
		var rows = 0;
		Gtk.TreeIter iter;
		var valid = channel_store.get_iter_first (out iter);
		while (valid) {
			++rows;
			valid = channel_store.iter_next (ref iter);
		}

		var children = new string[rows + 1];
		var i = 0;
		valid = channel_store.get_iter_first (out iter);
		while (valid) {
			assert (i < rows);
			Channel channel;
			channel_store.get (iter, Column.CHANNEL, out channel,
					   -1);
			children[i++] = channel.uuid;
			valid = channel_store.iter_next (ref iter);
		}
		settings.set_strv ("channel-children", children);
	}

	private void on_channel_add () {
		Channel channel = new Channel ();
		if (edit_channel (channel) && channel.name.length > 0)
			append_channel (channel);
	}

	private void on_channel_move_up () {
		var selection = channel_view.get_selection ();
		Gtk.TreeIter iter;
		if (!selection.get_selected (null, out iter))
			return;
		var path = channel_store.get_path (iter);
		assert (path != null);
		var depth = path.get_depth ();
		assert (depth == 1);
		var indices = path.get_indices ();
		if (indices[0] > 0) {
			Gtk.TreeIter previous_iter;
			channel_store.iter_nth_child (out previous_iter, null,
						      indices[0] - 1);
			channel_store.swap (iter, previous_iter);
			sync_channel_children ();
		}
	}

	private void on_channel_move_down () {
		var selection = channel_view.get_selection ();
		Gtk.TreeIter iter;
		if (!selection.get_selected (null, out iter))
			return;
		Gtk.TreeIter next_iter = iter;
		if (channel_store.iter_next (ref next_iter)) {
			channel_store.swap (iter, next_iter);
			sync_channel_children ();
		}
	}

	private void on_channel_edit () {
		var selection = channel_view.get_selection ();
		Gtk.TreeIter iter;
		if (!selection.get_selected (null, out iter))
			return;
		Channel channel;
		channel_store.get (iter, Column.CHANNEL, out channel, -1);
		if (edit_channel (channel))
			channel_store.set (iter,
					   Column.COUNTRY, channel.country,
					   Column.NAME, channel.name,
					   -1);
	}

	private void on_channel_delete () {
		var selection = channel_view.get_selection ();
		Gtk.TreeIter iter;
		if (!selection.get_selected (null, out iter))
			return;

		Channel channel;
		channel_store.get (iter, Column.CHANNEL, out channel, -1);
#if VALA_0_36
		channel_store.remove (ref iter);
#else
		channel_store.remove (iter);
#endif
		sync_channel_children ();

		/* Clear out settings debris from the deleted channel if
		 * possible.
		 */
		var settings = channel.settings;
		settings.reset ("");
		settings.apply ();
	}

	public void show () {
		fill_channel_list ();
		dialog.show_all ();
		dialog.run ();
		dialog.hide ();
	}

}

}
