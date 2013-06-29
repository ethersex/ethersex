/*
 * Copyright (c) 2007,2008 by Christian Dietrich <stettberger@dokucode.de>
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
#include <avr/pgmspace.h>
#include <avr/eeprom.h>

#include "config.h"
#include "core/debug.h"
#include "protocols/uip/uip.h"
#include "protocols/uip/parse.h"
#include "core/eeprom.h"

#include "protocols/ecmd/ecmd-base.h"

int16_t
parse_cmd_ipstats(char *cmd, char *output, uint16_t len)
{
  enum
  {
    INTERFACE,
    ADDRESS,
    GATEWAY,
    IP,
#ifdef UDP_SUPPORT
    UDP,
#endif
#ifdef TCP_SUPPORT
    TCP,
#endif
#ifdef ICMP_SUPPORT
    ICMP,
#endif
    LAST
  };

  /* trick: use bytes on cmd as "connection specific static variables" */
  if (cmd[0] != ECMD_STATE_MAGIC)
  {                             /* indicator flag: real invocation:  0 */
    cmd[0] = ECMD_STATE_MAGIC;  /*                 continuing call: 23 */
    cmd[1] = 0;                 /* counter for network interface */
    cmd[2] = INTERFACE;         /* counter for output lines for an interface */
  }
  uint16_t tmp;
#ifdef UIP_MULTI_STACK
#undef uip_hostaddr
#define uip_hostaddr (* (uip_stacks[cmd[1]].uip_hostaddr))

#undef uip_netmask
#define uip_netmask (* (uip_stacks[cmd[1]].uip_netmask))

#undef uip_prefix_len
#define uip_prefix_len (* (uip_stacks[cmd[1]].uip_prefix_len))

#undef uip_stat
#define uip_stat (* (uip_stacks[cmd[1]].uip_stat))
#endif /* UIP_MULTI_STACK */

  switch (cmd[2])
  {
    case INTERFACE:
      len = snprintf_P(output, len, PSTR("iface %d:"), cmd[1]);
      break;
    case ADDRESS:
      output[0] = ' ';
      output[1] = ' ';
      tmp = len;
      len = 2 + print_ipaddr(&uip_hostaddr, output + 2, len - 2);
      output[len++] = '/';
#ifdef IPV6_SUPPORT
      len += snprintf_P(output + len, tmp - len, PSTR("%d"), uip_prefix_len);
#else
      len += print_ipaddr(&uip_netmask, output + len, tmp - len);
#endif
      break;
    case GATEWAY:
      snprintf_P(output, len, PSTR("  gw: "));
      len = 6 + print_ipaddr(&uip_draddr, output + 6, len - 6);
      break;
    case IP:
      len = snprintf_P(output, len,
                       PSTR("  ip:   recv %5d sent %5d drop %5d"),
                       uip_stat.ip.recv, uip_stat.ip.sent, uip_stat.ip.drop);
      break;
#ifdef ICMP_SUPPORT
    case ICMP:
      len = snprintf_P(output, len,
                       PSTR("  icmp: recv %5d sent %5d drop %5d"),
                       uip_stat.icmp.recv, uip_stat.icmp.sent,
                       uip_stat.icmp.drop);
      break;
#endif
#ifdef UDP_SUPPORT
    case UDP:
      len = snprintf_P(output, len,
                       PSTR("  udp:  recv %5d sent %5d drop %5d"),
                       uip_stat.udp.recv, uip_stat.udp.sent, uip_stat.udp.drop);
      break;
#endif
#ifdef TCP_SUPPORT
    case TCP:
      len = snprintf_P(output, len,
                       PSTR("  tcp:  recv %5d sent %5d drop %5d"),
                       uip_stat.tcp.recv, uip_stat.tcp.sent, uip_stat.tcp.drop);
      break;
#endif
  }
  cmd[2]++;
  if (cmd[2] == LAST)
  {
    cmd[2] = 0;
    cmd[1]++;
    if (cmd[1] == STACK_LEN)
      return ECMD_FINAL(len);
  }

  return ECMD_AGAIN(len);
}

/*
  -- Ethersex META --
  block(Network IP Statistics)
  ecmd_feature(ipstats, "ipstats",, Display IP statistics.)
*/
