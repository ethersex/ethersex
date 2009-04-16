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
#include "speed_parser.h"
#include "config.h"
#include "core/debug.h"
/* we can manipulate crons via the ecmd speed protocol */
#include "services/cron/cron.h"
/* we can manipulate stella via the ecmd speed protocol */
#include "services/stella/stella.h"
/* we want to send via uip */
#include "protocols/uip/uip.h"
#include "protocols/uip/uip_router.h"

void
ecmd_speed_error()
{
	#ifdef DEBUG
	debug_printf("Ecmd_speed parse error\n");
	#endif
}

void
ecmd_speed_parse(struct ecmd_speed_cmd* buf, uint8_t len)
{
	// GETTER
	uint8_t size = 0;
	switch (buf->cmdid)
	{
		case ECMDS_ACTION_RESET:
		break;
		case ECMDS_ACTION_BOOTLOADER:
		break;
		case ECMDS_GET_PROTOCOL_VERSION:
		break;
		case ECMDS_GET_PROTOCOL_COMBAT_VERSION:
		break;
		case ECMDS_GET_ETHERSEX_VERSION:
		break;
		case ECMDS_GET_ETHERSEX_MAC_IP_GW_MASK:
		break;
		case ECMDS_GET_STELLA_COLORS:
		break;
		case ECMDS_GET_STELLA_FADE_FUNC_STEP:
		break;
		case ECMDS_GET_STELLA_MOODLIGHT_MASK:
		break;
		case ECMDS_GET_STELLA_MOODLIGHT_THRESHOLD:
		break;
		case ECMDS_GET_CRON_COUNT:
		break;
		case ECMDS_GET_CRONS:
		break;
		case ECMDS_JUMP_TO_FUNCTION:
		break;
		default:
			size = 1;
	}

	// SETTER
	if (size) while (len) switch (buf->cmdid)
	{
		case ECMDS_SET_ETHERSEX_MAC:
		break;
		case ECMDS_SET_ETHERSEX_IP:
		break;
		case ECMDS_SET_ETHERSEX_GW_IP:
		break;
		case ECMDS_SET_ETHERSEX_NETMASK:
		break;
		case ECMDS_SET_ETHERSEX_EVENTMASK:
		break;
		case ECMDS_SET_STELLA_INSTANT_COLOR:
			stella_setValue(STELLA_SET_IMMEDIATELY, buf->data[0], buf->data[1]);
		break;
		case ECMDS_SET_STELLA_FADE_COLOR:
			stella_setValue(STELLA_SET_FADE, buf->data[0], buf->data[1]);
		break;
		case ECMDS_SET_STELLA_FLASH_COLOR:
			stella_setValue(STELLA_SET_FLASHY, buf->data[0], buf->data[1]);
		break;
		case ECMDS_SET_STELLA_FADE_FUNC:
		break;
		case ECMDS_SET_STELLA_FADE_STEP:
		break;
		case ECMDS_SET_STELLA_SAVE_TO_EEPROM:
		break;
		case ECMDS_SET_STELLA_LOAD_FROM_EEPROM:
		break;
		case ECMDS_SET_STELLA_MOODLIGHT_MASK:
		break;
		case ECMDS_SET_STELLA_MOODLIGHT_THRESHOLD:
		break;
		case ECMDS_SET_CRON_REMOVE:
		break;
		case ECMDS_SET_CRON_ADD:
		break;
		case ECMDS_SET_PORTPIN:
		break;
		default:
			break;
	}
}

/*
struct ecmd_speed_cron_struct *r;
struct cron_event_t* job;
size_t size, size_extra;
uint8_t count;
struct ecmd_speed_cron_event_struct *jobstruct;

case STELLA_GET_CRONJOBS:
	size = UIP_APPDATA_SIZE-sizeof(struct ecmd_speed_cron_struct);
	r = ecmd_speed_net_response(STELLA_GET_CRONJOBS);
	r->count = cron_write((void*)(r+sizeof(struct ecmd_speed_cron_struct)), size);
	ecmd_speed_net_unicast(STELLA_HEADER+size);
	len -= 1; buf += 1;
	break;
case STELLA_GETVALUES:
	stella_getvalues(STELLA_UNICAST_GETVALUES);
	ecmd_speed_net_unicast(STELLA_HEADER+sizeof(struct ecmd_speed_response_detailed_struct));
	len -= 1; buf += 1;
	break;
	#ifdef STELLA_SUPPORT
case ECMDS_STELLA_SET_COLOR:
	// we assume all other commands are channel set commands
	stella_setValue(0, *buf, *(buf+1));
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
case STELLA_SAVE_TO_EEPROM:
	stella_storeToEEROM();
	len -= 1; buf += 1;
	break;
case STELLA_LOAD_FROM_EEPROM:
	stella_loadFromEEROM();
	len -= 1; buf += 1;
	break;
	#endif //STELLA_SUPPORT
	#ifdef CRON_SUPPORT
case STELLA_COUNT_CRONJOBS:
	r = ecmd_speed_net_response(STELLA_COUNT_CRONJOBS);
	r->count = cron_jobs();

	ecmd_speed_net_unicast(STELLA_HEADER+sizeof(struct ecmd_speed_cron_struct));
	len -= 1; buf += 1;
	break;

case STELLA_RM_CRONJOB:
	cron_jobrm(cron_getjob((uint8_t)buf[1]));
	len -= 2; buf += 2;
	break;
case STELLA_ADD_CRONJOB:
	++buf; --len;
	// we need 8+ parameters
	// minute, hour, day, month, dayofweek, times, appid, extrasize, (extradata)
	if (len<8)
	{
		ecmd_speed_protocol_error();
		break; //OMG, because we don't know where the command ends, exit parsing
	}

	jobstruct = (void*)buf;
	buf += sizeof(struct ecmd_speed_cron_event_struct);
	len -= sizeof(struct ecmd_speed_cron_event_struct);

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
			ecmd_speed_error();
			continue;
		}
		memcpy(data, buf, jobstruct->extrasize);
		buf += jobstruct->extrasize;
		len -= jobstruct->extrasize;
	}

	cron_jobadd(ecmd_speed_cron_callback, jobstruct->appid,
					 jobstruct->minute, jobstruct->hour,
					 jobstruct->day, jobstruct->month,
					 jobstruct->dayofweek, jobstruct->times, data);
					 break;
					 #endif // CRON_SUPPORT
*/