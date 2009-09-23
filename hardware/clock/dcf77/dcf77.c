/*
 *
 * Copyright (c) 2006, 2007 Jochen Roessner <jochen@lugrot.de>
 * Copyright (c) 2007 by Christian Dietrich <stettberger@dokucode.de>
 * Copyright (c) 2009 by Dirk Pannenbecker <dp@sd-gp.de>
 * Copyright (c) 2009 by Stefan Siegl <stesie@brokenpipe.de>
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

volatile struct dcf77_ctx dcf;

#define bcd2bin(data) (data - ((data/16) * 6))

#ifdef DEBUG_DCF77
# include "core/debug.h"
# define DCFDEBUG(a...)  debug_printf("dcf77: " a)
#else
# define DCFDEBUG(a...) do { } while(0)
#endif	/* DEBUG_DCF77 */

void
dcf77_init(void)
{
#ifdef DCF1_USE_PON_SUPPORT
  /* if module needs a peak on PON to enable dcf77 receiver
      configure pin as output, set low */
  PIN_SET(DCF1_PON);
#endif

#ifdef dcf77_configure_pcint
  /* configure */
  dcf77_configure_pcint ();
#elif defined(HAVE_DCF77_INT)
  /* Initialize "real" Interrupt */
  _EIMSK |= _BV(DCF77_INT_PIN);
  _EICRA = (_EICRA & ~DCF77_INT_ISCMASK) | DCF77_INT_ISC;
#else
  // Analog Comparator init
  ACSR |= _BV(ACIE);
#endif

#ifdef DCF1_USE_PON_SUPPORT
  for (uint8_t i=0;i<100;i++) {
    wdt_kick();
    _delay_ms(10);
  }
  PIN_CLEAR(DCF1_PON);
#endif
}

#ifdef DCF77_vect
SIGNAL (DCF77_vect)
#else
SIGNAL (SIG_COMPARATOR)
#endif
{
  uint8_t TCNT2temp = TCNT2;
  /* 1/256 since last signal pulse */
  uint16_t divtime = (TCNT2temp + (clock_get_time() - dcf.timerover) * 0xFF) - dcf.TCNT2last;

  if(divtime > 5)
  {
#ifdef HAVE_DCF1
    if (!PIN_HIGH(DCF1))
#else
    if((ACSR & _BV(ACO)) == 0)
#endif
    {
      DCFDEBUG("dcf : %u\n", divtime);
      if(divtime > 0x3F || divtime < 0x0B)
      {
        dcf.sync = 0;
        DCFDEBUG("aus 1\n");
      }
      if(dcf.sync > 0 && dcf.sync < 60)
      {
        switch (dcf.sync)
        {
          case 1:
                  dcf.timebyte = 1;
                  DCFDEBUG("in 1\n");
          break;
          case 16:
                  DCFDEBUG("%S\n", divtime < 34 ? PSTR("Normalantenne") : PSTR("Reserveantenne"));
          break;
          case 17:
                  DCFDEBUG("%S\n", divtime < 34 ? PSTR("Kein Wechsel von MEZ/MESZ") : PSTR("Am Ende dieser Stunde wird MEZ/MESZ umgestellt"));
          break;
          case 18:
                  dcf.timezone = divtime < 34 ? 0 : 1;
                  DCFDEBUG("in 17\n");
          break;
          case 19:
                  DCFDEBUG("%S\n", divtime < 34 ? PSTR("MESZ") : PSTR("MEZ"));
          break;
          case 20:
                  DCFDEBUG("%S\n", divtime < 34 ? PSTR("Keine Schaltsekunde") : PSTR("Am Ende dieser Stunde wird eine Schaltsekunde eingefügt"));
          break;
          case 22:
                  dcf.timebyte = 2;
                  dcf.timeparity = 0;
                  DCFDEBUG("in 22\n");
          break;
          case 29:
                  dcf.time[dcf.timebyte] >>= 1;
                  DCFDEBUG("Minute: %u\n",bcd2bin(dcf.time[dcf.timebyte]));
                  dcf.timebyte = 0;
          break;
          case 30:
                  dcf.timebyte = 3;
                  dcf.timeparity = 0;
                  DCFDEBUG("in 30\n");
          break;
          case 36:
                  dcf.time[dcf.timebyte] >>= 2;
                  DCFDEBUG("Stunde: %u\n",bcd2bin(dcf.time[dcf.timebyte]));
                  dcf.timebyte = 0;
          break;
          case 37:
                  dcf.timebyte = 4;
                  dcf.timeparity = 0;
                  DCFDEBUG("in 37\n");
          break;
          case 43:
                  dcf.time[dcf.timebyte] >>= 2;
                  DCFDEBUG("Tag: %u\n",bcd2bin(dcf.time[dcf.timebyte]));
                  dcf.timebyte = 5;
          break;
          case 46:
                  dcf.time[dcf.timebyte] >>= 5;
                  DCFDEBUG("Wochentag: %u\n",bcd2bin(dcf.time[dcf.timebyte]));
                  dcf.timebyte = 6;
          break;
          case 51:
                  dcf.time[dcf.timebyte] >>= 3;
                  DCFDEBUG("Monat: %u\n",bcd2bin(dcf.time[dcf.timebyte]));
                  dcf.timebyte = 7;
          break;
          case 59:
                  DCFDEBUG("Jahr: %u\n",2000+bcd2bin(dcf.time[dcf.timebyte]));
                  dcf.timebyte = 0;
          break;
        }
        if (divtime > 0x28)
        {
          dcf.timeparity ^= 1;
          if(dcf.timebyte != 0)
            dcf.time[dcf.timebyte] = (dcf.time[dcf.timebyte] >> 1) | 0x80;
          else if(dcf.timeparity != 0)
          {
            DCFDEBUG("sync lost: %d\n", dcf.sync);
            dcf.sync = 0;
          }
        }
        else
        {
          if(dcf.sync == 21)
          {
            dcf.sync = 0;
            DCFDEBUG("aus 2\n");
          }
          dcf.timeparity ^= 0;
          if(dcf.timebyte != 0)
            dcf.time[dcf.timebyte] >>= 1;
          else if(dcf.timeparity != 0)
          {
            dcf.sync = 0;
            DCFDEBUG("aus 3\n");
          }
        }
        if (dcf.sync == 59)
        {
          struct clock_datetime_t dcfdate;
          dcfdate.sec   = 0;
          dcfdate.min   = bcd2bin(dcf.time[2]);
          dcfdate.hour  = bcd2bin(dcf.time[3]);
          dcfdate.day   = bcd2bin(dcf.time[4]);
          dcfdate.month = bcd2bin(dcf.time[6]);
          // dcfdate.dow   = dow; // nach ISO erster Tag Montag, nicht So!
          dcfdate.year  = bcd2bin(dcf.time[7]);
          uint32_t timestamp = clock_utc2timestamp(&dcfdate, dcf.timezone);
          DCFDEBUG("unix-time %lu\n", timestamp);
          clock_set_time(timestamp);
          ntp_setstratum(1);
        }
        dcf.sync++;
      }
    }
    else
    {
      if(divtime > 0xF8 || divtime < 0xB8)
      {
        dcf.sync = 0;
        DCFDEBUG("aus 4; divtime: 0x%x %u\n",divtime,divtime);
      }
      if(divtime > 0x1C0 && divtime < 0x1F0 && dcf.sync == 0)
      {
        DCFDEBUG("start sync\n");
        dcf.sync = 1;
        TCNT2 = divtime;
        TCNT2temp = divtime;
      }
    }
    dcf.TCNT2last = TCNT2temp;
    dcf.timerover = clock_get_time();
  }
}

/*
  -- Ethersex META --
  header(hardware/clock/dcf77/dcf77.h)
  init(dcf77_init)
*/
