/*
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
 */

#include <avr/io.h>
#include <avr/pgmspace.h>
#include "config.h"
#include "protocols/uip/uip.h"
#include "protocols/uip/uip_router.h"
#include "core/debug.h"
#include "services/clock/clock.h"
#include "snmp_net.h"
#include "snmp.h"


#ifdef SNMP_SUPPORT

#define BUF ((struct uip_udpip_hdr *) (uip_appdata - UIP_IPUDPH_LEN))

#ifdef WHM_SUPPORT
uint8_t
uptime_reaction(uint8_t *ptr, struct snmp_varbinding *bind, void *userdata)
{
  uint32_t seconds = clock_get_time() - clock_get_startup();
  /* This long long (uint64_t) hack is necessary, because it seems to be, that
   * seconds = seconds * 100 doesn't work at all
   */
  seconds = seconds * 100LL;
  uint32_t *time_ptr = (void *) ptr + 2;

  ptr[0] = 0x43;
  ptr[1] = 4;
  *time_ptr = HTONL((uint32_t)seconds);

  return 6;
}
#endif

#ifdef ADC_SUPPORT
uint8_t
adc_reaction(uint8_t *ptr, struct snmp_varbinding *bind, void *userdata)
{
  ptr[0] = 2;
  ptr[1] = 2;
  if ( bind->len > 0 && bind->data[0] < ADC_CHANNELS) {
    ADMUX = (ADMUX & 0xF0) | bind->data[0];
    /* Start adc conversion */
    ADCSRA |= _BV(ADSC);
    /* Wait for completion of adc */
    while (ADCSRA & _BV(ADSC)) {}
    uint16_t adc = ADC;
    ptr[2] = adc >> 8;
    ptr[3] = adc & 0xff;
  } else {
    ptr[2] = ptr[3] = 0;
  }

  return 4;
}
#endif

uint8_t
string_pgm_reaction(uint8_t *ptr, struct snmp_varbinding *bind, void *userdata)
{
  (void) bind;
  ptr[0] = 4;
  ptr[1] = strlen_P((char *) userdata);
  memcpy_P(ptr + 2, userdata, ptr[1]);
  return ptr[1] + 2;
}

const char desc_value[] PROGMEM = SNMP_VALUE_DESCRIPTION;
const char desc_obj_name[] PROGMEM = "\x2b\x06\x01\x02\x01\x01\x01";

const char uptime_reaction_obj_name[] PROGMEM = "\x2b\x06\x01\x02\x01\x01\x03";

const char contact_value[] PROGMEM = SNMP_VALUE_CONTACT;
const char contact_obj_name[] PROGMEM = "\x2b\x06\x01\x02\x01\x01\x04";

const char hostname_reaction_obj_name[] PROGMEM = "\x2b\x06\x01\x02\x01\x01\x05";
const char hostname_value[] PROGMEM = CONF_HOSTNAME;

const char location_value[] PROGMEM = SNMP_VALUE_LOCATION;
const char location_obj_name[] PROGMEM = "\x2b\x06\x01\x02\x01\x01\x06";

const char adc_reaction_obj_name[] PROGMEM = ethersexExperimental "\01";

static struct snmp_reaction snmp_reactions[] = {
#ifdef ADC_SUPPORT
  {adc_reaction_obj_name, adc_reaction, NULL},
#endif
  {desc_obj_name, string_pgm_reaction, (void *)desc_value},
#ifdef WHM_SUPPORT
  {uptime_reaction_obj_name, uptime_reaction, NULL},
#endif
  {contact_obj_name, string_pgm_reaction, (void *)contact_value},
  {hostname_reaction_obj_name, string_pgm_reaction, (void *)hostname_value},
  {location_obj_name, string_pgm_reaction, (void *)location_value},
  {NULL, NULL, 0}
};

void
snmp_new_data(void)
{
  struct snmp_packet pkt;
  uint8_t pdu_type;
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
  pdu_type = pkt.pdu_type[0];

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

      uint8_t store_len = strlen_P((char *)snmp_reactions[z].obj_name);
      uint8_t request_len = pkt.binds[y].len;
      uint8_t cmp_len = store_len > request_len ? request_len : store_len;

      debug_printf("d: len %d, rlen %d\n", store_len, request_len);
      if (memcmp_P(pkt.binds[y].data, snmp_reactions[z].obj_name, cmp_len) == 0) {
        pkt.binds[y].type = z;
        if (pdu_type == 0xa1 && request_len == store_len) {
          if (snmp_reactions[z+1].obj_name) {
            pkt.binds[y].type += 1;
          } else {
            pkt.binds[y].type = 0xff;
          }
        }
        if (store_len < pkt.binds[y].len) {
          uint8_t *data_ptr = pkt.binds[y].data;
          pkt.binds[y].len = pkt.binds[y].len - store_len;
          pkt.binds[y].data = __builtin_alloca(pkt.binds[y].len);
          memcpy(pkt.binds[y].data, data_ptr + store_len,
                 pkt.binds[y].len);
        } else {
          pkt.binds[y].data = NULL;
          pkt.binds[y].len = 0;
        }

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
    if (type == 0xff) {
      /* This datatype is not defined */
      pkt.pdu_type[1 + 2 + 4 + 3] = 2; /* NoSuchName */;
      pkt.pdu_type[1 + 2 + 4 + 3 + 3 ] = ptr; /* Error in this Node */;
      varbind_start[1] = 0;
      break;
    }
    bind_start[0] = 0x30;
    bind_start[1] = 2 + strlen_P((char *)snmp_reactions[type].obj_name);
    /* Object identifier */
    bind_start[2] = 0x06; /* Object identifier start */
    bind_start[3] = bind_start[1] - 2;
    memcpy_P(bind_start + 4, snmp_reactions[type].obj_name, bind_start[3]);

    /* Variable part  of the object identifier*/
    memcpy(bind_start + 4 + bind_start[3], pkt.binds[ptr].data,
           pkt.binds[ptr].len);
    bind_start[3] += pkt.binds[ptr].len;
    bind_start[1] += pkt.binds[ptr].len;

    /* Append the value */
    bind_start[1] += snmp_reactions[type].cb(bind_start + 4 + bind_start[3],
                                             &pkt.binds[ptr],
                                             snmp_reactions[type].userdata);

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
  router_output();

  uip_slen = 0;

}

#endif

