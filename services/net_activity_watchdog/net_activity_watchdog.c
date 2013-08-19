/*
 *
 * Copyright (c) 2013 by Alexander Wuerstlein <arw@arw.name>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License (either version 2 or
 * version 3) as published by the Free Software Foundation.
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

#include "config.h"
#include "net_activity_watchdog.h"
#include "core/global.h"
#include <stdbool.h>

static volatile bool net_activity_detected = true;
static int minutes;

void net_activity_watchdog_feed(void)
{
	net_activity_detected = true;
}

void net_activity_watchdog_periodic(void)
{
	if (++minutes >= NET_ACTIVITY_WATCHDOG_INTERVAL) {
		minutes = 0;
		if (net_activity_detected) {
			net_activity_detected = false;
		} else {
			/* Reboot by watchdog if possible. In DEBUG mode or
			 * with TEENSY_SUPPORT, there is no watchdog. In that
			 * case, try to reboot by other means (jump to 0) */
#ifdef TEENSY_SUPPORT
			status.request_reset = 1;
#else /* TEENSY_SUPPORT */
#ifdef DEBUG
			status.request_reset = 1;
#else /* DEBUG */
			status.request_wdreset = 1;
#endif /* DEBUG */
#endif /* TEENSY_SUPPORT */
		}
	}
}

/*
  -- Ethersex META --
  header(services/net_activity_watchdog/net_activity_watchdog.h)
  timer(300, net_activity_watchdog_periodic())
*/
