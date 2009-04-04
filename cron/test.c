/*
* Copyright (c) 2009 by David Gräff <david.graeff@web.de>
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

#include "test.h"
#include "cron.h"
#include "../syslog/syslog.h"
#include "../config.h"
#include "../debug.h"

void test(void* data)
{
	#ifdef SYSLOG_SUPPORT
	syslog_send_P(PSTR("cron test event"));
	#endif
	#ifdef DEBUG
	debug_printf("cron test event\n");
	#endif
}

void
addcrontest()
{
	cron_jobadd(test, 'T', -1, -2, -1, -1, -1, INFINIT_RUNNING, NULL); /* when hour % 2 == 0 */
	cron_jobadd(test, 'T', 51, -1, -1, -1, -1, INFINIT_RUNNING, NULL); /* when minute is 51 */
	cron_jobadd(test, 'T', -2, -1, -1, -1, -1, INFINIT_RUNNING, NULL); /* when minute % 2 == 0 */
}

