/* TeleGNOME: the menu and toolbar 
   Arjan Scherpenisse
   31 march 2000
*/

#ifndef __MENU_H__
#define __MENU_H__

/* the menu */
static GnomeUIInfo file_menu[] = {
    GNOMEUIINFO_MENU_EXIT_ITEM(cb_quit, NULL),
    GNOMEUIINFO_END
};

static GnomeUIInfo settings_menu[] = {
    GNOMEUIINFO_MENU_PREFERENCES_ITEM(cb_preferences, NULL),
    GNOMEUIINFO_END
};

static GnomeUIInfo help_menu[] = {
    GNOMEUIINFO_HELP(PACKAGE),
    GNOMEUIINFO_MENU_ABOUT_ITEM(cb_about, NULL),
    GNOMEUIINFO_END
};

static GnomeUIInfo menubar[] = {
    GNOMEUIINFO_MENU_FILE_TREE(file_menu),
    GNOMEUIINFO_MENU_SETTINGS_TREE(settings_menu),
    GNOMEUIINFO_MENU_HELP_TREE(help_menu),
    GNOMEUIINFO_END
 };

#endif
