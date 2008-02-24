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

#ifndef _MAIN_H_
#define _MAIN_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include "gui.h"
#include "http.h"

/* 
typedef struct _TeleGnome{
    guint page_nr; 
    guint subpage_nr;
} TeleGnome;
*/


#define TG_STARTPAGE 100
#define TG_MIN_PAGE 100
#define TG_MAX_PAGE 999

#define TG_PAGE_SIZE 3     /* size of page size (=strlen("100")) */
#define TG_SUBPAGE_SIZE 2  /* size of subpage (ie. 100/<subpage> */

/* errors from HTTP connections */
#define TG_OK 0

#define TG_ERR_PIXBUF 1        /* corrupt image */
#define TG_ERR_VFS 2
#define TG_ERR_HTTPQUERY 3     /* error getting http query */

#define TG_NOTFOUND_PIXMAP 	"telegnome/testbeeld.png"
#define TG_LOGO_PIXMAP 		"telegnome/telegnome-logo.png"

#define TG_KB_TIMEOUT		2500 	/* the timeout before the input fields resets */
#define TG_LOGO_TIMEOUT		7500	/* the time the logo gets displayed */

/* TeleGnome telegnome; */

TeleView *currentview;

#endif /* _MAIN_H_ */
