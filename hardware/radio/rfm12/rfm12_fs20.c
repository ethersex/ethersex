/*
 *
 * Copyright (c) 2012 by Erik Kunze <ethersex@erik-kunze.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
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

#include <util/parity.h>

#include "config.h"
#include "core/bit-macros.h"

#include "rfm12.h"
#include "rfm12_ask.h"
#include "rfm12_fs20.h"

static void
send_bits(uint16_t data, uint8_t bits)
{
  if (bits == 8)
  {
    ++bits;
    data = (data << 1) | parity_even_bit(data);
  }

  for (uint16_t mask = _BV(bits - 1); mask; mask >>= 1)
  {
    /* Timing values empirically obtained, and used to adjust for on/off
     * delay in the RF12. The actual on-the-air bit timing we're after is
     * 600/600us for 1 and 400/400us for 0 - but to achieve that the RFM12B
     * needs to be turned on a bit longer and off a bit less. In addition
     * there is about 25 uS overhead in sending the on/off command over SPI.
     * With thanks to JGJ Veken for his help in getting these values right.
     */
    uint16_t width = data & mask ? 600 : 400;
    rfm12_ask_trigger(1, width + 150);
    rfm12_ask_trigger(0, width - 200);
  }
}

static void
fs20_send_internal(uint8_t fht, uint16_t house, uint8_t addr, uint8_t cmd,
                   uint8_t data)
{
  uint8_t sum = fht ? 0xc : 0x06;

  send_bits(1, 13);
  send_bits(HI8(house), 8);
  sum += HI8(house);
  send_bits(LO8(house), 8);
  sum += LO8(house);
  send_bits(addr, 8);
  sum += addr;
  send_bits(cmd, 8);
  sum += cmd;
  if (cmd & 0x20)
  {
    send_bits(data, 8);
    sum += data;
  }
  send_bits(sum, 8);
  send_bits(0, 1);
}

void
rfm12_fs20_send(uint16_t house, uint8_t addr, uint8_t cmd, uint8_t data)
{
  fs20_send_internal(0, house, addr, cmd, data);
}

#ifdef RFM12_ASK_FHT_SUPPORT
void
rfm12_fht_send(uint16_t house, uint8_t addr, uint8_t cmd, uint8_t data)
{
  fs20_send_internal(1, house, addr, cmd, data);
}
#endif
