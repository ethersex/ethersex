/*
 *
 * Copyright (c) 2006, 2007 Jochen Roessner <jochen@lugrot.de>
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
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include "core/bit-macros.h"
#include "../uip/uip.h"
#include "../ntp/ntp_net.h"
#include "../clock/clock.h"
#include "../syslog/syslog.h"
#include "dcf77.h"

volatile struct dcf77_ctx dcf;
	
#define bcd2bin(data) (data - ((data/16) * 6))

void
dcf77_init(void)
{
  // Analog Comparator init
  ACSR |= _BV(ACIE);

  dcf.valid = 0;
}

/* will work until 2100, but the unix epoch is over until then */
uint32_t
utc_to_unix_time(uint16_t year, uint8_t month, uint8_t day, 
                 uint8_t hour, uint8_t minute, uint8_t second)
{
  uint16_t days, year_days;

  days = ((year - 1972) / 4 + 1) + (year - 1970) * 365;

  year_days = (month / 2);
  if (month == 8 || month == 10)
    year_days += 1;
  
  days +=  30 * (month - 1) + year_days + day - 1;
  if (month > 2) {
    days -= 2;
    if (year % 4 == 0)
      days += 1;
  }
  
  return days * 86400 + hour * 3600 + minute * 60 + second;
}



SIGNAL (SIG_COMPARATOR)
{
	uint8_t TCNT2temp = TCNT2;
    /* 1/256 since last signal pulse */
	uint16_t divtime = (TCNT2temp + (clock_get_time() - dcf.timerover) * 0xFF)
                        - dcf.TCNT2last;
	if(divtime > 5){
		if((ACSR & _BV(ACO)) == 0)
		{
			if(divtime > 0x3F || divtime < 0x0B)
			{
				dcf.sync = 0;
                syslog_send_P(PSTR("aus 1"));
			}
			if(dcf.sync > 0 && dcf.sync < 60)
			{
				switch (dcf.sync)
				{
					case 1:
						dcf.timebyte = 1;
					break;
					case 22:
						dcf.timebyte = 2;
						dcf.timeparity = 0;
					break;
					case 29:
						dcf.time[dcf.timebyte] >>= 1;
						dcf.timebyte = 0;
					break;
					case 30:
						dcf.timebyte = 3;
						dcf.timeparity = 0;
					break;
					case 36:
						dcf.time[dcf.timebyte] >>= 2;
						dcf.timebyte = 0;
					break;
					case 37:
						dcf.timebyte = 4;
						dcf.timeparity = 0;
					break;
					case 43:
						dcf.time[dcf.timebyte] >>= 2;
						dcf.timebyte = 5;
					break;
					case 46:
						dcf.time[dcf.timebyte] >>= 5;
						dcf.timebyte = 6;
					break;
					case 51:
						dcf.time[dcf.timebyte] >>= 3;
						dcf.timebyte = 7;
					break;
					case 59:
						dcf.timebyte = 0;
					break;
				}
				if (divtime > 0x28)
				{
					dcf.timeparity ^= 1;
					if(dcf.timebyte != 0)
						dcf.time[dcf.timebyte] = (dcf.time[dcf.timebyte] >> 1) | 0x80;
					else if(dcf.timeparity != 0) {
                        syslog_sendf("dcf sync lost: %d", dcf.sync);
						dcf.sync = 0;
                      }
				}
				else
				{
					if(dcf.sync == 21)
					{
						dcf.sync = 0;
                        syslog_send_P(PSTR("aus 2"));
					}
					//neuer decode
					dcf.timeparity ^= 0;
					if(dcf.timebyte != 0)
						dcf.time[dcf.timebyte] >>= 1;
					else if(dcf.timeparity != 0) {
						dcf.sync = 0;
                        syslog_send_P(PSTR("aus 3"));
                    }
				}
				if (dcf.sync == 59)
				{
                  // FIXME
				//	settimenow = 1;
                  
                  uint32_t timestamp = utc_to_unix_time(bcd2bin(dcf.time[7]) + 2007, 
                                                        bcd2bin(dcf.time[6]),
                                                        bcd2bin(dcf.time[4]),
                                                        bcd2bin(dcf.time[3]),
                                                        bcd2bin(dcf.time[2]), 0);

                  syslog_sendf("dcf: %d %d", timestamp, dcf.time[1]);
				}
				dcf.sync++;
			}
		}
		else
		{
			if(divtime > 0xF8 || divtime < 0xB8)
			{
				dcf.sync = 0;
                syslog_send_P(PSTR("aus 4"));
			}
			if(divtime > 0x1C0 && divtime < 0x1F0 && dcf.sync == 0)
			{
              syslog_send_P(PSTR("dcf: start sync"));
				dcf.sync = 1;
				//TCNT2 = 0;
				TCNT2temp = 0;
			}
        }
		dcf.TCNT2last = TCNT2temp;
		dcf.timerover = clock_get_time();
	}
}

/*
  -- Ethersex META --
  init(dcf77_init)
*/
