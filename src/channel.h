/*
 * channel.h
 * A channel
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

#ifndef _CHANNEL_H_
#define _CHANNEL_H_

#include <gnome.h>

typedef struct _Channel {
    gint id;
    GString *name,
	*desc,
	*page_url,
	*subpage_url,
	*country;
} Channel;

Channel *channel_new(int id, const char *name, const char *desc, const char *page_url, const char *subpage_url, const char *country);
Channel *channel_new_from_config(int id);
void channel_save_to_config(Channel *channel);
void channel_free(Channel *channel);

#endif
