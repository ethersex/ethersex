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

#include <avr/pgmspace.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "protocols/ecmd/ecmd-base.h"
#include "protocols/syslog/syslog.h"

#include "ltc1257.h"

int16_t parse_cmd_ltc1257_delay(char *cmd, char *output, uint16_t len)
{
	uint16_t delay;
	uint8_t slen = 0;

	LTC1257_ECMD_DEBUG("parse_cmd_ltc1257_delay begin.\n");

	while(*cmd == ' ') cmd++;

	if(sscanf_P(cmd, PSTR("%u"), &delay) == 1)
	{
		/* set delay */

		/* disable interrupts */
		uint8_t sreg = SREG;
		cli();

		ltc1257_delay_set(&delay);

		/* re-enable interrupts */
		SREG = sreg;
	}
	else
	{
		/* return delay */

		/* disable interrupts */
		uint8_t sreg = SREG;
		cli();

		ltc1257_delay_get(&delay);

		/* re-enable interrupts */
		SREG = sreg;

		slen = sprintf_P(output, PSTR("%u"), delay);
	}
	
	LTC1257_ECMD_DEBUG("parse_cmd_ltc1257_delay end.\n");

	if(slen)
		return ECMD_FINAL(slen);
	else
		return ECMD_FINAL_OK;
}

int16_t parse_cmd_ltc1257_init(char *cmd, char *output, uint16_t len)
{
	LTC1257_ECMD_DEBUG("parse_cmd_ltc1257_init begin.\n");

	while(*cmd == ' ') cmd++;

	if(*cmd != '\0')
		return ECMD_ERR_PARSE_ERROR;

	/* disable interrupts */
	uint8_t sreg = SREG;
	cli();

	ltc1257_init();

	/* re-enable interrupts */
	SREG = sreg;

	LTC1257_ECMD_DEBUG("parse_cmd_ltc1257_init end.\n");

	return ECMD_FINAL_OK;
}

int16_t parse_cmd_ltc1257_set(char *cmd, char *output, uint16_t len)
{

	uint16_t *value[LTC1257_MAX_NUM_VALUES];
	uint8_t num_values;
	char tmp[6];
	uint8_t tmplen;

	LTC1257_ECMD_DEBUG("parse_cmd_ltc1257 begin.\n");

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

		LTC1257_ECMD_DEBUG("ecmd parsed argument %u, value %u.\n", num_values, *value[num_values]);
	}

	if(num_values == 0)
		return ECMD_ERR_PARSE_ERROR;

	LTC1257_ECMD_DEBUG("ecmd parsed %u arguments.\n", num_values);

	/* disable interrupts */
	uint8_t sreg = SREG;
	cli();

	ltc1257_set(value, num_values);

	/* re-enable interrupts */
	SREG = sreg;

	LTC1257_ECMD_DEBUG("parse_cmd_ltc1257 end.\n");

	return ECMD_FINAL_OK;
}

/*
	-- Ethersex META --
	block(Digital/Analog Conversion ([[DAC]]))
	ecmd_feature(ltc1257_set, "ltc1257_set ", `[VALUE0] [VALUE1] ..., Set output to value (value: 0-4095)')
	ecmd_feature(ltc1257_delay, "ltc1257_delay", `[VALUE], Set (if VALUE given) or get (no VALUE) delay for LTC1257 output bit changes in µs')
	ecmd_feature(ltc1257_init, "ltc1257_init", `Initialize LTC1257 output pins')
*/
