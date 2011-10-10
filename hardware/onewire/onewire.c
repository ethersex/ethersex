/*
 * Simple onewire library implementation
 *
 * Copyright (c) Alexander Neumann <alexander@bumpern.de>
 * Copyright (c) 2011 by Frank Sautter
 * Copyright (c) 2011 by Maximilian Güntner
 * Copyright (c) 2011 by Erik Kunze <ethersex@erik-kunze.de>
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
#include <util/atomic.h>
#include <util/delay.h>
#include <util/crc16.h>
#include <avr/interrupt.h>
#include "config.h"
#include "core/eeprom.h"
#include "onewire.h"
#include "core/bit-macros.h"

#define noinline __attribute__((noinline))

#include "core/debug.h"

/* global variables */
ow_global_t ow_global;

/* module local prototypes */
void noinline ow_set_address_bit(ow_rom_code_t *rom, uint8_t idx, uint8_t val);

void onewire_init(void)
{
    /* configure onewire pin as input */
    OW_CONFIG_INPUT(ONEWIRE_BUSMASK);

    /* release lock */
    ow_global.lock = 0;
}

/* low-level functions */

uint8_t noinline reset_onewire(uint8_t busmask)
{

    /* pull bus low */
    OW_CONFIG_OUTPUT(busmask);
    OW_LOW(busmask);

    /* wait 480us */
    _delay_loop_2(OW_RESET_TIMEOUT_1);

    /* release bus */
    OW_CONFIG_INPUT(busmask);

    /* wait 60us (maximal pause) + 30 us (half minimum pulse) */
    _delay_loop_2(OW_RESET_TIMEOUT_2);

    /* sample data */
    uint8_t data1 = OW_GET_INPUT(busmask);

    /* wait 390us */
    _delay_loop_2(OW_RESET_TIMEOUT_3);

    /* sample data again */
    uint8_t data2 = OW_GET_INPUT(busmask);

    /* if first sample is low and second sample is high, at least one device is
     * attached to this bus */
    return (uint8_t)(~data1 & data2 & busmask);

}

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
        ow_write(busmask, (uint8_t)(value & _BV(i)));
    }
}

void noinline ow_write_0(uint8_t busmask)
{
    /* a write 0 timeslot is initiated by holding the data line low for
     * approximately 80us */

    OW_LOW(busmask);
    _delay_loop_2(OW_WRITE_0_TIMEOUT);
    OW_HIGH(busmask);
}

uint8_t noinline ow_read(uint8_t busmask)
{
    /* a read timeslot is sent by holding the data line low for
     * 1us, then wait approximately 14us, then sample data and
     * wait */
    /* this is also used as ow_write_1, as the only difference
     * is that the return value is discarded */

    OW_CONFIG_OUTPUT(busmask);
    OW_LOW(busmask);

    _delay_loop_2(OW_READ_TIMEOUT_1);

    OW_HIGH(busmask);
    OW_CONFIG_INPUT(busmask);

    _delay_loop_2(OW_READ_TIMEOUT_2);

    /* sample data now */
    uint8_t data = (uint8_t)(OW_GET_INPUT(busmask) > 0);

    /* wait for remaining slot time */
    _delay_loop_2(OW_READ_TIMEOUT_3);

    OW_CONFIG_OUTPUT(busmask);
    return data;
}

uint8_t noinline ow_read_byte(uint8_t busmask)
{
    uint8_t data = 0;

    for (uint8_t i = 0; i < 8; i++) {
        data |= (uint8_t)(ow_read(busmask) << i);
    }

    return data;
}


/* mid-level functions */
int8_t noinline ow_read_rom(ow_rom_code_t *rom)
{
#if ONEWIRE_BUSCOUNT > 1
    uint8_t busmask = 1 << (ONEWIRE_STARTPIN); // FIXME: currently only on 1st bus
#else
    uint8_t busmask = ONEWIRE_BUSMASK;
#endif
    /* reset the bus */
    if (!reset_onewire(busmask))
        return -1;

    /* transmit command byte */
    ow_write_byte(busmask, OW_ROM_READ_ROM);

    /* read 64bit rom code */
    for (uint8_t i = 0; i < 8; i++) {

        /* read byte */
        rom->bytewise[i] = ow_read_byte(busmask);
    }

    /* check CRC (last byte) */
    if (rom->crc != crc_checksum(rom->bytewise, 7))
        return -2;

    return 1;
}

int8_t noinline ow_skip_rom(void)
{
    /* reset the bus */
    if (!reset_onewire(ONEWIRE_BUSMASK))
        return -1;

    /* transmit command byte */
    ow_write_byte(ONEWIRE_BUSMASK, OW_ROM_SKIP_ROM);

    return 1;
}

int8_t noinline ow_match_rom(ow_rom_code_t *rom)
{
    /* reset the bus */
    if (!reset_onewire(ONEWIRE_BUSMASK))
        return -1;

    /* transmit command byte */
    ow_write_byte(ONEWIRE_BUSMASK, OW_ROM_MATCH_ROM);

    /* transmit rom code */
    for (uint8_t i = 0; i < 8; i++) {
        for (uint8_t j = 0; j < 8; j++) {
            ow_write(ONEWIRE_BUSMASK, (uint8_t)(rom->bytewise[i] & _BV(j)));
        }
    }

    return 1;
}

void noinline ow_set_address_bit(ow_rom_code_t *rom, uint8_t idx, uint8_t val)
{
    uint8_t byte = idx / 8;
    uint8_t bit = (uint8_t)_BV(idx % 8);
		rom->bytewise[byte] &= (uint8_t)~bit;
		if (val) rom->bytewise[byte] |= bit;
}

#ifdef ONEWIRE_DETECT_SUPPORT
/* high-level functions */
int8_t noinline ow_search_rom(uint8_t busmask, uint8_t first)
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

    int8_t discrepancy = -1;

    /* reset the bus */
    if (!reset_onewire(busmask))
        return -1;

    /* transmit command byte */
    ow_write_byte(busmask, OW_ROM_SEARCH_ROM);

    for (uint8_t i = 0; i <64; i++) {

        /* read bits */
        uint8_t bit1 = ow_read(busmask);
        uint8_t bits = (uint8_t)((ow_read(busmask) << 1) | bit1);

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
                discrepancy = (int8_t)i;

            } else {

                uint8_t rom_bit = (uint8_t)(ow_global.current_rom.bytewise[i / 8] & _BV(i % 8));

                if (rom_bit == 0)
                    discrepancy = (int8_t)i;

                /* transmit last bit next time */
                bit1 = rom_bit;

            }

        } else {

            /* normal case, no discrepancy */
            ow_set_address_bit(&ow_global.current_rom, i, bit1);

        }

        OW_CONFIG_OUTPUT(busmask);

        /* select next bit */
        ow_write(busmask, bit1);

    }

    ow_global.last_discrepancy = discrepancy;

    /* new device discovered */
    return 1;
}
#endif /* ONEWIRE_DETECT_SUPPORT */

/*
 * temperature functions
 */

int8_t ow_temp_sensor(ow_rom_code_t *rom)
{
    /* check for known family code */
    return (int8_t)(rom->family == OW_FAMILY_DS1820 ||
                    rom->family == OW_FAMILY_DS1822 ||
                    rom->family == OW_FAMILY_DS18B20);
}

int8_t ow_temp_start_convert(ow_rom_code_t *rom, uint8_t wait)
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
    ow_write_byte(ONEWIRE_BUSMASK, OW_FUNC_CONVERT);

    OW_CONFIG_OUTPUT(ONEWIRE_BUSMASK);
    OW_HIGH(ONEWIRE_BUSMASK);

    if (!wait)
        return 0;

    _delay_ms(800); /* The specification say, that we have to wait at
                       least 500ms in parasite mode to wait for the
                       end of the conversion.  800ms works more reliably */

    while(!ow_read(ONEWIRE_BUSMASK));

    return 1;
}

int8_t ow_temp_read_scratchpad(ow_rom_code_t *rom, ow_temp_scratchpad_t *scratchpad)
{
    uint8_t busmask;
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
    ow_write_byte(ONEWIRE_BUSMASK, OW_FUNC_READ_SP);

#if ONEWIRE_BUSCOUNT > 1
    for (uint8_t bus = 0; bus < ONEWIRE_BUSCOUNT; bus++) {
        /* read 9 bytes from each onewire bus */
        busmask = (uint8_t)(1 << (bus + ONEWIRE_STARTPIN));
#else
        busmask = ONEWIRE_BUSMASK;
#endif
        for (uint8_t i = 0; i < 9; i++) {
            scratchpad->bytewise[i] = ow_read_byte(busmask);
        }

        /* check CRC (last byte) */
        if (scratchpad->crc == crc_checksum(&scratchpad->bytewise, 8)) {
            /* return if we got a valid response from one device */
            return 1;
        }
#if ONEWIRE_BUSCOUNT > 1
    }
#endif

    return -2;
}

int8_t ow_temp_power(ow_rom_code_t *rom)
{
#if ONEWIRE_BUSCOUNT > 1
    uint8_t busmask = 1 << (ONEWIRE_STARTPIN); // FIXME: currently only on 1st bus
#else
    uint8_t busmask = ONEWIRE_BUSMASK;
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
    ow_write_byte(busmask, OW_FUNC_READ_POWER);

    return (int8_t)(ow_read(busmask));
}

int16_t ow_temp_normalize(ow_rom_code_t *rom, ow_temp_scratchpad_t *sp)
{
    if (rom->family == OW_FAMILY_DS1820)
        return (int16_t)((sp->temperature & 0xfffe) << 7) - 0x40 + (((sp->count_per_c - sp->count_remain) << 8) / sp->count_per_c);
    else if (rom->family == OW_FAMILY_DS1822 || rom->family == OW_FAMILY_DS18B20)
        return (int16_t)(sp->temperature << 4);
    else
        return -1;
}

/*
 * DS2502 data functions
 */

int8_t ow_eeprom(ow_rom_code_t *rom)
{
    /* check for known family code */
    if (rom->family == OW_FAMILY_DS2502E48)
        return 1;

    return 0;
}

#ifdef ONEWIRE_DS2502_SUPPORT

int8_t ow_eeprom_read(ow_rom_code_t *rom, void *data)
{
#if ONEWIRE_BUSCOUNT > 1
    uint8_t busmask = 1 << (ONEWIRE_STARTPIN); // FIXME: currently only on 1st bus
#else
    uint8_t busmask = ONEWIRE_BUSMASK;
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
    ow_write_byte(busmask, OW_FUNC_READ_MEMORY);

    /* transmit address (mac address starts at offset 5 */
    ow_write_byte(busmask, 5);
    ow_write_byte(busmask, 0);

    /* read back crc sum of the command */
    uint8_t crc = ow_read_byte(busmask);

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
        *p-- = ow_read_byte(busmask);

    return 0;
}

#endif /* ONEWIRE_DS2502_SUPPORT */
#ifdef ONEWIRE_POLLING_SUPPORT
ow_sensor_t ow_sensors[OW_SENSORS_COUNT] = {{{{0}},0,0,0,0,0}};

static int8_t
ow_discover_sensor(void)
{
	uint8_t firstonbus = 0;
	int8_t ret=0;
	do
	{
		if (ow_global.lock == 0) {
			firstonbus = 1;
#if ONEWIRE_BUSCOUNT > 1
			ow_global.bus = 0;
#endif
#ifdef DEBUG_OW_POLLING
			debug_printf("Starting initial discovery\n");
#endif
			/*Prepare existing sensors*/
			for(uint8_t i=0;i<OW_SENSORS_COUNT;i++)
			{
				ow_sensors[i].present=0;
			}
		} else {
#ifdef DEBUG_OW_POLLING
			debug_printf("Staring another discovery\n");
#endif
			firstonbus = 0;
		}

#if ONEWIRE_BUSCOUNT > 1
		do
		{
#endif
			ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
			{
#if ONEWIRE_BUSCOUNT > 1
				ret = ow_search_rom((uint8_t)(1 << (ow_global.bus + ONEWIRE_STARTPIN)), firstonbus);
#else
				ret = ow_search_rom(ONEWIRE_BUSMASK, firstonbus);
#endif
			}

			/* make sure only one conversion happens at a time */
			ow_global.lock = 1;

			if(ret == 1) {
#ifdef DEBUG_OW_POLLING
					debug_printf("discovered device %02x %02x %02x %02x %02x %02x %02x %02x"
#if ONEWIRE_BUSCOUNT > 1
							" on bus %d"
#endif
							"\n",
							ow_global.current_rom.bytewise[0],
							ow_global.current_rom.bytewise[1],
							ow_global.current_rom.bytewise[2],
							ow_global.current_rom.bytewise[3],
							ow_global.current_rom.bytewise[4],
							ow_global.current_rom.bytewise[5],
							ow_global.current_rom.bytewise[6],
							ow_global.current_rom.bytewise[7]
#if ONEWIRE_BUSCOUNT > 1
							,ow_global.bus
#endif
							);
#endif
				uint8_t already_in=0;
				/*Determine whether this sensor is already present in our list*/
				for(uint8_t i=0;i<OW_SENSORS_COUNT;i++)
				{
					if(ow_global.current_rom.raw == ow_sensors[i].ow_rom_code.raw)
					{
						ow_sensors[i].present=1;
						already_in=1;
						/*We skip everything else to retain a regular update rate*/
						break;
					}

				}
				if(already_in == 0)
				{
					/*The sensor we found is not in our list, so we search for the first free sensor slot, e.g. the first slot where ow_rom_code is zero*/
					for(uint8_t i=0;i<OW_SENSORS_COUNT;i++)
					{
						if(ow_sensors[i].ow_rom_code.raw == 0)
						{
							/* We found a free slot...storing*/
							ow_sensors[i].ow_rom_code.raw=ow_global.current_rom.raw;
							ow_sensors[i].present=1;
							ow_sensors[i].read_delay=1; /*Read temperature asap - note: we will check for eeprom later*/
							break;
						}
					}
				}
			}
		} while (ret > 0);
#if ONEWIRE_BUSCOUNT > 1
		ow_global.bus++;
		firstonbus = 1;
	} while (ow_global.bus < ONEWIRE_BUSCOUNT - 1);
#endif
	ow_global.lock = 0;
	/*We finished the discovery process. Now we delete all removed sensors*/
	for(uint8_t i=0;i<OW_SENSORS_COUNT;i++)
	{
		/*Mark the slot as free*/
		if(ow_sensors[i].present == 0)
			ow_sensors[i].ow_rom_code.raw=0;
	}
	return 0;
}

/*This function will be called every 800 ms*/
void
ow_periodic(void)
{
	/*At startup we want an immediate discovery*/
	static uint16_t discover_delay=3;
	if(--discover_delay == 0)
	{
		discover_delay=OW_DISCOVER_DELAY;
		ow_discover_sensor();
#ifdef DEBUG_OW_POLLING
		for(uint8_t i=0, k=0;i<OW_SENSORS_COUNT;i++)
		{
			if(ow_sensors[i].ow_rom_code.raw != 0)
			{
				debug_printf("sensor #%d in list is: %02x %02x %02x %02x %02x %02x %02x %02x\n",
						++k,
						ow_sensors[i].ow_rom_code.bytewise[0],
						ow_sensors[i].ow_rom_code.bytewise[1],
						ow_sensors[i].ow_rom_code.bytewise[2],
						ow_sensors[i].ow_rom_code.bytewise[3],
						ow_sensors[i].ow_rom_code.bytewise[4],
						ow_sensors[i].ow_rom_code.bytewise[5],
						ow_sensors[i].ow_rom_code.bytewise[6],
						ow_sensors[i].ow_rom_code.bytewise[7]);
			}
		}
#endif
	}
	for(uint8_t i=0;i<OW_SENSORS_COUNT;i++)
	{
		if(ow_temp_sensor(&ow_sensors[i].ow_rom_code))
		{
			if(ow_sensors[i].converted == 1)
			{
				if(ow_sensors[i].convert_delay == 1)
					ow_sensors[i].convert_delay = 0;
				else
				{
#ifdef DEBUG_OW_POLLING
					debug_printf("reading temperature\n");
#endif
					int8_t ret;
					ow_temp_scratchpad_t sp;
					ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
					{
						ret = ow_temp_read_scratchpad(&ow_sensors[i].ow_rom_code, &sp);
					}
					if (ret != 1) {
#ifdef DEBUG_OW_POLLING
						debug_printf("scratchpad read failed: %d\n", ret);
#endif
						return;
					}
#ifdef DEBUG_OW_POLLING
					debug_printf("scratchpad read succeeded\n");
#endif
					int16_t temp = ow_temp_normalize(&ow_sensors[i].ow_rom_code, &sp);
#ifdef DEBUG_OW_POLLING
					debug_printf("temperature: %d.%d\n", HI8(temp), LO8(temp) > 0 ? 5 : 0);
#endif
					ow_sensors[i].temp=((int8_t) HI8(temp)) * 10 + HI8(((temp & 0x00ff) * 10) + 0x80);
					ow_sensors[i].converted = 0;
				}
			}
			if(--ow_sensors[i].read_delay == 0 && ow_sensors[i].converted == 0)
			{
				ow_sensors[i].read_delay=OW_READ_DELAY;
				ow_temp_start_convert_nowait(&ow_sensors[i].ow_rom_code);
				ow_sensors[i].convert_delay=1;
				ow_sensors[i].converted=1;
			}
		}
	}
}
#endif

/*
  -- Ethersex META --
  header(hardware/onewire/onewire.h)
  init(onewire_init)
  ifdef(`conf_ONEWIRE_POLLING',`timer(40, ow_periodic())')
*/
