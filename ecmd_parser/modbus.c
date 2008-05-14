/* vim:fdm=marker ts=4 et ai
 * {{{
 *
 * Copyright (c) 2008 by Christian Dietrich <stettberger@dokucode.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
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

#include <string.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "../config.h"
#include "../debug.h"
#include "../uip/uip.h"
#include "../modbus/modbus.h"
#include "../net/modbus_net.h"
#include "ecmd.h"

#define STATE(a) ((a)->appstate.modbus)
#define NIBBLE_TO_HEX(a) ((a) < 10 ? (a) + '0' : ((a) - 10 + 'a')) 

#if defined(MODBUS_SUPPORT) && !defined(TEENSY_SUPPORT)
int16_t parse_cmd_modbus_recv(char *cmd, char *output, uint16_t len)
/* {{{ */ {
  uint8_t cmd_len = strlen(cmd);
  uint8_t i;

  if ((cmd_len % 2) != 0) return -1;
  if (modbus_conn) 
    return snprintf_P(output, len, PSTR("modbus error: bus busy"));


  char hex[] = {0, 0, 0};
  char *message = __builtin_alloca(cmd_len / 2);

  if (!message) return snprintf_P(output, len, PSTR("modbus error: no memory"));

  for (i = 0; cmd_len != i; i += 2) {
    hex[0] = cmd[i];
    hex[1] = cmd[i+1];
    message[i / 2] = strtol(hex, NULL, 16);
  }
  
  uip_conn_t mb_conn;

  modbus_conn = &mb_conn;

  modbus_rxstart((uint8_t *)message, cmd_len / 2);

  while((volatile uint8_t *)modbus_conn) {
        _delay_ms(10);
        modbus_periodic();
  }

  if (STATE(&mb_conn).len == 0)
    return snprintf_P(output, len, PSTR("modbus_error: no answer"));

  uint16_t crc = modbus_crc_calc(STATE(&mb_conn).data, STATE(&mb_conn).len - 2);
  uint16_t crc_recv = 
          ((STATE(&mb_conn).data[STATE(&mb_conn).len - 1])  << 8) 
          | (STATE(&mb_conn).data[STATE(&mb_conn).len - 2]);
  if (crc != crc_recv) 
    return snprintf_P(output, len, PSTR("modbus error: crc error"));

  for (i = 0; i < STATE(&mb_conn).len - 2; i++) {
    if ((len - i*2) < 2) break;
    output[i * 2] = NIBBLE_TO_HEX(STATE(&mb_conn).data[i] >> 4);
    output[i * 2 + 1] = NIBBLE_TO_HEX(STATE(&mb_conn).data[i] & 0x0f);
  }

  return i * 2;
} /* }}} */
#endif
