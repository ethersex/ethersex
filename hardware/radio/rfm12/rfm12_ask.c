/*
 * Copyright (c) Gregor B.
 * Copyright (c) Guido Pannenbecker
 * Copyright (c) Stefan Riepenhausen
 * Copyright (c) 2009 Dirk Pannenbecker <dp@sd-gp.de>
 * Copyright (c) 2012-14 by Erik Kunze <ethersex@erik-kunze.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
 *
 * For more information on the GPL, please go to:
 * http://www.gnu.org/copyleft/gpl.html
 */

#include <stdlib.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#include "config.h"
#include "core/heartbeat.h"

#include "rfm12.h"
#include "rfm12_ask.h"


#if defined(RFM12_ASK_433_SUPPORT) || defined(RFM12_ASK_868_SUPPORT)
void
rfm12_ask_trigger(uint8_t level, uint16_t us)
{
  if (level)
  {
    rfm12_trans(RFM12_CMD_PWRMGT | RFM12_PWRMGT_ET | RFM12_PWRMGT_ES |
                RFM12_PWRMGT_EX);
#ifdef STATUSLED_RFM12_TX_SUPPORT
    PIN_SET(STATUSLED_RFM12_TX);
#endif
    ACTIVITY_LED_RFM12_TX;
  }
  else
  {
    rfm12_trans(RFM12_CMD_PWRMGT | RFM12_PWRMGT_EX);
#ifdef STATUSLED_RFM12_TX_SUPPORT
    PIN_CLEAR(STATUSLED_RFM12_TX);
#endif
  }
  for (; us > 0; us--)
    _delay_us(1);
}
#endif /* RFM12_ASK_433_SUPPORT || RFM12_ASK_868_SUPPORT */

#ifdef RFM12_ASK_EXTERNAL_FILTER_SUPPORT
void
rfm12_ask_external_filter_init(void)
{
  rfm12_prologue(RFM12_MODULE_ASK);
  rfm12_trans(RFM12_CMD_PWRMGT | RFM12_PWRMGT_ER | RFM12_PWRMGT_EBB);
  rfm12_trans(RFM12_CMD_DATAFILTER | RFM12_DATAFILTER_S);
  rfm12_epilogue();
}

void
rfm12_ask_external_filter_deinit(void)
{
  rfm12_ask_init();
}
#endif /* RFM12_ASK_EXTERNAL_FILTER_SUPPORT */

#ifdef RFM12_ASK_433_SUPPORT
void
rfm12_ask_init(void)
{
  /* wait until POR done */
  for (uint8_t i = 0; i < 15; i++)
    _delay_ms(10);

  rfm12_prologue(RFM12_MODULE_ASK);

  rfm12_trans(RFM12_CMD_LBDMCD | 0xE0);
  rfm12_trans(RFM12BAND(RFM12_FREQ_433920));
  rfm12_trans(RFM12_CMD_DATAFILTER | RFM12_DATAFILTER_AL | 0x03);
  rfm12_trans(RFM12_CMD_FIFORESET | 0x80 | RFM12_FIFORESET_DR);
  rfm12_trans(RFM12_CMD_WAKEUP);
  rfm12_trans(RFM12_CMD_DUTYCYCLE);
  rfm12_trans(RFM12_CMD_AFC | 0xF7);

#ifdef CONF_RFM12B_SUPPORT
  rfm12_trans(0xCED4);          /* Set Sync=2DD4 */
  rfm12_trans(0xCC16);          /* pll bandwitdh 0: max bitrate 86.2kHz */
#endif

#ifdef DEBUG
  uint16_t result = rfm12_trans(RFM12_CMD_STATUS);
  RFM12_DEBUG("rfm12_ask/init: %x", result);
#endif

#ifdef TEENSY_SUPPORT
  rfm12_trans(RFM12_CMD_FREQUENCY | RFM12FREQ(RFM12_FREQ_433920));
  rfm12_trans(RFM12_CMD_RXCTRL | 0x04ac);
#else
  rfm12_setfreq(RFM12FREQ(RFM12_FREQ_433920));
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
#endif /* RFM12_ASK_433_SUPPORT */

/*
  -- Ethersex META --
  header(hardware/radio/rfm12/rfm12_ask.h)
  ifdef(`conf_RFM12_ASK_433',`init(rfm12_ask_init)')
*/
