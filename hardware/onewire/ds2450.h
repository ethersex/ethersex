/*
 * Support for ADC DS2450
 * real hardware access - header file
 * Copyright (C) 2009 Meinhard Schneider <meini@meini.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */


#ifndef _DS2450_H
#define _DS2450_H

#include "onewire.h"

// XXX
#define noinline __attribute__((noinline))


/* Debugging */
#ifdef DEBUG_OW_DS2450_CORE
#include "core/debug.h"
#define DS2450_CORE_DEBUG(str...) debug_printf("DS2450: " str)
#else
#define DS2450_CORE_DEBUG(...)    ((void) 0)
#endif


#ifdef DEBUG_OW_DS2450_ECMD
#include "core/debug.h"
#define DS2450_ECMD_DEBUG(str...) debug_printf("DS2450: " str)
#else
#define DS2450_ECMD_DEBUG(...)    ((void) 0)
#endif


/* 1wire family definition for DS2450 */
#define OW_DS2450_FAMILY 0x20


/* memory map pages */
#define OW_DS2450_SECOND_MEM_ADDR 0x00


/* channel offset for A, B, C and D */
#define OW_DS2450_A_OFFSET 0
#define OW_DS2450_B_OFFSET 2
#define OW_DS2450_C_OFFSET 4
#define OW_DS2450_D_OFFSET 6


/* page 0 (conversion read-out)
 * (addresses for channel B, C and D can be computed from channel offsets) */
#define OW_DS2450_ADC_A_LSB 0x00
#define OW_DS2450_ADC_A_MSB 0x01


/* page 1 (control/status data)
 * (addresses for channel B, C and D can be computed from channel offsets) */
#define OW_DS2450_RC_A 0x08
#define OW_DS2450_RC0_A 0x08
#define OW_DS2450_RC1_A 0x08
#define OW_DS2450_RC2_A 0x08
#define OW_DS2450_RC3_A 0x08
#define OW_DS2450_OC_A 0x08
#define OW_DS2450_OE_A 0x08

#define OW_DS2450_IR_A 0x09
#define OW_DS2450_AEL_A 0x09
#define OW_DS2450_AEH_A 0x09
#define OW_DS2450_AFL_A 0x09
#define OW_DS2450_AFH_A 0x09
#define OW_DS2450_POR_A 0x09


/* page 1 masks (common to all channels) */
#define OW_DS2450_RC_MASK 0x0f
#define OW_DS2450_RC0_MASK 0x01
#define OW_DS2450_RC1_MASK 0x02
#define OW_DS2450_RC2_MASK 0x04
#define OW_DS2450_RC3_MASK 0x08
#define OW_DS2450_OC_MASK 0x40
#define OW_DS2450_OE_MASK 0x80

#define OW_DS2450_IR_MASK 0x01
#define OW_DS2450_AEL_MASK 0x04
#define OW_DS2450_AEH_MASK 0x08
#define OW_DS2450_AFL_MASK 0x10
#define OW_DS2450_AFH_MASK 0x20
#define OW_DS2450_POR_MASK 0x80


/* page 2 (alarm settings)
 * (addresses for channel B, C and D can be computed from channel offsets) */
#define OW_DS2450_ALARM_LOW_A 0x00
#define OW_DS2450_ALARM_HIGH_A 0x01


/* page 3 (factory calibration and VCC control) */
#define OW_DS2450_VCC_POWERED 0x1c
#define OW_DS2450_VCC_POWERED_ON 0x40
#define OW_DS2450_VCC_POWERED_OFF 0x00


/* last valid memory page addresses */
#define OW_DS2450_LAST_MEMPAGE_ADDR 0x1f


/* function commands */
#define OW_DS2450_READ_MEMORY 0xaa
#define OW_DS2450_WRITE_MEMORY 0x55
#define OW_DS2450_CONVERT 0x3c


/* match ROM vs. skip ROM */
int8_t ow_match_skip_rom(ow_rom_code_t * rom);


/* check CRC16 with seed */
void ow_crc16_seed(uint8_t * b, uint8_t len, uint16_t * seed);


/* check CRC16 seed after eating all data bytes an both CRC16 bytes
 * return 0 on success (CRC16 good), -1 on failure (CRC16 bad) */
int8_t ow_crc16_check(uint16_t * seed);


/* calculates CRC16 after eating all data bytes */
uint16_t ow_crc16_calc(uint16_t * seed);


/* input for ow_crc16_seed but bytewise */
void ow_crc16_seed_bytewise(uint8_t b, uint16_t * seed);


/* check sensor family againt DS2450 */
uint8_t ow_ds2450_sensor(ow_rom_code_t * rom);


/* get AD conversion resultion mask bits */
int8_t ow_ds2450_res_get(ow_rom_code_t * rom, uint8_t channel);


/* set AD conversion resultion mask bits */
int8_t ow_ds2450_res_set(ow_rom_code_t * rom, uint8_t channel, uint8_t res);


/* get output control bit */
int8_t ow_ds2450_oc_get(ow_rom_code_t * rom, uint8_t channel);


/* set output control bit */
int8_t ow_ds2450_oc_set(ow_rom_code_t * rom, uint8_t channel, uint8_t oc);


/* get output enable bit */
int8_t ow_ds2450_oe_get(ow_rom_code_t * rom, uint8_t channel);


/* set output enable bit */
int8_t ow_ds2450_oe_set(ow_rom_code_t * rom, uint8_t channel, uint8_t oe);


/* get AD conversion input voltage range (2.55/5.10V) bit */
int8_t ow_ds2450_range_get(ow_rom_code_t * rom, uint8_t channel);


/* set AD conversion input voltage range (2.55/5.10V) bit */
int8_t ow_ds2450_range_set(ow_rom_code_t * rom, uint8_t channel,
                           uint8_t range);


/* get power on reset bit */
int8_t ow_ds2450_por_get(ow_rom_code_t * rom, uint8_t channel);


/* set power on reset bit */
int8_t ow_ds2450_por_set(ow_rom_code_t * rom, uint8_t channel, uint8_t por);


/* get power mode */
int8_t ow_ds2450_power_get(ow_rom_code_t * rom);


/* set power mode */
int8_t ow_ds2450_power_set(ow_rom_code_t * rom, uint8_t vcc_powered);


/* do AD conversion */
int8_t ow_ds2450_convert(ow_rom_code_t * rom, uint8_t input_select,
                         uint8_t readout);


/* get AD conversion result for one or more channels */
int8_t ow_ds2450_get(ow_rom_code_t * rom, uint8_t channel_start,
                     uint8_t channel_stop, uint16_t * res);


/* read a memory page beginning from given address */
int8_t ow_ds2450_mempage_read(ow_rom_code_t * rom,
                              const int8_t mempage,
                              const uint8_t len, uint8_t * mem);


/* write a memory page beginning from given address */
int8_t ow_ds2450_mempage_write(ow_rom_code_t * rom, int8_t mempage,
                               uint8_t len, uint8_t * mem);
#endif
