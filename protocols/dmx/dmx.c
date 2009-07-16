/*
 * Copyright (c) 2009 by Dirk Pannenbecker <dp@sd-gp.de>
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
#include <avr/wdt.h>
#include <util/delay.h>
#include "config.h"

#ifndef DMX_USE_USART
#define DMX_USE_USART 0
#endif
#define USE_USART DMX_USE_USART
#define BAUD 250000
#include "core/usart.h"

/* We generate our own usart init module, for our usart port */
generate_usart_init_8N2()

#ifndef CONF_DMX_MAX_CHAN
#define CONF_DMX_MAX_CHAN 64
#endif
#define DMX_NUM_CHANNELS CONF_DMX_MAX_CHAN
unsigned char dmx_data[DMX_NUM_CHANNELS];
volatile uint8_t dmx_index;
volatile uint8_t dmx_txlen;
// rainbowcolor related functions, globals and constants
void dmx_handle_rainbow_colors(void);
uint8_t color_r, color_g, color_b = 0;
#define RAINBOW_DELAY 42

volatile uint8_t dmx_prg;

/**
 * Set channum DMX-channels
 */
void
dmx_set_chan_x(uint8_t startchan, uint8_t channum, uint8_t *chan)
{
  uint8_t i;
  if (dmx_txlen < startchan + channum)
    dmx_txlen = startchan + channum;

  for (i=0;i<channum;i++)
    dmx_data[startchan + i] = chan[i];

}

/**
 * Init DMX
 */
void
dmx_init(void)
{
  /* Initialize the usart module */
  usart_init();

  /* Clear the buffers */
  dmx_txlen = 0;
  dmx_index = 0;
  color_r = 255;
  color_g = 128;
  color_b = 0;
  dmx_prg = 1;
  dmx_set_chan_x(0, 4, (uint8_t []){color_r, color_g, color_b, 159});
  dmx_set_chan_x(4, 6, (uint8_t []){17, 128, 0, color_r, color_g, color_b});
  dmx_set_chan_x(44, 6, (uint8_t []){17, 255, 0, color_r, color_g, color_b});
}


/**
 * Fade r/g/b color in rainbowcolor-style
 */
void
dmx_handle_rainbow_colors(void)
{
  static uint8_t rainbow_step = 0;
  static uint16_t rainbow_delay = 0;
  if (rainbow_delay++ <= (RAINBOW_DELAY / dmx_txlen)) return;
  rainbow_delay = 0;
  switch(rainbow_step) {
    case 0:
      if (color_g > 1) {
        color_g--;
        color_b++;
      } else {
        rainbow_step++;
      }
    break;
    case 1:
      if (color_r > 1) {
        color_r--;
        color_g++;
      } else {
        rainbow_step++;
      }
    break;
    case 2:
      if (color_b > 1) {
        color_b--;
        color_r++;
      } else {
        rainbow_step = 0;
      }
    break;
  }
}

/**
 * Send DMX-channels via USART
 */
void
dmx_tx_start(void)
{
  uint8_t sreg = SREG; cli();
  /* Enable transmitter */
  PIN_SET(DMX_RS485EN); /* pull RS485EN pin high */

  /* Send RESET */
  PIN_CLEAR(DMX_RS485TX); /* pull TX pin low */
  _delay_us(88);
  /* End of RESET; Send MARK AFTER RESET */
  PIN_SET(DMX_RS485TX); /* pull TX pin high */
  _delay_us(8);

  /** Start a new dmx packet */
  /* Enable USART */
  usart(UCSR,B) = _BV(usart(TXEN));

  /* reset Transmit Complete flag */
  usart(UCSR,A) |= _BV(usart(TXC));

  /* Send Startbyte (not always 0!) */
  usart(UDR) = 0;

  /* Enable USART interrupt*/
  usart(UCSR,B) |= _BV(usart(TXCIE));
  SREG = sreg; sei();
}

void
dmx_tx_stop(void)
{
  uint8_t sreg = SREG; cli();

  /* Disable USART */
  usart(UCSR,B) = 0;

  /* Disable transmitter */
  PIN_CLEAR(DMX_RS485EN);

  SREG = sreg; sei();
  dmx_index = 0;  /* reset output channel index */
}

/**
 * Send DMX-packet
 */
void
dmx_periodic(void)
{
  wdt_kick();
  if(dmx_index == 0) {
    if(dmx_prg == 1) {
      dmx_handle_rainbow_colors();
      dmx_set_chan_x(0, 4, (uint8_t []){color_r, color_g, color_b, 159});
      dmx_set_chan_x(4, 12, (uint8_t []){1, 128, 0, color_r, color_g, color_b, color_r, color_g, color_b, color_r, color_g, color_b});
      dmx_set_chan_x(16, 12, (uint8_t []){1, 0xff, 0, color_r, 0, 0, 0, color_g, 0, 0, 0, color_b});
      dmx_set_chan_x(44, 6, (uint8_t []){17, 0xff, 0, color_r/2, color_g/2, color_b/2});
      wdt_kick();
    }
    dmx_tx_start();
  }
}

SIGNAL(usart(USART,_TX_vect))
{
  /* Send the rest */
  if(dmx_index < dmx_txlen) {
    if(usart(UCSR,A) & _BV(usart(UDRE))) {
      usart(UDR) = dmx_data[dmx_index++];

    }
  } else
    dmx_tx_stop();
}

/*
  -- Ethersex META --
  header(protocols/dmx/dmx.h)
  init(dmx_init)
  timer(10, dmx_periodic())
*/
