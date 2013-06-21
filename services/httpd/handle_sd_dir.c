/*
 * Copyright (c) 2009 by Stefan Siegl <stesie@brokenpipe.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * For more information on the GPL, please go to:
 * http://www.gnu.org/copyleft/gpl.html
 */

#include "config.h"
#include "httpd.h"

const char PROGMEM httpd_sd_dir_header[] =
    "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 3.2 Final//EN\">"
    "<html><body>"
    "<h1>Index of %s</h1>"
    "<table><tr><th>Name</th><th>Size</th></tr>\n";

const char PROGMEM httpd_sd_dir_parent[] =
    "<tr><td><a href='%s'>Parent Directory</a></td><td>-</td></tr>";

const char PROGMEM httpd_sd_dir_footer[] =
    "</table><hr>"
    "<address>Your friendly Ethersex HTTP service with VFS and mod_m4 :-)</address>"
    "</body></html>";

const char PROGMEM httpd_sd_dir_entry[] =
    "<tr><td><a href='%s'>%s</a></td><td>%ld</td></tr>\n";

const char PROGMEM httpd_sd_dir_entry_dir[] =
    "<tr><td><a href='%s/'>%s</a></td><td>-</td></tr>";

const char PROGMEM httpd_header_301_redirect[] =
    "HTTP/1.1 301 REDIRECT\n"
    "Location: %s/\n\n";

static void
httpd_handle_sd_dir_send_header (void)
{
    PASTE_RESET ();
    PASTE_P (httpd_header_200);
    PASTE_P (httpd_header_ct_html);
    PASTE_PF (httpd_sd_dir_header, STATE->u.dir.dirname);

    if (*STATE->u.dir.dirname != 0) {
	/* We're listing a sub-directory, therefore let's insert a link
	   to the parent directory. */
	char *ptr = STATE->u.dir.dirname + strlen (STATE->u.dir.dirname) - 1;
	if (*ptr == '/') *ptr = 0;

	ptr = strrchr (STATE->u.dir.dirname, '/') + 1;

	*ptr = 0;		/* Chop of last directory's name. */
	PASTE_PF (httpd_sd_dir_parent, STATE->u.dir.dirname);
    }

    PASTE_SEND ();
}


void
httpd_handle_sd_dir (void)
{
    if (uip_acked ()) {
	if (!STATE->header_acked)
	    STATE->header_acked = 1;

	else if (STATE->eof) {
	    uip_close ();
	    return;
	}
	
	do
	    if (!fat_read_dir (STATE->u.dir.handle, &STATE->u.dir.entries)) {
		STATE->eof = 1;
		break;
	    }
	while (STATE->u.dir.entries.long_name[0] == '.');
	    
    }

    if (!STATE->header_acked)
	httpd_handle_sd_dir_send_header ();

    else if (STATE->eof) {
	/* Send footer. */
	PASTE_RESET ();
	PASTE_P (httpd_sd_dir_footer);
	PASTE_SEND ();
    }

    else {
	/* Send file entry. */
	PASTE_RESET ();

	if (STATE->u.dir.entries.attributes & FAT_ATTRIB_DIR)
	    PASTE_PF (httpd_sd_dir_entry_dir,
		      STATE->u.dir.entries.long_name,
		      STATE->u.dir.entries.long_name);

	else
	    PASTE_PF (httpd_sd_dir_entry,
		      STATE->u.dir.entries.long_name,
		      STATE->u.dir.entries.long_name,
		      STATE->u.dir.entries.file_size);

	PASTE_SEND ();
    }
}


void
httpd_handle_sd_dir_redirect (void)
{
    if (uip_acked ())
	uip_close ();

    if (uip_poll ())
	return;

    PASTE_RESET ();
    PASTE_PF (httpd_header_301_redirect, STATE->u.dir.dirname);
    PASTE_SEND ();
}
