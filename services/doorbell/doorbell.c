/*
 * Copyright (c) 2009 by Stefan Riepenhausen <rhn@gmx.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
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

#include <avr/pgmspace.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "doorbell.h"

#include "protocols/sip/sip.h"

int8_t timer = 0;

/*
  If enabled in menuconfig, this function is called during boot up of ethersex
*/
int16_t
doorbell_init(void)
{
  DOORBELLDEBUG ("init\n");
  // enter your code here

  return ECMD_FINAL_OK;
}

/*
  If enabled in menuconfig, this function is periodically called
  change "timer(100,app_sample_periodic)" if needed
*/
int16_t
doorbell_periodic(void)
{
  DOORBELLDEBUG ("periodic\n");
  // enter your code here

	// Debug:
	if (PIN_HIGH(SCHALTER))
		PIN_SET(LEDGN);
	else
		PIN_CLEAR(LEDGN);

	// Ring
	if (PIN_HIGH(SCHALTER) {
		sip_start_ringing();
		timer = 30;
	}
	else {
		// Ring for a minimum time, even on a short press
		if (timer > 0)
			timer--;
		else
			sip_stop_ringing();
	}
  return ECMD_FINAL_OK;
}


/*
  -- Ethersex META --
  header(services/appsample/appsample.h)
  ifdef(`conf_DOORBELL_INIT_AUTOSTART',`init(doorbell_init)')
  ifdef(`conf_DOORBELL_PERIODIC_AUTOSTART',`timer(100,doorbell_periodic())')
*/
