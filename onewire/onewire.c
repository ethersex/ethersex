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
#include "crc.h"
#include "onewire.h"

#if !defined(ONEWIRE_PINNUM) || !defined(ONEWIRE_PIN) || !defined(ONEWIRE_PORT) || !defined(ONEWIRE_DDR)
#error ONEWIRE_PINNUM, ONEWIRE_PIN, ONEWIRE_DDR or ONEWIRE_PORT undefined!
#endif

#ifdef ONEWIRE_SUPPORT

#ifdef DEBUG
#include "../uart.h"
#endif

void init_onewire(void)
/* {{{ */ {

    /* configure onewire pin as input */
    OW_CONFIG_INPUT();

    /* enable pullup */
    OW_PULLUP();

} /* }}} */

/* low-level functions */

uint8_t reset_onewire(void)
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

void ow_write_0(void)
/* {{{ */ {

    /* a write 0 timeslot is initiated by holding the data line low for
     * approximately 80us */

    OW_LOW();
    _delay_loop_2(OW_WRITE_0_TIMEOUT);
    OW_HIGH();

} /* }}} */

void ow_write_1(void)
/* {{{ */ {

    /* a write 1 timeslot is initiated by holding the data line low for
     * approximately 4us, then restore the idle state and wait at least 80us */

    OW_LOW();
    _delay_loop_2(OW_WRITE_1_TIMEOUT_1);
    OW_HIGH();
    _delay_loop_2(OW_WRITE_1_TIMEOUT_2);

} /* }}} */

void ow_write(uint8_t value)
/* {{{ */ {

    if (value > 0)
        ow_write_1();
    else
        ow_write_0();

} /* }}} */

void ow_write_byte(uint8_t value)
/* {{{ */ {

    OW_CONFIG_OUTPUT();
    for (uint8_t i = 0; i < 8; i++) {

        ow_write(value & _BV(i));
    }

} /* }}} */

/* FIXME: (optimization) combine ow_write_1() and ow_read(), as they are very similar */
uint8_t ow_read(void)
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



/* mid-level functions */
int8_t ow_read_rom(struct ow_rom_code_t *rom)
/* {{{ */ {

    /* reset the bus */
    if (!reset_onewire())
        return -1;

    /* transmit command byte */
    ow_write_byte(OW_ROM_READ_ROM);

    /* read 64bit rom code */
    for (uint8_t i = 0; i < 8; i++) {

        /* read byte */
        rom->bytewise[i] = 0;
        for (uint8_t j = 0; j < 8; j++) {
            rom->bytewise[i] |= (ow_read() << j);
        }
    }

    /* check CRC (last byte) */
    if (rom->crc != crc_checksum(rom->bytewise, 7))
        return -2;

    return 1;

} /* }}} */

int8_t ow_skip_rom(void)
/* {{{ */ {

    /* reset the bus */
    if (!reset_onewire())
        return -1;

    /* transmit command byte */
    ow_write_byte(OW_ROM_SKIP_ROM);

    return 1;

} /* }}} */

int8_t ow_match_rom(struct ow_rom_code_t *rom)
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


void noinline ow_set_address_bit(struct ow_rom_code_t *rom, uint8_t idx, uint8_t val);
void ow_set_address_bit(struct ow_rom_code_t *rom, uint8_t idx, uint8_t val)
/* {{{ */ {

    uint8_t byte = idx / 8;
    uint8_t bit = idx % 8;

    if (val == 0)
        rom->bytewise[byte] &= ~_BV(bit);
    else
        rom->bytewise[byte] |= _BV(bit);

/* }}} */ }

/* high-level functions */
int8_t ow_search_rom(struct ow_rom_code_t roms[], uint8_t max)
/* {{{ */ {

    int8_t discrepancy;
    int8_t last_discrepancy = -1;
    uint8_t rom_index = 0;

    /* reset first rom code */
    for (uint8_t i = 0; i < 8; i++)
        roms[0].bytewise[i] = 0;

    do {

        /* copy old path used before */
        if (rom_index > 0) {

            for (uint8_t i = 0; i < 8; i++)
                roms[rom_index].bytewise[i] = roms[rom_index-1].bytewise[i];

        }

        uart_puts_P("\r\npre-address: ");
        for (uint8_t i = 0; i < 8; i++)
            uart_puthexbyte(roms[rom_index].bytewise[i]);
        uart_eol();

        discrepancy = -1;

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
                return rom_index;

            } else if (bits == 0) {

                if (i == last_discrepancy) {

                    /* set one */
                    ow_set_address_bit(&roms[rom_index], i, 1);

                    /* transmit one next time */
                    bit1 = 1;

                } else if (i > last_discrepancy) {

                    /* set zero */
                    ow_set_address_bit(&roms[rom_index], i, 0);
                    discrepancy = i;

                } else {

                    uint8_t rom_bit = roms[rom_index].bytewise[i / 8] & _BV(i % 8);

                    if (rom_bit == 0)
                        discrepancy = i;

                    /* transmit last bit next time */
                    bit1 = rom_bit;

                }

            } else {

                /* normal case, no discrepancy */
                ow_set_address_bit(&roms[rom_index], i, bit1);

            }

            OW_CONFIG_OUTPUT();

            /* select next bit */
            ow_write(bit1);

        }

        last_discrepancy = discrepancy;
        rom_index++;

    } while(rom_index < max && last_discrepancy >= 0);

    return rom_index;

#if 0
    uint8_t rom_index = 0;

    int8_t last_diff;
    int8_t done_diff = 65;

    do {

        /* reset the bus */
        if (!reset_onewire())
            return -1;

        /* transmit command byte */
        ow_write_byte(OW_ROM_SEARCH_ROM);

        last_diff = -1;

        for (uint8_t i = 0; i < 64; i++) {

            uint8_t byte = i / 8;
            uint8_t index = i % 8;

            if (index == 0)
                roms[rom_index].bytewise[byte] = 0;

            uint8_t bit1 = ow_read();
            uint8_t bits = (ow_read() << 1) | bit1;

            /* if we received two ones, something is wrong (no devices attached?) */
            if ( bits == 3 )
                return -2;

            /* if we received two zeroes, we found a difference */
            if (bits == 0) {

                /* if this difference is later than the last difference
                 * remembered and before the differences already processed,
                 * remember index and choose zero branch */
                if (i > last_diff && i < done_diff)
                    last_diff = i;

                /* if this difference is at index done_diff, choose one branch */
                else if (i == done_diff)
                    bit1 = 1;

            }

            /* remember bit1 (there exists at least one device with bit1 in it's id) */
            if (bit1)
                roms[rom_index].bytewise[byte] |= _BV(index);

            OW_CONFIG_OUTPUT();

            /* select next bit */
            ow_write(bit1);

            /* set done_diff to last difference */
            if (last_diff > -1)
                done_diff = last_diff;

        }

        rom_index++;

    } while (rom_index < max && last_diff > -1);

    return rom_index;
#endif

} /* }}} */

int8_t ow_start_convert(struct ow_rom_code_t *rom, uint8_t wait)
/* {{{ */ {

    int8_t ret;

    if (rom == NULL)
        ret = ow_skip_rom();
    else
        ret = ow_match_rom(rom);

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

int8_t ow_read_scratchpad(struct ow_rom_code_t *rom, struct ow_scratchpad_t *scratchpad)
/* {{{ */ {

    int8_t ret;

    if (rom == NULL)
        ret = ow_skip_rom();
    else
        ret = ow_match_rom(rom);

    if (ret < 0)
        return ret;

    /* transmit command byte */
    ow_write_byte(OW_FUNC_READ_SP);

    for (uint8_t i = 0; i < 9; i++) {

        /* read byte */
        scratchpad->bytewise[i] = 0;

        for (uint8_t j = 0; j < 8; j++)
            scratchpad->bytewise[i] |= (ow_read() << j);

    }

    /* check CRC (last byte) */
    if (scratchpad->crc != crc_checksum(&scratchpad->bytewise, 8))
        return -2;

    return 1;

} /* }}} */

int8_t ow_power(struct ow_rom_code_t *rom)
/* {{{ */ {

    int8_t ret;

    if (rom == NULL)
        ret = ow_skip_rom();
    else
        ret = ow_match_rom(rom);

    if (ret < 0)
        return ret;

    /* transmit command byte */
    ow_write_byte(OW_FUNC_READ_POWER);

    return ow_read();

} /* }}} */


#endif
