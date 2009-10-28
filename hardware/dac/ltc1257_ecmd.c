/*
 * Support for DAC LTC1257
 * ECMD interface
 * Copyright (C) 2009 Meinhard Schneider <meini@meini.org>
 * 
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 51 Franklin
 * Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <string.h>
#include "config.h"
#include "core/debug.h"
#include "protocols/ecmd/ecmd-base.h"
#include "protocols/syslog/syslog.h"

#include "ltc1257.h"

int16_t parse_cmd_ltc1257_delay(char *cmd, char *output, uint16_t len)
{
	uint8_t delay;
	uint8_t slen = 0;

#ifdef DEBUG_LTC1257
#ifdef DEBUG
	debug_printf("LTC1257: parse_cmd_ltc1257_delay begin.\n");
#endif
#ifdef SYSLOG_SUPPORT
	syslog_sendf("LTC1257: parse_cmd_ltc1257_delay begin.\n");
	syslog_flush();
#endif
#endif

	while(*cmd == ' ') cmd++;

	if(sscanf_P(cmd, PSTR("%u"), &delay) == 1)
	{
		/* set delay */
		ltc1257_delay_set(&delay);
	}
	else
	{
		/* return delay */
		ltc1257_delay_get(&delay);
		slen = sprintf_P(output, PSTR("%u"), delay);
	}
	
#ifdef DEBUG_LTC1257
#ifdef DEBUG
	debug_printf("LTC1257: parse_cmd_ltc1257_idelay end.\n");
#endif
#ifdef SYSLOG_SUPPORT
	syslog_sendf("LTC1257: parse_cmd_ltc1257_delay end.\n");
	syslog_flush();
#endif
#endif

	if(slen)
		return ECMD_FINAL(slen);
	else
		return ECMD_FINAL_OK;
}

int16_t parse_cmd_ltc1257_init(char *cmd, char *output, uint16_t len)
{
#ifdef DEBUG_LTC1257
#ifdef DEBUG
	debug_printf("LTC1257: parse_cmd_ltc1257_init begin.\n");
#endif
#ifdef SYSLOG_SUPPORT
	syslog_sendf("LTC1257: parse_cmd_ltc1257_init begin.\n");
	syslog_flush();
#endif
#endif

	while(*cmd == ' ') cmd++;

	if(*cmd != '\0')
		return ECMD_ERR_PARSE_ERROR;

	ltc1257_init();

#ifdef DEBUG_LTC1257
#ifdef DEBUG
	debug_printf("LTC1257: parse_cmd_ltc1257_init end.\n");
#endif
#ifdef SYSLOG_SUPPORT
	syslog_sendf("LTC1257: parse_cmd_ltc1257_init end.\n");
	syslog_flush();
#endif
#endif

	return ECMD_FINAL_OK;
}

int16_t parse_cmd_ltc1257_set(char *cmd, char *output, uint16_t len)
{

	uint16_t *value[LTC1257_MAX_NUM_VALUES];
	uint8_t num_values;
	char tmp[6];
	uint8_t tmplen;

#ifdef DEBUG_LTC1257
#ifdef DEBUG
	debug_printf("LTC1257: parse_cmd_ltc1257 begin.\n");
#endif
#ifdef SYSLOG_SUPPORT
	syslog_sendf("LTC1257: parse_cmd_ltc1257 begin.\n");
	syslog_flush();
#endif
#endif

	for(num_values = 0; *cmd; ++num_values)
	{
		while(*cmd == ' ') cmd++;

		if(num_values >= LTC1257_MAX_NUM_VALUES)
			return ECMD_ERR_PARSE_ERROR;

		if(sscanf_P(cmd, PSTR("%u"), value[num_values]) != 1)
			return ECMD_ERR_PARSE_ERROR;
	
		if(*value[num_values] > 4095)
			return ECMD_ERR_PARSE_ERROR;

		snprintf_P(tmp, sizeof(tmp), PSTR("%u"), *value[num_values]);
		tmplen = strlen(tmp);
		if(tmplen > 4)
			return ECMD_ERR_PARSE_ERROR;

		cmd += tmplen;

#ifdef DEBUG_LTC1257
#ifdef DEBUG
		debug_printf("LTC1257: ecmd parsed argument %u, value %u.\n", num_values, *value[num_values]);
#endif
#ifdef SYSLOG_SUPPORT
		syslog_sendf("LTC1257: ecmd parsed argument %u, value %u.\n", num_values, *value[num_values]);
		syslog_flush();
#endif
#endif
	}

	if(num_values == 0)
		return ECMD_ERR_PARSE_ERROR;

#ifdef DEBUG_LTC1257
#ifdef DEBUG
	debug_printf("LTC1257: ecmd parsed %u arguments.\n", num_values);
#endif
#ifdef SYSLOG_SUPPORT
	syslog_sendf("LTC1257: ecmd parsed %u arguments.\n", num_values);
	syslog_flush();
#endif
#endif

	ltc1257_set(value, num_values);

#ifdef DEBUG_LTC1257
#ifdef DEBUG
	debug_printf("LTC1257: parse_cmd_ltc1257 end.\n");
#endif
#ifdef SYSLOG_SUPPORT
	syslog_sendf("LTC1257: parse_cmd_ltc1257 end.\n");
	syslog_flush();
#endif
#endif

	return ECMD_FINAL_OK;
}

/*
	-- Ethersex META --
	block(Digital/Analog Conversion ([[DAC]]))
	ecmd_feature(ltc1257_set, "ltc1257_set ", `[VALUE0] [VALUE1] ..., Set output to value (value: 0-4095)')
	ecmd_feature(ltc1257_delay, "ltc1257_delay", `[VALUE], Set (if VALUE given) or get (no VALUE) delay for LTC1257 output bit changes in µs')
	ecmd_feature(ltc1257_init, "ltc1257_init", `Initialize LTC1257 output pins')
*/
