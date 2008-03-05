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
#include "../debug.h"
#include "../net/mdns_sd_net.h"
#include "mdns_sd.h"


#ifdef MDNS_SD_SUPPORT

#define BUF ((struct uip_udpip_hdr *) (uip_appdata - UIP_IPUDPH_LEN))

/* Skips an DNS Label, even if an pointer is at the end */
static uint8_t *
parse_name(uint8_t *query)
{
  uint8_t n;

  do {
    n = *query++;
    if ( n & 0xC0 ) 
      /* This is a pointer */
      break;
    else
      /* This is a label */
      query += n;
    /*    printf(".");*/
  } while(*query != 0);
  /*  printf("\n");*/
  return query + 1;
}

/* Appends an label to an dns packet at ptr.
 * label is stored in PROGMEM 
 */
uint8_t *
append_label(uint8_t *ptr, uint8_t *label)
{
  uint8_t *nlabel;
  do {
    uint8_t len;
    nlabel = (uint8_t *)strchr_P(label, '.');

    if (nlabel == NULL) len = strlen_P(label);
    else  len = nlabel - label;
    
    *ptr = len;
    memcpy_P(++ptr, label, len);
    ptr += len;
    label += len + 1;
  } while (nlabel);
  *ptr++ = 0;
  return ptr;
}

/* Appends an service as answer record to an dns packet:
 * base: start of the dns packet
 * start: start of the answer record
 * label: service type (e.g. _workstation._tcp.local). Stored in PGMSPACE
 * class: class of the answer record
 * type: type of the answer record
 * ttl: time to live
 * name: service name (e.g. ethersex). Stored in PROGMEM
 *
 * The Datafield of the answer will be label+name 
 * (e.g.  ethersex._workstation._tcp.local ) except direct is set to true
 */

static uint8_t *
append_service(uint8_t *base, uint8_t *start, uint8_t *label, 
               uint16_t class, uint16_t type, uint16_t ttl, 
               uint8_t *name, uint8_t direct) 
{
  struct dns_answer_info *answer;
  uint8_t *tmp = append_label(start, label);

  answer = (struct dns_answer_info *)tmp;
  answer->class = ntohs(class);
  answer->type = ntohs(type);
  answer->ttl[0] = 0;
  answer->ttl[1] = ntohs(ttl);
  if (direct) {
    start = append_label(answer->data, name);
  } else {
    tmp = answer->data;
    *tmp++ = strlen_P(name);
    memcpy_P(tmp, name, strlen_P(name));
    tmp += strlen_P(name);
    uint16_t *ptr = (uint16_t *)tmp;
    *ptr = ntohs(0xC000 | (start - base));
    start = (uint8_t *) (ptr + 1);
  }
  answer->len = ntohs((uint8_t *)start - (uint8_t *)answer->data);

  return start;
}
 
uint8_t
compare_label(uint8_t *base, uint8_t *label, uint8_t *data) 
{
  uint8_t n;
  do {
    n = *label++;
    if ( n & 0xC0 ) {
      label = base + (((n & (~0xC0)) << 8) | *label);
      continue;
    }
    else {
      uint8_t pgm_len;
      uint8_t *tmp = (uint8_t *)strchr_P(data, '.');

      if (!tmp) pgm_len = strlen_P(data);
      else pgm_len = tmp - data;

      if( (pgm_len != n) || (strncmp_P((char *)label, (char *)data, n) != 0))
        return 0;
      label += n;
      data += n + 1;
    }
  } while(*label != 0);
  return 1;
}

/* Search for an question in the dns body with label(PROGMEM) and type,
 * which is not answerd with len and data(PROGMEM).
 */
static uint8_t
has_answer(struct dns_body *body, uint8_t *label, uint16_t type, 
           uint8_t *data, uint8_t len)
{
  /* Search if the label is asked */
  uint8_t i;
  for (i = 0; i < body->questions; i++)
    if (body->quests[i].info->type == ntohs(type) 
        && (compare_label((uint8_t *)body->hdr, body->quests[i].label, label) != 0)) 
    {
      /* Yeah we have found an question, let's look now if the client has an
       * answer sent with the question, so that we musn't answer again */
      for (i = 0; i < body->answers; i++) {
        if (body->ans[i].info->type == ntohs(type) 
            && compare_label((uint8_t *)body->hdr, body->ans[i].label, label) 
            && compare_label((uint8_t *)body->hdr, body->ans[i].info->data, data))
          return 1;
      }
      return 0;
    }
  return 1;
}

void 
mdns_new_data(void)
{
  uint8_t i;
  struct dns_body body;
  body.hdr = (void *)uip_appdata;

  if (body.hdr->flags1 & DNS_FLAG1_RESPONSE) return;

  /* Parse the packet */
  body.questions = ntohs(body.hdr->numquestions);
  body.answers = ntohs(body.hdr->numanswers);

  body.quests = __builtin_alloca(sizeof(struct dns_question) * body.questions);
  body.ans = __builtin_alloca(sizeof(struct dns_answer) * body.answers);

  uint8_t *nameptr = (uint8_t *) body.hdr + sizeof(struct dns_hdr);

  for (i = 0; i < body.questions; i++) {
    body.quests[i].label = nameptr;
    body.quests[i].info = (struct dns_question_info *)parse_name(nameptr);
    nameptr = ((void *)body.quests[i].info) + 4;
  }

  for (i = 0; i < body.answers; i++) {
    body.ans[i].label = nameptr;
    body.ans[i].info = (struct dns_answer_info *)parse_name(nameptr);
    nameptr =  ((void *)body.ans[i].info) + 10 + ntohs(body.ans[i].info->len);
  }

  uint8_t services = 0;
  if (!has_answer(&body, PSTR("_services._dns-sd._udp.local"), 0xC, PSTR("_workstation._tcp.local"), 0))
    services = 1;
  else if (!has_answer(&body, PSTR("_workstation._tcp.local"), 0xC, PSTR("ethersex []._workstation._tcp.local"), 0))
    services = 2;
  if (!services) return;




  /* write an answer */
  body.hdr->flags1 |= DNS_FLAG1_RESPONSE | DNS_FLAG1_AUTHORATIVE;
  body.hdr->numquestions = 0;
  body.hdr->numauthrr = 0;
  body.hdr->numextrarr = 0;
  body.answers = 1;
  body.questions = 0;
  body.hdr->numanswers = ntohs(body.answers);

  nameptr = (uint8_t *) body.hdr + sizeof(struct dns_hdr);

  if (services == 1) 
    nameptr = append_service((uint8_t *)body.hdr, nameptr, PSTR("_services._dns-sd._udp.local"), 1, 0xC, 4500, PSTR("_workstation._tcp.local"), 1); 
  else 
    nameptr = append_service((uint8_t *)body.hdr, nameptr, PSTR("_workstation._tcp.local"), 1, 0xC, 4500, PSTR("ethersex []"), 0); 

  uip_udp_send((uint8_t *)nameptr - (uint8_t *)body.hdr);

  /* Send the packet */
  uip_udp_conn_t conn;
  uip_ipaddr_copy(conn.ripaddr, BUF->destipaddr);
  conn.rport = HTONS(MDNS_PORT);
  conn.lport = HTONS(MDNS_PORT);

  uip_udp_conn = &conn;

  /* Send immediately */
  uip_process(UIP_UDP_SEND_CONN); 
  fill_llh_and_transmit();

  uip_slen = 0;
}
#endif

