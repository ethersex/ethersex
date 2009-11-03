/*
 * Copyright (c) 2009 by Sylwester Sosnowski <esc@ext.no-route.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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

#include <avr/pgmspace.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "protocols/uip/uip.h"
#include "protocols/ecmd/parser.h"
#include "protocols/ecmd/ecmd-base.h"

#include "ustream.h"

static uip_conn_t *ustream_conn;

void ustream_main(void)
{
	if(uip_connected() || uip_rexmit())
	{
		uip_send(USTREAM_URI, strlen(USTREAM_URI));
		return;
	}

	if(uip_newdata())
	{
		// TBD: Push data to VS1053
		((char *) uip_appdata)[uip_len] = 0;
		printf("Got some data.\n");
	}
}


void ustream_periodic(void)
{
  if (!ustream_conn) ustream_init();
}

void ustream_init(void)
{
    USTREAMDEBUG ("Initializing ustream client\n");

    uip_ipaddr_t ip;

    set_CONF_USTREAM_IP(&ip);
    ustream_conn = uip_connect(&ip, HTONS(CONF_USTREAM_PORT), ustream_main);

    if (!ustream_conn)
    {
        USTREAMDEBUG ("no uip_conn available.\n");
        return;
    }
}

/*
  -- Ethersex META --
  header(protocols/ustream/ustream.h)
  net_init(ustream_init)
  timer(500, ustream_periodic())
*/

