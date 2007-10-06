/*                     -*- mode: C; c-file-style: "stroustrup"; -*-
 *
 * Copyright (c) 2007 by Stefan Siegl <stesie@brokenpipe.de>
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

#include <stdio.h>
#include <string.h>

#include "tetrirape_net.h"
#include "tetrirape_state.h"
#include "../tetrirape/tetrirape.h"
#include "../uip/uip.h"
#include "../config.h"

#ifdef TETRIRAPE_SUPPORT

/*
 * look into packet queue, whether there is a packet that can be sent ...
 */
#define tetrirape_send()        (tetrirape_try_unqueue(0))
#define tetrirape_rexmit()      (tetrirape_try_unqueue(1))
#define tetrirape_acked()       (tetrirape_try_unqueue(2))


void
tetrirape_net_init(void)
{
    uip_listen(HTONS(TETRIRAPE_PORT), tetrirape_net_main);
}



void
tetrirape_net_main(void)
{
    if(uip_connected())				/* init state area to zero */
	memset(&uip_conn->appstate, 0, 
               sizeof(struct tetrirape_connection_state_t));

    if(uip_acked())
	tetrirape_acked();

    if(uip_closed() || uip_aborted() || uip_timedout())
        tetrirape_disconnect();
	
    else if(uip_newdata()) {
	tetrirape_eat_data();

	if(! uip_outstanding(uip_conn))
	    tetrirape_send();
    }

    else if(uip_poll())
	tetrirape_send();

    else if(uip_rexmit())
	tetrirape_rexmit();
}



#endif /* TETRIRAPE_SUPPORT */
