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

#ifndef _HTTP_H_
#define _HTTP_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <glib.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <stdio.h>
#include <errno.h>

#define DEST_PORT 80
#define RECV_BUF_SIZE 20000

#define TEST_TELEGNOME 1
/* #undef TEST_TELEGNOME */

int get_page_entry (const gchar *page_entry);
gint get_the_image (GdkPixbuf **pixbuf);
int get_http_query (gchar* buffer, gint page_nr, gint subpage_nr);

#endif /* _HTTP_H_ */
