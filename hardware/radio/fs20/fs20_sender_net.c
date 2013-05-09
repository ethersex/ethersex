/*
 * Copyright (c) 2010 by Moritz Wenk <MoritzWenk@web.de>
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

/*
 * fs20_sender_net.c
 * 
 * Based heavily on watchasync.c
 * Purpose:
 * Send received FS20/FHT command to tcp port
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "config.h"
#include "core/debug.h"
#include "protocols/uip/uip.h"
#include "protocols/uip/parse.h"
#include "protocols/dns/resolv.h"
#include "core/portio/portio.h"
#include "protocols/ecmd/sender/ecmd_sender_net.h"

#include "fs20_sender_net.h"
#include "fs20.h"

#define FS20_QUEUE_NOITEM 255

static uint8_t fs20_sendstate = 0; // 0: Idle, 1: Message being sent, 2: Sending message failed
static uint8_t fs20_qpos = FS20_QUEUE_NOITEM;

static void fs20_net_main(void)  // Network-routine called by networkstack 
{
    if (uip_aborted() || uip_timedout()) // Connection aborted or timedout
    {
        // if connectionstate is new, we have to resend the packet, otherwise just ignore the event
        if (uip_conn->appstate.fs20.state == FS20_CONNSTATE_NEW)
        {
            fs20_sendstate = 2; // Ignore aborted, if already closed
            uip_conn->appstate.fs20.state = FS20_CONNSTATE_OLD;
            FS20S_DEBUG ("connection aborted\n");
            return;
        }
    }

    if (uip_closed()) // Closed connection does not expect any respond from us, resend if connnectionstate is new
    {
        if (uip_conn->appstate.fs20.state == FS20_CONNSTATE_NEW)
        {
            fs20_sendstate = 2; // Ignore aborted, if already closed
            uip_conn->appstate.fs20.state = FS20_CONNSTATE_OLD;
            FS20S_DEBUG ("new connection closed\n");
        } 
        else 
        {
            FS20S_DEBUG ("connection closed\n");
        }
        return;
    }

    if (uip_connected() || uip_rexmit()) 
    { 
        // (re-)transmit packet
        FS20S_DEBUG ("new connection or rexmit, sending message\n");
        char *p = uip_appdata;  // pointer set to uip_appdata, used to store string

        p += sprintf_P(p, PSTR("fs20 "));

		if (fs20_global.fs20.queue[fs20_qpos].ext)
		{
			p += sprintf_P(p, PSTR("%02x%02x%02x%02x%02x"), fs20_global.fs20.queue[fs20_qpos].data.edg.hc1,
					fs20_global.fs20.queue[fs20_qpos].data.edg.hc2, fs20_global.fs20.queue[fs20_qpos].data.edg.addr,
					fs20_global.fs20.queue[fs20_qpos].data.edg.cmd, fs20_global.fs20.queue[fs20_qpos].data.edg.cmd2);
		}
		else
		{
			p += sprintf_P(p, PSTR("%02x%02x%02x%02x"), fs20_global.fs20.queue[fs20_qpos].data.dg.hc1,
					fs20_global.fs20.queue[fs20_qpos].data.dg.hc2, fs20_global.fs20.queue[fs20_qpos].data.dg.addr,
					fs20_global.fs20.queue[fs20_qpos].data.dg.cmd);
		}

		fs20_global.fs20.queue[fs20_qpos].send = 0;

        uip_udp_send(p - (char *)uip_appdata);
        FS20S_DEBUG ("send %d bytes\n", p - (char *)uip_appdata);
        FS20S_DEBUG ("send %s\n", uip_appdata);
    }

    if (uip_acked()) // Send packet acked, 
    {
        if (uip_conn->appstate.fs20.state == FS20_CONNSTATE_NEW) // If packet is still new
        {
            fs20_sendstate = 0;  // Mark event as sent, go ahead in buffer
            uip_conn->appstate.fs20.state = FS20_CONNSTATE_OLD; // mark this packet as old, do not resend it
            uip_close();  // initiate closing of the connection
            FS20S_DEBUG ("packet sent, closing\n");
            return;
        } 
        else 
        {
            uip_abort();  // abort connection if old connection received an ack... this should not happen
        }
    }
}

static void fs20_dns_query_cb(char *name, uip_ipaddr_t *ipaddr)  // Callback for DNS query
{
#ifdef DEBUG_FS20_SENDER
	char buf[50];
	print_ipaddr(ipaddr, buf, 50);
    FS20S_DEBUG ("got dns response, connecting %s:%d\n", buf, CONF_FS20_PORT);
#endif

    uip_conn_t *conn = uip_connect(ipaddr, HTONS(CONF_FS20_PORT), fs20_net_main);  // create new connection with ipaddr found
    
    if (conn)  // if connection succesfully created
    {
        conn->appstate.fs20.state = FS20_CONNSTATE_NEW; // Set connection state to new, as data still has to be send
    } 
    else 
    {
        fs20_sendstate = 2;  // if no connection initiated, set state to Retry
    }
}

void fs20_sendmessage(void) // Send fs20/fht command from queue to tcp port
{
	fs20_sendstate = 1; // set new state in progress

	uip_ipaddr_t ipaddr;
	FS20S_DEBUG ("connecting %s\n", CONF_FS20_SERVICE);

	if (parse_ip(CONF_FS20_SERVICE, &ipaddr) == -1)
	{
		uip_ipaddr_t *ripaddr;
		// Try to find IPAddress
		if (!(ripaddr = resolv_lookup(CONF_FS20_SERVICE)))
		{
			resolv_query(CONF_FS20_SERVICE, fs20_dns_query_cb); // If not found: query DNS
		}
		else
		{
			fs20_dns_query_cb(NULL, ripaddr); // If found use IPAddress
		}
	}
	else
	{
		FS20S_DEBUG ("ip %s\n", CONF_FS20_SERVICE);
		fs20_dns_query_cb(NULL, &ipaddr);
	}

	return;
}

void fs20_sender_mainloop(void)  // Mainloop routine polls command queue
{
    if (fs20_sendstate != 1) // not busy sending 
    {
        if (fs20_sendstate == 2) // Message not sent successfully
        {
            FS20S_DEBUG ("Error, again please...\n"); 
            fs20_sendmessage();   // resend current command
        } 
        else // sendstate == 0 => Idle  // Previous send has been succesfull, send next event if any
        {
            fs20_qpos = FS20_QUEUE_NOITEM;
            
            for ( uint8_t i = 0; i < fs20_global.fs20.len; i++ ) 
            {
                if ( fs20_global.fs20.queue[i].send )
                {
                    fs20_qpos = i;
                    break;
                }
            }

            if ( fs20_qpos < FS20_QUEUE_LENGTH )
            {
                fs20_sendmessage();
            }
        }
    }
}

/*
  -- Ethersex META --
  header(hardware/radio/fs20/fs20_sender_net.h)
  mainloop(fs20_sender_mainloop)
  state_header(hardware/radio/fs20/fs20_sender_state.h)
  state_tcp(`struct fs20_sender_connection_state_t fs20;')
*/
