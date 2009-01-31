/*
 * (c) by Alexander Neumann <alexander@bumpern.de>
 * Copyright (c) 2008,2009 by Stefan Siegl <stesie@brokenpipe.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
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

#include <avr/pgmspace.h>
#include <avr/eeprom.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "httpd.h"
#include "base64.h"
#include "../eeprom.h"
#include "../ecmd_parser/ecmd.h"
#include "../config.h"
#include "../vfs/vfs.h"

#ifdef DEBUG_HTTPD
# include "../debug.h"
# define printf        debug_printf
#else
# define printf(...)   ((void)0)
#endif

char PROGMEM httpd_header_200[] =
"HTTP/1.1 200 OK\n"
"Connection: close\n";

char PROGMEM httpd_header_ct_css[] =
"Content-Type: text/css; charset=iso-8859-1\n\n";

char PROGMEM httpd_header_ct_html[] =
"Content-Type: text/html; charset=iso-8859-1\n\n";

char PROGMEM httpd_header_ct_xhtml[] =
"Content-Type: application/xhtml+xml; charset=iso-8859-1\n\n";

#ifdef ECMD_PARSER_SUPPORT
char PROGMEM httpd_header_200_ecmd[] =
/* {{{ */
/* Please note: this is the _whole_ header, no content-length must follow */
"HTTP/1.1 200 OK\n"
"Connection: close\n"
"Cache-Control: no-cache\n"
"Cache-Control: must-revalidate\n"
"Content-Type: text/plain; charset=iso-8859-1\n\n";
/* }}} */
#endif /* ECMD_PARSER_SUPPORT */

char PROGMEM httpd_header_400[] =
/* {{{ */
"HTTP/1.1 400 Bad Request\n"
"Connection: close\n"
"Content-Type: text/plain; charset=iso-8859-1\n";
/* }}} */

char PROGMEM httpd_header_gzip[] =
/* {{{ */
"Content-Encoding: gzip\n";

#ifdef HTTPD_AUTH_SUPPORT
char PROGMEM httpd_header_401[] =
/* {{{ */
"HTTP/1.1 401 UNAUTHORIZED\n"
"Connection: close\n"
"WWW-Authenticate: Basic realm=\"Secure Area\"\n"
"Content-Type: text/plain; charset=iso-8859-1\n";
/*}}}*/

char PROGMEM httpd_body_401[] =
/*{{{*/
"Authentification required\n";
/*}}}*/
#endif /* HTTPD_AUTH_SUPPORT */

char PROGMEM httpd_body_400[] =
/*{{{*/
"Bad Request\n";
/* }}} */

char PROGMEM httpd_header_404[] =
/* {{{ */
"HTTP/1.1 404 File Not Found\n"
"Connection: close\n"
"Content-Type: text/plain; charset=iso-8859-1\n";

char PROGMEM httpd_body_404[] =
"File Not Found\n";
/* }}} */

char PROGMEM httpd_header_length[] = "Content-Length: ";


void
httpd_init(void)
{
    uip_listen(HTONS(HTTPD_PORT), httpd_main);
    uip_listen(HTONS(HTTPD_ALTERNATE_PORT), httpd_main);
}



static inline void
httpd_cleanup (struct httpd_connection_state_t *state)
{
#ifdef VFS_SUPPORT
    if (state->fd) {
	printf("httpd: cleaning left-over vfs-handle at %p.\n", state->fd);

	vfs_close (state->fd);
	state->fd = NULL;
    }
#endif	/* VFS_SUPPORT */
}



void
httpd_main(void)
{
    struct httpd_connection_state_t *state = &uip_conn->appstate.httpd;

    if (uip_aborted() || uip_timedout()) {
	httpd_cleanup (state);
	printf ("httpd: connection aborted\n");
    }

    if (uip_closed()) {
	httpd_cleanup (state);
	state->state = HTTPD_STATE_CLOSED;
	printf ("httpd: connection closed\n");
    }

    if (uip_connected()) {
	printf ("httpd: new connection\n");

	/* initialize struct */
	state->handler = NULL;
    }

    if (uip_newdata()) {
	printf ("httpd: new data\n");
	httpd_handle_input ();
    }

    if(uip_rexmit() ||
       uip_newdata() ||
       uip_acked() ||
       uip_connected() ||
       uip_poll()) {

	/* Call associated handler, if set already. */
	if (state->handler)
	    state->handler ();
    }

}
