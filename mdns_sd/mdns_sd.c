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
#define HOSTNAME "ethersex"

#include "mdns_services.c"

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
append_label(uint8_t *ptr, const char *label)
{
  uint8_t *nlabel;
  if (label) {
    do {
      uint8_t len;
      nlabel = (uint8_t *)strchr_P(label, '.');

      if (nlabel == NULL) len = strlen_P(label);
      else  len = nlabel - (uint8_t *)label;
      
      *ptr = len;
      memcpy_P(++ptr, label, len);
      ptr += len;
      label += len + 1;
    } while (nlabel);
  }
  *ptr++ = 0;
  return ptr;
}

/* Append the header for an answer record, and returns the pointer to the len
 * field 
 */
static uint16_t *
append_answer_header(uint8_t *base, uint8_t *start, const char *label1,
                     const char *label2, uint16_t class, uint16_t type, uint16_t ttl)
{
  struct dns_answer_info *answer;
  uint8_t *tmp;
  /* When we have two labels append both */
  if (label2) {
     tmp = append_label(start, label1);
     tmp = append_label(tmp - 1, label2);
  } else 
    tmp = append_label(start, label1);

  answer = (struct dns_answer_info *)tmp;
  answer->class = ntohs(class);
  answer->type = ntohs(type);
  answer->ttl[0] = 0;
  answer->ttl[1] = ntohs(ttl);
  return &answer->len;
}

#if 0
static uint8_t *
append_service(uint8_t *base, uint8_t *start, uint8_t *label1,
               uint8_t *label2,uint16_t class, uint16_t type, uint16_t ttl, 
               uint8_t *name, uint16_t port, uint8_t method) 
{
  struct dns_answer_info *answer;
  uint8_t *tmp;
  /* When we have two labels append both */
  if (label2) {
     tmp = append_label(start, label1);
     tmp = append_label(tmp - 1, label2);
  } else 
    tmp = append_label(start, label1);

  answer = (struct dns_answer_info *)tmp;
  answer->class = ntohs(class);
  answer->type = ntohs(type);
  answer->ttl[0] = 0;
  answer->ttl[1] = ntohs(ttl);

  if (method == APPEND_DIRECT) {
    start = append_label(answer->data, name);
  } else if ( method == APPEND_WITH_LABEL) {
    tmp = answer->data;
    *tmp++ = strlen_P(name);
    memcpy_P(tmp, name, strlen_P(name));
    tmp += strlen_P(name);
    uint16_t *ptr = (uint16_t *)tmp;
   *ptr = ntohs(0xC000 | (start - base));
    start = (uint8_t *) (ptr + 1);
  } else if (method == APPEND_SRV) {
    uint16_t *ptr = (uint16_t *)answer->data;
    *ptr++ = 0; /* Priority */
    *ptr++ = 0; /* Weight */
    *ptr++ = ntohs(port); /* Port */
    start = append_label((uint8_t *)ptr, name);
  } else if (method == APPEND_IP) {
    memcpy(answer->data, (uint8_t *)name, sizeof(uip_ipaddr_t));
    start = answer->data + sizeof(uip_ipaddr_t);
  }
  answer->len = ntohs((uint8_t *)start - (uint8_t *)answer->data);

  return start;
}
#endif
 
uint8_t
compare_label(uint8_t *base, uint8_t *label, const char *data) 
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
      else pgm_len = tmp - (uint8_t *)data;

      if( (pgm_len != n) || (strncmp_P((char *)label, (char *)data, n) != 0))
        return 0;
      label += n;
      data += n + 1;
      /* If we first reach the end of data, we say: yes thats our guy */
      if (!tmp) return 1;
    }
  } while(*label != 0);
  return 1;
}

/* Search for an question in the dns body with label(PROGMEM) and type,
 * which is not answered with data(PROGMEM) ( when only_question is not set )
 */
static uint8_t
has_answer(struct dns_body *body, const char *label, uint16_t type, 
           const char *data, uint8_t only_question)
{
  /* Search if the label is asked */
  uint8_t i;
  for (i = 0; i < body->questions; i++)
    if (body->quests[i].info->type == ntohs(type) 
        && (compare_label((uint8_t *)body->hdr, body->quests[i].label, label) != 0)) 
    {
      if (only_question) return 0;
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

  for (i = 0; services[i].service; i++) {
    /* Search for questions, which are not answered */
    /* Service announcements */
    if (!has_answer(&body, PSTR("_services._dns-sd._udp.local"), 0xC, services[i].service, 0)) 
      services[i].state |= MDNS_STATE_SERVICE;
    /* PTR Requests */
    if (!has_answer(&body, services[i].service, 0x0C, services[i].name, 0))
      services[i].state |= MDNS_STATE_NAME;
    /* SRV Requests, to search for an answer, we always sent something back*/
    if (!has_answer(&body, services[i].name, 0x21, NULL, 1))
      services[i].state |= MDNS_STATE_SRV;
    /* SRV Requests, to search for an answer, we always sent something back*/
    if (!has_answer(&body, services[i].name, 0x10, services[i].text, 0))
      services[i].state |= MDNS_STATE_TEXT;
  }

  /* write an answer */
  body.hdr->flags1 |= DNS_FLAG1_RESPONSE | DNS_FLAG1_AUTHORATIVE;
  body.hdr->numquestions = 0;
  body.hdr->numauthrr = 0;
  body.hdr->numextrarr = 0;
  body.answers = 0;
  body.questions = 0;

  nameptr = (uint8_t *) body.hdr + sizeof(struct dns_hdr);
  
  uint8_t need_ip = 0;
  /* assemble the packet */
  for (i = 0; services[i].service; i++) {
    uint16_t *len_ptr = NULL;
    /* Service Requests */
    if (services[i].state & MDNS_STATE_SERVICE) {
      len_ptr = append_answer_header((uint8_t *)body.hdr, nameptr, 
                               PSTR("_services._dns-sd._udp.local"), NULL, 1, 
                               0xC, 600);
      nameptr = append_label((uint8_t *)(len_ptr + 1), services[i].service);
      *len_ptr = ntohs(nameptr - (uint8_t *)(len_ptr + 1));
      body.answers++;
    }
    /* PTR Requests */
    if (services[i].state & MDNS_STATE_NAME) {
      uint8_t *answer_base = nameptr;
      uint16_t *len_ptr = append_answer_header((uint8_t *)body.hdr, nameptr, 
                               services[i].service, NULL, 1, 
                               0xC, 600);
      nameptr = append_label((uint8_t *)(len_ptr + 1), services[i].name);
      uint16_t *ptr = (uint16_t *)(nameptr - 1);
      /* Append an pointer to services[i].service */
      *ptr = ntohs(0xC000 | (answer_base - (uint8_t *)body.hdr));
      nameptr = (uint8_t *) (ptr + 1);
      *len_ptr = ntohs(nameptr - (uint8_t *)(len_ptr + 1));
      body.answers++;
    }
    /* SRV Requests */
    if (services[i].state & MDNS_STATE_SRV) {
      uint16_t *len_ptr = append_answer_header((uint8_t *)body.hdr, nameptr, 
                               services[i].name, services[i].service, 0x8001, 
                               0x21, 600);
      uint16_t *ptr = (uint16_t *)(len_ptr + 1);
      *ptr++ = 0; /* Priority */
      *ptr++ = 0; /* Weight */
      *ptr++ = ntohs(services[i].port); /* Port */
      nameptr = append_label((uint8_t *)ptr, PSTR(HOSTNAME ".local"));
      /* An ip record will be appended */
      need_ip = 1;
      *len_ptr = ntohs(nameptr - (uint8_t *)(len_ptr + 1));
      body.answers++;
    }
    /* TXT Requests */
    if (services[i].state & MDNS_STATE_TEXT) {
      uint16_t *len_ptr = append_answer_header((uint8_t *)body.hdr, nameptr, 
                               services[i].name, services[i].service, 0x8001, 
                               0x10, 600);
      nameptr = append_label((uint8_t *)(len_ptr + 1), services[i].text);
      *len_ptr = ntohs(nameptr - (uint8_t *)(len_ptr + 1));
      body.answers++;
      
    }
    
    services[i].state = 0;

    if ((nameptr - (uint8_t *) body.hdr) > 512) break;
  }

  if (need_ip) {
      uint16_t *len_ptr = append_answer_header((uint8_t *)body.hdr, nameptr, 
                               PSTR(HOSTNAME ".local"), services[i].service, 0x8001, 
                               0x01, 600);
      nameptr = ((uint8_t *)len_ptr) + 2 + sizeof(uip_ipaddr_t);
      *len_ptr = ntohs(sizeof(uip_ipaddr_t));
      /* here we append our own ip address */
      memcpy((uint8_t *) (len_ptr + 1), uip_hostaddr, sizeof(uip_ipaddr_t));
      
      body.answers++;
  }

  /* We have nothing to say, so return */
  if (body.answers == 0) return;

  body.hdr->numanswers = ntohs(body.answers);

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

