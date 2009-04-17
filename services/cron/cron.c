/*
 * (c) by Alexander Neumann <alexander@bumpern.de>
 * Copyright (c) 2008 by Christian Dietrich <stettberger@dokucode.de>
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

#include <string.h>
#include <stdlib.h>
#include "cron.h"
#include "test.h"
#include "config.h"
#include "core/debug.h"
#include "protocols/ecmd/speed_parser.h"
#include "services/clock/clock.h"

uint32_t last_check;
struct cron_event_linkedlist* head;
struct cron_event_linkedlist* tail;
uint8_t cron_use_utc;

void
cron_init(void)
{
	#ifdef DEBUG_CRON
	debug_printf("cron init!\n");
	#endif

	// very important: set the linked lists head and tail to zero
	head = 0;
	tail = 0;

	// do we want to have some test entries?
	#ifdef CRON_SUPPORT_TEST
	addcrontest();
	#endif

	#ifdef CRON_DEFAULT_UTC
	cron_use_utc = USE_UTC;
	#else
	cron_use_utc = USE_LOCAL;
	#endif
}

void
cron_jobinsert_cb(
int8_t minute, int8_t hour, int8_t day, int8_t month, int8_t dayofweek,
uint8_t repeat, int8_t position, void (*handler)(void*), uint8_t extrasize, void* extradata)
{
	struct newone_t {
		char cmd;
		void (*handler)(void*);
	};

	struct newone_t* newone = realloc(extradata, extrasize+sizeof(struct newone_t));
	// realloc failed, abort creation of new cronjob
	if (!newone)
	{
		free(extradata);
		return;
	}

	memmove(newone+sizeof(struct newone_t),newone, extrasize);
	newone->cmd = ECMDS_JUMP_TO_FUNCTION;
	newone->handler = handler;

	cron_jobinsert(minute, hour, day, month, dayofweek, repeat, position, extrasize+sizeof(struct newone_t), newone);
}

void
cron_jobinsert(
int8_t minute, int8_t hour, int8_t day, int8_t month, int8_t dayofweek,
uint8_t repeat, int8_t position, uint8_t cmdsize, void* cmddata)
{
	// try to get ram space
	struct cron_event_linkedlist* newone = malloc(sizeof(struct cron_event_linkedlist));

	// no more ram available -> abort
	if (!newone) return;

	// create new entry
	newone->event.minute = minute;
	newone->event.hour = hour;
	newone->event.day = day;
	newone->event.month = month;
	newone->event.dayofweek = dayofweek;
	newone->event.repeat = repeat;
	newone->event.cmdsize = cmdsize;
	newone->event.cmddata = cmddata;

	// add to linked list
	if (!head)
	{ // special case: empty list (ignore position)
	newone->prev = 0;
	newone->next = 0;
	head = newone;
	tail = newone;
	#ifdef DEBUG_CRON
	debug_printf("cron add head!\n");
	#endif
	} else
	{
		uint8_t ss = 0;
		if (position>0)
		{
			struct cron_event_linkedlist* job = head;

			// jump to position
			while (job)
			{
				if (ss++ == position) break;
				job = job->next;
			}

			newone->prev = job->prev;
			job->prev = newone;
			newone->next = job;
			if (job==head) head = newone;
		} else // insert as last element
		{
			newone->next = 0;
			newone->prev = tail;
			tail->next = newone;
			tail = newone;
		}

		#ifdef DEBUG_CRON
		debug_printf("cron insert at %i!\n", ss);
		#endif
	}
}

void
cron_jobrm(struct cron_event_linkedlist* job)
{
	// null check
	if (!job) return;

	// free extradata
	free (job->event.cmddata);

	// remove link from element before this
	if (job == head) head = job->next;
	if (job->prev)
		job->prev->next = job->next;

	// remove link from element after this
	if (job == tail) tail = job->prev;
	if (job->next)
		job->next->prev = job->prev;

	// free the current element
	free (job);

	#ifdef DEBUG_CRON
	debug_printf("cron removed. Left %u\n", cron_jobs());
	#endif
}

uint8_t
cron_jobs()
{
	uint8_t ss = 0;
	// count remaining elements
	struct cron_event_linkedlist* job = head;
	while (job)
	{
		++ss;
		job = job->next;
	}
	return ss;
}


struct cron_event_linkedlist*
cron_getjob(uint8_t jobposition)
{
	// count remaining elements
	struct cron_event_linkedlist* job = head;
	while (job)
	{
		if (!jobposition) break;
		--jobposition;
		job = job->next;
	}

	if (jobposition)
		return 0;
	else
		return job;
}

uint16_t
cron_input(void* src)
{
	uint16_t len = 0;

	// map src buffer to job structure
	uint8_t position = *((uint8_t*)src);
	struct cron_event* jobstruct = ((void*)src+sizeof(uint8_t));
	src += cron_event_size;
	len += cron_event_size;

	// we allocate heap memory for extra data
	// this will be freed on cron job removal
	struct ch_value_struct *cmddata = 0;
	if (jobstruct->cmdsize)
	{
		len += jobstruct->cmdsize;
		cmddata = malloc(jobstruct->cmdsize);
		// we don't have memory space left on the heap -> abort
		if (!cmddata) return 0;
		memcpy(cmddata, src, jobstruct->cmdsize);
	}

	cron_jobinsert(jobstruct->minute, jobstruct->hour,
						jobstruct->day, jobstruct->month,
						jobstruct->dayofweek, jobstruct->repeat,
						position, jobstruct->cmdsize, cmddata);
	return len;
}

uint16_t
cron_output(void* target, uint16_t maxlen)
{
	// get cronjob data
	struct cron_event_linkedlist* job = cron_getjob(0);
	((uint8_t*)target)[0]= 0;

	// no jobs
	if (!job) return 0;

	// iterate over all jobs
	uint16_t size = 0;
	uint16_t temp;

	while (job && size<maxlen-cron_event_size)
	{
		++(((uint8_t*)target)[0]); // increment written_jobs counter
		struct cron_event* jobstruct = (void*)(target+size+1); //+1 for written_jobs

		/* data: some bytes of the cronjob structure are of interest */
		memcpy(jobstruct, job, cron_event_size);
		size += cron_event_size;

		temp = jobstruct->cmdsize;
		if (maxlen<temp) jobstruct->cmdsize = 0;

		/* copy extra data if any */
		if (jobstruct->cmdsize)
		{
			memcpy(jobstruct->cmddata, job->event.cmddata, jobstruct->cmdsize);
			size += jobstruct->cmdsize;
		}

		/* advance to the next job */
		job = job->next;
	}

	return size;
}

void
cron_periodic(void)
{
	struct clock_datetime_t d;
	uint32_t timestamp = clock_get_time();

	/* Check tasks at most once in a minute and only if at least one exists */
	if (!head || (timestamp - last_check) < 60) return;

	/* get time and date from unix timestamp */
	if (cron_use_utc)
		clock_datetime(&d, timestamp);
	else
		clock_localtime(&d, timestamp);

	/* check every event for a match */
	struct cron_event_linkedlist* current = head;
	struct cron_event_linkedlist* exec;
	uint8_t condition;
	while(current)
	{
		/* backup current cronjob and advance current */
		exec = current;
		current = current->next;

		/* check if cron 'exec' matches current time */
		for (condition = 0; condition < 5; ++condition)
		{
			/* if this field has a wildcard, just go on checking */
			if (exec->event.fields[condition] == -1)
				continue;

			/* If this field has an absolute value, check this value, if it does
			 * not match, this event does not match */
			if (exec->event.fields[condition] >= 0 && (exec->event.fields[condition] != d.cron_fields[condition]))
				break;

			/* If this field has a step value and that is within the steps,
			 * this event does not match */
			if (exec->event.fields[condition] < 0 && (d.cron_fields[condition] % -(exec->event.fields[condition])) )
				break;
		}

		/* if it matches all conditions , execute the handler function */
		if (condition==5) {
			#ifdef DEBUG_CRON
			debug_printf("cron match appid %c!\n", exec->event.appid);
			#endif

			#ifdef ECMD_SPEED_SUPPORT
			ecmd_speed_parse(exec->event.cmddata, exec->event.cmdsize);
			#else
			// without ecmd speed support we only support the jump command
			struct newone_t {
				char cmd;
				void (*handler)(void*);
				void* data;
			};
			struct newone_t *execcmd = exec->event.cmddata;
			if (exec->event.cmdsize && execcmd->cmd == ECMDS_JUMP_TO_FUNCTION)
			{
				if (execcmd->handler) execcmd->handler(execcmd->data);
			} else
			{
				debug_printf("cron wrong type!\n");
			}
			#endif

			/* Execute job endless if repeat value is equal to zero otherwise
			 * decrement the value and check if is equal to zero.
			 * If that is the case, it is time to kick out this cronjob. */
			if (exec->event.repeat > 0 && !(--exec->event.repeat))
				cron_jobrm(exec);

			exec = 0;
		}
	}

	/* save the actual timestamp */
	last_check = timestamp - d.sec;
}

/*
  -- Ethersex META --
  header(services/cron/cron.h)
  init(cron_init)
*/
