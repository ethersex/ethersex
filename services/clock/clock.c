/*
 * Copyright (c) 2007,2008 by Christian Dietrich <stettberger@dokucode.de>
 * Copyright (c) 2009 by Dirk Pannenbecker <dp@sd-gp.de>
 * Copyright (c) 2009 by Stefan Siegl <stesie@brokenpipe.de>
 * Copyright (c) 2011-2012 by Erik Kunze <ethersex@erik-kunze.de>
 * (c) by Alexander Neumann <alexander@bumpern.de>
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

#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "config.h"
#ifdef I2C_DS13X7_SUPPORT
#include "hardware/i2c/master/i2c_ds13x7.h"
#endif
#ifdef I2C_PCF8583_SUPPORT
#include "hardware/i2c/master/i2c_pcf8583.h"
#endif
#ifdef DCF77_SUPPORT
#include "hardware/clock/dcf77/dcf77.h"
#endif
#ifdef NTP_SUPPORT
#include "services/ntp/ntp.h"
#endif
#include "core/debug.h"
#include "core/periodic.h"
#include "clock.h"

static timestamp_t clock_timestamp;
static uint8_t ticks;
static timestamp_t sync_timestamp;
static timestamp_t n_sync_timestamp;
static timestamp_t n_sync_tick;
static int16_t delta;
static uint16_t ntp_count;
static uint16_t dcf_count;

#ifdef NTP_SUPPORT
#define NTP_RESYNC_PERIOD NTP_QUERY_INTERVAL
static uint16_t ntp_timer = 1;
#endif

#if defined(WHM_SUPPORT) || defined(UPTIME_SUPPORT) || defined(CONTROL6_SUPPORT)
timestamp_t uptime_timestamp;
#endif


void
clock_init(void)
{
#ifdef CLOCK_CRYSTAL_SUPPORT
  TIMER_8_AS_1_ASYNC_ON;
  TIMER_8_AS_1_COUNTER_CURRENT = 0;
  /* 128 prescaler to get every 1.0 second an interrupt (32768Hz/128=1Hz) */
  TIMER_8_AS_1_PRESCALER_128;

  /* Wait until the bytes are written */
  while (TIMER_8_AS_1_COUNTER_BUSY_TST);
  /* Clear the interrupt flags */
  TIMER_8_AS_1_INT_OVERFLOW_CLR;
  /* Enable the timer interrupt */
  TIMER_8_AS_1_INT_OVERFLOW_ON;
#endif

  /* reset dcf_count */
  dcf_count = 0;
}

#if defined(CLOCK_CRYSTAL_SUPPORT) || defined(CLOCK_CPU_SUPPORT)
#ifdef CLOCK_CPU_SUPPORT
ISR(TC1_VECTOR_OVERFLOW)
#else
ISR(TIMER_8_AS_1_VECTOR_OVERFLOW)
#endif
{
#ifdef DCF77_SUPPORT
  dcf77_tick();
#endif

#ifdef CLOCK_CPU_SUPPORT
  milliticks = 0;

  TC1_COUNTER_CURRENT = 65536 - CLOCK_SECONDS;
  TC1_COUNTER_COMPARE = 65536 - CLOCK_SECONDS + CLOCK_TICKS;
#endif

#if defined(NTP_SUPPORT) || defined(DCF77_SUPPORT)
  if (!sync_timestamp || sync_timestamp == clock_timestamp)
#endif
  {
    clock_timestamp++;
#if defined(WHM_SUPPORT) || defined(UPTIME_SUPPORT) || defined(CONTROL6_SUPPORT)
    uptime_timestamp++;
#endif
  }

  if (sync_timestamp)
    sync_timestamp++;
}
#endif

#ifdef NTP_SUPPORT
void
clock_periodic(void)
{
  if (ntp_timer)
    ntp_timer--;
  else
  {
    /* Retry in ~10 seconds */
    ntp_timer = 10;
    ntp_send_packet();
  }
}
#endif

void
clock_tick(void)
{
  if (++ticks >= 50)
  {
    /* Only clock here, when no crystal is connected */
#if !defined(CLOCK_CRYSTAL_SUPPORT) && !defined(CLOCK_CPU_SUPPORT)
    /* Don't wait for a sync, if no sync source is enabled */
#if defined(NTP_SUPPORT) || defined(DCF77_SUPPORT)
    if (!sync_timestamp || sync_timestamp == clock_timestamp)
#endif
    {
      clock_timestamp++;
#if defined(WHM_SUPPORT) || defined(UPTIME_SUPPORT) || defined(CONTROL6_SUPPORT)
      uptime_timestamp++;
#endif
    }

    if (sync_timestamp)
      sync_timestamp++;
#endif /* CLOCK_CRYSTAL_SUPPORT */

    ticks = 0;
  }
}

void
clock_set_time_raw(timestamp_t new_sync_timestamp)
{
  clock_timestamp = new_sync_timestamp;
}

void
clock_set_time_raw_hr(timestamp_t new_sync_timestamp, uint8_t new_ticks)
{
  clock_timestamp = new_sync_timestamp;
  ticks = new_ticks;
}

uint8_t
clock_get_ticks(void)
{
  return ticks;
}

void
clock_set_time(timestamp_t new_sync_timestamp)
{
  /* The clock was synced */
  if (sync_timestamp)
  {
    delta = new_sync_timestamp - sync_timestamp;
#if defined(CLOCK_NTP_ADJUST_SUPPORT) && !defined(CLOCK_CPU_SUPPORT)
    NTPADJDEBUG("sync timestamp delta is %d\n", delta);
    if (delta < -300 || delta > 300)
      NTPADJDEBUG("eeek, delta too large. " "not adjusting.\n");

    else if (sync_timestamp != clock_timestamp)
      NTPADJDEBUG("our clock is not up with ntp clock.\n");

    else if (NTP_RESYNC_PERIOD == -delta)
      NTPADJDEBUG("-delta equals resync period, eeek!? "
                  "clock isn't running at all.\n");
    else
    {
      uint32_t new_value = TC1_COUNTER_COMPARE;
      new_value *= NTP_RESYNC_PERIOD;
      new_value /= NTP_RESYNC_PERIOD + delta;

      NTPADJDEBUG("new OCR1A value %d\n", new_value);
      TC1_COUNTER_COMPARE = new_value;
    }
#endif /* CLOCK_NTP_ADJUST_SUPPORT */
  }

  sync_timestamp = new_sync_timestamp;
  n_sync_timestamp = new_sync_timestamp;
  n_sync_tick = TIMER_8_AS_1_COUNTER_CURRENT;

#if defined(CLOCK_DATETIME_SUPPORT) || defined(DCF77_SUPPORT) || defined(CLOCK_DATE_SUPPORT) || defined(CLOCK_TIME_SUPPORT)
  clock_reset_dst_change();
#endif

  /* Allow the clock to jump forward, but not to go backward
   * except the time difference is greater than 5 minutes */
  if (sync_timestamp > clock_timestamp ||
      (clock_timestamp - sync_timestamp) > 300)
    clock_timestamp = sync_timestamp;

#ifdef I2C_DS13X7_SUPPORT
  i2c_ds13x7_sync(sync_timestamp);
#endif
#ifdef I2C_PCF8583_SUPPORT
  i2c_pcf8583_set(sync_timestamp);
#endif

#ifdef NTP_SUPPORT
  ntp_timer = NTP_RESYNC_PERIOD;
#endif
}

timestamp_t
clock_get_time(void)
{
  return clock_timestamp;
}

timestamp_t
clock_last_sync(void)
{
  return n_sync_timestamp;
}

timestamp_t
clock_last_sync_tick(void)
{
  return n_sync_tick;
}

int16_t
clock_last_delta(void)
{
  return delta;
}

uint16_t
clock_dcf_count(void)
{
  return dcf_count;
}

void
set_dcf_count(const uint16_t new_dcf_count)
{
  dcf_count = (new_dcf_count == 0) ? 0 : dcf_count + new_dcf_count;
}

uint16_t
clock_ntp_count(void)
{
  return ntp_count;
}

void
set_ntp_count(const uint16_t new_ntp_count)
{
  ntp_count = (new_ntp_count == 0) ? 0 : ntp_count + new_ntp_count;
}

#ifdef NTP_SUPPORT
uint16_t
clock_last_ntp(void)
{
  return ntp_timer;
}
#endif

#if defined(WHM_SUPPORT) || defined(UPTIME_SUPPORT) || defined(CONTROL6_SUPPORT)
timestamp_t
clock_get_uptime(void)
{
  return uptime_timestamp;
}
#endif

/*
  -- Ethersex META --
  header(services/clock/clock.h)
  init(clock_init)
  timer(1, clock_tick())
  ifdef(`conf_NTP', `timer(50, clock_periodic())')
*/
