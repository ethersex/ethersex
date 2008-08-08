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

#include <avr/pgmspace.h>

#ifndef _SNMP_H
#define _SNMP_H
struct snmp_varbinding {
  uint8_t len;
  uint8_t *data;
  uint8_t type;
};

struct snmp_packet {
  uint8_t version;
  uint8_t *community;
  uint8_t *pdu_type;
  uint32_t request_id;
  uint8_t var_count;
  struct snmp_varbinding *binds;
};

typedef uint8_t (*snmp_reaction_callback_t)(uint8_t *ptr, 
                                            struct snmp_varbinding *bind,
                                            void *userdata);
struct snmp_reaction {
  const char *obj_name;
  snmp_reaction_callback_t cb;
  void *userdata;
};
  

void snmp_new_data(void);

#define ucdExperimental "\x2b\x06\x01\x04\x01\x8f\x65\x0d"
#define ethersexExperimental ucdExperimental "\x17"
#endif
