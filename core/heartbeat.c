/*
* Periodically blink heartbeat / activity LED
*
* Copyright (c) 2010 by Gerd v. Egidy <gerd@egidy.de>
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

#include <avr/io.h>

#include "config.h"
#include "core/periodic.h"
#include "core/heartbeat.h"
#include "core/debug.h"

void heartbeat_init(void)
{
	PIN_SET(STATUSLED_HB_ACT);
}

void heartbeat_periodic(void)
{
	PIN_TOGGLE(STATUSLED_HB_ACT);
}

/*
  -- Ethersex META --
  header(core/heartbeat.h)
  ifdef(`conf_STATUSLED_HB_ACT', `init(heartbeat_init)')
  ifdef(`conf_STATUSLED_HEARTBEAT_BOOTLOADER', `timer(15,  heartbeat_periodic())')
  ifdef(`conf_STATUSLED_HEARTBEAT_STANDARD',   `timer(100, heartbeat_periodic())')
*/
