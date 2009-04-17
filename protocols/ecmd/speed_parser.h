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

#ifndef ECMD_SPEED_PARSE_H_
#define ECMD_SPEED_PARSE_H_

#define ECMD_SPEED_PROTOCOL_VERSION 5
#define ECMD_SPEED_PROTOCOL_COMBAT_VERSION 5

enum ecmd_speed_commands
{
	ECMDS_GET_PROTOCOL_VERSION,
	ECMDS_GET_PROTOCOL_COMBAT_VERSION,
	ECMDS_GET_ETHERSEX_VERSION,
	ECMDS_GET_ETHERSEX_MAC_IP_GW_MASK,
	ECMDS_SET_ETHERSEX_MAC,
	ECMDS_SET_ETHERSEX_IP,
	ECMDS_SET_ETHERSEX_GW_IP,
	ECMDS_SET_ETHERSEX_NETMASK,
	ECMDS_SET_ETHERSEX_EVENTMASK,
	ECMDS_RESERVED1A,
	ECMDS_RESERVED2A,
	ECMDS_RESERVED3A,
	ECMDS_ACTION_RESET,
	ECMDS_ACTION_BOOTLOADER,
	ECMDS_GET_STELLA_COLORS,
	ECMDS_GET_STELLA_FADE_FUNC_STEP,
	ECMDS_GET_STELLA_MOODLIGHT_DATA,
	ECMDS_SET_STELLA_INSTANT_COLOR,
	ECMDS_SET_STELLA_FADE_COLOR,
	ECMDS_SET_STELLA_FLASH_COLOR,
	ECMDS_SET_STELLA_FADE_FUNC,
	ECMDS_SET_STELLA_FADE_STEP,
	ECMDS_SET_STELLA_SAVE_TO_EEPROM,
	ECMDS_SET_STELLA_LOAD_FROM_EEPROM,
	ECMDS_SET_STELLA_MOODLIGHT_MASK,
	ECMDS_SET_STELLA_MOODLIGHT_THRESHOLD,
	ECMDS_RESERVED1B,
	ECMDS_RESERVED2B,
	ECMDS_RESERVED3B,
	ECMDS_GET_CRON_COUNT,
	ECMDS_GET_CRONS,
	ECMDS_SET_CRON_REMOVE,
	ECMDS_SET_CRON_ADD,
	ECMDS_RESERVED1C,
	ECMDS_GET_PORTPINS,
	ECMDS_SET_PORTPIN,
	ECMDS_JUMP_TO_FUNCTION
};

struct ecmd_speed_cmd_t
{
	uint8_t cmdid;
	uint8_t data;
};
typedef struct ecmd_speed_cmd_t ecmd_speed_cmd;

struct ecmd_speed_response_t
{
	uint8_t id;
	uint8_t cmd;
	uint16_t size;
};
typedef struct ecmd_speed_response_t ecmd_speed_response;

void ecmd_speed_parse(char* buf, uint8_t len);
void ecmd_speed_error();

#endif
