/*
 * Copyright (c) 2006, 2007 Jochen Roessner <jochen@lugrot.de>
 * Copyright (c) 2007 by Christian Dietrich <stettberger@dokucode.de>
 * Copyright (c) 2009 by Dirk Pannenbecker <dp@sd-gp.de>
 * Copyright (c) 2009 by Stefan Siegl <stesie@brokenpipe.de>
 * Copyright (c) 2010 by Hans Baechle <hans.baechle@gmx.net>
 * Copyright (c) 2011-2015 by Erik Kunze <ethersex@erik-kunze.de>
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
#include <avr/interrupt.h>
#include <avr/wdt.h>

#include "config.h"
#ifdef DCF1_USE_PON_SUPPORT
#include <util/delay.h>
#endif
#ifdef NTPD_SUPPORT
#include "services/ntp/ntpd_net.h"
#endif
#include "services/clock/clock.h"
#ifdef CLOCK_PERIODIC_SUPPORT
#include "core/periodic.h"
#endif
#include "dcf77.h"


static struct
{
  /* dcf_time: dummy, flags (S,A2,Z2,Z1,A1,R,x,x), min, stunde, tag, wochentag, monat, jahr */
  uint8_t timezone;
  int8_t isdst;
  uint8_t time[0x8];
#ifdef CLOCK_CRYSTAL_SUPPORT
  uint8_t ticks;
  uint8_t timerlast;
#elif CLOCK_PERIODIC_SUPPORT
  periodic_timestamp_t timerlast;
#endif
  uint8_t timebyte;
  uint8_t timeparity;
  uint8_t bitcount;
  uint8_t valid;                // dcf77-data valid
} dcf;

// current timestamp
static uint32_t timestamp = 0;

// last timestamp
static uint32_t last_timestamp = 0;

// last valid timestamp
static uint32_t last_valid_timestamp = 0;


#ifdef DEBUG_DCF77
#include "core/debug.h"
#define DCFDEBUG(...)  debug_printf("dcf77: " __VA_ARGS__)
#else
#define DCFDEBUG(...)
#endif /* DEBUG_DCF77 */

#define LOW(x)		(x<=160)
#define HIGH(x)		(x>160)
#define BCD2BIN(x)	(x-((x/16)*6))


void
dcf77_init(void)
{
  DCFDEBUG("init dcf77\n");
#ifdef DCF1_USE_PON_SUPPORT
  DCFDEBUG("PON\n");
  /* if module needs a peak on PON to enable dcf77 receiver
   * configure pin as output, set low */
  PIN_SET(DCF1_PON);
#endif

#if defined(DCF77_PCINT_PIN)
  DCFDEBUG("configure_pcint\n");
  /* configure */
  dcf77_configure_pcint();
#elif defined(DCF77_INT_PIN)
  DCFDEBUG("HAVE_DCF77_INT\n");
  /* Initialize "real" Interrupt */
  _EIMSK |= _BV(DCF77_INT_PIN);
  _EICRA = (_EICRA & ~DCF77_INT_ISCMASK) | DCF77_INT_ISC;
#else
  DCFDEBUG("Analog Comparator\n");
  // Analog Comparator init
  ACSR |= _BV(ACIE);
#endif

#ifdef DCF1_USE_PON_SUPPORT
  for (uint8_t i = 0; i < 100; i++)
  {
    wdt_kick();
    _delay_ms(10);
  }
  PIN_CLEAR(DCF1_PON);
#endif
}

// compute unix-timestamp from dcf77_ctx
static inline uint32_t
compute_dcf77_timestamp(void)
{
  clock_datetime_t dcfdate;
  dcfdate.sec = 0;
  dcfdate.min = BCD2BIN(dcf.time[2]);
  dcfdate.hour = BCD2BIN(dcf.time[3]);
  dcfdate.day = BCD2BIN(dcf.time[4]);
  dcfdate.month = BCD2BIN(dcf.time[6]);
  dcfdate.dow = dcf.time[5];    // nach ISO erster Tag Montag, nicht So!
  dcfdate.year = 100 + (BCD2BIN(dcf.time[7]));
  dcfdate.isdst = dcf.isdst;
  return clock_mktime(&dcfdate, 1);
}

#ifdef DCF77_VECTOR
ISR(DCF77_VECTOR)
#else
ISR(ANALOG_COMP_vect)
#endif
{
#ifdef CLOCK_CRYSTAL_SUPPORT
  uint8_t timertemp = TIMER_8_AS_1_COUNTER_CURRENT;
  uint16_t divtime = (dcf.ticks * (uint16_t) 256) + timertemp - dcf.timerlast;
  divtime = divtime << 2;       // TICKS2MS, x*1000ms/256
#elif CLOCK_PERIODIC_SUPPORT
  periodic_timestamp_t timertemp;
  periodic_milliticks(&timertemp);
  uint32_t divtime = periodic_millis_elapsed(&dcf.timerlast);
#endif

  if (divtime < 20)             // ignore short spikes
    return;

#ifdef HAVE_DCF1
  if (PIN_HIGH(DCF1))
#else
  if (bit_is_set(ACSR, ACO))
#endif
  {                             // start of impulse
    if (divtime > 992 || divtime < 736)
    {
      dcf.bitcount = 0;
    }
    if (divtime > 1700 && divtime < 2000 && dcf.bitcount == 0)
    {
      if (dcf.valid == 1)
      {
        // set seconds
        clock_set_time(timestamp);
        // and reset milliseconds
#ifdef CLOCK_CRYSTAL_SUPPORT
        timertemp = 0;
        TIMER_8_AS_1_COUNTER_CURRENT = timertemp;
#elif CLOCK_PERIODIC_SUPPORT_SUPPORT
        timertemp = clock_get_time();
        clock_set_time_raw_hr(timertemp, 0);
#endif
        last_valid_timestamp = timestamp;
        set_dcf_count(1);
#ifdef NTP_SUPPORT
        set_ntp_count(0);
#endif
#ifdef NTPD_SUPPORT
        // our DCF-77 Clock ist a primary; intern stratum 0; so we offer stratum+1 ! //
        ntp_setstratum(0);
#endif
        DCFDEBUG("set unix-time %lu\n", timestamp);
        dcf.valid = 0;
      }
      DCFDEBUG("start sync\n");
      dcf.bitcount = 1;
    }
#ifdef CLOCK_CRYSTAL_SUPPORT
    dcf.ticks = 0;              // start time meassure for new bit
#endif
  }
  else
  {                             // end of impulse
    DCFDEBUG("pulse: %2u %4ums %c\n",
             dcf.bitcount,
             divtime,
             (char) ((divtime > 250 || divtime < 40) ?
                     'F' : HIGH(divtime) ? '1' : '0'));
    if (divtime > 250 || divtime < 40)
    {                           // invalid pulse
      dcf.bitcount = 0;
      dcf.valid = 0;
    }
    else
    {
      if (dcf.bitcount > 0 && dcf.bitcount < 60)
      {
        switch (dcf.bitcount)
        {
          case 1:
            dcf.timebyte = 1;
            dcf.valid = 0;
            break;
          case 16:
            DCFDEBUG("%S\n", LOW(divtime) ?
                     PSTR("Normalantenne") : PSTR("Reserveantenne"));
            break;
          case 17:
            DCFDEBUG("%S\n", LOW(divtime) ?
                     PSTR("Kein Wechsel von MEZ/MESZ") :
                     PSTR("Am Ende dieser Stunde wird MEZ/MESZ umgestellt"));
            break;
          case 18:
            dcf.timezone = LOW(divtime) ? 0 : 1;
            break;
          case 19:
            DCFDEBUG("%S\n", LOW(divtime) ? PSTR("MESZ") : PSTR("MEZ"));
            dcf.isdst = LOW(divtime) ? 1 : 0;
            break;
          case 20:
            DCFDEBUG("%S\n", LOW(divtime) ?
                     PSTR("Keine Schaltsekunde") :
                     PSTR
                     ("Am Ende dieser Stunde wird eine Schaltsekunde eingefuegt"));
            break;
          case 22:
            dcf.timebyte = 2;
            dcf.timeparity = 0;
            break;
          case 29:
            dcf.time[dcf.timebyte] >>= 1;
            DCFDEBUG("Minute: %u\n", BCD2BIN(dcf.time[dcf.timebyte]));
            dcf.timebyte = 0;
            break;
          case 30:
            dcf.timebyte = 3;
            dcf.timeparity = 0;
            break;
          case 36:
            dcf.time[dcf.timebyte] >>= 2;
            DCFDEBUG("Stunde: %u\n", BCD2BIN(dcf.time[dcf.timebyte]));
            dcf.timebyte = 0;
            break;
          case 37:
            dcf.timebyte = 4;
            dcf.timeparity = 0;
            break;
          case 43:
            dcf.time[dcf.timebyte] >>= 2;
            DCFDEBUG("Tag: %u\n", BCD2BIN(dcf.time[dcf.timebyte]));
            dcf.timebyte = 5;
            break;
          case 46:
            dcf.time[dcf.timebyte] >>= 5;
            DCFDEBUG("Wochentag: %u\n", BCD2BIN(dcf.time[dcf.timebyte]));
            dcf.timebyte = 6;
            break;
          case 51:
            dcf.time[dcf.timebyte] >>= 3;
            DCFDEBUG("Monat: %u\n", BCD2BIN(dcf.time[dcf.timebyte]));
            dcf.timebyte = 7;
            break;
          case 59:
            DCFDEBUG("Jahr: 20%02u\n", BCD2BIN(dcf.time[dcf.timebyte]));
            dcf.timebyte = 0;
            break;
        }
        if (HIGH(divtime))      // 1
        {
          dcf.timeparity ^= 1;
          if (dcf.timebyte)
          {
            dcf.time[dcf.timebyte] = (dcf.time[dcf.timebyte] >> 1) | 0x80;
          }
          else if (dcf.timeparity)
          {
            goto parity_error;
          }
        }
        else                    // 0
        {
          if (dcf.bitcount == 21)       // start of time information
          {
            dcf.bitcount = 0;
          }
          if (dcf.timebyte)
          {
            dcf.time[dcf.timebyte] >>= 1;
          }
          else if (dcf.timeparity)
          {
          parity_error:
            DCFDEBUG("parity error\n");
            dcf.bitcount = 0;
            dcf.valid = 0;
          }
        }
        if (dcf.bitcount == 59)
        {
          timestamp = compute_dcf77_timestamp();
          uint32_t diff = timestamp - last_timestamp;
          // we need 2 valid timestamps; diff = 60s
          DCFDEBUG("pre-sync act - last  %lu\n", diff);
          if (diff == 60)
          {
            // ok! timestamp is valid
            dcf.valid = 1;
          }
          else
          {
            // no! but remember timestamp
            dcf.valid = 0;
            set_dcf_count(0);
            last_timestamp = timestamp;
          }
        }
      }
      dcf.bitcount++;
    }
  }
  dcf.timerlast = timertemp;
}

uint32_t
dcf77_get_last_valid_timestamp(void)
{
  return last_valid_timestamp;
}

#ifdef CLOCK_CRYSTAL_SUPPORT
void
dcf77_tick(void)
{
  dcf.ticks++;
}
#endif

/*
 -- Ethersex META --
 header(hardware/clock/dcf77/dcf77.h)
 init(dcf77_init)
 */
