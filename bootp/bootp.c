/*
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

#include <avr/pgmspace.h>

#include <stdlib.h>
#include <string.h>

#include "../uip/uip.h"
#include "../uip/uip_arp.h"
#include "../eeprom.h"
#include "../net/bootp_net.h"
#include "../tftp/tftp.h"
#include "../dns/resolv.h"
#include "../dyndns/dyndns.h"

#include "bootp.h"
#include "bootphdr.h"
#include "../debug.h"

static unsigned char mincookie[] = {99, 130, 83, 99, 255} ;
static unsigned char replycookie[] = { 0x63, 0x82, 0x53, 0x63 };



/*
 * send bootp request to broadcast address
 */
void
bootp_send_request(void)
{
    int i;

    /* prepare bootp request packet */
    struct bootp *pk = uip_appdata;
    memset(pk, 0, sizeof(struct bootp));

    pk->bp_op = BOOTREQUEST;
    pk->bp_htype = HTYPE_ETHERNET;
    pk->bp_hlen = 6;
    for(i = 0; i < 4; i ++) {
	unsigned char c = rand() & 0xFF;       /* use upper bits ?? */
	
	pk->bp_xid[i] = c;
	uip_udp_conn->appstate.bootp.xid[i] = c;
    }
	
    pk->bp_flags |= HTONS(BPFLAG_BROADCAST);    /* request broadcast reply */
    memcpy(pk->bp_chaddr, uip_ethaddr.addr, 6);

    /* fill cookie into vendor data field */
    memcpy(pk->bp_vend, mincookie, sizeof(mincookie));

    /* broadcast the packet */
    uip_udp_send(sizeof(struct bootp));
}


void
bootp_handle_reply(void)
{
    int i;
    struct bootp *pk = uip_appdata;

    if(pk->bp_op != BOOTREPLY)
	return;					/* ugh? shouldn't happen */

    if(pk->bp_htype != HTYPE_ETHERNET)
	return;

    for(i = 0; i < 4; i ++) {
	if(pk->bp_xid[i] != uip_udp_conn->appstate.bootp.xid[i])
	    return;				/* session id doesn't match */

	if(pk->bp_vend[i] != replycookie[i])
	    return;				/* reply cookie doesn't match */
    }


    /*
     * looks like we have received a valid bootp reply, 
     * prepare to override eeprom configuration
     */
    uip_ipaddr_t ips[4];
    memset(&ips, 0, sizeof(ips));

    /* extract our ip addresses, subnet-mask and gateway ... */
    memcpy(&ips[0], pk->bp_yiaddr, 4);
    uip_sethostaddr(&ips[0]);

    unsigned char *ptr = pk->bp_vend + 4;
    while(*ptr != 0xFF) {
	switch(* ptr) {
	case TAG_SUBNET_MASK:
	    memcpy(&ips[1], &ptr[2], 4);
	    uip_setnetmask(&ips[1]);
	    break;

	case TAG_GATEWAY:
	    memcpy(&ips[2], &ptr[2], 4);
	    uip_setdraddr(&ips[2]);
	    break;
#ifdef DNS_SUPPORT
        case TAG_DOMAIN_SERVER:
	    memcpy(&ips[3], &ptr[2], 4);
            resolv_conf(&ips[3]);
            break;
#endif
	}

	ptr = ptr + ptr[1] + 2;
    }


    eeprom_save_config(uip_ethaddr.addr, ips[0], ips[1], ips[2], ips[3]);
    uip_udp_conn->appstate.bootp.configured = 1;
#ifdef DYNDNS_SUPPORT
    dyndns_update();
#endif

#ifdef TFTP_SUPPORT
    if(pk->bp_file[0] == 0)
	return;					/* no boot filename provided */

    /* create tftp connection, which will fire the download request */
    uip_ipaddr_t ip;
    uip_ipaddr(&ip, pk->bp_siaddr[0], pk->bp_siaddr[1],
	       pk->bp_siaddr[2], pk->bp_siaddr[3]);

    tftp_fire_tftpomatic(&ip, pk->bp_file);
#endif /* TFTP_SUPPORT */
}


