/* http.c */
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
  
#include <gnome.h>
#include <ghttp.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "http.h"
#include "main.h"
#include "prefs.h"

/*
 * get the pagenumber from the entrybox 
 */
int
get_page_entry (const gchar *page_entry)
{	
	guint page_nr;
	guint subpage_nr=0;
	gchar *page_entry_copy = g_strdup(page_entry);

	errno=0;

	/* TODO: Clean this up ;) */
	if (strlen (page_entry_copy) > 3) {  
		/* split in page and subpage */
		gchar *subpage_entry= &page_entry_copy[TG_PAGE_SIZE + 1];
		page_entry_copy[TG_PAGE_SIZE]='\0';
		subpage_nr= strtol(subpage_entry,NULL,10);
	}
   
	page_nr= strtol(page_entry_copy,NULL,10);
	if (0 != errno) {
		g_free(page_entry_copy);
		return -1;
	}
   
	currentview->page_nr=    page_nr;
	currentview->subpage_nr= subpage_nr;

	g_free(page_entry_copy);
	return 0;
}

/*
 * get the image from a remote site
 * if all's ok, return name in an GdkImlibImage
 */
gint
get_the_image (char **filename) 
{
    gint output;
    gint bytes_written;
    gchar http_query[100];
    gchar *http_proxy;
    gint retval=0;
    ghttp_request *request= NULL;
    
    if ( -1 == get_http_query(http_query, currentview->page_nr, currentview->subpage_nr))	
	return TG_ERR_HTTPQUERY;

    *filename=tempnam(NULL, TEMPNAM_PREFIX); 
    
    /* get the image from remote server */
    request= ghttp_request_new();
    if (-1 == ghttp_set_uri (request, http_query)) retval= TG_ERR_GHTTP;
		if ( http_proxy=gnome_config_get_string("/telegnome/Proxy/http_proxy") ) {
		    if (-1 == ghttp_set_proxy (request, http_proxy)) retval= TG_ERR_GHTTP; 
		}

    if (-1 == ghttp_set_sync (request, ghttp_sync)) retval= TG_ERR_GHTTP;
    if (-1 == ghttp_set_type (request, ghttp_type_get)) retval= TG_ERR_GHTTP;
    if (-1 == ghttp_prepare(request)) retval= TG_ERR_GHTTP;
    if (ghttp_done != ghttp_process(request)) retval= TG_ERR_GHTTP;
    
    /* write to file */
    if (-1 == (output=open(*filename, O_WRONLY | O_CREAT | O_SYNC | O_TRUNC))) 
	retval= TG_ERR_FILE;

    bytes_written= write(output, ghttp_get_body(request), ghttp_get_body_len(request));
    fsync(output); close(output);
    if ( bytes_written < PAGE_MINSIZE )
	return TG_ERR_TOOSMALL;

    /* set read permissions */
    if (0 != chmod (*filename, S_IRUSR)) retval= TG_ERR_CHMOD; 
    else {
	/* FIXME!!! */
	/* ghttp_request_destroy(request); */
	return TG_OK;
    }

    if (!(TG_ERR_FILE == retval)) 
	close (output);
    
    /* FIXME!!! */
    /* ghttp_request_destroy(request); */
    
    return retval;
}

int
get_http_query (gchar* buffer, gint page_nr, gint subpage_nr)
{
    if ( subpage_nr>0 ) {    /* do we have a subpage? */
	sprintf (  buffer, 
		   currentview->channel->subpage_url->str, 
		   page_nr, 
		   subpage_nr);
    } else {
	sprintf (  buffer, 
		   currentview->channel->page_url->str,
		   page_nr);
    }
    return 0;
}

/* gets rid of all those tmp files */
void 
cleanup()
{
    system("/bin/rm -f /tmp/" TEMPNAM_PREFIX "*");
}
