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

#include <avr/io.h>
#include <util/delay.h>
#include <util/crc16.h>

#include "../config.h"
#include "../eeprom.h"
#include "onewire.h"

#ifdef ONEWIRE_SUPPORT

#define noinline __attribute__((noinline))

#include "../debug.h"

/* global variables */
struct ow_global_t ow_global;

/* module local prototypes */
void noinline ow_set_address_bit(struct ow_rom_code_t *rom, uint8_t idx, uint8_t val);
uint8_t ow_read_byte(void);

void onewire_init(void)
/* {{{ */ {

    /* configure onewire pin as input */
    OW_CONFIG_INPUT();

    /* enable pullup */
    OW_PULLUP();

    /* reset lock */
    ow_global.lock = 0;

} /* }}} */

/* low-level functions */

uint8_t noinline reset_onewire(void)
/* {{{ */ {

    /* pull bus low */
    OW_CONFIG_OUTPUT();
    OW_LOW();

    /* wait 480us */
    _delay_loop_2(OW_RESET_TIMEOUT_1);

    /* release bus */
    OW_HIGH();
    OW_CONFIG_INPUT();

    /* wait 60us (maximal pause) + 30 us (half minimum pulse) */
    _delay_loop_2(OW_RESET_TIMEOUT_2);

    /* sample data */
    uint8_t data1 = OW_GET_INPUT();

    /* wait 390us */
    _delay_loop_2(OW_RESET_TIMEOUT_3);

    /* sample data again */
    uint8_t data2 = OW_GET_INPUT();

    /* if first sample is low and second sample is high, at least one device is
     * attached to this bus */
    return !data1 && data2;

} /* }}} */

void noinline ow_write_0(void)
/* {{{ */ {

    /* a write 0 timeslot is initiated by holding the data line low for
     * approximately 80us */

    OW_LOW();
    _delay_loop_2(OW_WRITE_0_TIMEOUT);
    OW_HIGH();

} /* }}} */

void noinline ow_write_1(void)
/* {{{ */ {

    /* a write 1 timeslot is initiated by holding the data line low for
     * approximately 4us, then restore the idle state and wait at least 80us */

    OW_LOW();
    _delay_loop_2(OW_WRITE_1_TIMEOUT_1);
    OW_HIGH();
    _delay_loop_2(OW_WRITE_1_TIMEOUT_2);

} /* }}} */

void noinline ow_write(uint8_t value)
/* {{{ */ {

    if (value > 0)
        ow_write_1();
    else
        ow_write_0();

} /* }}} */

void noinline ow_write_byte(uint8_t value)
/* {{{ */ {

    OW_CONFIG_OUTPUT();
    for (uint8_t i = 0; i < 8; i++) {

        ow_write(value & _BV(i));
    }

} /* }}} */

/* FIXME: (optimization) combine ow_write_1() and ow_read(), as they are very similar */
uint8_t noinline ow_read(void)
/* {{{ */ {

    /* a read timeslot is sent by holding the data line low for
     * 1us, then wait approximately 14us, then sample data and
     * wait  */

    OW_CONFIG_OUTPUT();
    OW_LOW();

    _delay_loop_2(OW_READ_TIMEOUT_1);

    OW_HIGH();
    OW_CONFIG_INPUT();

    _delay_loop_2(OW_READ_TIMEOUT_2);

    /* sample data now */
    uint8_t data = OW_GET_INPUT();

    /* wait for remaining slot time */
    _delay_loop_2(OW_READ_TIMEOUT_3);

    return data;

} /* }}} */

uint8_t noinline ow_read_byte(void)
/* {{{ */ {

    uint8_t data = 0;

    for (uint8_t i = 0; i < 8; i++) {
        data |= (ow_read() << i);
    }

    return data;

} /* }}} */


/* mid-level functions */
int8_t noinline ow_read_rom(struct ow_rom_code_t *rom)
/* {{{ */ {

    /* reset the bus */
    if (!reset_onewire())
        return -1;

    /* transmit command byte */
    ow_write_byte(OW_ROM_READ_ROM);

    /* read 64bit rom code */
    for (uint8_t i = 0; i < 8; i++) {

        /* read byte */
        rom->bytewise[i] = ow_read_byte();
    }

    /* check CRC (last byte) */
    if (rom->crc != crc_checksum(rom->bytewise, 7))
        return -2;

    return 1;

} /* }}} */

int8_t noinline ow_skip_rom(void)
/* {{{ */ {

    /* reset the bus */
    if (!reset_onewire())
        return -1;

    /* transmit command byte */
    ow_write_byte(OW_ROM_SKIP_ROM);

    return 1;

} /* }}} */

int8_t noinline ow_match_rom(struct ow_rom_code_t *rom)
/* {{{ */ {

    /* reset the bus */
    if (!reset_onewire())
        return -1;

    /* transmit command byte */
    ow_write_byte(OW_ROM_MATCH_ROM);

    /* transmit rom code */
    for (uint8_t i = 0; i < 8; i++) {

        for (uint8_t j = 0; j < 8; j++)
            ow_write(rom->bytewise[i] & _BV(j));

    }

    return 1;

} /* }}} */


void noinline ow_set_address_bit(struct ow_rom_code_t *rom, uint8_t idx, uint8_t val)
/* {{{ */ {

    uint8_t byte = idx / 8;
    uint8_t bit = idx % 8;

    if (val == 0)
        rom->bytewise[byte] &= ~_BV(bit);
    else
        rom->bytewise[byte] |= _BV(bit);

/* }}} */ }

/* high-level functions */
int8_t noinline ow_search_rom(uint8_t first)
/* {{{ */ {

    /* reset discover state machine */
    if (first) {
        ow_global.last_discrepancy = -1;

        /* reset rom code */
        for (uint8_t i = 0; i < 8; i++)
            ow_global.current_rom.bytewise[i] = 0;

    } else {

        /* if last_discrepancy is below zero, discovery is done */
        if (ow_global.last_discrepancy < 0)
            return 0;

    }

    uint8_t discrepancy = -1;

    /* reset the bus */
    if (!reset_onewire())
        return -1;

    /* transmit command byte */
    ow_write_byte(OW_ROM_SEARCH_ROM);

    for (uint8_t i = 0; i <64; i++) {

        /* read bits */
        uint8_t bit1 = ow_read();
        uint8_t bits = (ow_read() << 1) | bit1;

        if (bits == 3) {

            /* no devices, just return */
            return 0;

        } else if (bits == 0) {

            if (i == ow_global.last_discrepancy) {

                /* set one */
                ow_set_address_bit(&ow_global.current_rom, i, 1);

                /* transmit one next time */
                bit1 = 1;

            } else if (i > ow_global.last_discrepancy) {

                /* set zero */
                ow_set_address_bit(&ow_global.current_rom, i, 0);
                discrepancy = i;

            } else {

                uint8_t rom_bit = ow_global.current_rom.bytewise[i / 8] & _BV(i % 8);

                if (rom_bit == 0)
                    discrepancy = i;

                /* transmit last bit next time */
                bit1 = rom_bit;

            }

        } else {

            /* normal case, no discrepancy */
            ow_set_address_bit(&ow_global.current_rom, i, bit1);

        }

        OW_CONFIG_OUTPUT();

        /* select next bit */
        ow_write(bit1);

    }

    ow_global.last_discrepancy = discrepancy;

    /* new device discovered */
    return 1;

} /* }}} */

/*
 *
 * temperature functions
 *
 */

int8_t ow_temp_sensor(struct ow_rom_code_t *rom)
/* {{{ */ {

    /* check for known family code */
    if (rom->family == OW_FAMILY_DS1820 ||
            rom->family == OW_FAMILY_DS1822||
            rom->family == OW_FAMILY_DS18B20 )
        return 1;

    return 0;

} /* }}} */

int8_t ow_temp_start_convert(struct ow_rom_code_t *rom, uint8_t wait)
/* {{{ */ {

    int8_t ret;

    if (rom == NULL)
        ret = ow_skip_rom();
    else {

        /* check for known family code */
        if (!ow_temp_sensor(rom))
            return -2;

        ret = ow_match_rom(rom);

    }

    if (ret < 0)
        return ret;

    /* transmit command byte */
    ow_write_byte(OW_FUNC_CONVERT);

    OW_CONFIG_OUTPUT();
    OW_HIGH();

    if (!wait)
        return 0;

    while(!ow_read());

    return 1;

} /* }}} */

int8_t ow_temp_read_scratchpad(struct ow_rom_code_t *rom, struct ow_temp_scratchpad_t *scratchpad)
/* {{{ */ {

    int8_t ret;

    if (rom == NULL)
        ret = ow_skip_rom();
    else {

        /* check for known family code */
        if (!ow_temp_sensor(rom))
            return -3;

        ret = ow_match_rom(rom);
    }

    if (ret < 0)
        return ret;

    /* transmit command byte */
    ow_write_byte(OW_FUNC_READ_SP);

    for (uint8_t i = 0; i < 9; i++) {

        /* read byte */
        scratchpad->bytewise[i] = ow_read_byte();

    }

    /* check CRC (last byte) */
    if (scratchpad->crc != crc_checksum(&scratchpad->bytewise, 8))
        return -2;

    return 1;

} /* }}} */

int8_t ow_temp_power(struct ow_rom_code_t *rom)
/* {{{ */ {

    int8_t ret;

    if (rom == NULL)
        ret = ow_skip_rom();
    else {
        if (!ow_temp_sensor(rom))
            return -2;

        ret = ow_match_rom(rom);
    }

    if (ret < 0)
        return ret;

    /* transmit command byte */
    ow_write_byte(OW_FUNC_READ_POWER);

    return ow_read();

} /* }}} */

int16_t ow_temp_normalize(struct ow_rom_code_t *rom, struct ow_temp_scratchpad_t *sp)
/* {{{ */ {

    if (rom->family == OW_FAMILY_DS1820)
        return ((sp->temperature & 0xfffe) << 7) - 0x40 + (((sp->count_per_c - sp->count_remain) << 8) / sp->count_per_c);
    else if (rom->family == OW_FAMILY_DS1822 || rom->family == OW_FAMILY_DS18B20)
        return sp->temperature << 4;
    else
        return 0xffff;

} /* }}} */

/*
 *
 * DS2502 data functions
 *
 */

#ifdef ONEWIRE_DS2502_SUPPORT

int8_t ow_eeprom(struct ow_rom_code_t *rom)
/* {{{ */ {

    /* check for known family code */
    if (rom->family == OW_FAMILY_DS2502E48)
        return 1;

    return 0;

} /* }}} */

int8_t ow_eeprom_read(struct ow_rom_code_t *rom, void *data)
/* {{{ */ {

    int8_t ret;

    if (rom == NULL)
        ret = ow_skip_rom();
    else {

        /* check for known family code */
        if (!(rom->family == OW_FAMILY_DS2502E48))
            return -2;

        ret = ow_match_rom(rom);

    }

    if (ret < 0)
        return ret;

    /* transmit command byte */
    ow_write_byte(OW_FUNC_READ_MEMORY);

    /* transmit address (mac address starts at offset 5 */
    ow_write_byte(5);
    ow_write_byte(0);

    /* read back crc sum of the command */
    uint8_t crc = ow_read_byte();

    /* check crc */
    uint8_t crc2 = 0;
    crc2 = _crc_ibutton_update(crc2, OW_FUNC_READ_MEMORY);
    crc2 = _crc_ibutton_update(crc2, 5);
    crc2 = _crc_ibutton_update(crc2, 0);

    if (crc != crc2)
        return -2;

    uint8_t *p = (uint8_t *)data+5;

    /* read 6 byte of data */
    for (uint8_t i = 0; i < 6; i++)
        *p-- = ow_read_byte();


    return 0;

} /* }}} */

#endif /* ONEWIRE_DS2502_SUPPORT */

#endif
