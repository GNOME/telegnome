/* prefs.h
 * part of TeleGNOME, a GNOME app to view Teletext (Dutch)
 */

/*
** Copyright (C) 1999 Dirk-Jan C. Binnema <djcb@dds.nl>
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


#ifndef __PREFS_H__
#define __PREFS_H__

#include <gtk/gtk.h>

/* some defaults */
#define DEFAULT_SUB_PAGE_URL  	"http://teletekst.nos.nl/cgi-bin/tt/nos/gif/%d-%d"
#define DEFAULT_PAGE_URL      	"http://teletekst.nos.nl/cgi-bin/tt/nos/gif/%d/"
#define DEFAULT_INTERVAL      	"12000"

#define TELEGNOME_LOGO		"telegnome/telegnome-logo.png"
#define TELEGNOME_NOTFOUND	"telegnome/telegnome-logo.png"

void tg_prefs_show(void);
void tg_prefs_set_close_cb( void (*c)(void) );

#endif /* __PREFS_H__ */
