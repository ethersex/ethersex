/*
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

#include <util/delay.h>
#include <util/parity.h>

#include "config.h"
#include "core/bit-macros.h"
#include "core/bool.h"

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

  for (uint16_t mask = (uint16_t) _BV(bits - 1); mask; mask >>= 1)
  {
    /* Timing values empirically obtained, and used to adjust for on/off
     * delay in the RFM12. The actual on-the-air bit timing we're after is
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
  for (uint8_t i = 3; i; i--)
  {
    uint8_t sum = fht ? 0x0c : 0x06;

    rfm12_prologue(RFM12_MODUL_FS20);

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

    rfm12_epilogue();

    _delay_ms(10);
  }
}

void
rfm12_fs20_send(uint16_t house, uint8_t addr, uint8_t cmd, uint8_t data)
{
  fs20_send_internal(FALSE, house, addr, cmd, data);
}

#ifdef RFM12_ASK_FHT_SUPPORT
void
rfm12_fht_send(uint16_t house, uint8_t addr, uint8_t cmd, uint8_t data)
{
  fs20_send_internal(TRUE, house, addr, cmd, data);
}
#endif

void
rfm12_fs20_init(void)
{
  /* wait until POR done */
  for (uint8_t i = 15; i; i--)
    _delay_ms(10);

  rfm12_prologue(RFM12_MODUL_FS20);

  rfm12_trans(RFM12_CMD_LBDMCD | 0xE0);
  rfm12_trans(RFM12BAND(RFM12_FREQ_868300));
  rfm12_trans(RFM12_CMD_DATAFILTER | RFM12_DATAFILTER_AL | 0x03);
  rfm12_trans(RFM12_CMD_FIFORESET | 0x80 | RFM12_FIFORESET_DR);
  rfm12_trans(RFM12_CMD_WAKEUP);
  rfm12_trans(RFM12_CMD_DUTYCYCLE);
  rfm12_trans(RFM12_CMD_AFC | 0xF7);

#ifdef CONF_RFM12B_SUPPORT
  rfm12_trans(0xCED4);          /* Set Sync=2DD4 */
  rfm12_trans(0xCC16);          /* pll bandwitdh 0: max bitrate 86.2kHz */
#endif

  uint16_t result = rfm12_trans(RFM12_CMD_STATUS);
  (void) result;
  RFM12_DEBUG("rfm12_fs20/init: %x", result);

#ifdef TEENSY_SUPPORT
  rfm12_trans(RFM12_CMD_FREQUENCY | RFM12FREQ(RFM12_FREQ_868300));
  rfm12_trans(RFM12_CMD_RXCTRL | 0x04ac);
#else
  rfm12_setfreq(RFM12FREQ(RFM12_FREQ_868300));
  rfm12_setbandwidth(5, 1, 4);
#endif

  rfm12_epilogue();

#ifdef STATUSLED_RFM12_RX_SUPPORT
  PIN_CLEAR(STATUSLED_RFM12_RX);
#endif
#ifdef STATUSLED_RFM12_TX_SUPPORT
  PIN_CLEAR(STATUSLED_RFM12_TX);
#endif
}

/*
  -- Ethersex META --
  header(hardware/radio/rfm12/rfm12_fs20.h)
  init(rfm12_fs20_init)
*/
