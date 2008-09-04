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
#include "ecmd.h"

#define STATE(a) ((a)->appstate.modbus)
#define NIBBLE_TO_HEX(a) ((a) < 10 ? (a) + '0' : ((a) - 10 + 'a')) 

extern int16_t *modbus_recv_len_ptr;

#if defined(MODBUS_SUPPORT)
int16_t parse_cmd_modbus_recv(char *cmd, char *output, uint16_t len)
/* {{{ */ {
  uint8_t cmd_len = strlen(cmd);
  uint8_t i;

  if ((cmd_len % 2) != 0) return -1;
  if (modbus_recv_len_ptr) 
    return snprintf_P(output, len, PSTR("modbus error: bus busy"));


  char hex[] = {0, 0, 0};
  uint8_t buffer[MODBUS_BUFFER_LEN];

  for (i = 0; cmd_len != i; i += 2) {
    hex[0] = cmd[i];
    hex[1] = cmd[i+1];
    buffer[i / 2] = strtol(hex, NULL, 16);
  }
  
  int16_t recv_len = 0;

  modbus_rxstart((uint8_t *)buffer, cmd_len / 2,&recv_len);

  while((volatile uint8_t)recv_len == 0) {
        _delay_ms(10);
        modbus_periodic();
  }


  if (recv_len == -1)
    return snprintf_P(output, len, PSTR("modbus error: no answer"));

  uint16_t crc = modbus_crc_calc(buffer, recv_len - 2);
  uint16_t crc_recv = 
          ((buffer[recv_len - 1])  << 8) 
          | (buffer[recv_len - 2]);
  if (crc != crc_recv) 
    return snprintf_P(output, len, PSTR("modbus error: crc error"));

  for (i = 0; i < recv_len - 2; i++) {
    if ((len - i*2) < 2) break;
    output[i * 2] = NIBBLE_TO_HEX(buffer[i] >> 4);
    output[i * 2 + 1] = NIBBLE_TO_HEX(buffer[i] & 0x0f);
  }

  return i * 2;
} /* }}} */
#endif
