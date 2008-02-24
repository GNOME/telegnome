/* main.c 
 * part of TeleGNOME, a GNOME app to view Teletext (Dutch)
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <string.h>

#include <gnome.h>
#include <glib.h>
#include <ghttp.h>
#include "main.h"
#include "http.h"
#include "gui.h"
#include "prefs.h"


int 
main (int argc, char **argv)
{
	GtkWidget *gui;

	bindtextdomain(PACKAGE,GNOMELOCALEDIR);
	textdomain(PACKAGE);

	gnome_program_init (PACKAGE, VERSION, LIBGNOMEUI_MODULE, argc, argv,
			    NULL);

	/* build gui, handle cmd line args */
	if ((argc >1) && (strlen(argv[1])<6)) {
		gui= new_gui (argv[1]);
	} else {
		gui= new_gui ("100");
	} 

	gtk_widget_show_all(GTK_WIDGET(gui));
	gtk_main();

	return 0;
}
