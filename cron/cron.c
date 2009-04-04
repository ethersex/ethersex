/*
 * (c) by Alexander Neumann <alexander@bumpern.de>
 * Copyright (c) 2008 by Christian Dietrich <stettberger@dokucode.de>
 * Copyright (c) 2009 by David Gräff <david.graeff@web.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
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
#include "../config.h"
#include "../debug.h"

#ifdef CRON_SUPPORT

uint32_t last_check;
struct cron_event_t* head;
struct cron_event_t* tail;
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
	cron_use_utc = USE_LOCAL;

	// do we want to have some test entries?
	#ifdef CRON_SUPPORT_TEST
	addcrontest();
	#endif
}

void
cron_jobadd(   void (*handler)(void* data), char appid,
					int8_t minute, int8_t hour, int8_t day,
					int8_t month, int8_t dayofweek, uint8_t times, void* data)
{
	// try to get ram space
	struct cron_event_t* newone = malloc(sizeof(struct cron_event_t));

	// no more ram available -> abort
	if (!newone) return;

	// create new entry
	newone->handler = handler;
	newone->appid = appid;
	newone->minute = minute;
	newone->hour = hour;
	newone->day = day;
	newone->month = month;
	newone->dayofweek = dayofweek;
	newone->times = times;
	newone->extradata = data;
	newone->next = 0; // is always the last entry

	// add to linked list
	if (!head)
	{ // special case: empty list
		newone->prev = 0;
		head = newone;
		tail = newone;
		#ifdef DEBUG_CRON
		debug_printf("cron add head!\n");
		#endif
	} else
	{
		newone->prev = tail;
		tail->next = newone;
		tail = newone;

		#ifdef DEBUG_CRON
		debug_printf("cron add!\n");
		#endif
	}
}

void
cron_jobrm(struct cron_event_t* job)
{
	// null check
	if (!job) return;

	// free extradata
	free (job->extradata);

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
	struct cron_event_t* job = head;
	while (job)
	{
		++ss;
		job = job->next;
	}
	return ss;
}


struct cron_event_t*
cron_getjob(uint8_t jobposition)
{
	// count remaining elements
	struct cron_event_t* job = head;
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
	struct cron_event_t* current = head;
	struct cron_event_t* exec;
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
			if (exec->fields[condition] == -1)
				continue;

			/* If this field has an absolute value, check this value, if it does
			 * not match, this event does not match */
			if (exec->fields[condition] >= 0 && (exec->fields[condition] != d.cron_fields[condition]))
				break;

			/* If this field has a step value and that is within the steps,
			 * this event does not match */
			if (exec->fields[condition] < 0 && (d.cron_fields[condition] % -(exec->fields[condition])) )
				break;
		}

		/* if it matches all conditions , execute the handler function */
		if (condition==5) {
			#ifdef DEBUG_CRON
			debug_printf("cron match appid %c!\n", exec->appid);
			#endif
			exec->handler(exec->extradata);

			/* Execute job endless if times value is equal to zero otherwise
			 * decrement the value and check if is equal to zero.
			 * If that is the case, it is time to kick out this cronjob. */
			if (exec->times > 0 && !(--exec->times))
				cron_jobrm(exec);

			exec = 0;
		}
	}

	/* save the actual timestamp */
	last_check = timestamp - d.sec;
}

#endif
