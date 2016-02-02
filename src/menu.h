/* TeleGNOME: the menu and toolbar 
   Arjan Scherpenisse
   31 march 2000
*/

#ifndef __MENU_H__
#define __MENU_H__

#include <glib/gi18n.h>
#include <gtk/gtk.h>

static const GtkActionEntry entries[] = {
    /* Top level */
    { "FileMenu", GTK_STOCK_FILE, N_("_File") },
    { "SettingsMenu", NULL, N_("_Settings") },
    { "ChannelsMenu", NULL, N_("_Channels") },
    { "HelpMenu", GTK_STOCK_HELP },

    /* File menu */
    { "FileQuit", GTK_STOCK_QUIT, NULL, "<control>Q", NULL,
      G_CALLBACK (tg_gui_cb_quit) },

    /* Settings menu */
    { "SettingsPreferences", GTK_STOCK_PREFERENCES, NULL, NULL, NULL,
      G_CALLBACK (tg_gui_cb_preferences) },

    /* Help menu */
    { "HelpContents", GTK_STOCK_HELP, N_("_Contents"), "F1", NULL,
      G_CALLBACK (tg_gui_cb_help_contents) },
    { "HelpAbout", GTK_STOCK_ABOUT, NULL, NULL, NULL,
      G_CALLBACK (tg_gui_cb_about) }
};

#endif
