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

    GtkWidget *channel_list;
    GtkWidget *channel_label;
} TgPrefsWindow;

static TgPrefsWindow *prefs_window;

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
    gchar *info[2];

    gtk_clist_freeze(GTK_CLIST(prefs_window->channel_list));

    children = g_settings_get_strv(prefs_window->settings, "channel-children");
    for (childp = children; childp && *childp; ++childp) {
	channel = tg_channel_new(*childp, NULL);
	if (!channel)
	    continue;
	g_object_get(channel, "country", &info[0], "name", &info[1], NULL);
	newrow = gtk_clist_append(GTK_CLIST(prefs_window->channel_list), info);
	gtk_clist_set_row_data_full(GTK_CLIST(prefs_window->channel_list), newrow,
				    channel,
				    g_object_unref);
    }
    g_strfreev(children);
    gtk_clist_thaw(GTK_CLIST(prefs_window->channel_list));
}

/* pops up a modal dialog, editing the channel. */
static gboolean
tg_prefs_edit_channel(TgChannel *orig)
{
    GSettings *settings;
    GtkWidget *dialog, *table, *label, *name, *page, *subpage, *desc, *country, *frame;
    gint reply;
    gboolean changed = FALSE;

    g_assert(orig != NULL);

    settings = tg_channel_get_settings(orig);
    g_settings_delay(settings);

    dialog = gtk_dialog_new_with_buttons(
	_("New/Edit Channel"),
	GTK_WINDOW(prefs_window->dialog),
	GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
	GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
	GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT,
	NULL);

    table = gtk_table_new(5,2, FALSE);
    gtk_table_set_row_spacings(GTK_TABLE(table), 5);
    gtk_table_set_col_spacings(GTK_TABLE(table), 5);

    label = gtk_label_new(_("Name"));
    gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
    name = gtk_entry_new();
    g_settings_bind(settings, "name", name, "text", G_SETTINGS_BIND_DEFAULT);
    gtk_table_attach_defaults(GTK_TABLE(table), label, 0,1, 0,1);
    gtk_table_attach_defaults(GTK_TABLE(table), name, 1,2, 0,1);

    label = gtk_label_new(_("Description"));
    gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
    desc = gtk_entry_new();
    g_settings_bind(settings, "description", desc, "text", G_SETTINGS_BIND_DEFAULT);
    gtk_table_attach_defaults(GTK_TABLE(table), label, 0,1, 1,2);
    gtk_table_attach_defaults(GTK_TABLE(table), desc,  1,2, 1,2);

    label = gtk_label_new(_("Page url"));
    gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
    page = gtk_entry_new();
    g_settings_bind(settings, "page-url", page, "text", G_SETTINGS_BIND_DEFAULT);
    gtk_table_attach_defaults(GTK_TABLE(table), label, 0,1, 2,3);
    gtk_table_attach_defaults(GTK_TABLE(table), page,  1,2, 2,3);

    label = gtk_label_new(_("Subpage url"));
    gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
    subpage = gtk_entry_new();
    g_settings_bind(settings, "subpage-url", subpage, "text", G_SETTINGS_BIND_DEFAULT);
    gtk_table_attach_defaults(GTK_TABLE(table), label, 0,1,    3,4);
    gtk_table_attach_defaults(GTK_TABLE(table), subpage,  1,2, 3,4);

    label = gtk_label_new(_("Country"));
    gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
    country = gtk_entry_new();
    g_settings_bind(settings, "country", country, "text", G_SETTINGS_BIND_DEFAULT);
    gtk_table_attach_defaults(GTK_TABLE(table), label, 0,1, 4,5);
    gtk_table_attach_defaults(GTK_TABLE(table), country,  1,2, 4,5);

    frame = gtk_frame_new(_("Channel Information"));
    gtk_container_add(GTK_CONTAINER(frame), table);

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
    /* TODO memory leak: clear prefs_window->channel_list */
    g_clear_pointer(&prefs_window, g_free);
}

static void
tg_prefs_channel_list_click_cb(GtkWidget *clist, gint row, gint column,
			       GdkEventButton *event, gpointer data)
{
    TgChannel *channel;
    gchar *description;

    channel = gtk_clist_get_row_data(GTK_CLIST(clist), row);
    g_object_get(channel, "description", &description, NULL);
    gtk_label_set_text(GTK_LABEL(prefs_window->channel_label), description);
    g_free(description);
}

static void 
tg_prefs_sync_channel_children(void)
{
    GtkCList *channel_list;
    gchar **children;
    int i;
    TgChannel *channel;

    channel_list = GTK_CLIST(prefs_window->channel_list);
    children = g_new0(gchar *, channel_list->rows + 1);
    for (i = 0; i < channel_list->rows; ++i) {
	channel = gtk_clist_get_row_data(channel_list, i);
	g_object_get(channel, "uuid", &children[i], NULL);
    }
    g_settings_set_strv(prefs_window->settings, "channel-children",
			(const gchar **) children);
}

static void
tg_prefs_channel_add_cb(void)
{
    TgChannel *chan;
    gchar *info[2];

    chan = tg_channel_new(NULL, NULL);

    if (tg_prefs_edit_channel(chan)) {
	gchar *name;

	g_object_get(chan, "name", &name, NULL);
	if (strlen(name) > 0) {
	    GtkCList *channel_list = GTK_CLIST(prefs_window->channel_list);
	    gint new_row;

	    g_object_get(chan, "country", &info[0], "name", &info[1], NULL);
	    new_row = gtk_clist_append(channel_list, info);
	    gtk_clist_set_row_data(channel_list, new_row, (gpointer)chan);
	    tg_prefs_sync_channel_children();
	} else
	    g_object_unref(chan);
	g_free(name);
    } else
	g_object_unref(chan);
}

static void 
tg_prefs_channel_move_up_cb(void)
{
    GtkCList *channel_list;
    int row;

    channel_list = GTK_CLIST(prefs_window->channel_list);
    row = GPOINTER_TO_INT(channel_list->selection->data);

    if (row > 0) {
	gtk_clist_swap_rows(channel_list, row, row-1);
	tg_prefs_sync_channel_children();
    }
}

void 
tg_prefs_channel_move_down_cb(void)
{
    GtkCList *channel_list;
    int row;

    channel_list = GTK_CLIST(prefs_window->channel_list);
    row = GPOINTER_TO_INT(channel_list->selection->data);

    if (row < channel_list->rows - 1) {
	gtk_clist_swap_rows(channel_list, row, row+1);
	tg_prefs_sync_channel_children();
    }
}

static void
tg_prefs_channel_edit_cb(void)
{
    GtkCList *channel_list;
    int row;
    TgChannel *channel;

    channel_list = GTK_CLIST(prefs_window->channel_list);
    row = GPOINTER_TO_INT(channel_list->selection->data);

    channel = TG_CHANNEL(gtk_clist_get_row_data(channel_list, row));
    /* update the entry */
    if (tg_prefs_edit_channel(channel)) {
	/* ...and update the list.  The data is set automagically. */
	gchar *country, *name;
	g_object_get(channel, "country", &country, "name", &name, NULL);
	gtk_clist_set_text(channel_list, row, 0, country);
	gtk_clist_set_text(channel_list, row, 1, name);
	g_free(name);
	g_free(country);
    }
}

static void
tg_prefs_channel_delete_cb(void)
{
    GtkCList *channel_list;
    int row;
    TgChannel *channel;
    gchar *old_uuid;
    GSettingsBackend *backend;

    channel_list = GTK_CLIST(prefs_window->channel_list);
    if (channel_list->selection == NULL)
	return;

    row = GPOINTER_TO_INT(channel_list->selection->data);
    channel = TG_CHANNEL(gtk_clist_get_row_data(channel_list, row));
    g_object_get(channel, "uuid", &old_uuid, NULL);
    gtk_clist_remove(channel_list, row);
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
    GtkWidget *table, *frame, *label, *entry;
    GtkAdjustment *adj;

    g_assert(prefs_window != NULL);

    table = gtk_table_new(2,2, FALSE);
    gtk_table_set_row_spacings(GTK_TABLE(table), 5);
    gtk_table_set_col_spacings(GTK_TABLE(table), 5);

    label = gtk_label_new(_("Paging interval"));
    gtk_widget_set_tooltip_text(label, _("Specifies the interval for the auto-pager, in milliseconds."));

    adj = GTK_ADJUSTMENT(gtk_adjustment_new(8000.0, 1000.0, 60000.0, 1000.0, 10.0, 0.0));
    entry = gtk_spin_button_new(adj, 0.5, 0);

    gtk_table_attach_defaults(GTK_TABLE(table), label, 0,1, 0,1);
    gtk_table_attach_defaults(GTK_TABLE(table), entry, 1,2, 0,1);

    g_settings_bind(prefs_window->settings, "paging-interval", entry, "value", G_SETTINGS_BIND_DEFAULT);

    frame = gtk_frame_new(_("Miscelaneous"));

    gtk_container_set_border_width( GTK_CONTAINER(frame), 5);
    gtk_container_set_border_width( GTK_CONTAINER(table), 5);
    gtk_container_add( GTK_CONTAINER(frame), table);

    return frame;
}

static GtkWidget *
tg_prefs_construct_channels_page()
{
    GtkWidget *hbox, *vbox, *btn;
    char *titles[2] = { N_("Country"), N_("Name") };
    g_assert(prefs_window != NULL);

    hbox = gtk_hbox_new(FALSE, 0);

    vbox = gtk_vbox_new(FALSE, 0);

    /* the clist */
    prefs_window->channel_list = gtk_clist_new_with_titles( 2, titles );
    gtk_clist_set_column_width(GTK_CLIST(prefs_window->channel_list), 1, 200);
    gtk_box_pack_start(GTK_BOX(vbox), prefs_window->channel_list, TRUE, TRUE, 0);

    /* label for descriptions and stuff */
    prefs_window->channel_label = gtk_label_new("");
    gtk_container_set_border_width( GTK_CONTAINER(vbox), 10);
    gtk_box_pack_start(GTK_BOX(vbox), prefs_window->channel_label, TRUE, TRUE, 0);

    gtk_box_pack_start(GTK_BOX(hbox), vbox, TRUE, TRUE , 0);

    /* fill channel list */
    tg_prefs_fill_channel_list();
    
    g_signal_connect(G_OBJECT(prefs_window->channel_list), "select_row",
		     G_CALLBACK(tg_prefs_channel_list_click_cb),
		     NULL);

    vbox = gtk_vbox_new(TRUE, 0);
    
    /* move up button */
    btn = gtk_button_new_with_label(_("Move up"));
    gtk_box_pack_start(GTK_BOX(vbox), btn, FALSE, FALSE, 2);
    g_signal_connect(G_OBJECT(btn), "clicked", G_CALLBACK(tg_prefs_channel_move_up_cb), NULL);
    /* move down button */
    btn = gtk_button_new_with_label(_("Move down"));
    gtk_box_pack_start(GTK_BOX(vbox), btn, FALSE, FALSE, 2);
    g_signal_connect(G_OBJECT(btn), "clicked", G_CALLBACK(tg_prefs_channel_move_down_cb), NULL);
    /* add button */
    btn = gtk_button_new_with_label(_("Add..."));
    gtk_box_pack_start(GTK_BOX(vbox), btn, FALSE, FALSE, 2);
    g_signal_connect(G_OBJECT(btn), "clicked", G_CALLBACK(tg_prefs_channel_add_cb), NULL);

    /* delete button */
    btn = gtk_button_new_with_label(_("Delete"));
    gtk_box_pack_start(GTK_BOX(vbox), btn, FALSE, FALSE, 2);
    g_signal_connect(G_OBJECT(btn), "clicked", G_CALLBACK(tg_prefs_channel_delete_cb), NULL);

    /* edit buton */
    btn = gtk_button_new_with_label(_("Edit"));
    gtk_box_pack_start(GTK_BOX(vbox), btn, FALSE, FALSE, 2);
    g_signal_connect(G_OBJECT(btn), "clicked", G_CALLBACK(tg_prefs_channel_edit_cb), NULL);

    gtk_box_pack_start_defaults(GTK_BOX(hbox), vbox);
    
    gtk_container_set_border_width( GTK_CONTAINER(hbox), 5);
    return hbox;
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
	    GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
	    GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT,
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
