/*
 * Copyright (C) 2007, 2008 Christian Dietrich <stettberger@dokucode.de>
 * Copyright (c) 2009 by David Gräff <david.graeff@web.de>
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
#include "protocols/uip/uip.h"
#include "protocols/uip/uip_router.h"
#include "core/debug.h"
#include "protocols/ecmd/parser.h"
#include "protocols/ecmd/ecmd-base.h"

#include "config.h"

#define BUF ((struct uip_udpip_hdr *) (uip_appdata - UIP_IPUDPH_LEN))

void
uecmd_net_init()
{
  uip_ipaddr_t ip;
  uip_ipaddr_copy(&ip, all_ones_addr);

  uip_udp_conn_t *uecmd_conn = uip_udp_new(&ip, 0, uecmd_net_main);
  if (!uecmd_conn)
  {
    debug_printf("ecmd: udp failed\n");
    return;
  }

  uip_udp_bind(uecmd_conn, HTONS(ECMD_UDP_PORT));
}

void
uecmd_net_main()
{
  if (!uip_newdata())
    return;

  char *p = (char *) uip_appdata;
  /* This may be 1-2 chars too big in case there is a \r or \n, but it saves us a counting loop */
  char cmd[uip_datalen() + 1];
  char *dp = cmd;
  /* Copy over into temporary buffer, remove \r \n if present, add \0 */
  while (p < (char *) uip_appdata + uip_datalen())
  {
    if (*p == '\r' || *p == '\n')
      break;
    *dp++ = *p++;
  }
  *dp = 0;

  uip_slen = 0;
  while (uip_slen < UIP_BUFSIZE - UIP_IPUDPH_LEN)
  {
    int16_t len = ecmd_parse_command(cmd, ((char *) uip_appdata) + uip_slen,
                                     (UIP_BUFSIZE - UIP_IPUDPH_LEN) -
                                     uip_slen);
    uint8_t real_len = len;
    if (!is_ECMD_FINAL(len))
    {                           /* what about the errors ? */
      /* convert ECMD_AGAIN back to ECMD_FINAL */
      real_len = (uint8_t) ECMD_AGAIN(len);
    }
    uip_slen += real_len + 1;
    ((char *) uip_appdata)[uip_slen - 1] = '\n';
    if (real_len == len || len == 0)
      break;
  }

  /* Sent data out */

  uip_udp_conn_t echo_conn;
  uip_ipaddr_copy(echo_conn.ripaddr, BUF->srcipaddr);
  echo_conn.rport = BUF->srcport;
  echo_conn.lport = HTONS(ECMD_UDP_PORT);

  uip_udp_conn = &echo_conn;
  uip_process(UIP_UDP_SEND_CONN);
  router_output();

  uip_slen = 0;
}

/*
  -- Ethersex META --
  header(protocols/ecmd/via_udp/uecmd_net.h)
  net_init(uecmd_net_init)
*/
