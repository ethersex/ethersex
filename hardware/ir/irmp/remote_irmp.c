/*
 * Remote Infrared-Multiprotokoll-Decoder
 *
 * for additional information please
 * see https://www.mikrocontroller.net/articles/Remote_IRMP
 *
 * Copyright (c) 2014 by Erik Kunze <ethersex@erik-kunze.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
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

#include <stdint.h>
#include <string.h>

#include "config.h"
#include "core/debug.h"
#include "core/periodic.h"      /* for HZ */
#include "core/bit-macros.h"
#include "protocols/uip/uip.h"
#include "protocols/uip/parse.h"
#include "hardware/ir/irmp/irmp.h"

#include "remote_irmp.h"

#ifdef DEBUG_REMOTE_IRMP
#define IRMP_DEBUG(s,args...) debug_printf("IRMP UDP " s "\n", ## args)
#else
#define IRMP_DEBUG(a...)
#endif

#define BUF           ((struct uip_udpip_hdr *) ((uint8_t *)uip_appdata - UIP_IPUDPH_LEN))

#define STATE         (&irmp_conn->appstate.remote_irmp)

#define TIMEOUT       5*HZ      /* 5s */

static uip_udp_conn_t *irmp_conn;


void
remote_irmp_init(void)
{
  uip_ipaddr_t ip;
  uip_ipaddr_copy(&ip, all_ones_addr);

  irmp_conn = uip_udp_new(&ip, 0, remote_irmp_main);
  if (!irmp_conn)
  {
    IRMP_DEBUG("uip_udp_new failed");
    return;
  }

  memset(STATE, 0, sizeof(*STATE));
  uip_udp_bind(irmp_conn, HTONS(REMOTE_IRMP_PORT));
}

void
remote_irmp_main(void)
{
  uint8_t *p = (uint8_t *) uip_appdata;
  uint8_t response = '-';

  if (uip_newdata())
  {
#ifdef DEBUG_REMOTE_IRMP
    char buf[16];
    print_ipaddr((uip_ipaddr_t *) BUF->srcipaddr, buf, sizeof(buf));
    IRMP_DEBUG("cmd %c from %s", *((char *) uip_appdata), buf);
#endif

    if (!uip_len)
      return;

    uip_ipaddr_copy(STATE->reply_addr, BUF->srcipaddr);
    STATE->reply_port = BUF->srcport;

    if (STATE->state == REMOTE_IRMP_IDLE)
    {
      switch (*p & 0xdf)
      {
        case 'S':
          STATE->state = REMOTE_IRMP_SEND;
          if (uip_len == sizeof(irmp_data_t) + 1)
          {
            uint8_t b;
            b = p[2]; p[2] = p[3]; p[3] = b; /* NTOHS */
            b = p[4]; p[4] = p[5]; p[5] = b; /* NTOHS */
            irmp_write((irmp_data_t *) (p + 1));
            response = '+';
          }
          break;

        case 'R':
          STATE->state = REMOTE_IRMP_RECEIVE;
          STATE->timeout = TIMEOUT;
          return;

        case 'P':
          STATE->state = REMOTE_IRMP_PING;
          response = '+';
          break;

        default:
          break;
      }
    }

    uip_slen = 2;
  }
  else if (uip_poll())
  {
    if (STATE->state != REMOTE_IRMP_RECEIVE)
      return;

    irmp_data_t *irmp_data_p = irmp_read();
    if (irmp_data_p != 0)
    {
      p[2] = irmp_data_p->protocol;
      p[3] = HI8(irmp_data_p->address);
      p[4] = LO8(irmp_data_p->address);
      p[5] = HI8(irmp_data_p->command);
      p[6] = LO8(irmp_data_p->command);
      p[7] = irmp_data_p->flags;
      response = '+';
    }
    else
    {
      if (--STATE->timeout)
        return;

      IRMP_DEBUG("timeout");
    }

    p[0] = 'R';
    uip_slen = 8;
  }

  p[1] = response;

  uip_udp_conn_t reply_conn;
  uip_ipaddr_copy(reply_conn.ripaddr, STATE->reply_addr);
  reply_conn.rport = STATE->reply_port;
  reply_conn.lport = HTONS(REMOTE_IRMP_PORT);

  uip_udp_conn = &reply_conn;
  uip_process(UIP_UDP_SEND_CONN);
  router_output();

  uip_slen = 0;
  uip_len = 0;

  STATE->state = REMOTE_IRMP_IDLE;
}

/*
  -- Ethersex META --
  header(hardware/ir/irmp/remote_irmp.h)
  net_init(remote_irmp_init)

  state_header(hardware/ir/irmp/remote_irmp_state.h)
  state_udp(struct remote_irmp_connection_state_t remote_irmp)
*/
