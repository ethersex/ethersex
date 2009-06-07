/*
 *
 * Copyright (c) 2007 by Christian Dietrich <stettberger@dokucode.de>
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

#include <avr/pgmspace.h>

#ifndef _MDNS_SD_H
#define _MDNS_SD_H

/** \internal The DNS message header. */
struct dns_hdr {
  uint16_t id;
  uint8_t flags1, flags2;
#define DNS_FLAG1_RESPONSE        0x80
#define DNS_FLAG1_OPCODE_STATUS   0x10
#define DNS_FLAG1_OPCODE_INVERSE  0x08
#define DNS_FLAG1_OPCODE_STANDARD 0x00
#define DNS_FLAG1_AUTHORATIVE     0x04
#define DNS_FLAG1_TRUNC           0x02
#define DNS_FLAG1_RD              0x01
#define DNS_FLAG2_RA              0x80
#define DNS_FLAG2_NON_AUTH_OK     0x10
#define DNS_FLAG2_ERR_MASK        0x0f
#define DNS_FLAG2_ERR_NONE        0x00
#define DNS_FLAG2_ERR_NAME        0x03
  uint16_t numquestions;
  uint16_t numanswers;
  uint16_t numauthrr;
  uint16_t numextrarr;
};

/** \internal The DNS answer message structure. */
struct dns_answer {
  uint8_t *label;
  struct dns_answer_info {
    uint16_t type;
    uint16_t class;
    uint16_t ttl[2];
    uint16_t len;
    char data[];
  } *info;
};

struct dns_question {
  uint8_t *label;
  struct dns_question_info {
    uint16_t type;
    uint16_t class;
  } *info;
};

struct dns_body {
  struct dns_hdr *hdr;
  uint8_t questions;
  uint8_t answers;
  struct dns_question *quests;
  struct dns_answer *ans;
};

struct mdns_service {
  PGM_P service;
  PGM_P name;
  PGM_P text;
  uint16_t port;
  uint8_t state;
};

enum mdns_request_state {
  MDNS_STATE_SERVICE = 1,
  MDNS_STATE_NAME = 2,
  MDNS_STATE_SRV = 4,
  MDNS_STATE_TEXT = 8,
};

void mdns_new_data(void);

#endif /* _MDNS_SD_H */
