/*
 *         simple rc5 udp implementation
 *
 *    for additional information please
 *    see http://lochraster.org/etherrape
 *
 * (c) by Mario Goegel <mario@goegel.net>
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

#include <string.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>

#include "protocols/uip/uip.h"
#include "protocols/uip/uip_neighbor.h"
#include "protocols/uip/uip_router.h"
#include "config.h"
#include "core/debug.h"
#include "rc5.h"

#define noinline __attribute__((noinline))

/* global variables */
extern volatile struct rc5_global_t rc5_global;



/* local variables */

static uip_ipaddr_t addr;
static uip_udp_conn_t *udpconn;

/*
 *  Initialize network communication
 */
void rc5_net_init(void) {
    /* retrieve IP address from configure script */
    set_CONF_RC5_SERVER(&addr);

    /* init an UDP connection */
    udpconn = uip_udp_new(&addr, HTONS(RC5_UDPPORT), rc5_udp_recv);
    if (udpconn != NULL) {
        uip_udp_periodic_conn(udpconn);
        uip_udp_bind(udpconn, HTONS(RC5_UDPPORT));
#ifdef DEBUG_RC5
        debug_printf("RC5: UDP enabled!\n");
#endif
    } else {
#ifdef DEBUG_RC5
        debug_printf("RC5: UDP not enabled!\n");
#endif
    }
}

/*
 *  Send an UDP packet
 *
 *      code mostly taken from syslog source
 */
void rc5_udp_send(void) {
    if (udpconn == NULL)
        return;

    uip_stack_set_active(STACK_ENC);

    if (rc5_check_cache())
        return;

    uip_slen = 0;
    /* this code is possibly unnecessary - need to be tested
     * copied from syslog code  */
    uip_appdata = uip_sappdata = uip_buf + UIP_IPUDPH_LEN + UIP_LLH_LEN;

#ifdef RC5_SUPPORT_COUNTERS
    /* copy counters to udp buffer */
    memcpy(uip_appdata, &rc5_global.cnt, sizeof (rc5_global.cnt));
#else
    /* create udp string */
    char str[10];
    snprintf(str, 10,"%u;%02u;%02u\n",
                rc5_global.received_command.toggle_bit,
                rc5_global.received_command.address,
                rc5_global.received_command.code);
    memcpy(uip_appdata, str, strlen(str));
#endif

#ifdef RC5_SUPPORT_COUNTERS
    uip_udp_send(rc5_global.bitcount);
#else
    uip_udp_send(strlen(str));
#endif

    uip_udp_conn = udpconn;
    uip_process(UIP_UDP_SEND_CONN);
    router_output();

    uip_slen = 0;
}

/*
 *  callback function for UDP stack
 *      uip_poll has to be called before uip_udp_send
 */
void rc5_udp_recv(void) {
    if (!uip_poll())
        return;

#ifdef ENC28J60_SUPPORT
    if (rc5_check_cache())
        uip_slen = 1; /* Trigger xmit to do force ARP lookup. */
#endif
}

/*
 *  function to keep ARP cache for our peer up to date
 */
uint8_t
rc5_check_cache(void) {
    uip_ipaddr_t ipaddr;

#ifdef IPV6_SUPPORT

    if (memcmp(udpconn->ripaddr, uip_hostaddr, 8))
        /* Remote address is not on the local network, use router */
        uip_ipaddr_copy(&ipaddr, uip_draddr);
    else
        /* Remote address is on the local network, send directly. */
        uip_ipaddr_copy(&ipaddr, udpconn->ripaddr);

    if (uip_ipaddr_cmp(&ipaddr, &all_zeroes_addr))
        return 1; /* Cowardly refusing to send IPv6 packet to :: */

    if (uip_neighbor_lookup(ipaddr))
        return 0;

#else  /* IPV4_SUPPORT */

    if (!uip_ipaddr_maskcmp(udpconn->ripaddr, uip_hostaddr, uip_netmask))
        /* Remote address is not on the local network, use router */
        uip_ipaddr_copy(&ipaddr, uip_draddr);
    else
        /* Remote address is on the local network, send directly. */
        uip_ipaddr_copy(&ipaddr, udpconn->ripaddr);

#ifdef ETHERNET_SUPPORT
    /* uip_arp_lookup returns a pointer if the mac is in the arp cache */
    if (uip_arp_lookup(ipaddr))
#endif
        return 0;

#endif

    return 1;
}

/*
  -- Ethersex META --
  header(hardware/ir/rc5/rc5_net.h)
  net_init(rc5_net_init)
 */
