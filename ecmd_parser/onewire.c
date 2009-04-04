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

#include "../config.h"
#include "../debug.h"
#include "../uip/uip.h"
#include "../uip/uip_arp.h"
#include "../eeprom.h"
#include "../bit-macros.h"
#include "../onewire/onewire.h"
#include "ecmd.h"

#ifdef ONEWIRE_SUPPORT
/* parse an onewire rom address at cmd, write result to ptr */
int8_t parse_ow_rom(char *cmd, uint8_t *ptr)
{

#ifdef DEBUG_ECMD_OW_ROM
    debug_printf("called parse_ow_rom with string '%s'\n", cmd);
#endif

    /* check if enough bytes have been given */
    if (strlen(cmd) < 16) {
#ifdef DEBUG_ECMD_OW_ROM
        debug_printf("incomplete command\n");
#endif
        return -1;
    }

    char b[3];

    for (uint8_t i = 0; i < 8; i++) {
        memcpy(b, cmd, 2);
        cmd += 2;
        b[2] = '\0';
        uint16_t val = 0;
        sscanf_P(b, PSTR("%x"), &val);
        *ptr++ = LO8(val);
    }

    return 1;
}

#ifdef ONEWIRE_DETECT_SUPPORT
int16_t parse_cmd_onewire_list(char *cmd, char *output, uint16_t len)
{
    int16_t ret;

    if (ow_global.lock == 0) {
        ow_global.lock = 1;
#ifdef DEBUG_ECMD_OW_LIST
        debug_printf("called onewire list for the first time\n");
#endif

        /* disable interrupts */
        uint8_t sreg = SREG;
        cli();

        ret = ow_search_rom_first();

        /* re-enable interrupts */
        SREG = sreg;

        if (ret <= 0) {
#ifdef DEBUG_ECMD_OW_LIST
            debug_printf("no devices on the bus\n");
#endif
            return 0;
        }
    } else {
#ifdef DEBUG_ECMD_OW_LIST
        debug_printf("called onewire list again\n");
#endif

        /* disable interrupts */
        uint8_t sreg = SREG;
        cli();

        ret = ow_search_rom_next();

        SREG = sreg;
    }

    if (ret == 1) {
#ifdef DEBUG_ECMD_OW_LIST
        debug_printf("discovered a device: "
                "%02x %02x %02x %02x %02x %02x %02x %02x\n",
                ow_global.current_rom.bytewise[0],
                ow_global.current_rom.bytewise[1],
                ow_global.current_rom.bytewise[2],
                ow_global.current_rom.bytewise[3],
                ow_global.current_rom.bytewise[4],
                ow_global.current_rom.bytewise[5],
                ow_global.current_rom.bytewise[6],
                ow_global.current_rom.bytewise[7]);
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
            ret = -ret - 10;

#ifdef DEBUG_ECMD_OW_LIST
        debug_printf("returning %d\n", ret);
#endif
        return ret;

    } else if (ret == 0) {
        ow_global.lock = 0;
        return 0;
    }

    return -1;
}
#endif /* ONEWIRE_DETECT_SUPPORT */

int16_t parse_cmd_onewire_get(char *cmd, char *output, uint16_t len)
{
    int16_t ret;

    cmd++;
    debug_printf("called onewire_list with: \"%s\"\n", cmd);

    struct ow_rom_code_t rom;

    ret = parse_ow_rom(cmd, (void *)&rom);

    /* check for parse error */
    if (ret < 0)
        return -1;

    if (ow_temp_sensor(&rom)) {
        debug_printf("reading temperature\n");

        /* disable interrupts */
        uint8_t sreg = SREG;
        cli();

        struct ow_temp_scratchpad_t sp;
        ret = ow_temp_read_scratchpad(&rom, &sp);

        /* re-enable interrupts */
        SREG = sreg;

        if (ret != 1) {
            debug_printf("scratchpad read failed: %d\n", ret);
            return -2;
        }

        debug_printf("successfully read scratchpad\n");

        uint16_t temp = ow_temp_normalize(&rom, &sp);

        debug_printf("temperature: %d.%d\n", HI8(temp), LO8(temp) > 0 ? 5 : 0);

#ifdef TEENSY_SUPPORT
	strcpy_P (output, PSTR ("Temperatur: "));
	char *ptr = output + 12;

	itoa (HI8(temp), ptr, 10);
	ptr += strlen (ptr);

	*(ptr ++) = '.';
	itoa (HI8(((temp & 0x00ff) * 10) + 0x80), ptr, 10);
	return strlen (output);
#else
        ret = snprintf_P(output, len,
                PSTR("Temperatur: %3d.%1d"),
                (int8_t) HI8(temp),  HI8(((temp & 0x00ff) * 10) + 0x80));
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
            return -2;
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
	return strlen (output);
#else
        ret = snprintf_P(output, len, PSTR("unknown sensor type"));
#endif
    }

    return ret;
}

int16_t parse_cmd_onewire_convert(char *cmd, char *output, uint16_t len)
{
    int16_t ret;

    if (strlen(cmd) > 0)
        cmd++;

    debug_printf("called onewire_list with: \"%s\"\n", cmd);

    struct ow_rom_code_t rom, *romptr;

    ret = parse_ow_rom(cmd, (void *)&rom);

    /* check for romcode */
    if (ret < 0)
        romptr = NULL;
    else
        romptr = &rom;

    debug_printf("converting temperature...\n");

    /* disable interrupts */
    uint8_t sreg = SREG;
    cli();

    ret = ow_temp_start_convert_wait(romptr);

    SREG = sreg;

    if (ret == 1)
        /* done */
        return 0;
    else if (ret == -1)
        /* no device attached */
        return -2;
    else
        /* wrong rom family code */
        return -1;

}
#endif /* ONEWIRE_SUPPORT */

