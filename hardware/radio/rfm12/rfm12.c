/*
 * Copyright (c) 2007 Jochen Roessner <jochen@lugrot.de>
 * Copyright (c) 2007 Ulrich Radig <mail@ulrichradig.de>
 * Copyright (c) 2007, 2008 Stefan Siegl <stesie@brokenpipe.de>
 * Copyright (c) 2012 Erik Kunze <ethersex@erik-kunze.de>
 * Copyright (c) Benedikt K.
 * Copyright (c) Juergen Eckert
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

#include <stdio.h>
#include <stdlib.h>
#include <util/delay.h>

#include "config.h"
#include "core/heartbeat.h"
#include "core/bit-macros.h"
#include "core/spi.h"

#include "rfm12.h"


#define _RFM12_PORT(c) PORT_CHAR(SPI_CS_RFM12_ ## c ## _PORT)
#define RFM12_PORT(c)  _RFM12_PORT(c)
#define _RFM12_PIN(c) PIN_CHAR(SPI_CS_RFM12_ ## c ## _PIN)
#define RFM12_PIN(c)  _RFM12_PIN(c)


rfm12_modul_t rfm12_moduls[RFM12_MODULE_COUNT] = {
#ifdef RFM12_IP_SUPPORT
  {&RFM12_PORT(RFM12_IP_USE_RFM12),
   _BV(RFM12_PIN(RFM12_IP_USE_RFM12)),
   RxBW134, LNA_6, RSSI_79}
  ,
#endif
#ifdef RFM12_ASK_433_SUPPORT
  {&RFM12_PORT(RFM12_ASK_433_USE_RFM12),
   _BV(RFM12_PIN(RFM12_ASK_433_USE_RFM12)),
   RxBW134, LNA_6, RSSI_79}
  ,
#endif
#ifdef RFM12_ASK_FS20_SUPPORT
  {&RFM12_PORT(RFM12_ASK_868_USE_RFM12),
   _BV(RFM12_PIN(RFM12_ASK_868_USE_RFM12)),
   RxBW134, LNA_6, RSSI_79}
  ,
#endif
};

rfm12_modul_t *rfm12_modul = rfm12_moduls;


uint16_t
rfm12_trans(uint16_t val)
{
  *rfm12_modul->rfm12_port &= (uint8_t) ~rfm12_modul->rfm12_mask;

  /* spi clock down */
#ifdef CONF_RFM12_SLOW_SPI
  _SPCR0 |= (uint8_t) _BV(SPR1);
#else
  _SPCR0 |= (uint8_t) _BV(SPR0);
#endif

  uint16_t retval = (uint16_t) (spi_send(HI8(val)) << 8);
  retval += spi_send(LO8(val));

  /* spi clock high */
#ifdef CONF_RFM12_SLOW_SPI
  _SPCR0 &= (uint8_t) ~ _BV(SPR1);
#else
  _SPCR0 &= (uint8_t) ~ _BV(SPR0);
#endif

  *rfm12_modul->rfm12_port |= rfm12_modul->rfm12_mask;
  return retval;
}

#ifndef TEENSY_SUPPORT
uint16_t
rfm12_setbandwidth(uint8_t bandwidth, uint8_t gain, uint8_t drssi)
{
  bandwidth &= 7;
  rfm12_modul->rfm12_bandwidth = bandwidth;
  gain &= 3;
  rfm12_modul->rfm12_gain = gain;
  drssi &= 7;
  rfm12_modul->rfm12_drssi = drssi;

  uint8_t param = (uint8_t) ((uint8_t) (bandwidth << 5) |
                             (uint8_t) (gain << 3) | (uint8_t) (drssi));
  return rfm12_trans(RFM12_CMD_RXCTRL | RFM12_RXCTRL_P16_VDI | param);
}

uint16_t
rfm12_setfreq(uint16_t freq)
{
  if (freq < 96)                /* 430,2400MHz */
    freq = 96;
  else if (freq > 3903)         /* 439,7575MHz */
    freq = 3903;

  return rfm12_trans(RFM12_CMD_FREQUENCY | freq);
}

uint16_t
rfm12_setbaud(uint16_t baud)
{
  uint16_t retval = 0;
  if (baud >= 7)
  {
    /* Baudrate = 344827,58621 / (R + 1) / (1 + CS * 7) */
    if (baud < 54)
      retval = rfm12_trans(RFM12_CMD_DATARATE | RFM12_DATARATE_CS |
                           ((43104 / baud / 100) - 1));
    else
      retval = rfm12_trans(RFM12_CMD_DATARATE |
                           ((344828UL / baud / 100) - 1));
  }
  return retval;
}

uint16_t
rfm12_setpower(uint8_t power, uint8_t mod)
{
  uint8_t param = (uint8_t) ((uint8_t) (power & 7) |
                             (uint8_t) ((mod & 15) << 4));
  return rfm12_trans(RFM12_CMD_TXCONF | param);
}
#endif /* !TEENSY_SUPPORT */

uint16_t
rfm12_get_status(void)
{
  return rfm12_trans(RFM12_CMD_STATUS);
}

