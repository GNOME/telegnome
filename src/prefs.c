/* prefs.c
 * part of TeleGNOME, a GNOME app to view Teletext (Dutch)
 * This file constructs the preferences dialog.
 */

/*
** Copyright (C) 2000 Arjan Scherpenisse <acscherp@wins.uva.nl>
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

#include <glib.h>
#include <glib/gi18n.h>
#include <glib-object.h>
#include <gio/gio.h>
#define G_SETTINGS_ENABLE_BACKEND
#include <gio/gsettingsbackend.h>
#include <gtk/gtk.h>
#include <dconf.h>

#include "prefs.h"
#include "channel.h"
#include "main.h"


typedef struct _TgPrefsWindow {
    GSettings *settings;

    GtkWidget *dialog;

    GtkListStore *channel_store;
    GtkWidget *channel_view;
    GtkWidget *channel_label;
} TgPrefsWindow;

static TgPrefsWindow *prefs_window;

enum {
    COUNTRY_COLUMN,
    NAME_COLUMN,
    CHANNEL_COLUMN,
    N_COLUMNS
};

static void
tg_prefs_set_close_cb(GCallback c)
{
    g_assert (prefs_window != NULL);
    g_signal_connect (G_OBJECT (prefs_window->dialog), "response", c, NULL);
}

static void
tg_prefs_fill_channel_list()
{
    int newrow;
    gchar **children, **childp;
    TgChannel *channel;
    gchar *country, *name;

    children = g_settings_get_strv(prefs_window->settings, "channel-children");
    for (childp = children; childp && *childp; ++childp) {
	GtkTreeIter iter;

	channel = tg_channel_new(*childp, NULL);
	if (!channel)
	    continue;
	g_object_get(channel, "country", &country, "name", &name, NULL);
	gtk_list_store_append(prefs_window->channel_store, &iter);
	gtk_list_store_set(prefs_window->channel_store, &iter,
			   COUNTRY_COLUMN, country, NAME_COLUMN, name,
			   CHANNEL_COLUMN, channel, -1);
	g_free(name);
	g_free(country);
	g_object_unref(channel);
    }
    g_strfreev(children);
}

/* pops up a modal dialog, editing the channel. */
static gboolean
tg_prefs_edit_channel(TgChannel *orig)
{
    GSettings *settings;
    GtkWidget *dialog, *grid, *label, *name, *page, *subpage, *desc, *country, *frame;
    gint reply;
    gboolean changed = FALSE;

    g_assert(orig != NULL);

    settings = tg_channel_get_settings(orig);
    g_settings_delay(settings);

    dialog = gtk_dialog_new_with_buttons(
	_("New/Edit Channel"),
	GTK_WINDOW(prefs_window->dialog),
	GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
	_("_OK"), GTK_RESPONSE_ACCEPT,
	_("_Cancel"), GTK_RESPONSE_REJECT,
	NULL);

    grid = gtk_grid_new();
    gtk_grid_set_row_homogeneous(GTK_GRID(grid), TRUE);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 5);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 5);

    label = gtk_label_new(_("Name"));
    g_object_set(G_OBJECT(label), "xalign", 1.0, "yalign", 0.5, NULL);
    name = gtk_entry_new();
    g_settings_bind(settings, "name", name, "text", G_SETTINGS_BIND_DEFAULT);
    gtk_grid_attach(GTK_GRID(grid), label, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), name, 1, 0, 1, 1);

    label = gtk_label_new(_("Description"));
    g_object_set(G_OBJECT(label), "xalign", 1.0, "yalign", 0.5, NULL);
    desc = gtk_entry_new();
    g_settings_bind(settings, "description", desc, "text", G_SETTINGS_BIND_DEFAULT);
    gtk_grid_attach(GTK_GRID(grid), label, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), desc, 1, 1, 1, 1);

    label = gtk_label_new(_("Page url"));
    g_object_set(G_OBJECT(label), "xalign", 1.0, "yalign", 0.5, NULL);
    page = gtk_entry_new();
    g_settings_bind(settings, "page-url", page, "text", G_SETTINGS_BIND_DEFAULT);
    gtk_grid_attach(GTK_GRID(grid), label, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), page, 1, 2, 1, 1);

    label = gtk_label_new(_("Subpage url"));
    g_object_set(G_OBJECT(label), "xalign", 1.0, "yalign", 0.5, NULL);
    subpage = gtk_entry_new();
    g_settings_bind(settings, "subpage-url", subpage, "text", G_SETTINGS_BIND_DEFAULT);
    gtk_grid_attach(GTK_GRID(grid), label, 0, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), subpage, 1, 3, 1, 1);

    label = gtk_label_new(_("Country"));
    g_object_set(G_OBJECT(label), "xalign", 1.0, "yalign", 0.5, NULL);
    country = gtk_entry_new();
    g_settings_bind(settings, "country", country, "text", G_SETTINGS_BIND_DEFAULT);
    gtk_grid_attach(GTK_GRID(grid), label, 0, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), country, 1, 4, 1, 1);

    frame = gtk_frame_new(_("Channel Information"));
    gtk_container_add(GTK_CONTAINER(frame), grid);

    gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), frame);

    gtk_widget_show_all(dialog);

    reply = gtk_dialog_run(GTK_DIALOG(dialog));

    switch (reply) {
	case GTK_RESPONSE_ACCEPT:
	    g_settings_apply(settings);
	    changed = TRUE;
	    break;
	default:
	    g_settings_revert(settings);
	    changed = FALSE;
	    break;
    }
    gtk_widget_destroy(dialog);

    return changed;
}

static void
tg_prefs_response_cb(GtkDialog *dialog, gint response_id, gpointer user_data)
{
    gtk_widget_destroy(GTK_WIDGET(dialog));
    g_clear_object(&prefs_window->channel_store);
    g_clear_pointer(&prefs_window, g_free);
}

static void
tg_prefs_channel_selection_changed_cb(GtkTreeSelection *selection,
				      gpointer data)
{
    GtkTreeModel *model;
    GtkTreeIter iter;
    TgChannel *channel;
    gchar *description;

    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
	gtk_tree_model_get(model, &iter, CHANNEL_COLUMN, &channel, -1);
	g_object_get(channel, "description", &description, NULL);
	g_object_unref(channel);
    } else
	description = g_strdup("");
    gtk_label_set_text(GTK_LABEL(prefs_window->channel_label), description);
    g_free(description);
}

static void 
tg_prefs_sync_channel_children(void)
{
    int rows, i;
    GtkTreeModel *model;
    GtkTreeIter iter;
    gboolean valid;
    gchar **children;

    model = GTK_TREE_MODEL(prefs_window->channel_store);

    rows = 0;
    valid = gtk_tree_model_get_iter_first(model, &iter);
    while (valid) {
	++rows;
	valid = gtk_tree_model_iter_next(model, &iter);
    }

    children = g_new0(gchar *, rows + 1);
    i = 0;
    valid = gtk_tree_model_get_iter_first(model, &iter);
    while (valid) {
	TgChannel *channel;
	g_assert(i < rows);
	gtk_tree_model_get(model, &iter, CHANNEL_COLUMN, &channel, -1);
	g_object_get(channel, "uuid", &children[i++], NULL);
	g_object_unref(channel);
	valid = gtk_tree_model_iter_next(model, &iter);
    }
    g_settings_set_strv(prefs_window->settings, "channel-children",
			(const gchar **) children);
}

static void
tg_prefs_channel_add_cb(void)
{
    TgChannel *chan;

    chan = tg_channel_new(NULL, NULL);

    if (tg_prefs_edit_channel(chan)) {
	gchar *country, *name;

	g_object_get(chan, "country", &country, "name", &name, NULL);
	if (strlen(name) > 0) {
	    GtkTreeIter iter;
	    gint new_row;

	    gtk_list_store_append(prefs_window->channel_store, &iter);
	    gtk_list_store_set(prefs_window->channel_store, &iter,
			       COUNTRY_COLUMN, country, NAME_COLUMN, name,
			       CHANNEL_COLUMN, chan, -1);
	    tg_prefs_sync_channel_children();
	}
	g_free(name);
	g_free(country);
    }

    g_object_unref(chan);
}

static void 
tg_prefs_channel_move_up_cb(void)
{
    GtkTreeSelection *selection;
    GtkTreeModel *model;
    GtkTreeIter iter;

    selection = gtk_tree_view_get_selection(
	GTK_TREE_VIEW(prefs_window->channel_view));
    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
	GtkTreePath *path;
	gint *indices, depth;
	GtkTreeIter previous_iter;

	path = gtk_tree_model_get_path(model, &iter);
	g_assert(path);
	indices = gtk_tree_path_get_indices_with_depth(path, &depth);
	g_assert(depth == 1);
	if (indices[0] > 0) {
	    gtk_tree_model_iter_nth_child(model, &previous_iter, NULL,
					  indices[0] - 1);
	    gtk_list_store_swap(GTK_LIST_STORE(model), &iter, &previous_iter);
	    tg_prefs_sync_channel_children();
	}
	gtk_tree_path_free(path);
    }
}

void 
tg_prefs_channel_move_down_cb(void)
{
    GtkTreeSelection *selection;
    GtkTreeModel *model;
    GtkTreeIter iter;

    selection = gtk_tree_view_get_selection(
	GTK_TREE_VIEW(prefs_window->channel_view));
    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
	GtkTreeIter next_iter = iter;

	if (gtk_tree_model_iter_next(model, &next_iter)) {
	    gtk_list_store_swap(GTK_LIST_STORE(model), &iter, &next_iter);
	    tg_prefs_sync_channel_children();
	}
    }
}

static void
tg_prefs_channel_edit_cb(void)
{
    GtkTreeSelection *selection;
    GtkTreeModel *model;
    GtkTreeIter iter;
    TgChannel *channel;

    selection = gtk_tree_view_get_selection(
	GTK_TREE_VIEW(prefs_window->channel_view));
    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
	gtk_tree_model_get(model, &iter, CHANNEL_COLUMN, &channel, -1);
	/* update the entry */
	if (tg_prefs_edit_channel(channel)) {
	    /* ...and update the list.  The data is set automagically. */
	    gchar *country, *name;
	    g_object_get(channel, "country", &country, "name", &name, NULL);
	    gtk_list_store_set(GTK_LIST_STORE(model), &iter,
			       COUNTRY_COLUMN, country, NAME_COLUMN, name, -1);
	    g_free(name);
	    g_free(country);
	}
	g_object_unref(channel);
    }
}

static void
tg_prefs_channel_delete_cb(void)
{
    GtkTreeSelection *selection;
    GtkTreeModel *model;
    GtkTreeIter iter;
    TgChannel *channel;
    gchar *old_uuid;
    GSettingsBackend *backend;

    selection = gtk_tree_view_get_selection(
	GTK_TREE_VIEW(prefs_window->channel_view));
    if (!gtk_tree_selection_get_selected(selection, &model, &iter))
	return;

    gtk_tree_model_get(model, &iter, CHANNEL_COLUMN, &channel, -1);
    g_object_get(channel, "uuid", &old_uuid, NULL);
    gtk_list_store_remove(GTK_LIST_STORE(model), &iter);
    tg_prefs_sync_channel_children();

    /* Clear out settings debris from the deleted channel if possible. */
    backend = g_settings_backend_get_default();
    if (g_str_equal(G_OBJECT_TYPE_NAME(backend), "DConfSettingsBackend")) {
	gchar *path;
	DConfClient *client;

	path = g_strdup_printf("/org/gnome/telegnome/channel/%s/", old_uuid);
	client = dconf_client_new();
	dconf_client_write_sync(client, path, NULL, NULL, NULL, NULL);
	g_object_unref(client);
	g_free(path);
    }
    g_object_unref(G_OBJECT(backend));
    g_free(old_uuid);
}

/* not a good idea to have a 'misc' page, but i cant come up with a better name */
static GtkWidget *
tg_prefs_construct_misc_page(void)
{
    GtkWidget *grid, *frame, *label, *entry;
    GtkAdjustment *adj;

    g_assert(prefs_window != NULL);

    grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 5);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 5);

    label = gtk_label_new(_("Paging interval"));
    gtk_widget_set_tooltip_text(label, _("Specifies the interval for the auto-pager, in milliseconds."));

    adj = GTK_ADJUSTMENT(gtk_adjustment_new(8000.0, 1000.0, 60000.0, 1000.0, 10.0, 0.0));
    entry = gtk_spin_button_new(adj, 0.5, 0);

    gtk_grid_attach(GTK_GRID(grid), label, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry, 1, 0, 1, 1);

    g_settings_bind(prefs_window->settings, "paging-interval", entry, "value", G_SETTINGS_BIND_DEFAULT);

    frame = gtk_frame_new(_("Miscellaneous"));

    gtk_container_set_border_width(GTK_CONTAINER(frame), 5);
    gtk_container_set_border_width(GTK_CONTAINER(grid), 5);
    gtk_container_add(GTK_CONTAINER(frame), grid);

    return frame;
}

static GtkWidget *
tg_prefs_construct_channels_page()
{
    GtkWidget *grid, *button_grid, *btn;
    GtkTreeViewColumn *country_column, *name_column;
    GtkTreeSelection *selection;

    g_assert(prefs_window != NULL);

    grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);

    /* the list */
    prefs_window->channel_store = gtk_list_store_new(
	N_COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_OBJECT);
    prefs_window->channel_view = gtk_tree_view_new_with_model(
	GTK_TREE_MODEL(prefs_window->channel_store));
    country_column = gtk_tree_view_column_new_with_attributes(
	N_("Country"), gtk_cell_renderer_text_new(),
	"text", COUNTRY_COLUMN, NULL);
    gtk_tree_view_append_column(
	GTK_TREE_VIEW(prefs_window->channel_view), country_column);
    name_column = gtk_tree_view_column_new_with_attributes(
	N_("Name"), gtk_cell_renderer_text_new(),
	"text", NAME_COLUMN, NULL);
    gtk_tree_view_column_set_min_width(name_column, 200);
    gtk_tree_view_append_column(
	GTK_TREE_VIEW(prefs_window->channel_view), name_column);
    gtk_widget_set_hexpand(prefs_window->channel_view, TRUE);
    gtk_widget_set_halign(prefs_window->channel_view, GTK_ALIGN_FILL);
    gtk_widget_set_vexpand(prefs_window->channel_view, TRUE);
    gtk_widget_set_valign(prefs_window->channel_view, GTK_ALIGN_FILL);
    gtk_grid_attach(GTK_GRID(grid), prefs_window->channel_view, 0, 0, 1, 1);

    /* label for descriptions and stuff */
    prefs_window->channel_label = gtk_label_new("");
    gtk_grid_attach(GTK_GRID(grid), prefs_window->channel_label, 0, 1, 1, 1);

    /* fill channel list */
    tg_prefs_fill_channel_list();

    selection = gtk_tree_view_get_selection(
	GTK_TREE_VIEW(prefs_window->channel_view));
    gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);
    g_signal_connect(G_OBJECT(selection), "changed",
		     G_CALLBACK(tg_prefs_channel_selection_changed_cb),
		     NULL);

    button_grid = gtk_grid_new();
    gtk_orientable_set_orientation(GTK_ORIENTABLE(button_grid),
				   GTK_ORIENTATION_VERTICAL);
    gtk_grid_set_row_homogeneous(GTK_GRID(button_grid), TRUE);
    gtk_grid_set_row_spacing(GTK_GRID(button_grid), 4);

    /* move up button */
    btn = gtk_button_new_with_label(_("Move up"));
    gtk_container_add(GTK_CONTAINER(button_grid), btn);
    g_signal_connect(G_OBJECT(btn), "clicked", G_CALLBACK(tg_prefs_channel_move_up_cb), NULL);
    /* move down button */
    btn = gtk_button_new_with_label(_("Move down"));
    gtk_container_add(GTK_CONTAINER(button_grid), btn);
    g_signal_connect(G_OBJECT(btn), "clicked", G_CALLBACK(tg_prefs_channel_move_down_cb), NULL);
    /* add button */
    btn = gtk_button_new_with_label(_("Add..."));
    gtk_container_add(GTK_CONTAINER(button_grid), btn);
    g_signal_connect(G_OBJECT(btn), "clicked", G_CALLBACK(tg_prefs_channel_add_cb), NULL);

    /* delete button */
    btn = gtk_button_new_with_label(_("Delete"));
    gtk_container_add(GTK_CONTAINER(button_grid), btn);
    g_signal_connect(G_OBJECT(btn), "clicked", G_CALLBACK(tg_prefs_channel_delete_cb), NULL);

    /* edit button */
    btn = gtk_button_new_with_label(_("Edit"));
    gtk_container_add(GTK_CONTAINER(button_grid), btn);
    g_signal_connect(G_OBJECT(btn), "clicked", G_CALLBACK(tg_prefs_channel_edit_cb), NULL);

    gtk_grid_attach(GTK_GRID(grid), button_grid, 1, 0, 1, 2);

    gtk_container_set_border_width(GTK_CONTAINER(grid), 5);
    return grid;
}

void
tg_prefs_show(GtkWindow *parent, GCallback close_cb)
{
    if (prefs_window != NULL) {
	gdk_window_show(gtk_widget_get_window(prefs_window->dialog));
	gdk_window_raise(gtk_widget_get_window(prefs_window->dialog));
    } else {
	GtkWidget *content_area, *notebook, *page;

	prefs_window = g_new0(TgPrefsWindow, 1);

	prefs_window->settings = g_settings_new("org.gnome.telegnome");

	prefs_window->dialog = gtk_dialog_new_with_buttons(
	    _("TeleGNOME: Preferences"), parent,
	    GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
	    _("_OK"), GTK_RESPONSE_ACCEPT,
	    _("_Cancel"), GTK_RESPONSE_REJECT,
	    NULL);
	content_area = gtk_dialog_get_content_area(
	    GTK_DIALOG(prefs_window->dialog));
	notebook = gtk_notebook_new();
	gtk_container_add(GTK_CONTAINER(content_area), notebook);

	page = tg_prefs_construct_channels_page();
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), page,
				 gtk_label_new(_("Channels")));

	page = tg_prefs_construct_misc_page();
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), page,
				 gtk_label_new(_("Misc")));


	gtk_notebook_set_show_tabs (GTK_NOTEBOOK(notebook), TRUE);
	gtk_notebook_set_show_border (GTK_NOTEBOOK(notebook), TRUE);

	if (close_cb)
	    tg_prefs_set_close_cb (close_cb);
	g_signal_connect (G_OBJECT (prefs_window->dialog), "response",
			  G_CALLBACK (tg_prefs_response_cb), NULL);

	/* and, show them all */
	gtk_widget_show_all(prefs_window->dialog);
    }
}
