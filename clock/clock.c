/* vim:fdm=marker ts=4 et ai
 * {{{
 *
 * Copyright (c) 2007 by Christian Dietrich <stettberger@dokucode.de>
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

#include <avr/interrupt.h>
#include "../bit-macros.h"
#include "../uip/uip.h"
#include "../net/ntp_net.h"
#include "../dns/resolv.h"
#include "../ntp/ntp.h"
#include "clock.h"

static uint32_t timestamp = 1;
static uint32_t sync_timestamp = 0;

#ifdef NTP_SUPPORT
static uint16_t ntp_timer = 1;
#endif

void
clock_init(void)
{
#ifdef CLOCK_CRYSTAL_SUPPORT
  ASSR = _BV(AS2);
  TCNT2 = 0;
  /* 120 prescaler to get every second an interrupt */
  TCCR2B = _BV(CS22) | _BV(CS20);
  while(ASSR & ( _BV(TCN2UB) | _BV(TCR2BUB))) {}
  /* Clear the interrupt flags */
  TIFR2 &= ~_BV(TOV2);
  /* Enable the timer interrupt */
  TIMSK2 |= _BV(TOIE2);
#endif
}

#ifdef CLOCK_CRYSTAL_SUPPORT
SIGNAL(SIG_OVERFLOW2)
{
  if(sync_timestamp <= timestamp)
    timestamp ++;
}
#endif


void
clock_tick(void) 
{
#ifdef NTP_SUPPORT
  if(ntp_timer) {
    if((-- ntp_timer) == 0)
      ntp_send_packet();
  }

  if(timestamp <= 50 && (timestamp % 5 == 0))
    ntp_send_packet();
#endif
  /* Only clock here, when no crystal is connected */
#ifndef CLOCK_CRYSTAL_SUPPORT
  if(sync_timestamp <= timestamp)
    timestamp ++;
#endif
}

void
clock_set_time(uint32_t sync_timestamp)
{
  /* Allow the clock to jump forward, but never ever to go backward. */
  if (sync_timestamp > timestamp)
    timestamp = sync_timestamp;

#ifdef NTP_SUPPORT
  ntp_timer = 4096;
#endif
}

uint32_t
clock_get_time(void)
{
  return timestamp;
}
