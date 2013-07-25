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

#ifdef HTTPD_AUTH_SUPPORT

void
httpd_handle_401 (void)
{
    if (uip_acked ()) {
	uip_close ();
	return;
    }

    PASTE_RESET ();
    PASTE_P (httpd_header_401);
    PASTE_P (httpd_header_length);
    PASTE_LEN_P (httpd_body_401);
    PASTE_P (httpd_header_end);
    PASTE_P (httpd_body_401);
    PASTE_SEND ();
}

#endif	/* HTTPD_AUTH_SUPPORT */
