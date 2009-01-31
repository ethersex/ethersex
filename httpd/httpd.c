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
