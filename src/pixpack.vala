/*
 * Copyright (C) 2000 Dirk-Jan C. Binnema <djcb@dds.nl>
 * Copyright (C) 2008-2016 Colin Watson <cjwatson@debian.org>
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

/* Image-rendering widget. */

namespace Tg {

public class Pixpack : Gtk.Widget {
	private Gdk.Pixbuf? pixbuf;

	[Description (nick = "Autosize", blurb = "If true, set the widget's size request to the size of the image.")]
	public bool autosize { get; set; default = false; }

	public Pixpack () {
		base (can_focus: true, receives_default: true);

		this.pixbuf = null;

		this.set_has_window (true);

		this.unrealize.connect (on_unrealize);
	}

	public override void realize () {
		set_realized (true);
		Gtk.Allocation allocation;
		get_allocation (out allocation);
		var attributes = Gdk.WindowAttr () {
			window_type = Gdk.WindowType.CHILD,
			x = allocation.x,
			y = allocation.y,
			height = allocation.height,
			width = allocation.width,
			wclass = Gdk.WindowWindowClass.INPUT_OUTPUT,
			visual = get_visual (),
			event_mask = get_events () |
				     Gdk.EventMask.EXPOSURE_MASK
		};
		var attributes_mask = Gdk.WindowAttributesType.X |
				      Gdk.WindowAttributesType.Y |
				      Gdk.WindowAttributesType.VISUAL;
		var window = new Gdk.Window (get_parent_window (), attributes,
					     attributes_mask);
		set_window (window);
		window.set_user_data (this);
	}

	private void on_unrealize () {
		if (get_mapped ())
			unmap ();

		set_mapped (false);
	}

	public override bool draw (Cairo.Context cr) {
		if (pixbuf == null)
			return false;

		var width = get_allocated_width ();
		var height = get_allocated_height ();

		cr.set_source_rgb (0, 0, 0);
		cr.fill ();
		Gdk.cairo_set_source_pixbuf (cr, pixbuf, 0, 0);
		cr.scale (width / pixbuf.get_width (),
			  height / pixbuf.get_height ());
		cr.paint ();

		return true;
	}

	public void load_image (Gdk.Pixbuf pixbuf) {
		this.pixbuf = pixbuf;

		/* This forces a repaint. */
		if (autosize)
			set_size_request (this.pixbuf.get_width (),
					  this.pixbuf.get_height ());
		queue_draw ();
	}

	public void load_image_from_resource (string resource_path) {
		try {
			var pixbuf = new Gdk.Pixbuf.from_resource
				(resource_path);
			load_image (pixbuf);
		} catch (Error e) {
			assert_no_error (e);
		}
	}

}

}
