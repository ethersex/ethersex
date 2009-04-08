/*
 * Copyright (C) 2007, 2008 Christian Dietrich <stettberger@dokucode.de>
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
#include "uecmd_net.h"
#include "core/bit-macros.h"
#include "protocols/uip/uip.h"
#include "protocols/uip/uip_router.h"
#include "core/debug.h"
#include "../ecmd_parser/ecmd.h"


#include "config.h"

#define BUF ((struct uip_udpip_hdr *) (uip_appdata - UIP_IPUDPH_LEN))

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
    /* Add \0 to the data and remove \n from the data */
    char *p = (char *)uip_appdata;
    do {
      if (*p == '\r' || *p == '\n') {
        break;
      } 
    } while ( ++p <= ((char *)uip_appdata + uip_datalen()));

    /* Parse the Data */
    *p = 0;
    char cmd[p - (char *)uip_appdata];

    strncpy(cmd, uip_appdata, p - (char *)uip_appdata + 1);

    uip_slen = 0;
    while (uip_slen < UIP_BUFSIZE - UIP_IPUDPH_LEN) {
      int16_t len = ecmd_parse_command(cmd, ((char *)uip_appdata) + uip_slen, 
                        (UIP_BUFSIZE - UIP_IPUDPH_LEN) - uip_slen);
      uint8_t real_len = len;
      if (len < 0) {
        real_len = (uint8_t)  -len - 10;
      } 
      uip_slen += real_len + 1;
      ((char *)uip_appdata)[uip_slen - 1] = '\n';
      if (real_len == len || len == 0) 
        break;
    }

    /* Sent data out */

    uip_udp_conn_t echo_conn;
    uip_ipaddr_copy(echo_conn.ripaddr, BUF->srcipaddr);
    echo_conn.rport = BUF->srcport;
    echo_conn.lport = HTONS(UECMD_NET_PORT);

    uip_udp_conn = &echo_conn;
    uip_process(UIP_UDP_SEND_CONN); 
    router_output();

    uip_slen = 0;
  }
}

/*
  -- Ethersex META --
  net_init(uecmd_net_init)
*/
