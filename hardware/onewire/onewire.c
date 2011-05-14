/*
 *         simple onewire library implementation
 *
 *    for additional information please
 *    see http://koeln.ccc.de/prozesse/running/fnordlicht
 *
 * (c) by Alexander Neumann <alexander@bumpern.de>
 * Multibus support (c) 2011 by Frank Sautter
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

#include <avr/io.h>
#include <util/delay.h>
#include <util/crc16.h>

#include "config.h"
#include "core/eeprom.h"
#include "onewire.h"

#define noinline __attribute__((noinline))

#include "core/debug.h"

/* global variables */
struct ow_global_t ow_global;

/* module local prototypes */
void noinline ow_set_address_bit(struct ow_rom_code_t *rom, uint8_t idx, uint8_t val);

void onewire_init(void)
{

    /* configure onewire pin as input */
<<<<<<< HEAD
    OW_CONFIG_INPUT(ONEWIRE_BUSMASK);
=======
    OW_CONFIG_INPUT(ONEWIRE_MASK);
>>>>>>> effbd14f98712d15c5d13cea72e21f923aa3fa60

    /* release lock */
    ow_global.lock = 0;

}

/* low-level functions */

<<<<<<< HEAD
uint8_t noinline reset_onewire(uint8_t busmask)
{

    /* pull bus low */
    OW_CONFIG_OUTPUT(busmask);
    OW_LOW(busmask);
=======
uint8_t noinline reset_onewire(uint8_t mask)
{

    /* pull bus low */
    OW_CONFIG_OUTPUT(mask);
    OW_LOW(mask);
>>>>>>> effbd14f98712d15c5d13cea72e21f923aa3fa60

    /* wait 480us */
    _delay_loop_2(OW_RESET_TIMEOUT_1);

    /* release bus */
<<<<<<< HEAD
    OW_CONFIG_INPUT(busmask);
=======
    OW_CONFIG_INPUT(mask);
>>>>>>> effbd14f98712d15c5d13cea72e21f923aa3fa60

    /* wait 60us (maximal pause) + 30 us (half minimum pulse) */
    _delay_loop_2(OW_RESET_TIMEOUT_2);

    /* sample data */
<<<<<<< HEAD
    uint8_t data1 = OW_GET_INPUT(busmask);
=======
    uint8_t data1 = OW_GET_INPUT(mask);
>>>>>>> effbd14f98712d15c5d13cea72e21f923aa3fa60

    /* wait 390us */
    _delay_loop_2(OW_RESET_TIMEOUT_3);

    /* sample data again */
<<<<<<< HEAD
    uint8_t data2 = OW_GET_INPUT(busmask);
=======
    uint8_t data2 = OW_GET_INPUT(mask);
>>>>>>> effbd14f98712d15c5d13cea72e21f923aa3fa60

    /* if first sample is low and second sample is high, at least one device is
     * attached to this bus */
    return !data1 && data2;

}

<<<<<<< HEAD
void noinline ow_write(uint8_t busmask, uint8_t value)
{

    if (value > 0)
        ow_write_1(busmask);
    else
        ow_write_0(busmask);

}

void noinline ow_write_byte(uint8_t busmask, uint8_t value)
{

    OW_CONFIG_OUTPUT(busmask);
    for (uint8_t i = 0; i < 8; i++) {
        ow_write(busmask, value & _BV(i));
=======
void noinline ow_write(uint8_t mask, uint8_t value)
{

    if (value > 0)
        ow_write_1(mask);
    else
        ow_write_0(mask);

}

void noinline ow_write_byte(uint8_t mask, uint8_t value)
{

    OW_CONFIG_OUTPUT(mask);
    for (uint8_t i = 0; i < 8; i++) {
        ow_write(mask, value & _BV(i));
>>>>>>> effbd14f98712d15c5d13cea72e21f923aa3fa60
    }

}

<<<<<<< HEAD
void noinline ow_write_0(uint8_t busmask)
=======
void noinline ow_write_0(uint8_t mask)
>>>>>>> effbd14f98712d15c5d13cea72e21f923aa3fa60
{

    /* a write 0 timeslot is initiated by holding the data line low for
     * approximately 80us */

<<<<<<< HEAD
    OW_LOW(busmask);
    _delay_loop_2(OW_WRITE_0_TIMEOUT);
    OW_HIGH(busmask);

}

uint8_t noinline ow_read(uint8_t busmask)
=======
    OW_LOW(mask);
    _delay_loop_2(OW_WRITE_0_TIMEOUT);
    OW_HIGH(mask);

}

uint8_t noinline ow_read(uint8_t mask)
>>>>>>> effbd14f98712d15c5d13cea72e21f923aa3fa60
{

    /* a read timeslot is sent by holding the data line low for
     * 1us, then wait approximately 14us, then sample data and
     * wait */
    /* this is also used as ow_write_1, as the only difference
     * is that the return value is discarded */

<<<<<<< HEAD
    OW_CONFIG_OUTPUT(busmask);
    OW_LOW(busmask);

    _delay_loop_2(OW_READ_TIMEOUT_1);

    OW_HIGH(busmask);
    OW_CONFIG_INPUT(busmask);
=======
    OW_CONFIG_OUTPUT(mask);
    OW_LOW(mask);

    _delay_loop_2(OW_READ_TIMEOUT_1);

    OW_HIGH(mask);
    OW_CONFIG_INPUT(mask);
>>>>>>> effbd14f98712d15c5d13cea72e21f923aa3fa60

    _delay_loop_2(OW_READ_TIMEOUT_2);

    /* sample data now */
<<<<<<< HEAD
    uint8_t data = (OW_GET_INPUT(busmask) > 0);
=======
    uint8_t data = (OW_GET_INPUT(mask) > 0);
>>>>>>> effbd14f98712d15c5d13cea72e21f923aa3fa60

    /* wait for remaining slot time */
    _delay_loop_2(OW_READ_TIMEOUT_3);

<<<<<<< HEAD
    OW_CONFIG_OUTPUT(busmask);
=======
    OW_CONFIG_OUTPUT(mask);
>>>>>>> effbd14f98712d15c5d13cea72e21f923aa3fa60
    return data;

}

<<<<<<< HEAD
uint8_t noinline ow_read_byte(uint8_t busmask)
=======
uint8_t noinline ow_read_byte(uint8_t mask)
>>>>>>> effbd14f98712d15c5d13cea72e21f923aa3fa60
{

    uint8_t data = 0;

    for (uint8_t i = 0; i < 8; i++) {
<<<<<<< HEAD
        data |= (ow_read(busmask) << i);
=======
        data |= (ow_read(mask) << i);
>>>>>>> effbd14f98712d15c5d13cea72e21f923aa3fa60
    }

    return data;

}


/* mid-level functions */
int8_t noinline ow_read_rom(struct ow_rom_code_t *rom)
{

#ifdef ONEWIRE_MULTIBUS
<<<<<<< HEAD
    uint8_t busmask = 1 << (ONEWIRE_STARTPIN); // FIXME: currently only on 1st bus
#else
    uint8_t busmask = ONEWIRE_BUSMASK;
#endif
    /* reset the bus */
    if (!reset_onewire(busmask))
        return -1;

    /* transmit command byte */
    ow_write_byte(busmask, OW_ROM_READ_ROM);
=======
    uint8_t mask = 1 << (ONEWIRE_STARTPIN); // FIXME: currently only on 1st bus
#else
    uint8_t mask = ONEWIRE_MASK;
#endif
    /* reset the bus */
    if (!reset_onewire(mask))
        return -1;

    /* transmit command byte */
    ow_write_byte(mask, OW_ROM_READ_ROM);
>>>>>>> effbd14f98712d15c5d13cea72e21f923aa3fa60

    /* read 64bit rom code */
    for (uint8_t i = 0; i < 8; i++) {

        /* read byte */
<<<<<<< HEAD
        rom->bytewise[i] = ow_read_byte(busmask);
=======
        rom->bytewise[i] = ow_read_byte(mask);
>>>>>>> effbd14f98712d15c5d13cea72e21f923aa3fa60
    }

    /* check CRC (last byte) */
    if (rom->crc != crc_checksum(rom->bytewise, 7))
        return -2;

    return 1;

}

int8_t noinline ow_skip_rom(void)
{

    /* reset the bus */
<<<<<<< HEAD
    if (!reset_onewire(ONEWIRE_BUSMASK))
        return -1;

    /* transmit command byte */
    ow_write_byte(ONEWIRE_BUSMASK, OW_ROM_SKIP_ROM);
=======
    if (!reset_onewire(ONEWIRE_MASK))
        return -1;

    /* transmit command byte */
    ow_write_byte(ONEWIRE_MASK, OW_ROM_SKIP_ROM);
>>>>>>> effbd14f98712d15c5d13cea72e21f923aa3fa60

    return 1;

}

int8_t noinline ow_match_rom(struct ow_rom_code_t *rom)
{

    /* reset the bus */
<<<<<<< HEAD
    if (!reset_onewire(ONEWIRE_BUSMASK))
        return -1;

    /* transmit command byte */
    ow_write_byte(ONEWIRE_BUSMASK, OW_ROM_MATCH_ROM);
=======
    if (!reset_onewire(ONEWIRE_MASK))
        return -1;

    /* transmit command byte */
    ow_write_byte(ONEWIRE_MASK, OW_ROM_MATCH_ROM);
>>>>>>> effbd14f98712d15c5d13cea72e21f923aa3fa60

    /* transmit rom code */
    for (uint8_t i = 0; i < 8; i++) {
        for (uint8_t j = 0; j < 8; j++) {
<<<<<<< HEAD
            ow_write(ONEWIRE_BUSMASK, rom->bytewise[i] & _BV(j));
=======
            ow_write(ONEWIRE_MASK, rom->bytewise[i] & _BV(j));
>>>>>>> effbd14f98712d15c5d13cea72e21f923aa3fa60
        }
    }

    return 1;

}


void noinline ow_set_address_bit(struct ow_rom_code_t *rom, uint8_t idx, uint8_t val)
{

    uint8_t byte = idx / 8;
    uint8_t bit = idx % 8;

    if (val == 0)
        rom->bytewise[byte] &= ~_BV(bit);
    else
        rom->bytewise[byte] |= _BV(bit);

/* */ }

#ifdef ONEWIRE_DETECT_SUPPORT
/* high-level functions */
<<<<<<< HEAD
int8_t noinline ow_search_rom(uint8_t busmask, uint8_t first)
=======
int8_t noinline ow_search_rom(uint8_t mask, uint8_t first)
>>>>>>> effbd14f98712d15c5d13cea72e21f923aa3fa60
{

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
<<<<<<< HEAD
    if (!reset_onewire(busmask))
        return -1;

    /* transmit command byte */
    ow_write_byte(busmask, OW_ROM_SEARCH_ROM);
=======
    if (!reset_onewire(mask))
        return -1;

    /* transmit command byte */
    ow_write_byte(mask, OW_ROM_SEARCH_ROM);
>>>>>>> effbd14f98712d15c5d13cea72e21f923aa3fa60

    for (uint8_t i = 0; i <64; i++) {

        /* read bits */
<<<<<<< HEAD
        uint8_t bit1 = ow_read(busmask);
        uint8_t bits = (ow_read(busmask) << 1) | bit1;
=======
        uint8_t bit1 = ow_read(mask);
        uint8_t bits = (ow_read(mask) << 1) | bit1;
>>>>>>> effbd14f98712d15c5d13cea72e21f923aa3fa60

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

<<<<<<< HEAD
        OW_CONFIG_OUTPUT(busmask);

        /* select next bit */
        ow_write(busmask, bit1);
=======
        OW_CONFIG_OUTPUT(mask);

        /* select next bit */
        ow_write(mask, bit1);
>>>>>>> effbd14f98712d15c5d13cea72e21f923aa3fa60

    }

    ow_global.last_discrepancy = discrepancy;

    /* new device discovered */
    return 1;

}
#endif /* ONEWIRE_DETECT_SUPPORT */

/*
 *
 * temperature functions
 *
 */

int8_t ow_temp_sensor(struct ow_rom_code_t *rom)
{

    /* check for known family code */
    return (rom->family == OW_FAMILY_DS1820 ||
            rom->family == OW_FAMILY_DS1822 ||
            rom->family == OW_FAMILY_DS18B20);

}

int8_t ow_temp_start_convert(struct ow_rom_code_t *rom, uint8_t wait)
{

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
<<<<<<< HEAD
    ow_write_byte(ONEWIRE_BUSMASK, OW_FUNC_CONVERT);

    OW_CONFIG_OUTPUT(ONEWIRE_BUSMASK);
    OW_HIGH(ONEWIRE_BUSMASK);
=======
    ow_write_byte(ONEWIRE_MASK, OW_FUNC_CONVERT);

    OW_CONFIG_OUTPUT(ONEWIRE_MASK);
    OW_HIGH(ONEWIRE_MASK);
>>>>>>> effbd14f98712d15c5d13cea72e21f923aa3fa60

    if (!wait)
        return 0;

    _delay_ms(800); /* The specification say, that we have to wait at
                       least 500ms in parasite mode to wait for the
                       end of the conversion.  800ms works more reliably */

<<<<<<< HEAD
    while(!ow_read(ONEWIRE_BUSMASK));
=======
    while(!ow_read(ONEWIRE_MASK));
>>>>>>> effbd14f98712d15c5d13cea72e21f923aa3fa60

    return 1;

}

int8_t ow_temp_read_scratchpad(struct ow_rom_code_t *rom, struct ow_temp_scratchpad_t *scratchpad)
{

//    uint8_t bus;
<<<<<<< HEAD
    uint8_t busmask;
=======
    uint8_t mask;
>>>>>>> effbd14f98712d15c5d13cea72e21f923aa3fa60
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
<<<<<<< HEAD
    ow_write_byte(ONEWIRE_BUSMASK, OW_FUNC_READ_SP);
=======
    ow_write_byte(ONEWIRE_MASK, OW_FUNC_READ_SP);
>>>>>>> effbd14f98712d15c5d13cea72e21f923aa3fa60

#ifdef ONEWIRE_MULTIBUS
    for (uint8_t bus = 0; bus < ONEWIRE_COUNT; bus++) {
        /* read 9 bytes from each onewire bus */
<<<<<<< HEAD
        busmask = 1 << (bus + ONEWIRE_STARTPIN);
#else
        busmask = ONEWIRE_BUSMASK;
#endif
        for (uint8_t i = 0; i < 9; i++) {
            scratchpad->bytewise[i] = ow_read_byte(busmask);
=======
        mask = 1 << (bus + ONEWIRE_STARTPIN);
#else
        mask = ONEWIRE_MASK;
#endif
        for (uint8_t i = 0; i < 9; i++) {
            scratchpad->bytewise[i] = ow_read_byte(mask);
>>>>>>> effbd14f98712d15c5d13cea72e21f923aa3fa60
        }

        /* check CRC (last byte) */
        if (scratchpad->crc == crc_checksum(&scratchpad->bytewise, 8)) {
            /* return if we got a valid response from one device */
            return 1;
        }
#ifdef ONEWIRE_MULTIBUS
    }
#endif

    return -2;

}

int8_t ow_temp_power(struct ow_rom_code_t *rom)
{

#ifdef ONEWIRE_MULTIBUS
<<<<<<< HEAD
    uint8_t busmask = 1 << (ONEWIRE_STARTPIN); // FIXME: currently only on 1st bus
#else
    uint8_t busmask = ONEWIRE_BUSMASK;
=======
    uint8_t mask = 1 << (ONEWIRE_STARTPIN); // FIXME: currently only on 1st bus
#else
    uint8_t mask = ONEWIRE_MASK;
>>>>>>> effbd14f98712d15c5d13cea72e21f923aa3fa60
#endif
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
<<<<<<< HEAD
    ow_write_byte(busmask, OW_FUNC_READ_POWER);

    return ow_read(busmask);
=======
    ow_write_byte(mask, OW_FUNC_READ_POWER);

    return ow_read(mask);
>>>>>>> effbd14f98712d15c5d13cea72e21f923aa3fa60

}

int16_t ow_temp_normalize(struct ow_rom_code_t *rom, struct ow_temp_scratchpad_t *sp)
{

    if (rom->family == OW_FAMILY_DS1820)
        return ((sp->temperature & 0xfffe) << 7) - 0x40 + (((sp->count_per_c - sp->count_remain) << 8) / sp->count_per_c);
    else if (rom->family == OW_FAMILY_DS1822 || rom->family == OW_FAMILY_DS18B20)
        return sp->temperature << 4;
    else
        return 0xffff;

}

/*
 *
 * DS2502 data functions
 *
 */

#ifdef ONEWIRE_DS2502_SUPPORT

int8_t ow_eeprom(struct ow_rom_code_t *rom)
{

    /* check for known family code */
    if (rom->family == OW_FAMILY_DS2502E48)
        return 1;

    return 0;

}

int8_t ow_eeprom_read(struct ow_rom_code_t *rom, void *data)
{
#ifdef ONEWIRE_MULTIBUS
<<<<<<< HEAD
    uint8_t busmask = 1 << (ONEWIRE_STARTPIN); // FIXME: currently only on 1st bus
#else
    uint8_t busmask = ONEWIRE_BUSMASK;
=======
    uint8_t mask = 1 << (ONEWIRE_STARTPIN); // FIXME: currently only on 1st bus
#else
    uint8_t mask = ONEWIRE_MASK;
>>>>>>> effbd14f98712d15c5d13cea72e21f923aa3fa60
#endif
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
<<<<<<< HEAD
    ow_write_byte(busmask, OW_FUNC_READ_MEMORY);

    /* transmit address (mac address starts at offset 5 */
    ow_write_byte(busmask, 5);
    ow_write_byte(busmask, 0);

    /* read back crc sum of the command */
    uint8_t crc = ow_read_byte(busmask);
=======
    ow_write_byte(mask, OW_FUNC_READ_MEMORY);

    /* transmit address (mac address starts at offset 5 */
    ow_write_byte(mask, 5);
    ow_write_byte(mask, 0);

    /* read back crc sum of the command */
    uint8_t crc = ow_read_byte(mask);
>>>>>>> effbd14f98712d15c5d13cea72e21f923aa3fa60

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
<<<<<<< HEAD
        *p-- = ow_read_byte(busmask);
=======
        *p-- = ow_read_byte(mask);
>>>>>>> effbd14f98712d15c5d13cea72e21f923aa3fa60


    return 0;

}

#endif /* ONEWIRE_DS2502_SUPPORT */

/*
  -- Ethersex META --
  header(hardware/onewire/onewire.h)
  init(onewire_init)
*/
