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
#include "protocols/uip/check_cache.h"
#include "config.h"
#include "core/debug.h"
#include "rc5.h"
#include "rc5_net.h"

#define noinline __attribute__((noinline))

/* global variables */
extern volatile struct rc5_global_t rc5_global;

/* local variables */
static uip_ipaddr_t addr;
static uip_udp_conn_t *udpconn;

/*
 *  Initialize network communication
 */
void
rc5_net_init(void)
{
  /* retrieve IP address from configure script */
  set_CONF_RC5_SERVER(&addr);

  /* init an UDP connection */
  udpconn = uip_udp_new(&addr, HTONS(RC5_UDPPORT), rc5_udp_recv);
  if (udpconn != NULL)
  {
    uip_udp_periodic_conn(udpconn);
    uip_udp_bind(udpconn, HTONS(RC5_UDPPORT));
#ifdef DEBUG_RC5
    debug_printf("RC5: UDP enabled!\n");
#endif
  }
  else
  {
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
void
rc5_udp_send(void)
{
  if (udpconn == NULL)
    return;

#ifdef ETHERNET_SUPPORT
  if (uip_check_cache(&udpconn->ripaddr))
    return;
#endif

  uip_slen = 0;
  /* this code is possibly unnecessary - need to be tested
   * copied from syslog code  */
  uip_appdata = uip_sappdata = uip_buf + UIP_IPUDPH_LEN + UIP_LLH_LEN;

#ifdef RC5_SUPPORT_COUNTERS
  /* copy counters to udp buffer */
  memcpy(uip_appdata, &rc5_global.cnt, sizeof(rc5_global.cnt));
#else
  /* create udp string */
  char str[10];
  snprintf(str, 10, "%u;%02u;%02u\n",
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
void
rc5_udp_recv(void)
{
  if (!uip_poll())
    return;

#ifdef ETHERNET_SUPPORT
  if (udpconn && uip_check_cache(&udpconn->ripaddr))
    uip_slen = 1;               /* Trigger xmit to do force ARP lookup. */
#endif
}


/*
  -- Ethersex META --
  header(hardware/ir/rc5/rc5_net.h)
  net_init(rc5_net_init)
 */
