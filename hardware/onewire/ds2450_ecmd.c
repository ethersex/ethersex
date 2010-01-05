/*
 * Support for ADC DS2450
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
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "core/debug.h"
#include "protocols/ecmd/ecmd-base.h"
#include "protocols/syslog/syslog.h"

#include "ds2450.h"
#include "onewire.h"


/* from ecmd.c: */
int8_t parse_ow_rom(char *cmd, struct ow_rom_code_t *rom);


/* parse the command line and check for a rom code */
int8_t noinline ow_ecmd_parse_rom_arg(char **cmd, struct ow_rom_code_t **ptr_rom)
{
	int8_t ret;

	while(**cmd == ' ')
		(*cmd)++;

	if(strlen(*cmd) >= 16)
	{
		/* called with rom code */
#ifdef DEBUG_OW_DS2450_ECMD
		debug_printf("DS2450: ow_ecmd_parse_rom_arg: called with rom code.\n");
#endif
		*ptr_rom = malloc(sizeof(struct ow_rom_code_t));

#ifndef TEENSY_SUPPORT
		/* check if malloc did fine */
		if(!*ptr_rom)
		{
#ifdef DEBUG_OW_DS2450_ECMD
			debug_printf("DS2450: ow_ecmd_parse_rom_arg: malloc did not return memory pointer!\n");
#endif
			return ECMD_ERR_READ_ERROR;
		}
#endif

		ret = parse_ow_rom(*cmd, *ptr_rom);

		/* check for parse error */
		if(ret < 0)
		{
#ifdef DEBUG_OW_DS2450_ECMD
			debug_printf("DS2450: ow_ecmd_parse_rom_arg: parser error (parse_ow_rom ret: %i)!\n", ret);
#endif
			return ECMD_ERR_PARSE_ERROR;
		}

		/* move pointer behind rom code */
		*cmd += 16;

#ifdef DEBUG_OW_DS2450_ECMD
		uint8_t *addr = (*ptr_rom)->bytewise;
		debug_printf("DS2450: ow_ecmd_parse_rom_arg: parsed rom code: %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x.\n",
			addr[0], addr[1], addr[2], addr[3],
			addr[4], addr[5], addr[6], addr[7]);
#endif
	}
	else
	{
		/* no rom code, use skip command */
#ifdef DEBUG_OW_DS2450_ECMD
		debug_printf("DS2450: ow_ecmd_parse_rom_arg: called without rom code.\n");
#endif
		*ptr_rom = NULL;
	}

	while(**cmd == ' ')
		(*cmd)++;

	return 0;
}

/* convert channel character to decimal */
int8_t noinline ow_ds2450_channel_char(char c)
{
#ifdef DEBUG_OW_DS2450_ECMD
	debug_printf("DS2450: ow_ds2450_channel_char: called with character: %c.\n", c);
#endif
	if(c >= 65 && c <= 68)
	{
		/* channel ASCII value A to D (uppercase) */
		return c - 'A';
	}
	else if(c >= 97 && c <= 100)
	{
		/* channel ASCII value a to d (lowercase) */
		return c - 'a';
	}

#ifdef DEBUG_OW_DS2450_ECMD
	debug_printf("DS2450: ow_ds2450_channel_char: channel character invalid!\n", c);
#endif

	return -1;
}


int16_t parse_cmd_onewire_ds2450_power(char *cmd, char *output, uint16_t len)
{
	int8_t ret;
	uint8_t ecmd_return_len = 0;
	uint8_t power;
	struct ow_rom_code_t *ptr_rom;

	ret = ow_ecmd_parse_rom_arg(&cmd, &ptr_rom);
	if(ret != 0)
		return ret;

	/* next byte may be the power value, if so: set power value, if not: read and print power value */
	if(*cmd == '\0')
	{
		/* disable interrupts */
		uint8_t sreg = SREG;
		cli();

		ret = ow_ds2450_power_get(ptr_rom);

		/* re-enable interrupts */
		SREG = sreg;

#ifdef DEBUG_OW_DS2450_ECMD
		debug_printf("DS2450: parse_cmd_onewire_ds2450_power: ow_ds2450_power_get ret: %i.\n", ret);
#endif

		if(ret < 0)
			return ECMD_ERR_READ_ERROR;

		ecmd_return_len = snprintf_P(output, len, PSTR("POWER: %i"), ret);
	}
	else
	{
#ifdef DEBUG_OW_DS2450_ECMD
		uint8_t sret = sscanf_P(cmd, PSTR("%1hx"), &power);
		debug_printf("DS2450: parse_cmd_onewire_ds2450_power: sscanf_P ret: %i, power: %u.\n", sret, power);
		if(sret != 1)
#else
		if(sscanf_P(cmd, PSTR("%1hx"), &power) != 1)
#endif
			return ECMD_ERR_PARSE_ERROR;

		if(!(power == 0 || power == 1))
			return ECMD_ERR_PARSE_ERROR;

#ifndef TEENSY_SUPPORT
		/* argument should have been 1-digit, so move cmd pointer one position forward */
		++cmd;

		/* no more characters should be waiting in cmd */
		if(strlen(cmd) > 0)
		{
#ifdef DEBUG_OW_DS2450_ECMD
			debug_printf("DS2450: parse_cmd_onewire_ds2450_power: cmd still contains data after parsing all argumentes!\n");
#endif
			return ECMD_ERR_PARSE_ERROR;
		}
#endif

		/* disable interrupts */
		uint8_t sreg = SREG;
		cli();

		ret = ow_ds2450_power_set(ptr_rom, power);

		/* re-enable interrupts */
		SREG = sreg;

#ifdef DEBUG_OW_DS2450_ECMD
		debug_printf("DS2450: parse_cmd_onewire_ds2450_power: ow_ds2450_power_set ret: %i\n", ret);
#endif

		if(ret != 0)
			return ECMD_ERR_READ_ERROR;
	}

	if(ecmd_return_len)
		return ECMD_FINAL(ecmd_return_len);
	else
		return ECMD_FINAL_OK;
}

int16_t parse_cmd_onewire_ds2450_res(char *cmd, char *output, uint16_t len)
{
	int8_t ret;
	int8_t channel_requested;
	uint8_t ecmd_return_len = 0;
	uint8_t res;
	struct ow_rom_code_t *ptr_rom;

	ret = ow_ecmd_parse_rom_arg(&cmd, &ptr_rom);
	if(ret != 0)
		return ret;

	if(*cmd == '\0')
	{
		/* no more data to read from cmd */
#ifdef DEBUG_OW_DS2450_ECMD
		debug_printf("DS2450: parse_cmd_onewire_ds2450_res: not enogh arguments, expecting at least a channel name!\n");
#endif
		return ECMD_ERR_PARSE_ERROR;
	}
	else
	{
		/* next byte should be a channel character */
		channel_requested = ow_ds2450_channel_char(*cmd);
		if(channel_requested < 0)
			return ECMD_ERR_PARSE_ERROR;

		/* channel char must have been 1-digit, so move cmd pointer one position forward */
		++cmd;

		while(*cmd == ' ')
			++cmd;

		/* next byte may be the res value, if so: set res value, if not: read and print res value */
		if(*cmd == '\0')
		{
			/* disable interrupts */
			uint8_t sreg = SREG;
			cli();

			ret = ow_ds2450_res_get(ptr_rom, channel_requested);

			/* re-enable interrupts */
			SREG = sreg;

#ifdef DEBUG_OW_DS2450_ECMD
			debug_printf("DS2450: parse_cmd_onewire_ds2450_res: ow_ds2450_res_get ret: %i.\n", ret);
#endif

			if(ret < 0)
				return ECMD_ERR_READ_ERROR;

			ecmd_return_len = snprintf_P(output, len, PSTR("ch %c RES: %02x"), (unsigned char) channel_requested+65, ret);
		}
		else
		{
#ifdef DEBUG_OW_DS2450_ECMD
			uint8_t sret = sscanf_P(cmd, PSTR("%2hhx"), &res);
			debug_printf("DS2450: parse_cmd_onewire_ds2450_res: sscanf_P ret: %i, res: %02x.\n", sret, res);
			if(sret != 1)
#else
			if(sscanf_P(cmd, PSTR("%2hhx"), &res) != 1)
#endif
				return ECMD_ERR_PARSE_ERROR;

			/* valid values are 0x1 (1 bit resultion) up to 0xf (15 bit resultion) */
			/* for 16 bit resultion the four least significant bits must set to zero (0x0), we will also accept 0x10 as decimal representation of 16 */
			if(res == 0x10)
				res = 0x00;
			else if(res > OW_DS2450_RC_MASK)
				return ECMD_ERR_PARSE_ERROR;

#ifndef TEENSY_SUPPORT
			/* argument should have been 2-digit, so move cmd pointer two positions forward */
			cmd += 2;

			/* no more characters should be waiting in cmd */
			if(strlen(cmd) > 0)
			{
#ifdef DEBUG_OW_DS2450_ECMD
				debug_printf("DS2450: parse_cmd_onewire_ds2450_res: cmd still contains data after parsing all argumentes!\n");
#endif
				return ECMD_ERR_PARSE_ERROR;
			}
#endif

			/* disable interrupts */
			uint8_t sreg = SREG;
			cli();

			ret = ow_ds2450_res_set(ptr_rom, channel_requested, res);

			/* re-enable interrupts */
			SREG = sreg;

#ifdef DEBUG_OW_DS2450_ECMD
			debug_printf("DS2450: parse_cmd_onewire_ds2450_res: ow_ds2450_res_set ret: %i\n", ret);
#endif

			if(ret != 0)
				return ECMD_ERR_READ_ERROR;
		}
	}

	if(ecmd_return_len)
		return ECMD_FINAL(ecmd_return_len);
	else
		return ECMD_FINAL_OK;
}

int16_t parse_cmd_onewire_ds2450_oc(char *cmd, char *output, uint16_t len)
{
	int8_t ret;
	int8_t channel_requested;
	uint8_t ecmd_return_len = 0;
	uint8_t oc;
	struct ow_rom_code_t *ptr_rom;

	ret = ow_ecmd_parse_rom_arg(&cmd, &ptr_rom);
	if(ret != 0)
		return ret;

	if(*cmd == '\0')
	{
		/* no more data to read from cmd */
#ifdef DEBUG_OW_DS2450_ECMD
		debug_printf("DS2450: parse_cmd_onewire_ds2450_oc: not enogh arguments, expecting at least a channel name!\n");
#endif
		return ECMD_ERR_PARSE_ERROR;
	}
	else
	{
		/* next byte should be a channel character */
		channel_requested = ow_ds2450_channel_char(*cmd);
		if(channel_requested < 0)
			return ECMD_ERR_PARSE_ERROR;

		/* channel char must have been 1-digit, so move cmd pointer one position forward */
		++cmd;

		while(*cmd == ' ')
			++cmd;

		/* next byte may be the OC value, if so: set OC value, if not: read and print OC value */
		if(*cmd == '\0')
		{
			/* disable interrupts */
			uint8_t sreg = SREG;
			cli();

			ret = ow_ds2450_oc_get(ptr_rom, channel_requested);

			/* re-enable interrupts */
			SREG = sreg;

#ifdef DEBUG_OW_DS2450_ECMD
			debug_printf("DS2450: parse_cmd_onewire_ds2450_oc: ow_ds2450_oc_get ret: %i.\n", ret);
#endif

			if(ret < 0)
				return ECMD_ERR_READ_ERROR;

			ecmd_return_len = snprintf_P(output, len, PSTR("ch %c OC: %i"), (unsigned char) channel_requested+65, ret);
		}
		else
		{
#ifdef DEBUG_OW_DS2450_ECMD
			uint8_t sret = sscanf_P(cmd, PSTR("%1hx"), &oc);
			debug_printf("DS2450: parse_cmd_onewire_ds2450_oc: sscanf_P ret: %i, OC: %u.\n", sret, oc);
			if(sret != 1)
#else
			if(sscanf_P(cmd, PSTR("%1hx"), &oc) != 1)
#endif
				return ECMD_ERR_PARSE_ERROR;

			if(!(oc == 0 || oc == 1))
				return ECMD_ERR_PARSE_ERROR;

#ifndef TEENSY_SUPPORT
			/* argument should have been 1-digit, so move cmd pointer one position forward */
			++cmd;

			/* no more characters should be waiting in cmd */
			if(strlen(cmd) > 0)
			{
#ifdef DEBUG_OW_DS2450_ECMD
				debug_printf("DS2450: parse_cmd_onewire_ds2450_oc: cmd still contains data after parsing all argumentes!\n");
#endif
				return ECMD_ERR_PARSE_ERROR;
			}
#endif

			/* disable interrupts */
			uint8_t sreg = SREG;
			cli();

			ret = ow_ds2450_oc_set(ptr_rom, channel_requested, oc);

			/* re-enable interrupts */
			SREG = sreg;

#ifdef DEBUG_OW_DS2450_ECMD
			debug_printf("DS2450: parse_cmd_onewire_ds2450_oc: ow_ds2450_oc_set ret: %i\n", ret);
#endif

			if(ret != 0)
				return ECMD_ERR_READ_ERROR;
		}
	}

	if(ecmd_return_len)
		return ECMD_FINAL(ecmd_return_len);
	else
		return ECMD_FINAL_OK;
}

int16_t parse_cmd_onewire_ds2450_oe(char *cmd, char *output, uint16_t len)
{
	int8_t ret;
	int8_t channel_requested;
	uint8_t ecmd_return_len = 0;
	uint8_t oe;
	struct ow_rom_code_t *ptr_rom;

	ret = ow_ecmd_parse_rom_arg(&cmd, &ptr_rom);
	if(ret != 0)
		return ret;

	if(*cmd == '\0')
	{
		/* no more data to read from cmd */
#ifdef DEBUG_OW_DS2450_ECMD
		debug_printf("DS2450: parse_cmd_onewire_ds2450_oe: not enogh arguments, expecting at least a channel name!\n");
#endif
		return ECMD_ERR_PARSE_ERROR;
	}
	else
	{
		/* next byte should be a channel character */
		channel_requested = ow_ds2450_channel_char(*cmd);
		if(channel_requested < 0)
			return ECMD_ERR_PARSE_ERROR;

		/* channel char must have been 1-digit, so move cmd pointer one position forward */
		++cmd;

		while(*cmd == ' ')
			++cmd;

		/* next byte may be the oe value, if so: set oe value, if not: read and print oe value */
		if(*cmd == '\0')
		{
			/* disable interrupts */
			uint8_t sreg = SREG;
			cli();

			ret = ow_ds2450_oe_get(ptr_rom, channel_requested);

			/* re-enable interrupts */
			SREG = sreg;

#ifdef DEBUG_OW_DS2450_ECMD
			debug_printf("DS2450: parse_cmd_onewire_ds2450_oe: ow_ds2450_oe_get ret: %i.\n", ret);
#endif

			if(ret < 0)
				return ECMD_ERR_READ_ERROR;

			ecmd_return_len = snprintf_P(output, len, PSTR("ch %c OE: %i"), (unsigned char) channel_requested+65, ret);
		}
		else
		{
#ifdef DEBUG_OW_DS2450_ECMD
			uint8_t sret = sscanf_P(cmd, PSTR("%1hx"), &oe);
			debug_printf("DS2450: parse_cmd_onewire_ds2450_oe: sscanf_P ret: %i, oe: %u.\n", sret, oe);
			if(sret != 1)
#else
			if(sscanf_P(cmd, PSTR("%1hx"), &oe) != 1)
#endif
				return ECMD_ERR_PARSE_ERROR;

			if(!(oe == 0 || oe == 1))
				return ECMD_ERR_PARSE_ERROR;

#ifndef TEENSY_SUPPORT
			/* argument should have been 1-digit, so move cmd pointer one position forward */
			++cmd;

			/* no more characters should be waiting in cmd */
			if(strlen(cmd) > 0)
			{
#ifdef DEBUG_OW_DS2450_ECMD
				debug_printf("DS2450: parse_cmd_onewire_ds2450_oe: cmd still contains data after parsing all argumentes!\n");
#endif
				return ECMD_ERR_PARSE_ERROR;
			}
#endif

			/* disable interrupts */
			uint8_t sreg = SREG;
			cli();

			ret = ow_ds2450_oe_set(ptr_rom, channel_requested, oe);

			/* re-enable interrupts */
			SREG = sreg;

#ifdef DEBUG_OW_DS2450_ECMD
			debug_printf("DS2450: parse_cmd_onewire_ds2450_oe: ow_ds2450_oe_set ret: %i\n", ret);
#endif

			if(ret != 0)
				return ECMD_ERR_READ_ERROR;
		}
	}

	if(ecmd_return_len)
		return ECMD_FINAL(ecmd_return_len);
	else
		return ECMD_FINAL_OK;
}

int16_t parse_cmd_onewire_ds2450_range(char *cmd, char *output, uint16_t len)
{
	int8_t ret;
	int8_t channel_requested;
	uint8_t ecmd_return_len = 0;
	uint8_t range;
	struct ow_rom_code_t *ptr_rom;

	ret = ow_ecmd_parse_rom_arg(&cmd, &ptr_rom);
	if(ret != 0)
		return ret;

	if(*cmd == '\0')
	{
		/* no more data to read from cmd */
#ifdef DEBUG_OW_DS2450_ECMD
		debug_printf("DS2450: parse_cmd_onewire_ds2450_range: not enogh arguments, expecting at least a channel name!\n");
#endif
		return ECMD_ERR_PARSE_ERROR;
	}
	else
	{
		/* next byte should be a channel character */
		channel_requested = ow_ds2450_channel_char(*cmd);
		if(channel_requested < 0)
			return ECMD_ERR_PARSE_ERROR;

		/* channel char must have been 1-digit, so move cmd pointer one position forward */
		++cmd;

		while(*cmd == ' ')
			++cmd;

		/* next byte may be the range value, if so: set range value, if not: read and print range value */
		if(*cmd == '\0')
		{
			/* disable interrupts */
			uint8_t sreg = SREG;
			cli();

			ret = ow_ds2450_range_get(ptr_rom, channel_requested);

			/* re-enable interrupts */
			SREG = sreg;

#ifdef DEBUG_OW_DS2450_ECMD
			debug_printf("DS2450: parse_cmd_onewire_ds2450_range: ow_ds2450_range_get ret: %i.\n", ret);
#endif

			if(ret < 0)
				return ECMD_ERR_READ_ERROR;

			ecmd_return_len = snprintf_P(output, len, PSTR("ch %c RANGE: %i"), (unsigned char) channel_requested+65, ret);
		}
		else
		{
#ifdef DEBUG_OW_DS2450_ECMD
			uint8_t sret = sscanf_P(cmd, PSTR("%1hx"), &range);
			debug_printf("DS2450: parse_cmd_onewire_ds2450_range: sscanf_P ret: %i, range: %u.\n", sret, range);
			if(sret != 1)
#else
			if(sscanf_P(cmd, PSTR("%1hx"), &range) != 1)
#endif
				return ECMD_ERR_PARSE_ERROR;

			if(!(range == 0 || range == 1))
				return ECMD_ERR_PARSE_ERROR;

#ifndef TEENSY_SUPPORT
			/* argument should have been 1-digit, so move cmd pointer one position forward */
			++cmd;

			/* no more characters should be waiting in cmd */
			if(strlen(cmd) > 0)
			{
#ifdef DEBUG_OW_DS2450_ECMD
				debug_printf("DS2450: parse_cmd_onewire_ds2450_range: cmd still contains data after parsing all argumentes!\n");
#endif
				return ECMD_ERR_PARSE_ERROR;
			}
#endif

			/* disable interrupts */
			uint8_t sreg = SREG;
			cli();

			ret = ow_ds2450_range_set(ptr_rom, channel_requested, range);

			/* re-enable interrupts */
			SREG = sreg;

#ifdef DEBUG_OW_DS2450_ECMD
			debug_printf("DS2450: parse_cmd_onewire_ds2450_range: ow_ds2450_range_set ret: %i\n", ret);
#endif

			if(ret != 0)
				return ECMD_ERR_READ_ERROR;
		}
	}

	if(ecmd_return_len)
		return ECMD_FINAL(ecmd_return_len);
	else
		return ECMD_FINAL_OK;
}

int16_t parse_cmd_onewire_ds2450_por(char *cmd, char *output, uint16_t len)
{
	int8_t ret;
	int8_t channel_requested;
	uint8_t ecmd_return_len = 0;
	uint8_t por;
	struct ow_rom_code_t *ptr_rom;

	ret = ow_ecmd_parse_rom_arg(&cmd, &ptr_rom);
	if(ret != 0)
		return ret;

	if(*cmd == '\0')
	{
		/* no more data to read from cmd */
#ifdef DEBUG_OW_DS2450_ECMD
		debug_printf("DS2450: parse_cmd_onewire_ds2450_por: not enogh arguments, expecting at least a channel name!\n");
#endif
		return ECMD_ERR_PARSE_ERROR;
	}
	else
	{
		/* next byte should be a channel character */
		channel_requested = ow_ds2450_channel_char(*cmd);
		if(channel_requested < 0)
			return ECMD_ERR_PARSE_ERROR;

		/* channel char must have been 1-digit, so move cmd pointer one position forward */
		++cmd;

		while(*cmd == ' ')
			++cmd;

		/* next byte may be the por value, if so: set por value, if not: read and print por value */
		if(*cmd == '\0')
		{
			/* disable interrupts */
			uint8_t sreg = SREG;
			cli();

			ret = ow_ds2450_por_get(ptr_rom, channel_requested);

			/* re-enable interrupts */
			SREG = sreg;

#ifdef DEBUG_OW_DS2450_ECMD
			debug_printf("DS2450: parse_cmd_onewire_ds2450_por: ow_ds2450_por_get ret: %i.\n", ret);
#endif

			if(ret < 0)
				return ECMD_ERR_READ_ERROR;

			ecmd_return_len = snprintf_P(output, len, PSTR("ch %c POR: %i"), (unsigned char) channel_requested+65, ret);
		}
		else
		{
#ifdef DEBUG_OW_DS2450_ECMD
			uint8_t sret = sscanf_P(cmd, PSTR("%1hx"), &por);
			debug_printf("DS2450: parse_cmd_onewire_ds2450_por: sscanf_P ret: %i, por: %u.\n", sret, por);
			if(sret != 1)
#else
			if(sscanf_P(cmd, PSTR("%1hx"), &por) != 1)
#endif
				return ECMD_ERR_PARSE_ERROR;

			if(!(por == 0 || por == 1))
				return ECMD_ERR_PARSE_ERROR;

#ifndef TEENSY_SUPPORT
			/* argument should have been 1-digit, so move cmd pointer one position forward */
			++cmd;

			/* no more characters should be waiting in cmd */
			if(strlen(cmd) > 0)
			{
#ifdef DEBUG_OW_DS2450_ECMD
				debug_printf("DS2450: parse_cmd_onewire_ds2450_por: cmd still contains data after parsing all argumentes!\n");
#endif
				return ECMD_ERR_PARSE_ERROR;
			}
#endif

			/* disable interrupts */
			uint8_t sreg = SREG;
			cli();

			ret = ow_ds2450_por_set(ptr_rom, channel_requested, por);

			/* re-enable interrupts */
			SREG = sreg;

#ifdef DEBUG_OW_DS2450_ECMD
			debug_printf("DS2450: parse_cmd_onewire_ds2450_por: ow_ds2450_por_set ret: %i\n", ret);
#endif

			if(ret != 0)
				return ECMD_ERR_READ_ERROR;
		}
	}

	if(ecmd_return_len)
		return ECMD_FINAL(ecmd_return_len);
	else
		return ECMD_FINAL_OK;
}


int16_t parse_cmd_onewire_ds2450_convert(char *cmd, char *output, uint16_t len)
{
	int8_t ret;
	uint8_t input_select, readout;
	struct ow_rom_code_t *ptr_rom;

	ret = ow_ecmd_parse_rom_arg(&cmd, &ptr_rom);
	if(ret != 0)
		return ret;

	/* maybe two byte following: input select mask and read-out control */
	/* default values: 0000 1111 for input_select (convert all channels) */
	input_select = 0x0f;

	/* default values: 0000 0000 for readout (no preset, leave as is) */
	readout = 0x00;

	if(*cmd == '\0')
	{
		/* no more data to read from cmd */
#ifdef DEBUG_OW_DS2450_ECMD
		debug_printf("DS2450: parse_cmd_onewire_ds2450_convert: no input select mask and read-out control given, using default values.\n");
#endif
	}
	else
	{
		uint8_t ret = sscanf_P(cmd, PSTR("%2hhx %2hhx"), &input_select, &readout);
#ifdef DEBUG_OW_DS2450_ECMD
		debug_printf("DS2450: parse_cmd_onewire_ds2450_convert: sscanf_P ret: %i, input select mask: %02x, read-out control: %02x.\n", ret, input_select, readout);
#endif
		if(ret != 2)
			return ECMD_ERR_PARSE_ERROR;

		if(input_select == 0x00 || input_select > 0x0f)
			return ECMD_ERR_PARSE_ERROR;

		/* check for invalid read out combinations */
		uint8_t readout_check = readout;
		for(uint8_t i = 0; i < 4; ++i)
		{
			if((readout_check & 0x03) > 2)
				return ECMD_ERR_PARSE_ERROR;
			readout_check >>= 2;
		}

#ifndef TEENSY_SUPPORT
		/* argument should have been 2*2-digit plus space, so move cmd pointer 5 positions forward */
		cmd += 5;

		/* no more characters should be waiting in cmd */
		if(strlen(cmd) > 0)
		{
#ifdef DEBUG_OW_DS2450_ECMD
			debug_printf("DS2450: parse_cmd_onewire_ds2450_convert: cmd still contains data after parsing all argumentes!\n");
#endif
			return ECMD_ERR_PARSE_ERROR;
		}
#endif
	}

	/* disable interrupts */
	uint8_t sreg = SREG;
	cli();

	ret = ow_ds2450_convert(ptr_rom, input_select, readout);

	/* re-enable interrupts */
	SREG = sreg;

#ifdef DEBUG_OW_DS2450_ECMD
	debug_printf("DS2450: parse_cmd_onewire_ds2450_convert: ow_ds2450_convert ret: %i\n", ret);
#endif

	if(ret != 0)
		return ECMD_ERR_READ_ERROR;

	return ECMD_FINAL_OK;
}

int16_t parse_cmd_onewire_ds2450_get(char *cmd, char *output, uint16_t len, uint8_t raw)
{
	int8_t ret;
	int8_t channel_requested;
	uint8_t ecmd_return_len;
	struct ow_rom_code_t *ptr_rom;

	ret = ow_ecmd_parse_rom_arg(&cmd, &ptr_rom);
	if(ret != 0)
		return ret;

	if(*cmd == '\0')
	{
		/* no more data to read from cmd */
#ifdef DEBUG_OW_DS2450_ECMD
		debug_printf("DS2450: parse_cmd_onewire_ds2450_get: no channel given.\n");
#endif
		channel_requested = -1;
	}
	else
	{
		/* next byte should be a channel character */
		channel_requested = ow_ds2450_channel_char(*cmd);
		if(channel_requested < 0)
			return ECMD_ERR_PARSE_ERROR;
#ifndef TEENSY_SUPPORT
		/* argument should have been 1-digit, so move cmd pointer one position forward */
		++cmd;

		/* no more characters should be waiting in cmd */
		if(strlen(cmd) > 0)
		{
#ifdef DEBUG_OW_DS2450_ECMD
			debug_printf("DS2450: parse_cmd_onewire_ds2450_get: cmd still contains data after parsing all argumentes!\n");
#endif
			return ECMD_ERR_PARSE_ERROR;
		}
#endif
	}

	/* disable interrupts */
	uint8_t sreg = SREG;
	cli();

	if(channel_requested >= 0)
	{
		/* request just one channel */
		uint16_t val;

		ret = ow_ds2450_get(ptr_rom, channel_requested, channel_requested, &val);

		/* re-enable interrupts */
		SREG = sreg;

#ifdef DEBUG_OW_DS2450_ECMD
		debug_printf("DS2450: parse_cmd_onewire_ds2450_get: ow_ds2450_get ret: %i.\n", ret);
#endif

		if(ret <= 0)
			return ECMD_ERR_READ_ERROR;

#ifdef DEBUG_OW_DS2450_ECMD
		debug_printf("DS2450: parse_cmd_onewire_ds2450_get: ow_ds2450_get channel: %c, val: %i.\n", (unsigned char) channel_requested+65, val);
#endif

		ecmd_return_len = snprintf_P(output, len, PSTR("ch %c: %05u"), (unsigned char) channel_requested+65, val);
	}
	else
	{
		/* request all channels */
		uint16_t val[4];

		ret = ow_ds2450_get(ptr_rom, 0, 3, val);

		/* re-enable interrupts */
		SREG = sreg;

#ifdef DEBUG_OW_DS2450_ECMD
		debug_printf("DS2450: parse_cmd_onewire_ds2450_get: ow_ds2450_get ret: %i.\n", ret);
#endif

		if(ret <= 0)
			return ECMD_ERR_READ_ERROR;

		ecmd_return_len = 0;
		for(uint8_t i = 0; i < 4; ++i)
		{
#ifdef DEBUG_OW_DS2450_ECMD
			debug_printf("DS2450: parse_cmd_onewire_ds2450_get: ow_ds2450 channel: %c, val: %u.\n", (unsigned char) i+65, val[i]);
#endif
			ret = snprintf_P(output, len, PSTR("ch %c: %05u\n"), (unsigned char) i+65, val[i]);
			output += ret;
			len -= ret;
			ecmd_return_len += ret;
		}
		/* remove last newline character */
		--output;
		*output = '\0';
		--ecmd_return_len;
	}

	return ECMD_FINAL(ecmd_return_len);
}


/*
	-- Ethersex META --
	block([[Dallas_1-wire_Bus]])
	ecmd_feature(onewire_ds2450_power, "1w ds2450 power", [DEVICE], `get/set power supply of device (global)')
	ecmd_feature(onewire_ds2450_res, "1w ds2450 res", [DEVICE], `get/set bit resolution of AD convert (per channel)')
	ecmd_feature(onewire_ds2450_oc, "1w ds2450 oc", [DEVICE], `get/set output control (per channel)')
	ecmd_feature(onewire_ds2450_oe, "1w ds2450 oe", [DEVICE], `get/set output enable (per channel)')
	ecmd_feature(onewire_ds2450_range, "1w ds2450 range", [DEVICE], `get/set input voltage range (per channel)')
	ecmd_feature(onewire_ds2450_por, "1w ds2450 por", [DEVICE], `get/set power on reset (per channel)')
	ecmd_feature(onewire_ds2450_convert, "1w ds2450 convert", [DEVICE], `start conversion (with optional input mask and read out control)')
	ecmd_feature(onewire_ds2450_get, "1w ds2450 get", [DEVICE], `get conversion result (one or all channels)')
*/
