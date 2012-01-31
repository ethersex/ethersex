/*
 * (c) by Alexander Neumann <alexander@bumpern.de>
 * Copyright (c) 2008 by Christian Dietrich <stettberger@dokucode.de>
 * Copyright (c) 2009 by David Gräff <david.graeff@web.de>
 * Copyright (c) 2010 by iT Engineering Stefan Müller <mueller@ite-web.de>
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
#include "config.h"
#include "cron.h"
#include "test.h"
#include "core/debug.h"
#include "protocols/ecmd/parser.h"
#include "protocols/ecmd/via_tcp/ecmd_state.h"
#include "services/clock/clock.h"

#ifdef CRON_VFS_SUPPORT
#include "core/vfs/vfs.h"
#include "core/vfs/vfs-util.h"
#define CRON_FILENAME "crn.t"
#endif

uint32_t last_check;
struct cron_event_linkedlist* head;
struct cron_event_linkedlist* tail;
uint8_t cron_use_utc;

#ifdef CRON_VFS_SUPPORT
void
cron_load()
{
	struct vfs_file_handle_t* file;
	uint8_t extrasize;
	uint16_t wsize;
	uint8_t count, i;
	struct cron_event_linkedlist* newone;
	vfs_size_t position = sizeof(count);

	file = vfs_open(CRON_FILENAME);
	if(file == NULL) {
		#ifdef DEBUG_CRON
			debug_printf("cron: no cron file found!\n");
		#endif
		return;
	}

	if (vfs_read(file, &count, sizeof(count)) != sizeof(count)) goto end;
	#ifdef DEBUG_CRON
		debug_printf("cron: file with %i entries found\n", count);
	#endif

	// read each cron event
	for(i=0; i< count; i++) {
		// read extrasize
		if (vfs_read( file, &extrasize, sizeof(extrasize)) != sizeof(extrasize)) goto end;
		// try to get ram space
		wsize = sizeof(struct cron_event_linkedlist)+extrasize;
		newone = malloc(wsize);

		#ifdef DEBUG_CRON
			debug_printf("cron: try to allocate size of %i consist of struct %i and extrasize %i!\n", wsize, sizeof(struct cron_event_linkedlist), extrasize);
		#endif


		// no more ram available -> abort
		if (!newone) {
			#ifdef DEBUG_CRON
			  debug_printf("cron: not enough ram!\n");
			#endif
			return;
		}

		wsize = sizeof(struct cron_event) + extrasize;
		if (vfs_fseek(file, position, SEEK_SET) != 0) goto end;
		if (vfs_read(file, &newone->event, wsize) != wsize) goto end;
		position += wsize;
		cron_insert(newone, CRON_APPEND);
	}
end:
	vfs_close(file);
}

int8_t 
cron_write_error(struct vfs_file_handle_t* file)
{
	vfs_close(file);

	/* truncate file */
	vfs_close(vfs_open(CRON_FILENAME));
	return -1;
}

int8_t
cron_save()
{
	struct vfs_file_handle_t* file;
	uint8_t count = 0;
	uint8_t saved_count = 0; 
	vfs_size_t filesize = sizeof(count);
	vfs_size_t tempsize = 0;
	vfs_size_t test = 0;

	#ifdef DEBUG_CRON
		debug_printf("cron: saving jobs\n");
	#endif

	file = vfs_create(CRON_FILENAME);

	if(file == NULL) {
		#ifdef DEBUG_CRON
			debug_printf("cron: can't create file\n");
		#endif
		return -1;
	}

	// placeholder
	struct cron_event_linkedlist* job = head;
	while (job) {
		if(job->event.persistent) {
			count++;
		}
		job = job->next;
	}

	if (vfs_write(file, &count, sizeof(count)) != sizeof(count))
		return cron_write_error(file);
		

	job = head;
	while (job) {
		if(job->event.persistent) {
			#ifdef DEBUG_CRON
				debug_printf("cron: writing job %i\n", count);
			#endif

			tempsize = sizeof(struct cron_event)+job->event.extrasize;
			test = sizeof(struct cron_event) + 0;
			#ifdef DEBUG_CRON
				debug_printf("cron: try to allocate size of %i consist of struct %i and extrasize %i!\n", tempsize, test , job->event.extrasize);
			#endif

			if (vfs_write(file, &job->event, tempsize ) != tempsize)
				return cron_write_error(file);
			filesize += tempsize;
			saved_count++;
		}
		job = job->next;
		// reset watchdog only if it seems that everything is going right
		if (saved_count <= count)
		{
			wdt_kick();
		}
	}
	#ifdef DEBUG_CRON
		debug_printf("cron: all jobs written with total size of %i\n", filesize);
	#endif

	vfs_truncate(file, filesize);
	vfs_close(file);
	return saved_count;
}

uint8_t
cron_make_persistent(uint8_t jobnumber)
{
	struct cron_event_linkedlist* job;

	job = cron_getjob(jobnumber);
	if(job == NULL)
	{
		#ifdef DEBUG_CRON
			debug_printf("cron: job not found\n");
		#endif
		return 0;
	}

	job->event.persistent = 1;
	return 1;
}
#endif

void
cron_init(void)
{

	// very important: set the linked lists head and tail to zero
	head = 0;
	tail = 0;

	// do we want to have some test entries?
	#ifdef CRON_SUPPORT_TEST
		#ifdef DEBUG_CRON
		debug_printf("cron: add test entries\n");
		#endif
	addcrontest();
	#endif

	#ifdef CRON_DEFAULT_UTC
	cron_use_utc = USE_UTC;
	#else
	cron_use_utc = USE_LOCAL;
	#endif

	#ifdef CRON_VFS_SUPPORT
	// load cron jobs form VFS
	cron_load();
	#endif
}


void
cron_jobinsert_callback(
int8_t minute, int8_t hour, int8_t day, int8_t month, days_of_week_t dayofweek,
uint8_t repeat, int8_t position, void (*handler)(void*), uint8_t extrasize, void* extradata)
{
	// emcd set?
	if (!handler || (extrasize==0 && extradata)) return;

	// try to get ram space
	struct cron_event_linkedlist* newone = malloc(sizeof(struct cron_event_linkedlist)+extrasize);

	// no more ram available -> abort
	if (!newone)
	{
		#ifdef DEBUG_CRON
		debug_printf("cron: not enough ram!\n");
		#endif
		return;
	}

	// create new entry
	newone->event.minute = minute;
	newone->event.hour = hour;
	newone->event.day = day;
	newone->event.month = month;
	newone->event.dayofweek = dayofweek;
	newone->event.repeat = repeat;
	newone->event.persistent = 0;
	newone->event.cmd = CRON_JUMP;
	newone->event.extrasize = extrasize;
	newone->event.handler = handler;
	strncpy(&(newone->event.extradata), extradata, extrasize);
	cron_insert(newone, position);
}

void
cron_jobinsert_ecmd(
	int8_t minute, int8_t hour, int8_t day, int8_t month, days_of_week_t dayofweek,
	uint8_t repeat,	int8_t position, char* ecmd)
{
	uint8_t ecmdsize;
	struct cron_event_linkedlist* newone;
	
	ecmdsize = strlen(ecmd);
	if (!ecmd || ecmdsize==0) return;
	//if (ecmd[ecmdsize-1] != '\n') ecmdsize++;

	// try to get ram space
	newone = malloc(sizeof(struct cron_event_linkedlist)+ecmdsize);

	// no more ram available -> abort
	if (!newone)
	{
		#ifdef DEBUG_CRON
		debug_printf("cron: not enough ram!\n");
		#endif
		return;
	}

	// create new entry
	newone->event.minute = minute;
	newone->event.hour = hour;
	newone->event.day = day;
	newone->event.month = month;
	newone->event.dayofweek = dayofweek;
	newone->event.repeat = repeat;
	newone->event.persistent = 0;
	newone->event.cmd = CRON_ECMD;
	newone->event.extrasize = ecmdsize;
	strncpy(&(newone->event.ecmddata), ecmd, ecmdsize+1);
	cron_insert(newone, position);
}

void
cron_insert(struct cron_event_linkedlist* newone, int8_t position)
{
	// add to linked list
	if (!head)
	{ // special case: empty list (ignore position)
	newone->prev = 0;
	newone->next = 0;
	head = newone;
	tail = newone;
	#ifdef DEBUG_CRON
	debug_printf("cron: insert head\n");
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
			#ifdef DEBUG_CRON
			if (newone->event.cmd == CRON_JUMP)
			  debug_printf("cron: insert at %i jump \n", ss);
			else
			  debug_printf("cron: insert at %i ecmd %s\n", ss, newone->event.ecmddata);
			#endif
		} else // insert as last element
		{
			newone->next = 0;
			newone->prev = tail;
			tail->next = newone;
			tail = newone;
			#ifdef DEBUG_CRON
			debug_printf("cron: append\n");
			#endif
		}
	}
}

void
cron_jobrm(struct cron_event_linkedlist* job)
{
	// null check
	if (!job) return;

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
	debug_printf("cron: removed. Left %u\n", cron_jobs());
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

void
cron_periodic(void)
{
	clock_datetime_t d;
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
	uint8_t counter = 0;
	while(current)
	{
		/* backup current cronjob and advance current */
		exec = current;
		current = current->next;
		++counter;

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
		/* check if cron 'exec' matches weekdays */
		if(condition==4){
			if(exec->event.fields[condition] & (1 << d.cron_fields[condition]) )
				condition++;
		}


		#ifdef DEBUG_CRON
		//debug_printf("..checked %u (%u) with %u\n", counter, exec->event.cmd, condition);
		#endif

		/* if it matches all conditions , execute the handler function */
		if (condition==5) {
			if (exec->event.cmd == CRON_JUMP)
			{
				#ifdef DEBUG_CRON
				debug_printf("cron: match %u (JUMP %p)\n", counter, &(exec->event.handler));
				#endif
				#ifndef DEBUG_CRON_DRYRUN
				exec->event.handler(&(exec->event.extradata));
				#endif
			} else if (exec->event.cmd == CRON_ECMD)
			{
				// ECMD PARSER
				#ifdef DEBUG_CRON
				debug_printf("cron: match %u (%s)\n", counter, (char*)&(exec->event.ecmddata));
				#endif
				#ifndef DEBUG_CRON_DRYRUN
				char output[ECMD_INPUTBUF_LENGTH];
				uint16_t l = ecmd_parse_command((char*)&(exec->event.ecmddata), output, sizeof(output)-1);
				#ifdef DEBUG_CRON
				if (is_ECMD_FINAL(l) || is_ECMD_AGAIN(l)) {
					output[is_ECMD_AGAIN(l) ? ECMD_AGAIN(l) : l] = 0;
					debug_printf("cron output %s\n", output);
				}
				#endif
				#endif
			}

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
  timer(50, cron_periodic())
  initearly(cron_init)
*/
