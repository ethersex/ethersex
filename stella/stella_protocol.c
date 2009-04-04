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

#include <stdint.h>
#include <string.h>
#include "stella.h"
#include "stella_net.h"
#include "stella_protocol.h"
#include "../config.h"
#include "../debug.h"
/* we can manipulate crons via the stella protocol */
#include "../cron/cron.h"
/* we want to send via uip */
#include "../uip/uip.h"
#include "../uip/uip_router.h"

void
stella_protocol_parse(char* buf, uint8_t len)
{
	struct stella_cron_struct *r;
	struct cron_event_t* job;
	size_t size, size_extra;
	uint8_t count;
	struct stella_cron_event_struct *jobstruct;

	while (len) switch (*buf)
	{
		case STELLA_SET_COLOR_0:
		case STELLA_SET_COLOR_1:
		case STELLA_SET_COLOR_2:
		case STELLA_SET_COLOR_3:
		case STELLA_SET_COLOR_4:
		case STELLA_SET_COLOR_5:
		case STELLA_SET_COLOR_6:
		case STELLA_SET_COLOR_7:
			count = (uint8_t)buf[0];
			if (count < STELLA_PINS)
			{
				stella_brightness[ count ] = (uint8_t)buf[1];
				stella_fade[ count ] = (uint8_t)buf[1];
				stella_sync = UPDATE_VALUES;
			}
			len -= 2; buf += 2;
			break;
		case STELLA_FADE_COLOR_0:
		case STELLA_FADE_COLOR_1:
		case STELLA_FADE_COLOR_2:
		case STELLA_FADE_COLOR_3:
		case STELLA_FADE_COLOR_4:
		case STELLA_FADE_COLOR_5:
		case STELLA_FADE_COLOR_6:
		case STELLA_FADE_COLOR_7:
			count = (uint8_t)buf[0] & 0x07;
			if (count < STELLA_PINS)
				stella_fade[count] = (uint8_t)buf[1];
			len -= 2; buf += 2;
			break;
		case STELLA_FLASH_COLOR_0:
		case STELLA_FLASH_COLOR_1:
		case STELLA_FLASH_COLOR_2:
		case STELLA_FLASH_COLOR_3:
		case STELLA_FLASH_COLOR_4:
		case STELLA_FLASH_COLOR_5:
		case STELLA_FLASH_COLOR_6:
		case STELLA_FLASH_COLOR_7:
			count = (uint8_t)buf[0] & 0x07;
			if (count < STELLA_PINS)
			{
				stella_brightness[ count ] = (uint8_t)buf[1];
				stella_fade[ count ] = 0;
				stella_sync = UPDATE_VALUES;
			}
			len -= 2; buf += 2;
			break;
		case STELLA_SELECT_FADE_FUNC:
			if (buf[1] < FADE_FUNC_LEN)
				stella_fade_func = buf[1];
			len -= 2; buf += 2;
			break;
		case STELLA_FADE_STEP:
			stella_fade_step = buf[1];
			len -= 2; buf += 2;
			break;
		#ifdef STELLA_MOODLIGHT
		case STELLA_MOODLIGHT_MASK:
			stella_moodlight_mask = buf[1];
			len -= 2; buf += 2;
			break;
		case STELLA_MOODLIGHT_THRESHOLD:
			stella_moodlight_threshold = buf[1];
			len -= 2; buf += 2;
			break;
		#endif
		#ifdef STELLA_RESPONSE
		case STELLA_UNICAST_GETVALUES:
			stella_net_wb_getvalues(STELLA_UNICAST_GETVALUES);
			stella_net_unicast(STELLA_HEADER+sizeof(struct stella_response_detailed_struct));
			len -= 1; buf += 1;
			break;
		case STELLA_BROADCAST_GETVALUES:
			stella_net_wb_getvalues(STELLA_BROADCAST_GETVALUES);
			stella_net_broadcast(STELLA_HEADER+sizeof(struct stella_response_detailed_struct));
			len -= 1; buf += 1;
			break;
		#endif
		case STELLA_SAVE_TO_EEPROM:
			stella_storeToEEROM();
			len -= 1; buf += 1;
			break;
		case STELLA_LOAD_FROM_EEPROM:
			stella_loadFromEEROM();
			len -= 1; buf += 1;
			break;
		#ifdef CRON_SUPPORT
		#ifdef STELLA_RESPONSE
		case STELLA_COUNT_CRONJOBS:
			/* fill structure */
			r = stella_net_response(STELLA_COUNT_CRONJOBS);
			r->count = cron_jobs();

			stella_net_unicast(STELLA_HEADER+sizeof(struct stella_cron_struct));
			len -= 1; buf += 1;
			break;
		case STELLA_GET_CRONJOBS:
			/* fill structure */
			r = stella_net_response(STELLA_GET_CRONJOBS);

			// get cronjob data
			job = cron_getjob(0);

			// no jobs
			if (!job)
			{
				r->count = 0;
				stella_net_unicast(STELLA_HEADER+sizeof(struct stella_cron_struct));
			}
			// iterate over all jobs, take 7 first byte of a job and extradata if available
			else
			{
				size = sizeof(struct stella_cron_struct);
				count = 0;
				while (job && size<UIP_APPDATA_SIZE)
				{
					++count;
					jobstruct = (void*)(r+size);

					/* data: some bytes of the cronjob structure are of interest */
					memcpy(jobstruct, job, sizeof(struct stella_cron_event_struct));
					size += sizeof(struct stella_cron_event_struct);

					/* determine extra data length */
					size_extra = 0;
					if (job->extradata)
					{
						// we take the size, that malloc stores right
						// infront of allocated heap memory
						size_extra = *((size_t*)(job->extradata - sizeof(size_t)));
						// copy extradata only if size does not exceed 255 bytes
						if (size_extra>255) size_extra = 0;
					}
					jobstruct->extrasize = (uint8_t)size_extra;

					/* copy extra data if any */
					if (size_extra)
					{
						memcpy(((char*)uip_appdata)+size, job->extradata, size_extra);
						size += size_extra;
					}

					/* advance to the next job */
					job = job->next;
				}
				// assign count
				r->count = count;
				/* send */
				stella_net_unicast(STELLA_HEADER+size);
			}
			len -= 1; buf += 1;
			break;
		#endif //STELLA_RESPONSE
		case STELLA_RM_CRONJOB:
			// get cronjob data
			job = cron_getjob((uint8_t)buf[1]);
			if (job) // remove it
				cron_jobrm(job);
			len -= 2; buf += 2;
			break;
		case STELLA_ADD_CRONJOB:
			++buf; --len;
			// we need 8+ parameters
			// minute, hour, day, month, dayofweek, times, appid, extrasize, (extradata)
			if (len<8) continue;

			jobstruct = (void*)buf;
			buf += sizeof(struct stella_cron_event_struct);
			len -= sizeof(struct stella_cron_event_struct);

			// we allocate heap memory for extra data
			// this will be freed if the cron job gets removed
			struct ch_value_struct *data = 0;
			if (jobstruct->extrasize)
			{
				data = malloc(jobstruct->extrasize);
				// we don't have memory space left on the heap -> abort
				if (!data)
				{
					buf += jobstruct->extrasize;
					len -= jobstruct->extrasize;
					continue;
				}
				memcpy(data, buf, jobstruct->extrasize);
				buf += jobstruct->extrasize;
				len -= jobstruct->extrasize;
			}

			cron_jobadd(stella_cron_callback, jobstruct->appid,
							 jobstruct->minute, jobstruct->hour,
							 jobstruct->day, jobstruct->month,
							 jobstruct->dayofweek, jobstruct->times, data);
			break;
		#endif // CRON_SUPPORT
		default:
			break;
	}
}