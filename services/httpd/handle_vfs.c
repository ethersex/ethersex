/*
 * Copyright (c) 2009 by Stefan Siegl <stesie@brokenpipe.de>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License (either version 2 or
 * version 3) as published by the Free Software Foundation.
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

static void
httpd_handle_vfs_send_header (void)
{
    PASTE_RESET ();
    PASTE_P (httpd_header_200);

    vfs_size_t len = vfs_size (STATE->u.vfs.fd);
    if (len > 0) {
	/* send content-length header */
	PASTE_P (httpd_header_length);
	PASTE_LEN (len);
    }

    /* Check whether the file is gzip compressed. */
#ifndef VFS_TEENSY
    unsigned char buf[2];
    if (VFS_HAVE_FUNC (STATE->u.vfs.fd, fseek)) {
	/* Rewind stream first, might be a rexmit */
	vfs_rewind (STATE->u.vfs.fd);

	vfs_read (STATE->u.vfs.fd, buf, 2);
	vfs_rewind (STATE->u.vfs.fd);
    } else
	goto no_gzip;

    if (buf[0] == 0x1f && buf[1] == 0x8b)
#endif	/* not VFS_TEENSY, inlined files are always gzip'd */
	PASTE_P (httpd_header_gzip);

#ifndef VFS_TEENSY
no_gzip:
#endif	/* not VFS_TEENSY, inlined files are always gzip'd */
    if (STATE->u.vfs.content_type == 'X')
	PASTE_P (httpd_header_ct_xhtml);
    else if (STATE->u.vfs.content_type == 'S')
	PASTE_P (httpd_header_ct_css);
    else
	PASTE_P (httpd_header_ct_html);

    PASTE_SEND ();
}


void
httpd_handle_vfs_send_body (void)
{
    vfs_fseek (STATE->u.vfs.fd, STATE->u.vfs.acked, SEEK_SET);
    vfs_size_t len = vfs_read (STATE->u.vfs.fd, uip_appdata, uip_mss ());

    if (len <= 0) {
	uip_abort ();
	httpd_cleanup ();
	return;
    }

    if (len < uip_mss ())	/* Short read -> EOF */
	STATE->eof = 1;

    STATE->u.vfs.sent = STATE->u.vfs.acked + len;
    uip_send (uip_appdata, len);
}

void
httpd_handle_vfs (void)
{
    if (uip_acked ()) {
	if (STATE->header_acked)
	    STATE->u.vfs.acked = STATE->u.vfs.sent;
	else {
	    STATE->header_acked = 1;
	    STATE->u.vfs.acked = 0;
	}
    }

    if (!STATE->header_acked)
	httpd_handle_vfs_send_header ();

    else if (STATE->eof && !uip_rexmit())
	uip_close ();

    else
	httpd_handle_vfs_send_body ();
}
