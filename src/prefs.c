/* prefs.c
 * part of TeleGNOME, a GNOME app to view Teletext (Dutch)
 * This file constructs the preferences dialog.
 */

/*
** Copyright (C) 2000 Arjan Scherpenisse <acscherp@wins.uva.nl>
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

#include <gtk/gtk.h>
#include <libgnome/libgnome.h>
#include <libgnomeui/libgnomeui.h>

#include "prefs.h"
#include "channel.h"
#include "main.h"

/* not a good idea to have a 'misc' page, but i cant come up with a better name */
GtkWidget *construct_misc_page();
GtkWidget *construct_channels_page();

gboolean edit_channel(Channel *orig);		/* makes an edit dialog for the channel */
void edit_channel_changed(GtkWidget *dialog, gpointer data);	/* callback for changed */

void prefs_cancel_cb(void);
void prefs_apply_cb(GnomePropertyBox *propertybox, gint page_num);

void prefs_channel_add_cb(void);
void prefs_channel_move_up_cb(void);
void prefs_channel_move_down_cb(void);
void prefs_channel_edit_cb(void);
void prefs_channel_delete_cb(void);


void prefs_channel_list_click_cb( GtkWidget *clist, gint row, gint column,
				  GdkEventButton *event, gpointer data);
void fill_channel_list();
void prefs_channels_renum();

typedef struct _PrefsWindow {
    GnomePropertyBox *box;

    GtkWidget *page_entry;
    GtkWidget *sub_page_entry;

    GtkWidget *interval_entry;
    GtkWidget *proxy_entry;

    GtkWidget *channel_list;
    GtkWidget *channel_label;
    gint channel_count;
    
    void (*close_callback)();
} PrefsWindow;

static PrefsWindow *prefs_window;

void set_close_cb( void (*c)() )
{
    g_assert( prefs_window != NULL );
    prefs_window->close_callback = c;
}

void
prefs_show(void)
{
    GtkWidget *page;

    if (prefs_window != NULL) {
	gdk_window_show(gtk_widget_get_window(GTK_WIDGET(prefs_window->box)));
	gdk_window_raise(gtk_widget_get_window(GTK_WIDGET(prefs_window->box)));
    } else {
	prefs_window = g_malloc(sizeof(PrefsWindow));

	prefs_window->box = GNOME_PROPERTY_BOX (gnome_property_box_new());
	gtk_window_set_title (GTK_WINDOW(prefs_window->box), _("TeleGNOME: Preferences"));

	page = construct_channels_page();
	gtk_notebook_append_page(GTK_NOTEBOOK(prefs_window->box->notebook),
				 page,
				 gtk_label_new(_("Channels")));

	page = construct_misc_page();
	gtk_notebook_append_page(GTK_NOTEBOOK(prefs_window->box->notebook),
				 page,
				 gtk_label_new(_("Misc")));


	gtk_notebook_set_show_tabs (GTK_NOTEBOOK(prefs_window->box->notebook),
				    TRUE);
	gtk_notebook_set_show_border (GTK_NOTEBOOK(prefs_window->box->notebook),
				      TRUE);

	g_signal_connect (G_OBJECT (prefs_window->box), "apply",
			  G_CALLBACK (prefs_apply_cb), NULL);
	g_signal_connect (G_OBJECT (prefs_window->box), "destroy",
			  G_CALLBACK (prefs_cancel_cb), NULL);

	g_signal_connect_swapped(G_OBJECT(prefs_window->interval_entry), "changed",
				 G_CALLBACK(gnome_property_box_changed),
				 G_OBJECT(prefs_window->box));
	
	g_signal_connect_swapped(G_OBJECT(prefs_window->proxy_entry), "changed",
				 G_CALLBACK(gnome_property_box_changed),
				 G_OBJECT(prefs_window->box));


	/* and, show them all */
	gtk_widget_show_all(GTK_WIDGET(prefs_window->box));
    }
}

void 
fill_channel_list()
{
    int i, newrow;
    Channel *channel;
    char *info[2];

    prefs_window->channel_count = gnome_config_get_int_with_default("/telegnome/Channels/count=0", NULL);

    gtk_clist_freeze(GTK_CLIST(prefs_window->channel_list));

    for (i=0; i<prefs_window->channel_count; i++) {
	channel = channel_new_from_config(i);
	info[0] = channel->country->str;
	info[1] = channel->name->str;
	newrow = gtk_clist_append(GTK_CLIST(prefs_window->channel_list), info);
	gtk_clist_set_row_data_full(GTK_CLIST(prefs_window->channel_list), newrow,
				    channel,
				    (GDestroyNotify)(channel_free));
    }
    gtk_clist_thaw(GTK_CLIST(prefs_window->channel_list));
}

GtkWidget *
construct_misc_page()
{
    GtkWidget *table, *frame, *label, *entry, *proxy_label, *proxy_entry;
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

    gtk_spin_button_set_value(GTK_SPIN_BUTTON(entry), 
			      gnome_config_get_int_with_default("/telegnome/Paging/interval=" DEFAULT_INTERVAL,NULL));
    proxy_label = gtk_label_new(_("Proxy server"));
    proxy_entry = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(proxy_entry), 100);
    gtk_entry_set_text(GTK_ENTRY(proxy_entry), 
		       gnome_config_get_string_with_default("/telegnome/Proxy/http_proxy=" "",NULL));

    gtk_table_attach_defaults(GTK_TABLE(table), proxy_label, 0,1, 1,2);
    gtk_table_attach_defaults(GTK_TABLE(table), proxy_entry, 1,2, 1,2);

    frame = gtk_frame_new(_("Miscelaneous"));

    gtk_container_set_border_width( GTK_CONTAINER(frame), 5);
    gtk_container_set_border_width( GTK_CONTAINER(table), 5);
    gtk_container_add( GTK_CONTAINER(frame), table);

    prefs_window->interval_entry = entry;
    prefs_window->proxy_entry = proxy_entry;
    return frame;
}

GtkWidget *
construct_channels_page()
{
    GtkWidget *hbox, *vbox, *btn;
    char *titles[2] = { N_("Country"), N_("Name") };
    g_assert(prefs_window != NULL);

    hbox = gtk_hbox_new(FALSE, 0);

    vbox = gtk_vbox_new(FALSE, 0);

    /* the clist */
    prefs_window->channel_list = gtk_clist_new_with_titles( 2, titles );
    gtk_box_pack_start(GTK_BOX(vbox), prefs_window->channel_list, TRUE, TRUE, 0);

    /* label for descriptions and stuff */
    prefs_window->channel_label = gtk_label_new("");
    gtk_container_set_border_width( GTK_CONTAINER(vbox), 10);
    gtk_box_pack_start(GTK_BOX(vbox), prefs_window->channel_label, TRUE, TRUE, 0);

    gtk_box_pack_start(GTK_BOX(hbox), vbox, TRUE, TRUE , 0);

    /* fill channel list */
    fill_channel_list();
    
    g_signal_connect(G_OBJECT(prefs_window->channel_list), "select_row",
		     G_CALLBACK(prefs_channel_list_click_cb),
		     NULL);

    vbox = gtk_vbox_new(TRUE, 0);
    
    /* move up button */
    btn = gtk_button_new_with_label(_("Move up"));
    /* btn = gnome_stock_or_ordinary_button(GNOME_STOCK_PIXMAP_ADD); */
    gtk_box_pack_start(GTK_BOX(vbox), btn, FALSE, FALSE, 2);
    g_signal_connect(G_OBJECT(btn), "clicked", G_CALLBACK(prefs_channel_move_up_cb), NULL);
    /* move down button */
    btn = gtk_button_new_with_label(_("Move down"));
    /* btn = gnome_stock_or_ordinary_button(GNOME_STOCK_PIXMAP_ADD); */
    gtk_box_pack_start(GTK_BOX(vbox), btn, FALSE, FALSE, 2);
    g_signal_connect(G_OBJECT(btn), "clicked", G_CALLBACK(prefs_channel_move_down_cb), NULL);
    /* add button */
    btn = gtk_button_new_with_label(_("Add..."));
    /* btn = gnome_stock_or_ordinary_button(GNOME_STOCK_PIXMAP_ADD); */
    gtk_box_pack_start(GTK_BOX(vbox), btn, FALSE, FALSE, 2);
    g_signal_connect(G_OBJECT(btn), "clicked", G_CALLBACK(prefs_channel_add_cb), NULL);

    /* delete button */
    btn = gtk_button_new_with_label(_("Delete"));
    /* btn = gnome_stock_or_ordinary_button(GNOME_STOCK_PIXMAP_REMOVE); */
    gtk_box_pack_start(GTK_BOX(vbox), btn, FALSE, FALSE, 2);
    g_signal_connect(G_OBJECT(btn), "clicked", G_CALLBACK(prefs_channel_delete_cb), NULL);

    /* edit buton */
    btn = gtk_button_new_with_label(_("Edit"));
    /* btn = gnome_stock_or_ordinary_button(GNOME_STOCK_PIXMAP_PROPERTIES); */
    gtk_box_pack_start(GTK_BOX(vbox), btn, FALSE, FALSE, 2);
    g_signal_connect(G_OBJECT(btn), "clicked", G_CALLBACK(prefs_channel_edit_cb), NULL);

    gtk_box_pack_start_defaults(GTK_BOX(hbox), vbox);
    
    gtk_container_set_border_width( GTK_CONTAINER(hbox), 5);
    return hbox;
}

void 
edit_channel_changed(GtkWidget *dialog, gpointer data)
{
    gnome_dialog_set_sensitive(GNOME_DIALOG(data), 0, TRUE);
}


/* pops up a modal dialog, editing the channel. */
gboolean
edit_channel(Channel *orig)
{
    GtkWidget *dialog, *table, *label, *name, *page, *subpage, *desc, *country, *frame;
    int reply;
    gboolean changed = FALSE;

    g_assert(orig != NULL);

    dialog = gnome_dialog_new(_("New/Edit Channel"),
			   GNOME_STOCK_BUTTON_OK,
			   GNOME_STOCK_BUTTON_CANCEL, NULL);
    gnome_dialog_set_sensitive(GNOME_DIALOG(dialog), 0, FALSE);

    table = gtk_table_new(5,2, FALSE);
    gtk_table_set_row_spacings(GTK_TABLE(table), 5);
    gtk_table_set_col_spacings(GTK_TABLE(table), 5);
    
    
    label = gtk_label_new(_("Name"));
    gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
    name = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(name), orig->name->str);
    gtk_table_attach_defaults(GTK_TABLE(table), label, 0,1, 0,1);
    gtk_table_attach_defaults(GTK_TABLE(table), name, 1,2, 0,1);
    g_signal_connect(G_OBJECT(name), "changed", G_CALLBACK(edit_channel_changed), (gpointer)dialog);

    label = gtk_label_new(_("Description"));
    gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
    desc = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(desc), orig->desc->str);
    gtk_table_attach_defaults(GTK_TABLE(table), label, 0,1, 1,2);
    gtk_table_attach_defaults(GTK_TABLE(table), desc,  1,2, 1,2);
    g_signal_connect(G_OBJECT(desc), "changed", G_CALLBACK(edit_channel_changed), (gpointer)dialog);

    label = gtk_label_new(_("Page url"));
    gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
    page = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(page), orig->page_url->str);
    gtk_table_attach_defaults(GTK_TABLE(table), label, 0,1, 2,3);
    gtk_table_attach_defaults(GTK_TABLE(table), page,  1,2, 2,3);
    g_signal_connect(G_OBJECT(page), "changed", G_CALLBACK(edit_channel_changed), (gpointer)dialog);

    label = gtk_label_new(_("Subpage url"));
    gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
    subpage = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(subpage), orig->subpage_url->str);
    gtk_table_attach_defaults(GTK_TABLE(table), label, 0,1,    3,4);
    gtk_table_attach_defaults(GTK_TABLE(table), subpage,  1,2, 3,4);
    g_signal_connect(G_OBJECT(subpage), "changed", G_CALLBACK(edit_channel_changed), (gpointer)dialog);

    label = gtk_label_new(_("Country"));
    gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
    country = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(country), orig->country->str);
    gtk_table_attach_defaults(GTK_TABLE(table), label, 0,1, 4,5);
    gtk_table_attach_defaults(GTK_TABLE(table), country,  1,2, 4,5);
    g_signal_connect(G_OBJECT(country), "changed", G_CALLBACK(edit_channel_changed), (gpointer)dialog);

    frame = gtk_frame_new(_("Channel Information"));
    gtk_container_add(GTK_CONTAINER(frame), table);

    gtk_container_add( GTK_CONTAINER(GNOME_DIALOG(dialog)->vbox), frame);

    gtk_widget_show_all(dialog);

    reply = gnome_dialog_run(GNOME_DIALOG(dialog));

    if ( reply == 0 ) {
	/* update the server entry accordingly */
	g_string_assign(orig->name, gtk_entry_get_text(GTK_ENTRY(name)));
	g_string_assign(orig->desc,  gtk_entry_get_text(GTK_ENTRY(desc)));
	g_string_assign(orig->page_url,  gtk_entry_get_text(GTK_ENTRY(page)));
	g_string_assign(orig->subpage_url, gtk_entry_get_text(GTK_ENTRY(subpage)));
	g_string_assign(orig->country, gtk_entry_get_text(GTK_ENTRY(country)));
	changed = TRUE;
    } else {
	changed = FALSE;
    }
    gtk_widget_destroy(dialog);

    return changed;
}



/* *------------------------------------------------------------------------* */

/* pref window callbacks */
void 
prefs_channel_move_up_cb(void)
{
    GList *list;
    int row;
    
    if ((list = GTK_CLIST(prefs_window->channel_list)->selection) == NULL)
	return;
    row = GPOINTER_TO_INT(list->data);
    
    if (row >0) {
	gtk_clist_swap_rows (GTK_CLIST(prefs_window->channel_list), row, row-1);
	gnome_property_box_changed(GNOME_PROPERTY_BOX(prefs_window->box));
	prefs_channels_renum();
    }
}

void 
prefs_channel_move_down_cb(void)
{
    GList *list;
    int row;
    
    if ((list = GTK_CLIST(prefs_window->channel_list)->selection) == NULL)
	return;
    row = GPOINTER_TO_INT(list->data);

    if (row < GTK_CLIST( prefs_window->channel_list)->rows-1) {
	gtk_clist_swap_rows (GTK_CLIST(prefs_window->channel_list), row, row+1);
	gnome_property_box_changed(GNOME_PROPERTY_BOX(prefs_window->box));
	prefs_channels_renum();
    }
}


void
prefs_cancel_cb(void)
{
    g_object_unref(prefs_window->box);
    g_free(prefs_window);
    prefs_window = NULL;
}

void 
prefs_apply_cb(GnomePropertyBox *propertybox, gint page_num)
{
    int i;
    Channel *channel;

    if (page_num == -1) return;

    gnome_config_set_int("/telegnome/Paging/interval", 
			 gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(prefs_window->interval_entry)));
    gnome_config_set_string("/telegnome/Proxy/http_proxy", gtk_entry_get_text(GTK_ENTRY(prefs_window->proxy_entry)));

    gnome_config_set_int("/telegnome/Channels/count", GTK_CLIST(prefs_window->channel_list)->rows);

    for (i=0; i < GTK_CLIST(prefs_window->channel_list)->rows; i++) {
	channel = gtk_clist_get_row_data(GTK_CLIST(prefs_window->channel_list), i);
	channel_save_to_config(channel);
    }
	
    gnome_config_sync();

    (*prefs_window->close_callback)();
}

void prefs_channel_list_click_cb( GtkWidget *clist, gint row, gint column,
				  GdkEventButton *event, gpointer data)
{
    Channel *channel;
    channel = gtk_clist_get_row_data(GTK_CLIST(clist), row);
    
    gtk_label_set_text(GTK_LABEL(prefs_window->channel_label), 
		       channel->desc->str);
}

void prefs_channel_add_cb(void)
{
    Channel *chan;
    char *info[2];
    int id;

    chan = channel_new(-1,"","","","","");
    if (edit_channel(chan)) {
	if (chan->name->len > 0) {
	    info[0] = chan->country->str;
	    info[1] = chan->name->str;
	    id = gtk_clist_append(GTK_CLIST(prefs_window->channel_list), info);
	    chan->id = id;
	    gtk_clist_set_row_data(GTK_CLIST(prefs_window->channel_list), id, (gpointer)chan);
	    gnome_property_box_changed(GNOME_PROPERTY_BOX(prefs_window->box));
	}
    }
}

void prefs_channel_edit_cb(void)
{
	GList *list;
	int row;

	Channel *channel = NULL;

	if ((list = GTK_CLIST(prefs_window->channel_list)->selection) == NULL)
		return;
	row = GPOINTER_TO_INT(list->data);

	channel = (Channel *)gtk_clist_get_row_data(GTK_CLIST(prefs_window->channel_list), row);
	/* update the entry */
	if (edit_channel (channel)) {
	    /* ...and update the list. the data is set automagicly. */
	   gtk_clist_set_text(GTK_CLIST(prefs_window->channel_list), row, 0, channel->country->str);
	   gtk_clist_set_text(GTK_CLIST(prefs_window->channel_list), row, 1, channel->name->str);
	   gnome_property_box_changed(GNOME_PROPERTY_BOX(prefs_window->box));
	}

}

void prefs_channel_delete_cb(void)
{
    GList *list;
    int row;
    
    if ((list = GTK_CLIST(prefs_window->channel_list)->selection) == NULL)
	return;
    
    row = GPOINTER_TO_INT(list->data);
    gtk_clist_remove(GTK_CLIST(prefs_window->channel_list), row);

   prefs_channels_renum();

    gnome_property_box_changed(GNOME_PROPERTY_BOX(prefs_window->box));
}

void 
prefs_channels_renum()
{
    int i;
    Channel *channel;
    for (i=0; i < GTK_CLIST(prefs_window->channel_list)->rows; i++) {
	channel = gtk_clist_get_row_data(GTK_CLIST(prefs_window->channel_list), i);
	channel->id = i;
    }
}
