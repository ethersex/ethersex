/*
 * Copyright (c) 2006, 2007 Jochen Roessner <jochen@lugrot.de>
 * Copyright (c) 2007 by Christian Dietrich <stettberger@dokucode.de>
 * Copyright (c) 2009 by Dirk Pannenbecker <dp@sd-gp.de>
 * Copyright (c) 2009 by Stefan Siegl <stesie@brokenpipe.de>
 * Copyright (c) 2010 by Hans Baechle <hans.baechle@gmx.net>
 * Copyright (c) 2011 by Erik Kunze <ethersex@erik-kunze.de>
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
#include "services/ntp/ntpd_net.h"
#include "services/clock/clock.h"
#include "dcf77.h"

#ifdef DCF1_USE_PON_SUPPORT
#include <util/delay.h>
#endif

static volatile struct
{
  /* dcf_time: dummy, flags (S,A2,Z2,Z1,A1,R,x,x), min, stunde, tag, wochentag, monat, jahr */
  uint8_t timezone;
  uint8_t time[0x8];
  uint32_t timerover;
  uint8_t timerlast;
  uint8_t timebyte;
  uint8_t timeparity;
  uint8_t bitcount;
  uint8_t valid; // dcf77-data valid
} dcf;

// act. dcf timestamp
static uint32_t timestamp = 0;

// last dcf77 timestamp
static uint32_t last_timestamp = 0;

// last valid timestamp
static uint32_t last_valid_timestamp = 0;

#define bcd2bin(data) (data - ((data / 16) * 6))

#ifdef DEBUG_DCF77
# include "core/debug.h"
# define DCFDEBUG(a...)  debug_printf("dcf77: " a)
# define DCFDEBUG2(a...) debug_printf(a)
#else
# define DCFDEBUG(a...) do { } while(0)
#endif /* DEBUG_DCF77 */

#ifdef CLOCK_CRYSTAL_SUPPORT
// freq 32768kHz / 128 prescaler / 2^8 timer = 1sec
#define MS(x)		((uint16_t)(x##UL*256UL/1000UL)) // x*256/1000ms
#define TICK2MS(x)	((uint16_t)((x)*4))              // x*1000ms/256
#define LOW(x)		(x<MS(135))
#define HIGH(x)		(x>MS(135))
#else
#error not supported yet
#endif

void
dcf77_init (void)
{
  DCFDEBUG ("init dcf77\n");
#ifdef DCF1_USE_PON_SUPPORT
  DCFDEBUG ("PON\n");
  /* if module needs a peak on PON to enable dcf77 receiver
     configure pin as output, set low */
  PIN_SET (DCF1_PON);
#endif

#if defined(DCF77_PCINT_PIN)
  DCFDEBUG ("configure_pcint\n");
  /* configure */
  dcf77_configure_pcint ();
#elif defined(DCF77_INT_PIN)
  DCFDEBUG ("HAVE_DCF77_INT\n");
  /* Initialize "real" Interrupt */
  _EIMSK |= _BV (DCF77_INT_PIN);
  _EICRA = (_EICRA & ~DCF77_INT_ISCMASK) | DCF77_INT_ISC;
#else
  DCFDEBUG ("Analog Comparator\n");
  // Analog Comparator init
  ACSR |= _BV (ACIE);
#endif

#ifdef DCF1_USE_PON_SUPPORT
  for (uint8_t i = 0; i < 100; i++)
    {
      wdt_kick ();
      _delay_ms (10);
    }
  PIN_CLEAR (DCF1_PON);
#endif
}

// compute unix-timestamp from dcf77_ctx
static inline uint32_t
compute_dcf77_timestamp (void)
{
  struct clock_datetime_t dcfdate;
  dcfdate.sec = 0;
  dcfdate.min = bcd2bin (dcf.time[2]);
  dcfdate.hour = bcd2bin (dcf.time[3]);
  dcfdate.day = bcd2bin (dcf.time[4]);
  dcfdate.month = bcd2bin (dcf.time[6]);
  //dcfdate.dow   = dow; // nach ISO erster Tag Montag, nicht So!
  dcfdate.year = 100 + (bcd2bin (dcf.time[7]));
  return clock_utc2timestamp (&dcfdate, dcf.timezone);
}

#ifdef DCF77_VECTOR
ISR (DCF77_VECTOR)
#else
ISR (ANALOG_COMP_vect)
#endif
{
  uint8_t timertemp = TIMER_8_AS_1_COUNTER_CURRENT;
  /* 1/256s since last signal pulse */
  uint16_t divtime = ((clock_get_time () - dcf.timerover) * 255) +
	             timertemp - dcf.timerlast;

  if (divtime < MS (20))	// ignore short spikes
    return;

#ifdef HAVE_DCF1
  if (PIN_HIGH (DCF1))
#else
  if ((ACSR & _BV (ACO)) != 0)
#endif
    {				// start of impulse
      if (divtime > MS (992) || divtime < MS (736))
	{
	  dcf.bitcount = 0;
	}
      if (divtime > MS (1700) && divtime < MS (2000) && dcf.bitcount == 0)
	{
	  if (dcf.valid == 1)
	    {
	      clock_set_time (timestamp);
	      TIMER_8_AS_1_COUNTER_CURRENT = 0;
	      last_valid_timestamp = timestamp;
	      set_dcf_count (1);
	      set_ntp_count (0);
#ifdef NTPD_SUPPORT
	      // our DCF-77 Clock ist a primary; intern stratum 0; so we offer stratum+1 ! //
	      ntp_setstratum (0);
#endif
	      DCFDEBUG ("set unix-time %lu\n", timestamp);
	      dcf.valid = 0;
	    }
	  DCFDEBUG ("start sync\n");
	  // FIXME: divtime may overflow 8 bit
	  dcf.bitcount = 1;
	  TIMER_8_AS_1_COUNTER_CURRENT = divtime;
	  timertemp = divtime;
	}
    }
  else
    {				// end of impulse
      DCFDEBUG ("pulse: %2u %4ums %c\n",
		dcf.bitcount,
		TICK2MS(divtime),
		(char)((divtime > MS (252) || divtime < MS (44)) ?
		       'F' : HIGH(divtime) ? '1' : '0'));
      if (divtime > MS (250) || divtime < MS (40))
	{			// invalid pulse
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
		  DCFDEBUG ("%S\n", LOW (divtime) ?
			    PSTR ("Normalantenne") :
			    PSTR ("Reserveantenne"));
		  break;
		case 17:
		  DCFDEBUG ("%S\n", LOW (divtime) ?
			    PSTR ("Kein Wechsel von MEZ/MESZ") :
			    PSTR ("Am Ende dieser Stunde wird MEZ/MESZ umgestellt"));
		  break;
		case 18:
		  dcf.timezone = LOW (divtime) ? 0 : 1;
		  break;
		case 19:
		  DCFDEBUG ("%S\n", LOW (divtime) ?
			    PSTR ("MESZ") :
			    PSTR ("MEZ"));
		  break;
		case 20:
		  DCFDEBUG ("%S\n", LOW (divtime) ?
			    PSTR ("Keine Schaltsekunde") :
			    PSTR ("Am Ende dieser Stunde wird eine Schaltsekunde eingefuegt"));
		  break;
		case 22:
		  dcf.timebyte = 2;
		  dcf.timeparity = 0;
		  break;
		case 29:
		  dcf.time[dcf.timebyte] >>= 1;
		  DCFDEBUG ("Minute: %u\n", bcd2bin (dcf.time[dcf.timebyte]));
		  dcf.timebyte = 0;
		  break;
		case 30:
		  dcf.timebyte = 3;
		  dcf.timeparity = 0;
		  break;
		case 36:
		  dcf.time[dcf.timebyte] >>= 2;
		  DCFDEBUG ("Stunde: %u\n", bcd2bin (dcf.time[dcf.timebyte]));
		  dcf.timebyte = 0;
		  break;
		case 37:
		  dcf.timebyte = 4;
		  dcf.timeparity = 0;
		  break;
		case 43:
		  dcf.time[dcf.timebyte] >>= 2;
		  DCFDEBUG ("Tag: %u\n", bcd2bin (dcf.time[dcf.timebyte]));
		  dcf.timebyte = 5;
		  break;
		case 46:
		  dcf.time[dcf.timebyte] >>= 5;
		  DCFDEBUG ("Wochentag: %u\n",
			    bcd2bin (dcf.time[dcf.timebyte]));
		  dcf.timebyte = 6;
		  break;
		case 51:
		  dcf.time[dcf.timebyte] >>= 3;
		  DCFDEBUG ("Monat: %u\n", bcd2bin (dcf.time[dcf.timebyte]));
		  dcf.timebyte = 7;
		  break;
		case 59:
		  DCFDEBUG ("Jahr: 20%02u\n", bcd2bin (dcf.time[dcf.timebyte]));
		  dcf.timebyte = 0;
		  break;
		}
	      if (divtime > MS (160))	// 1
		{
		  dcf.timeparity ^= 1;
		  if (dcf.timebyte)
		    {
		      dcf.time[dcf.timebyte] =
			(dcf.time[dcf.timebyte] >> 1) | 0x80;
		    }
		  else if (dcf.timeparity)
		    {
		      goto parity_error;
		    }
		}
	      else			// 0
		{
		  if (dcf.bitcount == 21) // start of time information
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
		      DCFDEBUG ("parity error\n");
		      dcf.bitcount = 0;
		      dcf.valid = 0;
		    }
		}
	      if (dcf.bitcount == 59)
		{
		  timestamp = compute_dcf77_timestamp ();
		  uint32_t diff = timestamp - last_timestamp;
		  // we need 2 valid timestamps; diff = 60s
		  DCFDEBUG ("pre-sync act - last  %lu\n", diff);
		  if (diff == 60)
		    {
		      // ok! timestamp is valid
		      dcf.valid = 1;
		    }
		  else
		    {
		      // no! but remember timestamp
		      dcf.valid = 0;
		      set_dcf_count (0);
		      last_timestamp = timestamp;
		    }
		}
	    }
	  dcf.bitcount++;
	}
    }
  dcf.timerlast = timertemp;
  dcf.timerover = clock_get_time ();
}

uint32_t
getLastValidDCFTimeStamp (void)
{
  return last_valid_timestamp;
}

/*
 -- Ethersex META --
 header(hardware/clock/dcf77/dcf77.h)
 init(dcf77_init)
 */
