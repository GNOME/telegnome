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
#include "gui.h"
#include "main.h"


typedef struct _TgPrefsWindow {
    GtkBuilder *builder;

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

    gtk_list_store_clear(prefs_window->channel_store);

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

    label = gtk_label_new(_("Page URL"));
    g_object_set(G_OBJECT(label), "xalign", 1.0, "yalign", 0.5, NULL);
    page = gtk_entry_new();
    g_settings_bind(settings, "page-url", page, "text", G_SETTINGS_BIND_DEFAULT);
    gtk_grid_attach(GTK_GRID(grid), label, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), page, 1, 2, 1, 1);

    label = gtk_label_new(_("Subpage URL"));
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

void
tg_prefs_show(TgGui *gui, GCallback close_cb)
{
    if (!prefs_window) {
	GError *error = NULL;

	prefs_window = g_new0(TgPrefsWindow, 1);

	prefs_window->settings = g_settings_new("org.gnome.telegnome");

	prefs_window->builder = gtk_builder_new();
	gtk_builder_expose_object(prefs_window->builder, "main_window",
				  G_OBJECT(tg_gui_get_window(gui)));
	if (!gtk_builder_add_from_resource(prefs_window->builder, TG_PREFS_UI,
					   &error))
	    g_error("failed to add UI: %s", error->message);
	prefs_window->dialog = GTK_WIDGET
	    (gtk_builder_get_object(prefs_window->builder, "prefs_dialog"));
	prefs_window->channel_store = GTK_LIST_STORE
	    (gtk_builder_get_object(prefs_window->builder,
				    "prefs_channel_store"));
	prefs_window->channel_view = GTK_WIDGET
	    (gtk_builder_get_object(prefs_window->builder,
				    "prefs_channel_view"));
	prefs_window->channel_label = GTK_WIDGET
	    (gtk_builder_get_object(prefs_window->builder,
				    "prefs_channel_label"));

	gtk_builder_add_callback_symbols
	    (prefs_window->builder,
	     "tg_prefs_channel_selection_changed_cb",
	     G_CALLBACK(tg_prefs_channel_selection_changed_cb),
	     "tg_prefs_channel_move_up_cb",
	     G_CALLBACK(tg_prefs_channel_move_up_cb),
	     "tg_prefs_channel_move_down_cb",
	     G_CALLBACK(tg_prefs_channel_move_down_cb),
	     "tg_prefs_channel_add_cb", G_CALLBACK(tg_prefs_channel_add_cb),
	     "tg_prefs_channel_delete_cb",
	     G_CALLBACK(tg_prefs_channel_delete_cb),
	     "tg_prefs_channel_edit_cb", G_CALLBACK(tg_prefs_channel_edit_cb),
	     NULL);
	gtk_builder_connect_signals(prefs_window->builder, NULL);

	if (close_cb)
	    tg_prefs_set_close_cb (close_cb);
    }

    tg_prefs_fill_channel_list();
    gtk_widget_show_all(prefs_window->dialog);
    gtk_dialog_run(GTK_DIALOG(prefs_window->dialog));
    gtk_widget_hide(prefs_window->dialog);
}
