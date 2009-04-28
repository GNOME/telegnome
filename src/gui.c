/*
 * gui.c 
 */

/*
**    Copyright (C) 1999, 2000,
**    Dirk-Jan C. Binnema <djcb@dds.nl>,
**    Arjan Scherpenisse <acscherp@wins.uva.nl>
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

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <libgnome/libgnome.h>
#include <libgnomeui/libgnomeui.h>

#include "gui.h"
#include "main.h"
#include "prefs.h"
#include "menu.h"
#include "channel.h"

static Gui gui;

void prefs_close_cb();
void update_title_bar();
void refresh_timer();

gint gui_keyboard_timer(gpointer g);
gint gui_logo_timer(gpointer g);
gint gui_pager_timer(gpointer g);
void gui_restore_session(void);
void gui_save_session(void);
void cb_toggle_paging(GtkWidget *w, gpointer data);

void die (GnomeClient *client, gpointer client_data);
int 
save_yourself(GnomeClient *client, int phase, GnomeSaveStyle save_style, int shutdown, 
	      GnomeInteractStyle interact_style, int fast, gpointer client_data);

void load_channels_from_config();
GtkWidget *create_channel_menu();
void gui_channel_select(GtkWidget *w, gpointer data);
void refresh_channel_menu();

/*******************************
 * return the app gui, with startpage or NULL
 */
GtkWidget *
new_gui (gchar* startpage) 
{
    GtkWidget *app, *toolbar, *statusbar;
    GdkPixbuf *pixbuf;
    GError *error = NULL;

    /* the app */
    app= gnome_app_new (PACKAGE, _("TeleGNOME: Teletext for GNOME"));
    /* gtk_window_set_policy(GTK_WINDOW (app), FALSE, FALSE, TRUE); */
    gtk_widget_realize(GTK_WIDGET(app));

    toolbar= new_toolbar();

    /* attach a keyboard event */
    g_signal_connect (G_OBJECT (app),
		      "key_press_event",
		      G_CALLBACK (cb_keypress), NULL);
    
    /* attach the menu */
    gnome_app_create_menus(GNOME_APP(app), menubar);

    gnome_app_add_toolbar(GNOME_APP(app), GTK_TOOLBAR(toolbar), "nav_toolbar", 0, BONOBO_DOCK_TOP, 2, 0, 0);

    /* the view */
    currentview = tele_view_new();

    tele_view_set_error_handler(currentview, print_in_statusbar);
    /* the statusbar */
    statusbar= gnome_appbar_new(TRUE,TRUE,GNOME_PREFERENCES_NEVER);
    gnome_app_set_statusbar(GNOME_APP(app), statusbar);

    /* make menu hints display on the appbar */
    gnome_app_install_menu_hints(GNOME_APP(app), menubar);

    /* all the contents */
    gnome_app_set_contents(GNOME_APP(app), tele_view_get_widget(currentview));

    /* save some pointers for reference later */

    gui.statusbar= statusbar;
    gui.app= app;

    g_signal_connect (G_OBJECT (app), "delete_event",
		      G_CALLBACK (cb_quit),
		      NULL);

    gui.client = gnome_master_client();
    g_signal_connect (G_OBJECT (gui.client), "save_yourself",
		      G_CALLBACK (save_yourself),
		      NULL); /* fixme? */
    g_signal_connect (G_OBJECT (gui.client), "die",
		      G_CALLBACK (die), NULL);

    
    gtk_widget_show_all(app);

    gui_restore_session();

    gui.channels = NULL;
    gui.channel_menu = NULL;

    refresh_channel_menu();

    /* FIXME: */
    /* set the current view, at elem 0 */
    currentview->channel = (Channel *)g_slist_nth_data(gui.channels, gui.default_server);

    update_title_bar();

    /* check if we are connected to a session manager. If so,
       load up the last page we were visiting. Otherwise,
       start with a logo */
    update_entry(0,0);
    pixbuf = gdk_pixbuf_new_from_file(
	gnome_program_locate_file(NULL, GNOME_FILE_DOMAIN_PIXMAP,
				  TG_LOGO_PIXMAP, TRUE, NULL),
	&error);
    tele_view_update_pixmap(currentview, pixbuf);
    g_object_unref(pixbuf);
    
    /* only auto-change to a page if it was saved the last time */

    if (currentview->page_nr >0 )
	gui.logo_timer = gtk_timeout_add(TG_LOGO_TIMEOUT,gui_logo_timer, NULL);
    else
	gui.logo_timer = -1;
    
    /*
    if (GNOME_CLIENT_CONNECTED (gui.client)) {
	update_entry(currentview->page_nr, currentview->subpage_nr);
	get_the_page(TRUE);
	g_print("we are connected to a session manager");
    } else {
	g_print("we are NOT connected to a session manager");
    }
    */

    return app;
}

/* SESSION MANAGEMENT CALLS */
void
die (GnomeClient *client, gpointer client_data)
{
    /* Just exit in a friendly way.  We don't need to
       save any state here, because the session manager
       should have sent us a save_yourself-message
       before.  */
    gtk_exit (0);
}

int 
save_yourself(GnomeClient *client, int phase,
	      GnomeSaveStyle save_style, int shutdown,
	      GnomeInteractStyle interact_style, int fast,
	      gpointer client_data)
{
    /*this is a "discard" command for discarding data from
      a saved session, usually this will work*/
    char *argv[]= { "rm", "-r", NULL };
    
    /* Save the state using gnome-config stuff. */
    gui_save_session();

    /* Here is the real SM code. We set the argv to the
       parameters needed to restart/discard the session that
       we've just saved and call the
       gnome_session_set_*_command to tell the session
       manager it. */
    /* argv[2]= gnome_config_get_real_path (prefix); */
    gnome_client_set_discard_command (client, 3, argv);
    
    /* Set commands to clone and restart this application.
       Note that we use the same values for both -- the
            session management code will automatically add
            whatever magic option is required to set the session
            id on startup. The client_data was set to the
            command used to start this application when
            save_yourself handler was connected. */
    argv[0]= (gchar*) client_data;
    gnome_client_set_clone_command (client, 1, argv);
    gnome_client_set_restart_command (client, 1, argv);
    
    return TRUE;
 }
void
gui_restore_session(void)
{
    /* the kb timer */
    gui.kb_timer = -1;
    gui.kb_status = INPUT_NEW;

    gui.page_progress = 0;
    gui.page_timer = -1;
    gui.page_status = FALSE;
    gui.default_server = gnome_config_get_int_with_default("/telegnome/Default/server=0", NULL);

    gui.page_msecs = gnome_config_get_int_with_default("/telegnome/Paging/interval=" DEFAULT_INTERVAL, NULL);
    gui.progress = gnome_appbar_get_progress(GNOME_APPBAR(gui.statusbar));
    gtk_progress_bar_set_fraction(gui.progress, 0.0);

    /* the zoom button */
    /* FIXME */ /*
    currentview->zoom_factor=gnome_config_get_int_with_default("/telegnome/Zooming/factor=1", NULL);
    gtk_label_set(GTK_LABEL(gui.zoomlabel), currentview->zoom_factor==1?"100%":"400%");
    if (currentview->zoom_factor==2) gtk_toggle_button_toggled(GTK_TOGGLE_BUTTON(gui.zoombutton));
		*/

    /* the current page */
    currentview->page_nr = gnome_config_get_int_with_default("/telegnome/Paging/page_nr=-1", NULL);
    currentview->subpage_nr = gnome_config_get_int_with_default("/telegnome/Paging/subpage_nr=-1", NULL);

    /* g_print("Number: %d/%d\n", currentview->page_nr, currentview->subpage_nr); */
}

void gui_save_session(void)
{
    gnome_config_set_bool("/telegnome/Paging/enabled", gui.page_status);
    gnome_config_set_int("/telegnome/Paging/interval", gui.page_msecs);
    gnome_config_set_int("/telegnome/Paging/page_nr", currentview->page_nr);
    gnome_config_set_int("/telegnome/Paging/subpage_nr", currentview->subpage_nr);
    gnome_config_set_int("/telegnome/Zooming/factor", currentview->zoom_factor);
    gnome_config_set_int("/telegnome/Default/server", gui.default_server);
    gnome_config_sync();
}


/*******************************
 * create a new toolbar 
 */
GtkWidget *
new_toolbar ()
{
    GtkWidget *icon, *toolbar, *entry, *hbox, *w;
    
    toolbar= gtk_toolbar_new();

    hbox = gtk_hbox_new(FALSE, 0);
    
    w = gtk_label_new(_("Page:"));
    gtk_box_pack_start(GTK_BOX(hbox), w, FALSE, FALSE, 5);
    
    /* add the entry */
    entry= new_entry();
    gtk_widget_set_tooltip_text(entry, _("Page number"));

    gtk_box_pack_start(GTK_BOX(hbox), entry, FALSE, FALSE, 5);
    gtk_toolbar_append_widget (GTK_TOOLBAR(toolbar), hbox, "", "");

    icon = gtk_image_new_from_stock(GTK_STOCK_JUMP_TO,
				    GTK_ICON_SIZE_LARGE_TOOLBAR);
    gtk_toolbar_append_item(GTK_TOOLBAR(toolbar), 
			    NULL,  _("Go To Page"),
			    NULL, icon, G_CALLBACK(cb_goto_page), NULL);
    
    icon = gtk_image_new_from_stock(GTK_STOCK_GO_BACK,
				    GTK_ICON_SIZE_LARGE_TOOLBAR);
    gtk_toolbar_append_item(GTK_TOOLBAR(toolbar), 
			    NULL,  _("Get Previous Page"),
			    NULL, icon, G_CALLBACK(cb_prev_page), NULL);
    
    icon = gtk_image_new_from_stock(GTK_STOCK_GO_FORWARD,
				    GTK_ICON_SIZE_LARGE_TOOLBAR);
    gtk_toolbar_append_item(GTK_TOOLBAR(toolbar), 
			    NULL, _("Get Next Page"),
			    NULL, icon, G_CALLBACK(cb_next_page), NULL);
    
    icon = gtk_image_new_from_stock(GTK_STOCK_HOME,
				    GTK_ICON_SIZE_LARGE_TOOLBAR);
    gtk_toolbar_append_item(GTK_TOOLBAR(toolbar), 
			    NULL, _("Go to the home page"),
			    NULL, icon, G_CALLBACK(cb_home), NULL);
    
    icon = gtk_image_new_from_stock(GTK_STOCK_MEDIA_PLAY,
				    GTK_ICON_SIZE_LARGE_TOOLBAR);
    w = gtk_toggle_button_new();
    gui.pagebutton = w;
    gtk_container_add(GTK_CONTAINER(w), icon);
    g_signal_connect(G_OBJECT(w), "clicked",
		     G_CALLBACK(cb_toggle_paging), NULL);
    gtk_toolbar_append_widget(GTK_TOOLBAR(toolbar), w, _("Toggles auto-paging"), NULL);

    /* FIXME */ /*
    gui.zoomlabel = gtk_label_new(_("100%"));
    w = gtk_toggle_button_new();
    gui.zoombutton = w;
    gtk_container_add(GTK_CONTAINER(w), gui.zoomlabel);
    g_signal_connect(G_OBJECT(w), "clicked", G_CALLBACK(cb_zoom), NULL);
    gtk_toolbar_append_widget(GTK_TOOLBAR(toolbar), w, _("Toggles zooming"), NULL);
    */

    /* 
    w = gtk_combo_new();
    gtk_toolbar_append_widget(GTK_TOOLBAR(toolbar), w, _("Selects a channel"), NULL);
    */
    gtk_widget_show_all(toolbar);
    
    return toolbar;
}

/*************************
 * Loads all the channels from the config and puts them in the gui.channels GSList
 */
void
load_channels_from_config()
{
    int count,i;
    Channel *channel;

    if (gui.channels != NULL) {
	GSList *old_channels = gui.channels;
	while (old_channels != NULL) {
	    channel_free((Channel *)old_channels->data);
	    old_channels = g_slist_next(old_channels);
	}
	g_slist_free(gui.channels);
	gui.channels = NULL;
    }

    count = gnome_config_get_int_with_default("/telegnome/Channels/count=0", NULL);
    if (count > 0) {
	for (i=0; i<count; i++) {
	    channel = channel_new_from_config(i);
	    gui.channels = g_slist_append(gui.channels, (gpointer)channel);
	}
    } else {
	/* nothing set up yet, fill in some default */
	count = 1;
	channel = channel_new(0, "NOS Teletext", "The Dutch teletext pages",
			      "http://teletekst.nos.nl/cgi-bin/tt/nos/gif/%d/",
			      "http://teletekst.nos.nl/cgi-bin/tt/nos/gif/%d-%d",
			      "nl");
	gui.channels = g_slist_append(gui.channels, (gpointer)channel);
	/* ...and save it to the config */
	gnome_config_set_int("/telegnome/Channels/count", 1);
	channel_save_to_config(channel);
    }
}

/*************************
 * create the channel menu
 */
GtkWidget *
create_channel_menu()
{
    GtkWidget *menu, *item;
    int i;
    Channel *channel;

    g_assert(gui.channels != NULL);
    menu = gtk_menu_new();

    for (i=0; i<g_slist_length(gui.channels); i++) {
	channel = (Channel *)g_slist_nth_data(gui.channels, i);

	item = gtk_menu_item_new_with_label(channel->name->str);

	g_signal_connect(G_OBJECT(item), "activate",
			 G_CALLBACK(gui_channel_select), (gpointer)channel);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
	gtk_widget_show(item);
    }

    item = gtk_menu_item_new_with_label(_("Channels"));
    gtk_widget_show(item);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(item), menu);

    return item;
}

void update_title_bar()
{
    char buf[100];
    /* update the title bar */
    if ((currentview != NULL) && (currentview->channel != NULL) &&
	(currentview->channel->name != NULL) && (currentview->channel->desc != NULL)) {
	sprintf(buf, _("TeleGNOME: %s (%s)"), currentview->channel->name->str, currentview->channel->desc->str);
	gtk_window_set_title(GTK_WINDOW(gui.app), buf);
    }
}

void refresh_channel_menu()
{
    /* dispose the menu if it was already added */
    if (gui.channel_menu != NULL) {
	g_object_unref(gui.channel_menu);
    }
    
    /* load the channels from disk */
    load_channels_from_config();

    /* create the menu */
    gui.channel_menu = create_channel_menu();
    
    /* and add it to the menu bar */
    gtk_menu_shell_insert(GTK_MENU_SHELL(GNOME_APP(gui.app)->menubar), gui.channel_menu, 2);
}

/******************************* 
 * create a new entry 
 */
GtkWidget * 
new_entry ()
{
	GtkWidget *entry=NULL;
	entry=gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(entry),
				 TG_PAGE_SIZE + 1 + TG_SUBPAGE_SIZE);
	
	/* hack */
	gtk_widget_set_usize(GTK_WIDGET(entry),
			     (8 * (TG_PAGE_SIZE + 1 + TG_SUBPAGE_SIZE)),20);
	
	/*gtk_entry_set_text(GTK_ENTRY(entry), startpage);*/
	      
	g_signal_connect(G_OBJECT(entry), "activate", 
			 G_CALLBACK(cb_goto_page),NULL);

	/* save entry for later ref */
	gui.entry= entry;

	return entry;
}

/*******************************
 * print a string in the statusbar
 */
void
print_in_statusbar(const char *buf)  /*FIXME: buffersize*/
{
    g_assert(buf != NULL);
    gnome_appbar_set_status(GNOME_APPBAR(gui.statusbar), buf);
    gtk_widget_show(GTK_WIDGET(gui.statusbar));
}

/*******************************
 * update the entry box with the current values of page & subpage
 */
int
update_entry(gint page_nr, gint subpage_nr)
{
    gchar full_num[10];
    
    if (subpage_nr > 0)
	sprintf(full_num,"%d/%d", page_nr, subpage_nr);
    else
	sprintf(full_num,"%d", page_nr);
    gtk_entry_set_text(GTK_ENTRY(gui.entry), full_num);
    
    return 0;
}

/*******************************
 * try to get the page, and do something smart if it fails
 * if redraw == TRUE, redraws whole app
 * (needed when zoom_factor has changed)
 */
void
get_the_page(gboolean redraw)
{
    /* hide the app */
    if (redraw)
	gtk_widget_hide(GTK_WIDGET(gui.app));
    
    /* stop the logo timer */
    if (gui.logo_timer != -1)
	gtk_timeout_remove(gui.logo_timer);
    gui.logo_timer = -1;

    tele_view_update_page(currentview, &currentview->page_nr, &currentview->subpage_nr);

    update_entry(currentview->page_nr, currentview->subpage_nr);
    print_in_statusbar ("");

    if (redraw) 
      gtk_widget_show_all (GTK_WIDGET(gui.app));
}


/******************************* 
 * callbacks 
 */
void
cb_quit (GtkWidget* widget, gpointer data)
{
    /* save some */
    gui_save_session();

    /* free the channels */
    if (gui.channels != NULL) {
	GSList *old_channels = gui.channels;
	while (old_channels != NULL) {
	    channel_free((Channel *)old_channels->data);
	    old_channels = g_slist_next(old_channels);
	}
	g_slist_free(gui.channels);
	gui.channels = NULL;
    }
    tele_view_free(currentview);

    /* get outta here ;) */
    gtk_main_quit();
}

void
cb_about (GtkWidget* widget, gpointer data)
{
    static GtkWidget *about;
    const gchar *authors[]= { "Dirk-Jan C. Binnema <djcb@dds.nl>",
			      "Arjan Scherpenisse <acscherp@wins.uva.nl>",
			      "Colin Watson <cjwatson@debian.org>",
			      NULL    };

    if (about) {
	gdk_window_show(about->window);
	gdk_window_raise(about->window);
	return;
    }

    about = gtk_about_dialog_new();
    g_object_set(
	about,
	"program-name", PACKAGE,
	"version", VERSION,
	"copyright", "\xc2\xa9 1999, 2000 Dirk-Jan C. Binnema, "
		     "Arjan Scherpenisse; "
		     "\xc2\xa9 2008 Colin Watson",
	"comments", _("Teletext for GNOME"),
	"license", _("GNU General Public License, version 2 or later"),
	"website", "http://telegnome.sourceforge.net/",
	"authors", authors,
	"translator-credits", _("translator-credits"),
	NULL);

    gtk_window_set_transient_for(GTK_WINDOW(about), GTK_WINDOW(gui.app));
    gtk_window_set_destroy_with_parent(GTK_WINDOW(about), TRUE);

    g_signal_connect(about, "destroy", G_CALLBACK(gtk_widget_destroyed),
		     &about);
    g_signal_connect(about, "response", G_CALLBACK(gtk_widget_destroy),
		     NULL);

    gtk_widget_show(about);
}

void 
cb_preferences (GtkWidget* widget, gpointer data)
{
    prefs_show();
    set_close_cb(prefs_close_cb);
}

void
cb_next_page (GtkWidget* widget, gpointer data)
{	
    if (currentview->subpage_nr == 0) 
	currentview->page_nr++;
    else
	currentview->subpage_nr++;
    
    update_entry(currentview->page_nr, currentview->subpage_nr);
    get_the_page(FALSE); /* dont redraw */ 
}

void
cb_prev_page (GtkWidget* widget, gpointer data)
{	
    if (currentview->subpage_nr>0)
	currentview->subpage_nr--;
    if (currentview->subpage_nr==0)
	currentview->page_nr--;
    
    update_entry(currentview->page_nr, currentview->subpage_nr);
    get_the_page(FALSE);
}

void
cb_home (GtkWidget* widget, gpointer data)
{	
    currentview->subpage_nr=0;
    currentview->page_nr=100;
    
    update_entry(currentview->page_nr, currentview->subpage_nr);
    get_the_page(FALSE);
}

void
cb_goto_page (GtkWidget *widget, gpointer data)
{	
	      
    gui.kb_status = INPUT_NEW;
    if ( -1 == get_page_entry (gtk_entry_get_text(GTK_ENTRY(gui.entry)))) {
	print_in_statusbar(_("Error in page entry"));
	return;
    }
    get_the_page(FALSE);
}


/*
 * handler for zoom button
 */
void 
cb_zoom(GtkWidget *widget, gpointer data)
{
    /* new: just toggle it on click */
    if (currentview->zoom_factor==1) {
	gtk_label_set_text(GTK_LABEL(gui.zoomlabel),"400%");
	currentview->zoom_factor=2;
    } else if (currentview->zoom_factor==2) {
	gtk_label_set_text(GTK_LABEL(gui.zoomlabel),"100%");
	currentview->zoom_factor=1;
    }		
    /* now, get the page with the new zoom settings */
    get_the_page(TRUE);
}

void
cb_drag (GtkWidget *widget, GdkDragContext *context, GtkSelectionData *selection_data,
	 guint info, guint32 time)
{
	const gchar *entry;

	switch (info) {

	case TARGET_GIF:
		break;
		
	case TARGET_FILE:
		break;

	case TARGET_ROOTWIN:
		break;

	case TARGET_TXT:

	default:	
		entry=gtk_entry_get_text(GTK_ENTRY(gui.entry));
		gtk_selection_data_set(selection_data, 
				       selection_data->target, 8,
				       (const guchar *)entry, strlen(entry));
		
		break;
	}
}

gint 
cb_keypress (GtkWidget *widget, GdkEventKey *event)
{
    if (event->keyval == GDK_KP_Enter) {
	cb_goto_page(NULL, NULL);
	update_entry(currentview->page_nr, currentview->subpage_nr);
	return 0;
    }
    
    /* g_print("keypress\n"); */
    if (!gtk_widget_is_focus(GTK_WIDGET(gui.entry)))
	gtk_widget_grab_focus(GTK_WIDGET(gui.entry));

    if (gui.kb_status == INPUT_NEW) {
	gtk_entry_set_text(GTK_ENTRY(gui.entry), "");
    }

    if (gui.kb_timer != -1) 
	gtk_timeout_remove(gui.kb_timer);
    gui.kb_timer = gtk_timeout_add(TG_KB_TIMEOUT, gui_keyboard_timer, NULL);
    gui.kb_status = INPUT_CONTINUED;
    return 0;
}

gint 
gui_keyboard_timer(gpointer g) 
{
    gtk_timeout_remove(gui.kb_timer);
    gui.kb_timer = -1;
    gui.kb_status = INPUT_NEW;
    return 0;
}


void 
cb_toggle_paging(GtkWidget *w, gpointer data) 
{
    gui.page_msecs = gnome_config_get_int_with_default("/telegnome/Paging/interval=" DEFAULT_INTERVAL, NULL);
    gtk_progress_bar_set_fraction(gui.progress, 0.0);
    if (gui.page_status==TRUE) {
	if (gui.page_timer != -1) gtk_timeout_remove(gui.page_timer);
	gui.page_timer = -1;
	gui.page_status = FALSE;
	gui.page_progress = 0;
    } else {
	gui.page_progress = 0;
	gui.page_status = TRUE;
	gui.page_timer = gtk_timeout_add(gui.page_msecs/100, gui_pager_timer, NULL);
    }
}

gint
gui_pager_timer(gpointer g)
{
    gui.page_progress += gui.page_msecs/100;
    gtk_progress_bar_set_fraction(gui.progress, gui.page_progress / (gdouble)gui.page_msecs);

    if (gui.page_progress >= gui.page_msecs) {
	gui.page_progress = 0;
	gtk_progress_bar_set_fraction(gui.progress, 0.0);
	cb_next_page(NULL, NULL);
    }
    return 1;
}


/* removes the logo from the screen and goes to a page */
gint 
gui_logo_timer(gpointer g) 
{
    if (gui.logo_timer != -1)
	gtk_timeout_remove(gui.logo_timer);
    gui.logo_timer = -1;
    get_the_page(FALSE);
    return 0;
}

/* changes the channel */
void 
gui_channel_select(GtkWidget *w, gpointer data)
{
    Channel *channel;
    g_assert(data != NULL);

    channel = (Channel *)data;

    currentview->channel = channel;
    currentview->page_nr = 100;
    currentview->subpage_nr = 0;

    update_title_bar();

    /* g_print("Channel Selected: %s (%s)\n", channel->name->str, channel->desc->str); */
    get_the_page(FALSE);
}

void refresh_timer()
{
    gdouble perc = gtk_progress_bar_get_fraction(gui.progress);

    gui.page_msecs = gnome_config_get_int_with_default("/telegnome/Paging/interval=" DEFAULT_INTERVAL, NULL);
    gui.progress = gnome_appbar_get_progress(GNOME_APPBAR(gui.statusbar));
    gtk_progress_bar_set_fraction(gui.progress, perc);

    if (gui.page_status == TRUE) {
	gtk_timeout_remove(gui.page_timer);
	gui.page_timer = gtk_timeout_add(gui.page_msecs/100, gui_pager_timer, NULL);
    }
    
    gui.page_progress =(int)((gui.page_msecs/100)*perc);
}

void prefs_close_cb()
{
    refresh_channel_menu();
    refresh_timer();
}
