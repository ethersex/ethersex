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


#include <string.h>
#include <avr/io.h>
#include <util/crc16.h>

#include "core/eeprom.h"
#include "config.h"
#include "modbus_net.h"
#include "modbus.h"
#include "modbus_client.h"

#include "modbus_client_cb.c"


#ifdef MODBUS_CLIENT_SUPPORT

void
modbus_client_process(uint8_t *data, uint8_t length, int16_t *recv_len)
{
  union ModbusRTU *rtu = (void *)data;
  /* read holding/input registers */
  if (data[1] == MODBUS_CMD_READ_HOLDING || data[1] == MODBUS_CMD_READ_INPUTS) {
    /* is holding requested ? */
    uint8_t holding = (data[1] == MODBUS_CMD_READ_HOLDING) ? 1 : 0;
    uint8_t ptr = ntohs(rtu->read.ptr);
    uint8_t len = ntohs(rtu->read.len);
    uint8_t *answer = __builtin_alloca(3 + len * 2);
    /* The requested addresses are not in our addres space */
    if ((ptr + len) > (holding ? MODBUS_HOLD_REGISTERS : MODBUS_INPUT_REGISTERS)){
      data[2] = 2;
      goto send_error;
    }
    answer[0] = MODBUS_ADDRESS;
    answer[1] = rtu->read.cmd;
    answer[2] = len * 2;
    uint8_t i = 0;
    while ( i < len) {
      /* Read the registers */
      uint16_t tmp;
      if (holding)
        tmp = modbus_read_holding(ptr + i);
      else
        tmp = modbus_read_input(ptr + i);
      answer[3 + i * 2] =  tmp >> 8;
      answer[4 + i * 2] =  tmp ;
      i++;
    }
    *recv_len = answer[2] + 3;
    memcpy(data, answer, *recv_len);
    goto send_message;
    /* write single register */
  } else if (data[1] == MODBUS_CMD_WRITE_HOLDING) {
    if (ntohs(rtu->write.ptr) >= MODBUS_HOLD_REGISTERS) {
      data[2] = 2;
      goto send_error;
    }
    modbus_write_holding(ntohs(rtu->write.ptr), ntohs(rtu->write.value));
    /* Echo the message */
    *recv_len = length;
    goto send_message;
  /* Write multiple registers */
  } else if (data[1] == MODBUS_CMD_WRITE_MULTIPLE_HOLDING) {
    uint8_t ptr = ntohs(rtu->xwrite.ptr);
    uint8_t len = ntohs(rtu->xwrite.len);
    if (ptr + len >= MODBUS_HOLD_REGISTERS) {
      data[2] = 2;
      goto send_error;
    }
    uint8_t i;
    for (i = 0; i < len; i++)
      modbus_write_holding(ptr + i, ntohs(rtu->xwrite.data[i]));


    /* Write an answer */
    data[0] = MODBUS_ADDRESS;
    data[1] = MODBUS_CMD_WRITE_HOLDING;
    data[2] = 0;
    data[3] = ptr;
    data[4] = 0;
    data[5] = len;
    *recv_len = 6;
    goto send_message;
  } else {
    data[2] = 1;
send_error:
    data[0] = MODBUS_ADDRESS;
    data[1] |= 0x80;
    *recv_len = 3;
    goto send_message;
  }
send_message:

  if (data[0] == MODBUS_BROADCAST) {
    *recv_len = -1;
    return;
  }

  uint16_t crc = modbus_crc_calc((uint8_t *)data, *recv_len);

  data[*recv_len] = crc & 0xff;
  (*recv_len) ++;
  data[*recv_len] = (crc >> 8) & 0xff;
  (*recv_len) ++;
}


#endif
