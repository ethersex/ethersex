/* vim:fdm=marker ts=4 et ai
 * {{{
 *
 * Copyright (c) 2008 by Christian Dietrich <stettberger@dokucode.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
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
 }}} */

#include <avr/io.h>
#include <avr/pgmspace.h>
#include "../config.h"
#include "../uip/uip.h"
#include "../uip/uip_arp.h"
#include "../syslog/syslog.h"
#include "../debug.h"
#include "../net/snmp_net.h"
#include "snmp.h"


#ifdef SNMP_SUPPORT

#define BUF ((struct uip_udpip_hdr *) (uip_appdata - UIP_IPUDPH_LEN))

uint8_t 
uptime_reaction(char *ptr)
{
  ptr[0] = 5;
  ptr[1] = 0;
  return 2;
}

uint8_t 
hostname_reaction(char *ptr)
{
  ptr[0] = 4;
  ptr[1] = strlen_P(PSTR(CONF_HOSTNAME));
  memcpy_P(ptr + 2, PSTR(CONF_HOSTNAME), ptr[1]);
  return ptr[1] + 2;
}

static struct snmp_reaction snmp_reactions[] = {
  {"\x2b\x06\x01\x02\x01\x03", uptime_reaction, 1},
  {"\x2b\x06\x01\x02\x01\x01\x05", hostname_reaction, 1},
  {NULL, NULL, 0}
};

void
snmp_new_data(void)
{
  struct snmp_packet pkt;
  uint8_t tmp, *request = (uint8_t *)uip_appdata;
  uint16_t ptr = 0;
  /* Parse the packet */
  ptr += 4; /* Consume packet encapsulation and version field */
  pkt.version = request[ptr];
  /* Community string */
  ptr += 2;
  tmp = request[ptr]; /* Community string length */
  pkt.community = &request[ptr + 1];
  pkt.pdu_type = &request[ptr + 1 + tmp]; /* We must save the pdu type, becaus 
                                          it is overwritten in the next step */
  pkt.community[tmp] = 0;
  ptr +=  1 + tmp + 4;
  memcpy(&pkt.request_id, &request[ptr], 4);
  ptr += 11; /* Skip error status, error index and the varbind encapsulation*/
  uint8_t *varbind_start = request + ptr - 1;

  uint16_t x, var_len = request[ptr];
  uint8_t y;

  pkt.var_count = 0;
  ptr += 2;
  for(x = 0; x != var_len;) {
    x += 2 + request[ptr + x];
    pkt.var_count++;
  }
  pkt.binds = __builtin_alloca(pkt.var_count * sizeof(struct snmp_varbinding));
  for(tmp = 0, x = 0; x < var_len;) {
    pkt.binds[tmp].len = request[ptr + x + 2];
    pkt.binds[tmp].data = &request[ptr + x + 3];
    tmp++;

    x += 2 + request[ptr + x];
  }
  
  /* interpret the different mibs */
  uint8_t z;
  for (y = 0; y < pkt.var_count; y++) {
    pkt.binds[y].type = 0xff;
                 
    for (z = 0; snmp_reactions[z].obj_name; z++) {
  /*    syslog_sendf("%d %d; %d %x : %d %x | ", y, z,
                   pkt.binds[y].len, pkt.binds[y].data[0],
                   strlen((char *)snmp_reactions[z].obj_name)
                          + snmp_reactions[z].append_zero,
                          snmp_reactions[z].obj_name[0]); */
      if (memcmp(pkt.binds[y].data, snmp_reactions[z].obj_name,
                 strlen((char *)snmp_reactions[z].obj_name) 
                 + snmp_reactions[z].append_zero) == 0) {
        pkt.binds[y].type = z;
        break;
      }
    }
  }

  /* We assemble the packet within the received packet */
  pkt.pdu_type[0] = 0xa2; /* Our packet type is an get-response packet */
  request[1] = pkt.pdu_type - request;

  varbind_start[0] = 0x30; /* Varbind encapsulation */
  varbind_start[1] = 0;

  uint8_t *bind_start = varbind_start + 2;
  for (ptr = 0; ptr < pkt.var_count; ptr++) {
    uint8_t type = pkt.binds[ptr].type;
    syslog_sendf("%d ", ptr);
    if (type == 0xff) {
      /* This datatype is not defined */
      pkt.pdu_type[1 + 2 + 4 + 3] = 2; /* NoSuchName */;
      pkt.pdu_type[1 + 2 + 4 + 3 + 3 ] = ptr; /* Error in this Node */;
      varbind_start[1] = 0;
      break;
    }
    bind_start[0] = 0x30;
    bind_start[1] = 2 + strlen((char *)snmp_reactions[type].obj_name) 
                      + snmp_reactions[type].append_zero;
    bind_start[2] = 0x06; /* Object identifier start */
    bind_start[3] = bind_start[1] - 2;
    memcpy(bind_start + 4, snmp_reactions[type].obj_name, bind_start[3]);
    bind_start[1] += snmp_reactions[type].cb(bind_start + 4 + bind_start[3]);

    varbind_start[1] += bind_start[1] + 2;

    bind_start += bind_start[1] + 2;
  }

  pkt.pdu_type[1] = varbind_start[1] + 14;
  request[1] += pkt.pdu_type[1];


  uip_udp_send(request[1] + 2);
  /* Send the packet */
  uip_udp_conn_t conn;
  uip_ipaddr_copy(conn.ripaddr, BUF->srcipaddr);
  conn.rport = BUF->srcport;
  conn.lport = HTONS(SNMP_PORT);

  uip_udp_conn = &conn;

  /* Send immediately */
  uip_process(UIP_UDP_SEND_CONN); 
  fill_llh_and_transmit();

  uip_slen = 0;

}

#endif

