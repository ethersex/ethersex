/*                     -*- mode: C; c-file-style: "stroustrup"; -*-
 *
 * Copyright (c) 2009 by Christian Dietrich <stettberger@dokucode.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by 
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
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
#include <string.h>
#include <stdio.h>
#include "wol.h"
#include "protocols/uip/uip.h"
#include "protocols/uip/uip_arp.h"
#include "protocols/uip/uip_router.h"
#include "protocols/ecmd/ecmd-base.h"
#include "protocols/uip/parse.h"
#include "config.h"


int16_t parse_cmd_wol (char *cmd, char *output, uint16_t len) 
{
    struct uip_eth_addr mac;

    if (parse_mac(cmd, &mac) == -1)
        return ECMD_ERR_PARSE_ERROR;

    uint8_t i;
    uint8_t *buf = (uint8_t *)  &uip_buf[UIP_LLH_LEN + UIP_IPUDPH_LEN];
    for (i = 0; i < 6; i++)
        buf[i] = 0xff;
    for (i = 0; i < 16; i++)
        memcpy(&buf [6 + i * 6], mac.addr, 6);

    uip_udp_conn_t conn;
    uip_ipaddr_copy(conn.ripaddr, all_ones_addr);

    conn.lport = HTONS(9);
    conn.rport = HTONS(9);
    uip_udp_conn = &conn;

    uip_slen = 6 + 6 * 16;
    uip_process(UIP_UDP_SEND_CONN);
    router_output();
    uip_slen = 0;

    i = snprintf_P(output, len, PSTR("send wol packet to "));
    i += print_mac(&mac, &output[i], len - i);
    return i;
}

/*
  -- Ethersex META --
  block(Miscelleanous)
  ecmd_feature(wol, "wol ",MAC,Send WAKE-ON-LAN command to MAC)
*/
