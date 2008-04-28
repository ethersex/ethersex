/* vim:fdm=marker ts=4 et ai
 * {{{
 *
 * Copyright (C) 2007, 2008 Christian Dietrich <stettberger@dokucode.de>
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
 }}} */

#include <string.h>
#include "uecmd_net.h"
#include "../bit-macros.h"
#include "../uip/uip.h"
#include "../debug.h"
#include "../ecmd_parser/ecmd.h"


#include "../config.h"

#define BUF ((struct uip_udpip_hdr *) (uip_appdata - UIP_IPUDPH_LEN))

#ifdef UECMD_SUPPORT

void uecmd_net_init() {
  /* If teensy support is enabled we use udp */
  uip_ipaddr_t ip;
  uip_ipaddr_copy (&ip, all_ones_addr);

  uip_udp_conn_t *udp_echo_conn = uip_udp_new (&ip, 0, uecmd_net_main);

  if (!udp_echo_conn) 
    return; /* dammit. */

  uip_udp_bind (udp_echo_conn, HTONS (UECMD_NET_PORT));
}

void uecmd_net_main() {
  if(uip_newdata()) {
    uip_udp_conn_t echo_conn;
    uip_ipaddr_copy(echo_conn.ripaddr, BUF->srcipaddr);
    echo_conn.rport = BUF->srcport;
    echo_conn.lport = HTONS(UECMD_NET_PORT);

    uip_udp_conn = &echo_conn;
    /* Add \0 to the data and remove \n from the data */
    char *p = (char *)uip_appdata;
    do {
      if (*p == '\r' || *p == '\n') {
        *p = 0;
        break;
      } 
    } while ( ++p < ((uint8_t *)uip_appdata + uip_datalen()));
    /* Parse the Data */
    uip_slen = ecmd_parse_command(uip_appdata, uip_appdata, 
                                  UIP_BUFSIZE - UIP_IPUDPH_LEN) + 1;

    ((char *)uip_appdata)[uip_slen - 1] = '\n';
    uip_process(UIP_UDP_SEND_CONN); 
    fill_llh_and_transmit();

    uip_slen = 0;
  }
}
#endif
