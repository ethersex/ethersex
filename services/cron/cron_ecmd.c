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

#include <stdlib.h>
#include <avr/pgmspace.h>
#include <avr/io.h>
#include <string.h>

#include "config.h"
#include "core/debug.h"
#include "cron.h"

#include "protocols/ecmd/ecmd-base.h"
#include "protocols/ecmd/via_tcp/ecmd_state.h"


int16_t parse_cmd_cron_list (char *cmd, char *output, uint16_t len)
{
	/* We can't output the whole cronjob list. Just print out the amount of jobs
	 * if the user did not specify anything and if he provided us with a number
	 * print out that specific job with all known details */
	uint8_t jobposition;
	uint8_t ret = sscanf_P(cmd, PSTR("%hhu"), &jobposition);

	if (ret == 1)
	{ // the user wants to know details
		struct cron_event_linkedlist* jobll = cron_getjob(jobposition);
		if (!jobll) return ECMD_FINAL_OK;
		struct cron_event* job = &(jobll->event);
		if (!job) return ECMD_FINAL_OK;
		if (job->cmd == CRON_JUMP)
			return ECMD_FINAL(snprintf_P(output, len, PSTR("jump %i %i %i %i %i %i %p"), \
			job->repeat, job->hour, job->minute, job->day, job->month, job->dayofweek, job->handler));
		else if (job->cmd == CRON_ECMD)
			return ECMD_FINAL(snprintf_P(output, len, PSTR("ecmd %i %i %i %i %i %i %s"), \
			job->repeat, job->hour, job->minute, job->day, job->month, job->dayofweek, &(job->ecmddata)));			
	}

	// print out the amount of jobs
	return ECMD_FINAL(snprintf_P(output, len, PSTR("Jobs: %u"), cron_jobs()));
}

#ifdef CRON_VFS_SUPPORT
int16_t parse_cmd_cron_save (char *cmd, char *output, uint16_t len)
{

	return ECMD_FINAL(snprintf_P(output, len, PSTR("%u jobs saved"),cron_save()));
}

uint16_t parse_cmd_cron_make_persistent (char *cmd, char *output, uint16_t len)
{
	uint8_t jobposition;
	sscanf_P(cmd, PSTR("%hhu"), &jobposition);

	cron_make_persistent(jobposition);
	return ECMD_FINAL_OK;

}
#endif

int16_t parse_cmd_cron_rm (char *cmd, char *output, uint16_t len)
{
	uint8_t jobposition;
	uint8_t ret = sscanf_P(cmd, PSTR("%hhu"), &jobposition);

	if (ret == 1)
	{
		cron_jobrm(cron_getjob(jobposition));
		return ECMD_FINAL(snprintf_P(output, len, PSTR("rm cron %u"), jobposition));
	} else {
		while (head) cron_jobrm(head);
		return ECMD_FINAL(snprintf_P(output, len, PSTR("rm all cron")));
	}
}

// generic cron add
// Fields: Min Hour Day Month Dow ecmd
int16_t parse_cmd_cron_add (char *cmd, char *output, uint16_t len)
{
	int8_t minute, hour, day, month, dayofweek;
	int i;
	char ecmd[ECMD_INPUTBUF_LENGTH];

	sscanf_P(cmd, PSTR("%hhi %hhi %hhi %hhi %hhi %n"), &minute, &hour, &day, &month, &dayofweek, &i);
	strncpy(ecmd,cmd+i,ECMD_INPUTBUF_LENGTH);
	cron_jobinsert_ecmd(minute, hour, day, month, dayofweek, INFINIT_RUNNING, CRON_APPEND, ecmd);

	return ECMD_FINAL_OK;
}


/*
  -- Ethersex META --
  block([[CRON-Dienst]])
  ecmd_feature(cron_list, "cron_list",, Show all cron entries)
ecmd_ifdef(CRON_VFS_SUPPORT)
  ecmd_feature(cron_save, "cron_save",, Saves all persistent jobs)
  ecmd_feature(cron_make_persistent, "cron_make_persistent",, Mark a Job as persistent)
ecmd_endif()
  ecmd_feature(cron_rm, "cron_rm", POSITION, Remove one cron entry)
  ecmd_feature(cron_add, "cron_add", MIN HOUR DAY MONTH DOW ECMD, Add ECMD to cron to be executed at given time)
*/
