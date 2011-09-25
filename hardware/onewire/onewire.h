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

#define LOW(x) ((uint8_t)(x))
#define HIGH(x) ((uint8_t)((x) >> 8))

/* rom commands */
#define OW_ROM_SEARCH_ROM 0xF0
#define OW_ROM_READ_ROM 0x33
#define OW_ROM_MATCH_ROM 0x55
#define OW_ROM_SKIP_ROM 0xCC
#define OW_ROM_ALARM_SEARCH 0xEC

/* families */
#define OW_FAMILY_DS1820 0x10
#define OW_FAMILY_DS18B20 0x28
#define OW_FAMILY_DS1822 0x22
#define OW_FAMILY_DS2502E48 0x89
#define OW_FAMILY_DS2502 0x09

/* ds18s20 functions commands */

/* temperature */
#define OW_FUNC_CONVERT 0x44
#define OW_FUNC_WRITE_SP 0x4E
#define OW_FUNC_READ_SP 0xBE
#define OW_FUNC_COPY_SP 0x48
#define OW_FUNC_RECALL_EE 0xB8
#define OW_FUNC_READ_POWER 0xB4

/* data (only reading data is supported so far) */
#define OW_FUNC_READ_MEMORY 0xF0
#define OW_FUNC_READ_STATUS 0xAA
#define OW_FUNC_READ_DATA_CRC 0xC3

/* */

/* timing constants */
/* onewire needs a 480us reset timeout, suitable for use with the delay_loop() functions */
#define OW_RESET_TIMEOUT_1 (F_CPU / 1000000 * 480 / 4)
/* we wait 90us before sampling data for the low-pulse detection */
#define OW_RESET_TIMEOUT_2 (F_CPU / 1000000 * 90 / 4)
/* and wait 480us - 90us = 390us before initiating the next onewire command */
#define OW_RESET_TIMEOUT_3 (F_CPU / 1000000 * 390 / 4)

#if OW_RESET_TIMEOUT_1 > 65535
#error "OW_CONFIG_TIMEOUT_1 bigger than 64k?!"
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
/* */

/* macros */
#define OW_CONFIG_INPUT(busmask)			\
  /* enable pullup */					\
  ONEWIRE_PORT = (uint8_t)(ONEWIRE_PORT | busmask);	\
  /* configure as input */				\
  ONEWIRE_DDR = (uint8_t)(ONEWIRE_DDR & (uint8_t)~busmask);

#define OW_CONFIG_OUTPUT(busmask)			\
  /* configure as output */				\
  ONEWIRE_DDR = (uint8_t)(ONEWIRE_DDR | busmask);

#define OW_LOW(busmask)					\
  /* drive pin low */					\
  ONEWIRE_PORT = (uint8_t)(ONEWIRE_PORT & (uint8_t)~busmask);

#define OW_HIGH(busmask)				\
  /* drive pin high */					\
  ONEWIRE_PORT = (uint8_t)(ONEWIRE_PORT | busmask);

#define OW_PULLUP(busmask)				\
  /* pull up resistor */				\
  ONEWIRE_PORT = (uint8_t)(ONEWIRE_PORT | busmask);

#define OW_GET_INPUT(busmask)				\
  (ONEWIRE_PIN & busmask)

/* symbolic names for the restriction of the list comamnd to certain types.
 * These values are used only to filter the output of the list command */
#define OW_LIST_TYPE_ALL		0
#define OW_LIST_TYPE_TEMP_SENSOR	1
#define OW_LIST_TYPE_EEPROM		2


/* structures */
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

struct ow_temp_scratchpad_t {
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
/*Polling Support*/
#ifdef ONEWIRE_POLLING_SUPPORT
struct ow_sensor_t {
	/*May be just store the code and calculate the crc with each call + hardcode the family, would save 16bytes*/
	struct ow_rom_code_t ow_rom_code;
	/*We just store the temperature in order to keep memory footfrint as low as possible. We store in deci degrees (DD) => 36.4° == 364*/
	int16_t temp;
	uint16_t read_delay;   /*time between polling the sensor*/
	uint8_t convert_delay; /*we need to wait 800ms for the sensor to convert the temperatures*/
	uint8_t converted; /*when this is set, we will wait convert_delay to be 0 and then read the scratchpad*/
	uint8_t present; /*this is set during discovery - all sensors with present == 0 will be deleted after the discovery*/
};
/* */

extern struct ow_sensor_t ow_sensors[OW_SENSORS_COUNT];
extern uint16_t discover_delay;
#endif

/* global variables */
struct ow_global_t {
    uint8_t lock;
    int8_t last_discrepancy;
#ifdef ONEWIRE_DS2502_SUPPORT
    int8_t list_type;
#endif
    struct ow_rom_code_t current_rom;
#if ONEWIRE_BUSCOUNT > 1
    uint8_t bus;
#endif
};

extern struct ow_global_t ow_global;

/* prototypes */
void onewire_init(void);

/* low level functions */
uint8_t reset_onewire(uint8_t busmask);
void ow_write_0(uint8_t busmask);
#define ow_write_1(busmask) ow_read(busmask)

void ow_write(uint8_t busmask, uint8_t value);
void ow_write_byte(uint8_t busmask, uint8_t value);
uint8_t ow_read(uint8_t busmask);
uint8_t ow_read_byte(uint8_t busmask);

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

/* detect rom codes on the onewire bus. call ow_search_rom_first() for initial
 * search, ow_search_rom_next() for next device, until 0 is returned.
 *
 * return values:
 *    0: no more devices on the bus
 *    1: next device id has been placed in ow_global.current_rom
 *   -1: no presence pulse has been detected, no device connected?
 */
#define ow_search_rom_first(busmask) ow_search_rom(busmask,1)
#define ow_search_rom_next(busmask) ow_search_rom(busmask, 0)
int8_t ow_search_rom(uint8_t busmask, uint8_t first);


/*
 *
 * temperature functions (DS1820, DS1822)
 *
 */

/* check if a node is a temperature sensor
 *
 * return values:
 *  0: other node
 *  1: temperature sensor (DS1820 or DS1822)
 */
int8_t ow_temp_sensor(struct ow_rom_code_t *rom);

/* start temperature conversion on sensor with given id, or to all sensors (via
 * skip_rom) if NULL.  If wait is set, busy-loop until conversion is done on all
 * adressed sensors.  If rom is not NULL, the rom family code is checked for known
 * temperature sensors.
 *
 * return values:
 *    0: conversion initiated (but not done)
 *    1: conversion done
 *   -1: no presence pulse has been detected, no device connected?
 *   -2: family code is unknown
 */
#define ow_temp_start_convert_wait(rom) ow_temp_start_convert(rom, 1)
#define ow_temp_start_convert_nowait(rom) ow_temp_start_convert(rom, 0)
int8_t ow_temp_start_convert(struct ow_rom_code_t *rom, uint8_t wait);

/* read scratchpad memory of sensor with given id (may be null, if only one
 * sensor is connected). If rom is not NULL, the rom family code is checked for
 * known temperature sensors.
 *
 * return values:
 *    1: memory read
 *   -1: no presence pulse has been detected, no device connected?
 *   -2: crc check failed, multiple devices on the same bus?
 *   -3: family code is unknown
 */
int8_t ow_temp_read_scratchpad(struct ow_rom_code_t *rom, struct ow_temp_scratchpad_t *scratchpad);

/* check for parasite powered devices, if rom is NULL, all devices are queried.
 *
 * return values:
 *    0: device(s) parasite powered
 *    1: device(s) external powered
 *   -1: no presence pulse has been detected, no device connected?
 *   -2: given rom code is no temperature sensor
 */
int8_t ow_temp_power(struct ow_rom_code_t *rom);


/* return normalized temperature for device
 *
 * return values:
 * int16_t, 8.8 fixpoint value
 * 0xffff on error (eg unknown device)
 */
int16_t ow_temp_normalize(struct ow_rom_code_t *rom, struct ow_temp_scratchpad_t *sp);


/*
 *
 * DS2502 data functions
 *
 */

/* check if a node is an eeprom
 *
 * return values:
 *  0: other node
 *  1: eeprom
 */
int8_t ow_eeprom(struct ow_rom_code_t *rom);

/* read 6 bit (48 byte) of eeprom memory
 *
 * return values:
 * 0: read successful
 * -1: no device responded
 * -2: crc error
 * -3: unknown rom family code
 */
int8_t ow_eeprom_read(struct ow_rom_code_t *rom, void *data);

/*
 *
 * ECMD functions
 *
 */

/* parse an onewire rom address in cmd string
 *
 * *rom: contains parsed rom adress after sucessul parsing
 *
 * return values:
 * 0: parsing successful
 * -1: string could not be parsed
 */
int8_t parse_ow_rom(char *cmd, struct ow_rom_code_t *rom);


/* list onewiredevices of requested type on all OW-buses */
int16_t parse_cmd_onewire_list(char *cmd, char *output, uint16_t len);


/* get temperature of specifued OW-device */
int16_t parse_cmd_onewire_get(char *cmd, char *output, uint16_t len);


/* issue temperatur convert command on all OW-buses */
int16_t parse_cmd_onewire_convert(char *cmd, char *output, uint16_t len);

/* Polling functions*/
int8_t ow_discover_sensor();
void ow_periodic();


#endif /* ONEWIRE_SUPPORT */

#endif /* ONEWIRE_H */
