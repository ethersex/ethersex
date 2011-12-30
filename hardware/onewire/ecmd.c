/*
 *
 * Copyright (c) by Alexander Neumann <alexander@bumpern.de>
 * Copyright (c) 2007 by Stefan Siegl <stesie@brokenpipe.de>
 * Copyright (c) 2007 by Christian Dietrich <stettberger@dokucode.de>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License (either version 2 or
 * version 3) as published by the Free Software Foundation.
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
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>

#include "config.h"
#include "core/debug.h"
#include "core/eeprom.h"
#include "core/bit-macros.h"
#include "hardware/onewire/onewire.h"

#include "protocols/ecmd/ecmd-base.h"


/* parse an onewire rom address at cmd, write result to ptr */
int8_t parse_ow_rom(char *cmd, ow_rom_code_t *rom)
{
    uint8_t *addr = rom->bytewise;
    uint8_t end;

#ifdef DEBUG_ECMD_OW_ROM
    debug_printf("called parse_ow_rom with string '%s'\n", cmd);
#endif

    /* read 8 times 2 hex chars into a byte */
    int ret = sscanf_P(cmd, PSTR("%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%c"),
                       addr+0, addr+1, addr+2, addr+3,
                       addr+4, addr+5, addr+6, addr+7,
                       &end);

#ifdef DEBUG_ECMD_OW_ROM
    debug_printf("scanf returned %d\n", ret);
#endif

    if ((ret == 8) || ((ret == 9) && (end == ' '))) {
#ifdef DEBUG_ECMD_OW_ROM
        debug_printf("read rom %02x %02x %02x %02x %02x %02x %02x %02x\n",
                     addr[0], addr[1], addr[2], addr[3],
                     addr[4], addr[5], addr[6], addr[7]);
#endif
        return 0;
    }

    return -1;
}

#ifdef ONEWIRE_DETECT_SUPPORT
#ifdef ONEWIRE_POLLING_SUPPORT
int16_t parse_cmd_onewire_list(char *cmd, char *output, uint16_t len)
{
	#ifdef ONEWIRE_DS2502_SUPPORT
	int8_t list_type;
        while (*cmd == ' ')
            cmd++;
	switch (*cmd) {
		case 't':
			list_type = OW_LIST_TYPE_TEMP_SENSOR;
			break;
		case 'e':
			list_type = OW_LIST_TYPE_EEPROM;
			break;
		case '\0':
			list_type = OW_LIST_TYPE_ALL;
			break;
		default:
			return ECMD_ERR_PARSE_ERROR;
	}
	cmd++; /* for static bytes */
	#endif
	/* trick: use bytes on cmd as "connection specific static variables" */
	if (cmd[0] != 23)	/* indicator flag: real invocation:  0 */
	{
		cmd[0] = 23;	/* continuing call: 23 */
		cmd[1] = 0;	/* counter for sensors in list*/
	}
	uint8_t i = cmd[1];
	/* This is a special case: the while loop below printed a sensor which was last in the list,
	   so we still need to send an 'OK' after the sensor id */
	if(i>=OW_SENSORS_COUNT)
	{
		return ECMD_FINAL_OK;
	}
	int16_t ret = 0;
	do
	{
		if(ow_sensors[i].ow_rom_code.raw != 0)
		{
			#ifdef ONEWIRE_DS2502_SUPPORT
        		if ((list_type == OW_LIST_TYPE_ALL) || (list_type == OW_LIST_TYPE_TEMP_SENSOR && ow_temp_sensor(&ow_sensors[i].ow_rom_code)) || (list_type == OW_LIST_TYPE_EEPROM && ow_eeprom(&ow_sensors[i].ow_rom_code))) {
			#endif
				ret = snprintf_P(output, len,
				PSTR("%02x%02x%02x%02x%02x%02x%02x%02x"),
				ow_sensors[i].ow_rom_code.bytewise[0],
				ow_sensors[i].ow_rom_code.bytewise[1],
				ow_sensors[i].ow_rom_code.bytewise[2],
				ow_sensors[i].ow_rom_code.bytewise[3],
				ow_sensors[i].ow_rom_code.bytewise[4],
				ow_sensors[i].ow_rom_code.bytewise[5],
				ow_sensors[i].ow_rom_code.bytewise[6],
				ow_sensors[i].ow_rom_code.bytewise[7]
				);
			#ifdef ONEWIRE_DS2502_SUPPORT
			}
			#endif
		}
		i++;
	} while(ret == 0 && i<OW_SENSORS_COUNT);
	/* The while loop exited either because a sensor has been found or because there is no sensor left, let's check for that */
	if(ret == 0)
	{
		/* => i has reached OW_SENSORS_COUNT */
		return ECMD_FINAL_OK;
	}
	/* else, ret is != 0 which means a sensor has been found and this functions has to be called again
	   to prevent a buffer overflow 
	*/
	/* Save i to cmd[1] */

	cmd[1] = i;

	return	ECMD_AGAIN(ret);
}
#else
int16_t parse_cmd_onewire_list(char *cmd, char *output, uint16_t len)
{

    uint8_t firstonbus = 0;
    int16_t ret;

    if (ow_global.lock == 0) {
        firstonbus = 1;
#if ONEWIRE_BUSCOUNT > 1
        ow_global.bus = 0;
#endif
#ifdef DEBUG_ECMD_OW_LIST
        debug_printf("called onewire list for the first time\n");
#endif

#ifdef ONEWIRE_DS2502_SUPPORT
	/* parse optional parameters */
        while (*cmd == ' ')
            cmd++;
	switch (*cmd) {
		case 't':
			ow_global.list_type = OW_LIST_TYPE_TEMP_SENSOR;
			break;
		case 'e':
			ow_global.list_type = OW_LIST_TYPE_EEPROM;
			break;
		case '\0':
			ow_global.list_type = OW_LIST_TYPE_ALL;
			break;
		default:
			return ECMD_ERR_PARSE_ERROR;
	}
#endif
    } else {
#ifdef DEBUG_ECMD_OW_LIST
        debug_printf("called onewire list again\n");
#endif
        firstonbus = 0;
    }

#if defined ONEWIRE_DS2502_SUPPORT || ONEWIRE_BUSCOUNT > 1
list_next: ;
#endif

    /* disable interrupts */
    uint8_t sreg = SREG;
    cli();

#if ONEWIRE_BUSCOUNT > 1
    ret = ow_search_rom((uint8_t)(1 << (ow_global.bus + ONEWIRE_STARTPIN)), firstonbus);
#else
    ret = ow_search_rom(ONEWIRE_BUSMASK, firstonbus);
#endif

    /* re-enable interrupts */
    SREG = sreg;

    /* make sure only one conversion happens at a time */
    ow_global.lock = 1;

    if (ret == 1) {
#ifdef ONEWIRE_DS2502_SUPPORT
        if ((ow_global.list_type == OW_LIST_TYPE_ALL) ||
            ((ow_global.list_type == OW_LIST_TYPE_TEMP_SENSOR) &&
             (ow_temp_sensor(&ow_global.current_rom))) ||
            ((ow_global.list_type == OW_LIST_TYPE_EEPROM) &&
             (ow_eeprom(&ow_global.current_rom)))) {
           /* only print device rom address if it matches the selected list type */
#endif

#ifdef DEBUG_ECMD_OW_LIST
           debug_printf("discovered device "
#if ONEWIRE_BUSCOUNT > 1
                    "%02x %02x %02x %02x %02x %02x %02x %02x on bus %d\n",
#else
                    "%02x %02x %02x %02x %02x %02x %02x %02x\n",
#endif
                    ow_global.current_rom.bytewise[0],
                    ow_global.current_rom.bytewise[1],
                    ow_global.current_rom.bytewise[2],
                    ow_global.current_rom.bytewise[3],
                    ow_global.current_rom.bytewise[4],
                    ow_global.current_rom.bytewise[5],
                    ow_global.current_rom.bytewise[6],
                    ow_global.current_rom.bytewise[7]
#if ONEWIRE_BUSCOUNT > 1
                    ,ow_global.bus);
#else
		    );
#endif
#endif
           ret = snprintf_P(output, len,
                    PSTR("%02x%02x%02x%02x%02x%02x%02x%02x"),
                    ow_global.current_rom.bytewise[0],
                    ow_global.current_rom.bytewise[1],
                    ow_global.current_rom.bytewise[2],
                    ow_global.current_rom.bytewise[3],
                    ow_global.current_rom.bytewise[4],
                    ow_global.current_rom.bytewise[5],
                    ow_global.current_rom.bytewise[6],
                    ow_global.current_rom.bytewise[7]);

#ifdef DEBUG_ECMD_OW_LIST
            debug_printf("generated %d bytes\n", ret);
#endif

            /* set return value that the parser has to be called again */
            if (ret > 0)
                ret = ECMD_AGAIN(ret);

#ifdef DEBUG_ECMD_OW_LIST
            debug_printf("returning %d\n", ret);
#endif
            return ECMD_FINAL(ret);

#ifdef ONEWIRE_DS2502_SUPPORT
        } else {
            /* device did not match list type: try again */
            firstonbus = 0;
            goto list_next;
        }
#endif
    }

#if ONEWIRE_BUSCOUNT > 1
#ifdef DEBUG_ECMD_OW_LIST
    if (ret != 0) {
        debug_printf("no devices on bus %d\n", ow_global.bus);
    }
#endif
    if (ow_global.bus < ONEWIRE_BUSCOUNT - 1) {
        ow_global.bus++;
        firstonbus = 1;
        goto list_next;
    }
#endif
    ow_global.lock = 0;
    return ECMD_FINAL_OK;

}
#endif /* ONEWIRE_POLLING_SUPPORT*/
#endif /* ONEWIRE_DETECT_SUPPORT */
#ifdef ONEWIRE_POLLING_SUPPORT
int16_t parse_cmd_onewire_get(char *cmd, char *output, uint16_t len)
{
    ow_rom_code_t rom;
    int16_t ret;
    ret = parse_ow_rom(cmd, &rom);
    if (ret < 0)
        return ECMD_ERR_PARSE_ERROR;
    if (ow_temp_sensor(&rom)) {
	/*Search the sensor...*/
	        for(uint8_t i=0;i<OW_SENSORS_COUNT;i++)
                {
			if(ow_sensors[i].ow_rom_code.raw == rom.raw)
			{
				/*Found it*/
				int16_t temp=ow_sensors[i].temp;
				div_t res = div(temp,10);
				ret = snprintf_P(output, len, PSTR("%d.%1d"), res.quot,res.rem);
				return ECMD_FINAL(ret);
			}
		}
		/*Sensor is not in list*/
		ret = snprintf_P(output, len, PSTR("Sensor not in list!"));
	        return ECMD_FINAL(ret);
#ifdef ONEWIRE_DS2502_SUPPORT
    } else if (ow_eeprom(&rom)) {
        debug_printf("reading mac\n");

        /* disable interrupts */
        uint8_t sreg = SREG;
        cli();

        uint8_t mac[6];
        ret = ow_eeprom_read(&rom, mac);

        /* re-enable interrupts */
        SREG = sreg;

        if (ret != 0) {
            debug_printf("mac read failed: %d\n", ret);
            return ECMD_ERR_READ_ERROR;
        }

        debug_printf("successfully read mac\n");

        debug_printf("mac: %02x:%02x:%02x:%02x:%02x:%02x\n",
                mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

        ret = snprintf_P(output, len,
                PSTR("mac: %02x:%02x:%02x:%02x:%02x:%02x"),
                mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
#endif /* ONEWIRE_DS2502_SUPPORT */
    } else {
        debug_printf("unknown sensor type\n");
#ifdef TEENSY_SUPPORT
        strcpy_P (output, PSTR("unknown sensor type"));
        return ECMD_FINAL(strlen(output));
#else
        ret = snprintf_P(output, len, PSTR("unknown sensor type"));
#endif
    }

    return ECMD_FINAL(ret);

}
#else
int16_t parse_cmd_onewire_get(char *cmd, char *output, uint16_t len)
{
    ow_rom_code_t rom;
    int16_t ret;

    while (*cmd == ' ')
        cmd++;
    debug_printf("called onewire_get with: \"%s\"\n", cmd);

    ret = parse_ow_rom(cmd, &rom);

    /* check for parse error */
    if (ret < 0)
        return ECMD_ERR_PARSE_ERROR;

    if (ow_temp_sensor(&rom)) {
        debug_printf("reading temperature\n");

        /* disable interrupts */
        uint8_t sreg = SREG;
        cli();

        ow_temp_scratchpad_t sp;
        ret = ow_temp_read_scratchpad(&rom, &sp);

        /* re-enable interrupts */
        SREG = sreg;

        if (ret != 1) {
            debug_printf("scratchpad read failed: %d\n", ret);
            return ECMD_ERR_READ_ERROR;
        }

        debug_printf("successfully read scratchpad\n");

        int16_t temp = ow_temp_normalize(&rom, &sp);

        debug_printf("temperature: %d.%d\n", HI8(temp), LO8(temp) > 0 ? 5 : 0);

        int8_t sign = (int8_t)(temp < 0);

#ifdef TEENSY_SUPPORT
        if (sign) {
            temp = -temp;
            output[0] = '-';
        }
        /* Here sign is 0 or 1 */
        itoa (HI8(temp), output + sign, 10);
        char *ptr = output + strlen (output);

        *(ptr ++) = '.';
        itoa (HI8(((temp & 0x00ff) * 10) + 0x80), ptr, 10);
        return ECMD_FINAL(strlen(output));
#else
        if (sign) temp = -temp;
        ret = snprintf_P(output, len, PSTR("%s%d.%1d"),
                         sign?"-":"", (int8_t) HI8(temp), HI8(((temp & 0x00ff) * 10) + 0x80));
#endif

#ifdef ONEWIRE_DS2502_SUPPORT
    } else if (ow_eeprom(&rom)) {
        debug_printf("reading mac\n");

        /* disable interrupts */
        uint8_t sreg = SREG;
        cli();

        uint8_t mac[6];
        ret = ow_eeprom_read(&rom, mac);

        /* re-enable interrupts */
        SREG = sreg;

        if (ret != 0) {
            debug_printf("mac read failed: %d\n", ret);
            return ECMD_ERR_READ_ERROR;
        }

        debug_printf("successfully read mac\n");

        debug_printf("mac: %02x:%02x:%02x:%02x:%02x:%02x\n",
                mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

        ret = snprintf_P(output, len,
                PSTR("mac: %02x:%02x:%02x:%02x:%02x:%02x"),
                mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
#endif /* ONEWIRE_DS2502_SUPPORT */
    } else {
        debug_printf("unknown sensor type\n");
#ifdef TEENSY_SUPPORT
	strcpy_P (output, PSTR("unknown sensor type"));
	return ECMD_FINAL(strlen(output));
#else
        ret = snprintf_P(output, len, PSTR("unknown sensor type"));
#endif
    }

    return ECMD_FINAL(ret);
}
#endif
#ifdef ONEWIRE_POLLING_SUPPORT
int16_t parse_cmd_onewire_convert(char *cmd, char *output, uint16_t len)
{
        return ECMD_FINAL_OK;
}
#else
int16_t parse_cmd_onewire_convert(char *cmd, char *output, uint16_t len)
{
    int16_t ret;

    while (*cmd == ' ')
        cmd++;
    debug_printf("called onewire_convert with: \"%s\"\n", cmd);

    ow_rom_code_t rom, *romptr;

    ret = parse_ow_rom(cmd, &rom);

    /* check for romcode */
    romptr = (ret < 0) ? NULL : &rom;

    debug_printf("converting temperature...\n");

    /* disable interrupts */
    uint8_t sreg = SREG;
    cli();

    ret = ow_temp_start_convert_wait(romptr);

    SREG = sreg;

    if (ret == 1)
        /* done */
        return ECMD_FINAL_OK;
    else if (ret == -1)
        /* no device attached */
        return ECMD_ERR_READ_ERROR;
    else
        /* wrong rom family code */
        return ECMD_ERR_PARSE_ERROR;

}
#endif

/*
  -- Ethersex META --
  block([[Dallas_1-wire_Bus]])
  ecmd_ifdef(ONEWIRE_DETECT_SUPPORT)
    ecmd_feature(onewire_list, "1w list",,Return a list of the connected onewire devices)
  ecmd_endif()
  ecmd_feature(onewire_get, "1w get", DEVICE, Return temperature value of onewire DEVICE (provide 64-bit ID as 16-hex-digits))
  ecmd_feature(onewire_convert, "1w convert", [DEVICE], Trigger temperature conversion of either DEVICE or all connected devices)
*/
