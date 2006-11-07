/* vim:fdm=marker ts=4 et ai
 * {{{
 *         simple onewire library implementation
 *
 *    for additional information please
 *    see http://koeln.ccc.de/prozesse/running/fnordlicht
 *
 * (c) by Alexander Neumann <alexander@bumpern.de>
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
 }}} */


#ifndef ONEWIRE_H
#define ONEWIRE_H

/* For an introduction to the onewire bus protocol see
 * http://www.ibutton.com/ibuttons/standard.pdf and the ds18s20 datasheet */

/* configuration:
 *
 * if you want to use this library, define ONEWIRE_SUPPORT somewhere.
 * pin defines (eg for using PD6 as a port pin):
 *
 * #define ONEWIRE_PINNUM PD6
 * #define ONEWIRE_PIN PIND
 * #define ONEWIRE_PORT PORTD
 * #define ONEWIRE_DDR DDRD
 *
 */

#include <stdint.h>

#ifdef ONEWIRE_SUPPORT

/* constants */
/* {{{ */

#define NULL ((void *)0)
#define LOW(x) ((uint8_t)(x))
#define HIGH(x) ((uint8_t)((x) >> 8))

/* rom commands */
#define OW_ROM_SEARCH_ROM 0xF0
#define OW_ROM_READ_ROM 0x33
#define OW_ROM_MATCH_ROM 0x55
#define OW_ROM_SKIP_ROM 0xCC
#define OW_ROM_ALARM_SEARCH 0xEC

/* families */
#define OW_FAMILY_DS18S20 0x10

/* ds18s20 functions commands */
#define OW_FUNC_CONVERT 0x44
#define OW_FUNC_WRITE_SP 0x4E
#define OW_FUNC_READ_SP 0xBE
#define OW_FUNC_COPY_SP 0x48
#define OW_FUNC_RECALL_EE 0xB8
#define OW_FUNC_READ_POWER 0xB4

/* }}} */

/* timing constants */
/* {{{ */
/* onewire needs a 480us reset timeout, suitable for use with the delay_loop() functions */
#define OW_RESET_TIMEOUT_1 (F_CPU / 1000000 * 480 / 4)
/* we wait 90us before sampling data for the low-pulse detection */
#define OW_RESET_TIMEOUT_2 (F_CPU / 1000000 * 90 / 4)
/* and wait 480us - 90us = 390us before initiating the next onewire command */
#define OW_RESET_TIMEOUT_3 (F_CPU / 1000000 * 390 / 4)

#if OW_RESET_TIMEOUT_1 > 65535
#error OW_CONFIG_TIMEOUT_1 bigger than 64k?!
#endif

/* a write 0 timeslot is initiated by holding the data line low for
 * approximately 80us */
#define OW_WRITE_0_TIMEOUT (F_CPU / 1000000 * 84 / 4)

/* a write 1 timeslot is initiated by holding the data line low for
 * approximately 4us and then waiting at least 60us */
#define OW_WRITE_1_TIMEOUT_1 (F_CPU / 1000000 *  4 / 4)
#define OW_WRITE_1_TIMEOUT_2 (F_CPU / 1000000 * 80 / 4)

/* a read timeslot is initiated by holding the data line low for 1us and
 * sampling data after 14us and wait for the remaining slot time */
#define OW_READ_TIMEOUT_1 (F_CPU / 1000000 * 1 / 4)
#define OW_READ_TIMEOUT_2 (F_CPU / 1000000 * 14 / 4)
#define OW_READ_TIMEOUT_3 (F_CPU / 1000000 * 65 / 4)
/* }}} */

/* macros */
/* {{{ */
#define OW_CONFIG_INPUT()                        \
    /* {{{ */                                    \
    do {                                         \
        /* enable pullup */                      \
        ONEWIRE_PORT |= _BV(ONEWIRE_PINNUM);     \
        /* configure as input */                 \
        ONEWIRE_DDR &= ~_BV(ONEWIRE_PINNUM);     \
    } while (0)
    /* }}} */

#define OW_CONFIG_OUTPUT()                       \
    /* {{{ */                                    \
        /* configure as output */                \
        ONEWIRE_DDR |= _BV(ONEWIRE_PINNUM);      \
    /* }}} */

#define OW_LOW()                                 \
    /* {{{ */                                    \
        /* configure drive low */                \
        ONEWIRE_PORT &= ~_BV(ONEWIRE_PINNUM);    \
    /* }}} */

#define OW_HIGH()                                \
    /* {{{ */                                    \
        /* configure drive high */               \
        ONEWIRE_PORT |= _BV(ONEWIRE_PINNUM);     \
    /* }}} */

#define OW_PULLUP()                              \
    /* {{{ */                                    \
        /* pull up resistor */                   \
        ONEWIRE_PORT |= _BV(ONEWIRE_PINNUM);     \
    /* }}} */

#define OW_GET_INPUT()                           \
    /* {{{ */                                    \
        ((ONEWIRE_PIN & _BV(ONEWIRE_PINNUM)) > 0)
    /* }}} */

/* }}} */

/* structures */
/* {{{ */
struct ow_rom_code_t {
    union {
        uint64_t raw;
        uint8_t bytewise[8];
        struct {
            uint8_t family;
            uint8_t code[6];
            uint8_t crc;
        };
    };
};

struct ow_scratchpad_t {
    union {
        uint8_t bytewise[9];
        struct {
            union {
                struct {
                    uint8_t temperature_low;
                    uint8_t temperature_high;
                };
                uint16_t temperature;
            };

            uint8_t th;
            uint8_t tl;
            uint8_t reserved1;
            uint8_t reserved2;
            uint8_t count_remain;
            uint8_t count_per_c;
            uint8_t crc;
        };
    };
};

/* }}} */

/* prototypes */
void init_onewire(void);

/* low level functions */
uint8_t reset_onewire(void);
void ow_write_0(void);
void ow_write_1(void);
void ow_write(uint8_t value);
void ow_write_byte(uint8_t value);
uint8_t ow_read(void);

/* high level functions */

/* tries to read the code of the attached device, if there is only one
 *
 * return values:
 *    1: code read successfully, code written to given ow_rom_code_t structure
 *   -1: no presence pulse has been detected, no device connected?
 *   -2: crc check failed, multiple devices on the same bus? use search_rom()
 */
int8_t ow_read_rom(struct ow_rom_code_t *rom);

/* skip rom addressing, only works if there is exactly one device on the bus or
 * if this command should go to ALL onewire devices!
 *
 * return values:
 *    1: skip rom command issued successfully
 *   -1: no presence pulse has been detected, no device connected?
 */
int8_t ow_skip_rom(void);

/* address one sensor.
 *
 * return values:
 *    1: match rom command issued successfully
 *   -1: no presence pulse has been detected, no device connected?
 */
int8_t ow_match_rom(struct ow_rom_code_t *rom);

/* detect rom codes of up to max devices on the bus, write their code to the
 * ow_rom_code_t struct array
 *
 * return values:
 *  n>0: successfully learned n device rom codes (n <= max)
 *   -1: no presence pulse has been detected, no device connected?
 */
int8_t ow_search_rom(struct ow_rom_code_t roms[], uint8_t max);

/* start temperature conversion on sensor with given id, or to all sensors (via
 * skip_rom) if NULL.  If wait is set, busy-loop until conversion is done on all
 * adressed sensors.
 *
 * return values:
 *    0: conversion initiated (but not done)
 *    1: conversion done
 *   -1: no presence pulse has been detected, no device connected?
 */
int8_t ow_start_convert(struct ow_rom_code_t *rom, uint8_t wait);

/* read scratchpad memory of sensor with given id (may be null, if only one
 * sensor is connected) skip_rom)
 *
 * return values:
 *    1: memory read
 *   -1: no presence pulse has been detected, no device connected?
 *   -2: crc check failed, multiple devices on the same bus?
 */
int8_t ow_read_scratchpad(struct ow_rom_code_t *rom, struct ow_scratchpad_t *scratchpad);

/* check for parasite powered devices, if rom is NULL, all devices are queried.
 *
 * return values:
 *    0: device(s) parasite powered
 *    1: device(s) external powered
 *   -1: no presence pulse has been detected, no device connected?
 */
int8_t ow_power(struct ow_rom_code_t *rom);

#endif

#endif
