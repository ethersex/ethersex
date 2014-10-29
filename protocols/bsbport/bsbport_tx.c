/*
 *
 * Copyright (c) 2013-2014 by Daniel Lindner <daniel.lindner@gmx.de>
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
#include <util/delay.h>
#include <util/atomic.h>
#include <string.h>
#include "core/eeprom.h"
#include "core/bit-macros.h"
#include "config.h"
#include "bsbport.h"
#include "bsbport_helper.h"
#include "bsbport_tx.h"

// Low-Level sending of message to bus
uint8_t
bsbport_send(uint8_t * const msg)
{

  msg[SOT] = 0xDC;
  msg[SRC] = 0x80 | BSBPORT_OWNADDRESS;

  {
    uint16_t crc = bsbport_crc(msg, msg[LEN] - 2);
    msg[msg[LEN] - 2] = HI8(crc);
    msg[msg[LEN] - 1] = LO8(crc);
  }

#ifdef DEBUG_BSBPORT_TX
  debug_printf("Send MSG: %02x%02x%02x%02x%02x%02x%02x%02x%02x ", msg[SOT],
               msg[SRC], msg[DEST], msg[LEN], msg[TYPE], msg[P1], msg[P2],
               msg[P3], msg[P4]);
  debug_printf("Data:");
  for (uint8_t i = DATA; i < msg[LEN] - 2; i++)
  {
    debug_printf("%02x", msg[i]);
  }
  debug_printf("CRC: %02x%02x ", msg[msg[LEN] - 2], msg[msg[LEN] - 1]);
#endif
  return bsbport_txstart(msg, msg[LEN]);
}


// High-level sending to bus
uint8_t
bsbport_query(const uint8_t A1, const uint8_t A2, const uint8_t A3,
              const uint8_t A4, const uint8_t dest)
{
  uint8_t msg[11];
  msg[LEN] = 11;
  msg[TYPE] = QUERY;
  msg[DEST] = dest;
  // Adress switch A1,A2 for a query
  msg[P1] = A2;
  msg[P2] = A1;
  msg[P3] = A3;
  msg[P4] = A4;
  return bsbport_send(msg);
}

// High-level sending to bus
uint8_t
bsbport_set(const uint8_t A1, const uint8_t A2, const uint8_t A3,
            const uint8_t A4, const uint8_t dest, const uint8_t * const data,
            const uint8_t datalen)
{
  uint8_t msg[BSBPORT_MESSAGE_MAX_LEN];
  msg[LEN] = datalen + 11;
  msg[TYPE] = SET;
  msg[DEST] = 0x80 & dest;
  // Adress switch A1,A2 for a set msg
  msg[P1] = A2;
  msg[P2] = A1;
  msg[P3] = A3;
  msg[P4] = A4;

  for (uint8_t i = DATA; i < msg[LEN] - 2; i++)
  {
    msg[i] = data[i - DATA];
  }

  return bsbport_send(msg);
}
