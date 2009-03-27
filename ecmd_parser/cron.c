/*
 * Copyright (c) 2009 by David Gräff <david.graeff@web.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include "../config.h"
#include "../debug.h"
#include "../bit-macros.h"
#include "ecmd.h"

#ifdef CRON_SUPPORT

#include "../cron/cron.h"

int16_t parse_cmd_cron_list (char *cmd, char *output, uint16_t len)
{
	/* We can't output the whole cronjob list. Just print out the amount of jobs
	 * if the user did not specify anything and if he provided us with a number
	 * print out that specific job with all known details */
	uint8_t jobposition;
	uint8_t ret = sscanf_P(cmd, PSTR("%u"), &jobposition);

	if (ret == 1)
	{ // the user wants to know details
		struct cron_event_t* job = cron_getjob(jobposition);
		return snprintf_P(output, len, PSTR("AppID\tRep\thh:min\td.m\twod\n" "%c\t%i\t%i:%i\t%i.%i\t%i\n"), \
		job->appid, job->times, job->hour, job->minute, job->day, job->month, job->dayofweek);
	} else
	{ // print out the amount of jobs
		return snprintf_P(output, len, PSTR("Jobs: %u"), cron_jobs());
	}
}

int16_t parse_cmd_cron_rm (char *cmd, char *output, uint16_t len)
{
	uint8_t jobposition;
	uint8_t ret = sscanf_P(cmd, PSTR("%u"), &jobposition);

	if (ret == 1)
	{
		cron_jobrm(cron_getjob(jobposition));
		return snprintf_P(output, len, PSTR("rm cron %u"), jobposition);
	} else {
		while (head) cron_jobrm(head);
		return snprintf_P(output, len, PSTR("rm all cron"));
	}
}

#endif  /* CRON_SUPPORT */
