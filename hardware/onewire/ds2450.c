/*
 * Support for ADC DS2450
 * real hardware access
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
 * this program; ifnot, write to the Free Software Foundation, Inc., 51 Franklin
 * Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */


#include <avr/io.h>
#include <stdlib.h>
#include <util/crc16.h>
#include <util/delay.h>

#include "config.h"
#include "core/eeprom.h"

#include "ds2450.h"
#include "onewire.h"


/*
 * Missing get- and set-functions for:
 * - alarm enable low (AEL)
 * - alarm enable high (AEH)
 * - alarm flag low (AFL)
 * - alarm flag high (AFH)
 * - alarm treshold voltages (high and low)
 *
 * Missing functions for
 * - conditional search
*/


/* match ROM vs. skip ROM */
int8_t noinline
ow_match_skip_rom(ow_rom_code_t * rom)
{
  int8_t ret;

  if (rom == NULL)
  {
    DS2450_CORE_DEBUG
      ("ow_match_skip_rom: rom == NULL, using skip command.\n");
    ret = ow_skip_rom();
  }
  else
  {
    if (!ow_ds2450_sensor(rom))
    {
      DS2450_CORE_DEBUG("ow_match_skip_rom: family code mismatch!\n");
      ret = -3;
    }
    else
    {
      DS2450_CORE_DEBUG
        ("ow_match_skip_rom: rom != NULL, using match command.\n");
      ret = ow_match_rom(rom);
    }
  }

  DS2450_CORE_DEBUG("ow_match_skip_rom: returning: %i.\n", ret);

  return ret;
}


/* check CRC16 with seed
 *
 * starting value for seed: 0
 *
 * input for just checking CRC16:
 * - all data bytes (in order they were send/received)
 * - two CRC16 bytes (in order they were received)
 * -> seed should result in 0xB001 ifeverything is fine!
 *     (check with ow_crc16_check)
 *
 * input for calculation CRC16:
 * - all data bytes (in order they were send/received)
 * -> CRC16 is stored as complement in seed!
 *    (use ow_crc16_calc to convert)
 */
void noinline
ow_crc16_seed(uint8_t * b, uint8_t len, uint16_t * seed)
{
  uint8_t i, j;

  for (i = 0; i < len; ++i)
  {
    DS2450_CORE_DEBUG
      ("ow_crc16_seed: byte: %02x, CRC16 starting seed: %04x\n", b[i], *seed);
    *seed ^= b[i];
    for (j = 0; j < 8; ++j)
    {
      if (*seed & 1)
        *seed = (*seed >> 1) ^ 0xA001;
      else
        *seed = (*seed >> 1);
    }
    DS2450_CORE_DEBUG("DS2450:\tCRC16 ending seed: %04x.\n", *seed);
  }
}


/* check CRC16 seed after eating all data bytes an both CRC16 bytes
 * return 0 on success (CRC16 good), -1 on failure (CRC16 bad) */
int8_t noinline
ow_crc16_check(uint16_t * seed)
{
  if (*seed == 0xB001)
  {
    /* good */
    DS2450_CORE_DEBUG("ow_crc16_check: CRC16 good!\n");
    return 0;
  }
  else
  {
    /* bad */
    DS2450_CORE_DEBUG("ow_crc16_check: CRC16 bad!\n");
    return -1;
  }
}


/* calculates CRC16 after eating all data bytes */
uint16_t noinline
ow_crc16_calc(uint16_t * seed)
{
  DS2450_CORE_DEBUG
    ("ow_crc16_calc: CRC16 seed: %04x, CRC16 seed complement: %04x.\n", *seed,
     ~(*seed));
  return ~(*seed);
}


/* input for ow_crc16_seed but bytewise */
void noinline
ow_crc16_seed_bytewise(uint8_t b, uint16_t * seed)
{
  ow_crc16_seed(&b, 1, seed);
}


/* check sensor family againt DS2450 */
uint8_t noinline
ow_ds2450_sensor(ow_rom_code_t * rom)
{
  if (rom->family == OW_DS2450_FAMILY)
  {
    DS2450_CORE_DEBUG("ow_ds2450_sensor: family code matched.\n");
    return 1;
  }

  DS2450_CORE_DEBUG("ow_ds2450_sensor: family code (%02x) mismatch.\n",
                    rom->family);

  return 0;
}


/* get AD conversion resultion mask bits */
int8_t
ow_ds2450_res_get(ow_rom_code_t * rom, uint8_t channel)
{
  int8_t ret;
  uint8_t b;

  ret = ow_ds2450_mempage_read(rom, OW_DS2450_RC0_A + channel * 2, 1, &b);

  if (ret != 1)
    return -2;

  /* check returned memory page data */

  return b & OW_DS2450_RC_MASK;
}


/* set AD conversion resultion mask bits */
int8_t
ow_ds2450_res_set(ow_rom_code_t * rom, uint8_t channel, uint8_t res)
{
  int8_t ret;
  uint8_t b;

  /* we need to read the corresponding status byte since we want only modify
   * the last four bits of this byte, the rest should be unchanged */
  ret = ow_ds2450_mempage_read(rom, OW_DS2450_RC0_A + channel * 2, 1, &b);

  if (ret != 1)
    return -2;

  /* overwrite with given result bits */
  res &= OW_DS2450_RC_MASK;
  b &= ~OW_DS2450_RC_MASK;
  b |= res;

  ret = ow_ds2450_mempage_write(rom, OW_DS2450_RC0_A + channel * 2, 1, &b);

  if (ret != 1)
    return -2;

  return 0;
}


/* get output control bit */
int8_t
ow_ds2450_oc_get(ow_rom_code_t * rom, uint8_t channel)
{
  int8_t ret;
  uint8_t b;

  ret = ow_ds2450_mempage_read(rom, OW_DS2450_OC_A + channel * 2, 1, &b);

  if (ret != 1)
    return -2;

  /* check returned memory page data */
  if (b & OW_DS2450_OC_MASK)
    return 1;

  return 0;
}


/* set output control bit */
/* (oc == 0 => make output transistor conducting) */
/* (oc == 1 => make output transistor non-conducting) */
int8_t
ow_ds2450_oc_set(ow_rom_code_t * rom, uint8_t channel, uint8_t oc)
{
  int8_t ret;
  uint8_t b;

  /* we need to read the corresponding status byte since we want only modify
   * the last bit of this byte, the rest should be unchanged */
  ret = ow_ds2450_mempage_read(rom, OW_DS2450_OC_A + channel * 2, 1, &b);

  if (ret != 1)
    return -2;

  if (oc)
    oc = 0xff;
  else
    oc = 0x00;

  /* overwrite with given output control bit */
  oc &= OW_DS2450_OC_MASK;
  b &= ~OW_DS2450_OC_MASK;
  b |= oc;

  ret = ow_ds2450_mempage_write(rom, OW_DS2450_OC_A + channel * 2, 1, &b);

  if (ret != 1)
    return -2;

  return 0;
}


/* get output enable bit */
int8_t
ow_ds2450_oe_get(ow_rom_code_t * rom, uint8_t channel)
{
  int8_t ret;
  uint8_t b;

  ret = ow_ds2450_mempage_read(rom, OW_DS2450_OE_A + channel * 2, 1, &b);

  if (ret != 1)
    return -2;

  /* check returned memory page data */
  if (b & OW_DS2450_OE_MASK)
    return 1;

  return 0;
}


/* set output enable bit */
/* (oe == 0 => use channel for AD conversion) */
/* (oe == 1 => use channel as output channel) */
int8_t
ow_ds2450_oe_set(ow_rom_code_t * rom, uint8_t channel, uint8_t oe)
{
  int8_t ret;
  uint8_t b;

  /* we need to read the corresponding status byte since we want only modify
   * the last bit of this byte, the rest should be unchanged */
  ret = ow_ds2450_mempage_read(rom, OW_DS2450_OE_A + channel * 2, 1, &b);

  if (ret != 1)
    return -2;

  if (oe)
    oe = 0xff;
  else
    oe = 0x00;

  /* overwrite with given output control bit */
  oe &= OW_DS2450_OE_MASK;
  b &= ~OW_DS2450_OE_MASK;
  b |= oe;

  ret = ow_ds2450_mempage_write(rom, OW_DS2450_OE_A + channel * 2, 1, &b);

  if (ret != 1)
    return -2;

  return 0;
}


/* get AD conversion input voltage range (2.55/5.10V) bit */
int8_t
ow_ds2450_range_get(ow_rom_code_t * rom, uint8_t channel)
{
  int8_t ret;
  uint8_t b;

  ret = ow_ds2450_mempage_read(rom, OW_DS2450_IR_A + channel * 2, 1, &b);

  if (ret != 1)
    return -2;

  /* check returned memory page data */
  if (b & OW_DS2450_IR_MASK)
    return 1;

  return 0;
}


/* set AD conversion input voltage range (2.55/5.10V) bit */
/* (range == 0 => 2.55V) */
/* (range == 1 => 5.10V) */
int8_t
ow_ds2450_range_set(ow_rom_code_t * rom, uint8_t channel, uint8_t range)
{
  int8_t ret;
  uint8_t b;

  /* we need to read the corresponding status byte since we want only modify
   * the last bit of this byte, the rest should be unchanged */
  ret = ow_ds2450_mempage_read(rom, OW_DS2450_IR_A + channel * 2, 1, &b);

  if (ret != 1)
    return -2;

  /* overwrite with given range bit */
  range &= OW_DS2450_IR_MASK;
  b &= ~OW_DS2450_IR_MASK;
  b |= range;

  ret = ow_ds2450_mempage_write(rom, OW_DS2450_IR_A + channel * 2, 1, &b);

  if (ret != 1)
    return -2;

  return 0;
}


/* get power on reset bit */
int8_t
ow_ds2450_por_get(ow_rom_code_t * rom, uint8_t channel)
{
  int8_t ret;
  uint8_t b;

  ret = ow_ds2450_mempage_read(rom, OW_DS2450_POR_A + channel * 2, 1, &b);

  if (ret != 1)
    return -2;

  /* check returned memory page data */
  if (b & OW_DS2450_POR_MASK)
    return 1;

  return 0;
}


/* set power on reset bit */
/* (por == 0 => do not respond to conditional search command) */
/* (por == 1 => respond to conditional search command) */
int8_t
ow_ds2450_por_set(ow_rom_code_t * rom, uint8_t channel, uint8_t por)
{
  int8_t ret;
  uint8_t b;

  /* we need to read the corresponding status byte since we want only modify
   * the last bit of this byte, the rest should be unchanged */
  ret = ow_ds2450_mempage_read(rom, OW_DS2450_POR_A + channel * 2, 1, &b);

  if (ret != 1)
    return -2;

  /* overwrite with given output control bit */
  por &= OW_DS2450_POR_MASK;
  b &= ~OW_DS2450_POR_MASK;
  b |= por;

  ret = ow_ds2450_mempage_write(rom, OW_DS2450_POR_A + channel * 2, 1, &b);

  if (ret != 1)
    return -2;

  return 0;
}


/* get power mode */
int8_t
ow_ds2450_power_get(ow_rom_code_t * rom)
{
  int8_t ret;
  uint8_t b;

  ret = ow_ds2450_mempage_read(rom, OW_DS2450_VCC_POWERED, 1, &b);

  if (ret != 1)
    return -2;

  /* check returned memory page data */
  if (b == OW_DS2450_VCC_POWERED_ON)
  {
    DS2450_CORE_DEBUG("ow_ds2450_power_get: VCC power: ON\n");
    return 1;
  }
  else if (b == OW_DS2450_VCC_POWERED_OFF)
  {
    DS2450_CORE_DEBUG("ow_ds2450_power_get: VCC power: OFF\n");
    return 0;
  }

  DS2450_CORE_DEBUG("ow_ds2450_power_get: VCC power: unknown value: %02x!\n",
                    b);
  return -2;
}


/* set power mode */
int8_t
ow_ds2450_power_set(ow_rom_code_t * rom, uint8_t vcc_powered)
{
  int8_t ret;

  if (vcc_powered)
    vcc_powered = OW_DS2450_VCC_POWERED_ON;
  else
    vcc_powered = OW_DS2450_VCC_POWERED_OFF;

  ret = ow_ds2450_mempage_write(rom, OW_DS2450_VCC_POWERED, 1, &vcc_powered);

  if (ret != 1)
    return -2;

  return 0;
}


/* do AD conversion */
int8_t
ow_ds2450_convert(ow_rom_code_t * rom, uint8_t input_select, uint8_t readout)
{
  // FIXME: currently only on 1st bus
  uint8_t mask = 1 << (ONEWIRE_STARTPIN);
  int8_t ret;
  uint16_t seed = 0;

  /* match ROM vs. skip ROM */
  ret = ow_match_skip_rom(rom);
  if (ret < 0)
  {
    return ret;
  }

  ow_write_byte(mask, OW_DS2450_CONVERT);
  ow_crc16_seed_bytewise(OW_DS2450_CONVERT, &seed);
  ow_write_byte(mask, input_select);
  ow_crc16_seed_bytewise(input_select, &seed);
  ow_write_byte(mask, readout);
  ow_crc16_seed_bytewise(readout, &seed);

  /* read CRC16 */
  ow_crc16_seed_bytewise(ow_read_byte(mask), &seed);
  ow_crc16_seed_bytewise(ow_read_byte(mask), &seed);

  /* check CRC16 */
  if (ow_crc16_check(&seed) < 0)
  {
    DS2450_CORE_DEBUG("ow_ds2450_convert: CRC16 invalid!\n");
    return -2;
  }

  return 0;
}


/* get AD conversion result for one or more channels
 *
 * channel_start is first channel to be read
 * channel_stop is last channel to be read
 * (evident: channel_start <= channel_stop)
 * res is loaded with ADC results (2 bytes per channel)
 * returns the number of channels processed
 */
int8_t
ow_ds2450_get(ow_rom_code_t * rom, uint8_t channel_start,
              uint8_t channel_stop, uint16_t * res)
{
  int8_t ret;
  int8_t num_channels = channel_stop - channel_start;
  uint8_t i;
  uint8_t *b;

  /* increment num_channels before comparing, so that channel_start = 0
   * and channel_stop = 0 (which should lead to just one channel -
   * A - gets read) will lead to num_channels = 1 */
  if (++num_channels <= 0 || num_channels > 4)
    return -4;

  b = __builtin_alloca(sizeof(uint8_t) * num_channels * 2);

#ifndef TEENSY_SUPPORT
  /* check if malloc did fine */
  if (!b)
  {
    DS2450_CORE_DEBUG
      ("ow_ds2450_get: malloc did not return memory pointer!\n");
    return -4;
  }
#endif

  ret =
    ow_ds2450_mempage_read(rom, OW_DS2450_ADC_A_LSB + channel_start * 2,
                           num_channels * 2, b);

  if (ret != num_channels * 2)
  {
    return -2;
  }

  for (i = 0; i < num_channels; ++i)
  {
    /* transform two 8 bit values (LSB and MSB) to one 16 bit value */
    res[i] = (b[(2 * i) + 1] << 8) + b[(2 * i) + 0];

    DS2450_CORE_DEBUG
      ("ow_ds2450_get: channel %c: LSB: %02x, MSB: %02x, res: %04x.\n",
       (unsigned char) i + 65, b[(2 * i) + 0], b[(2 * i) + 1], res[i]);
  }

  return num_channels;
}


/* read a memory page beginning from given address
 *
 * mempage is starting address, memory get's read until end of memory page is
 * reached (between 1 and 8 byte)
 * len counts maximal bytes read
 * mem pointer has to be big enough for the number of bytes that should be
 * read (1..8 byte)
 * returns the number of bytes successfully read, -1 on failure
 */
int8_t
ow_ds2450_mempage_read(ow_rom_code_t * rom, const int8_t mempage,
                       const uint8_t len, uint8_t * mem)
{
  // FIXME: currently only on 1st bus
  uint8_t mask = 1 << (ONEWIRE_STARTPIN);
  int8_t ret;
  uint16_t seed = 0;

  if (mempage > OW_DS2450_LAST_MEMPAGE_ADDR)
    return -4;

  /* number of bytes from requested memory page that remain */
  uint8_t bytes_remaining = 8 - (mempage % 8);

  if (len == 0 || len > bytes_remaining)
    return -4;

  /* match ROM vs. skip ROM */
  ret = ow_match_skip_rom(rom);
  if (ret < 0)
  {
    return ret;
  }

  DS2450_CORE_DEBUG
    ("ow_ds2450_mempage_read: memory page starting addr: %02x, bytes "
     "remaining: %i, len: %i.\n", mempage, bytes_remaining, len);

  ow_write_byte(mask, OW_DS2450_READ_MEMORY);
  ow_crc16_seed_bytewise(OW_DS2450_READ_MEMORY, &seed);
  ow_write_byte(mask, mempage);
  ow_crc16_seed_bytewise(mempage, &seed);
  ow_write_byte(mask, OW_DS2450_SECOND_MEM_ADDR);
  ow_crc16_seed_bytewise(OW_DS2450_SECOND_MEM_ADDR, &seed);

  /* read data from memory page */
  ret = 0;
  for (uint8_t i = 0; i < bytes_remaining; ++i)
  {
    if (i < len)
    {
      /* still reading data that should be collected */
      mem[i] = ow_read_byte(mask);

      DS2450_CORE_DEBUG("ow_ds2450_mempage_read: addr: %02x, val: %02x.\n",
                        mempage + i, mem[i]);

      /* feeding value to CRC16 calculation */
      ow_crc16_seed_bytewise(mem[i], &seed);

      ++ret;
    }
    else
    {
      /* all requested data was read, but still need to read more data to
       * calculate CRC16 correctly. just feeding value to CRC16 calculation */
#ifdef DEBUG_OW_DS2450_CORE
      uint8_t b = ow_read_byte(mask);
      ow_crc16_seed_bytewise(b, &seed);
#else
      ow_crc16_seed_bytewise(ow_read_byte(mask), &seed);
#endif

      DS2450_CORE_DEBUG
        ("ow_ds2450_mempage_read: addr: %02x, val: %02x (dropped).\n",
         mempage + i, b);
    }
  }

  /* CRC16 */
  ow_crc16_seed_bytewise(ow_read_byte(mask), &seed);
  ow_crc16_seed_bytewise(ow_read_byte(mask), &seed);

  /* check CRC16 */
  if (ow_crc16_check(&seed) < 0)
  {
    DS2450_CORE_DEBUG("ow_ds2450_mempage_read: CRC16 invalid!\n");
    return -2;
  }

  return ret;
}


/* write a memory page beginning from given address
 *
 * mempage is starting address, memory gets written until end of memory page
 * is reached (between 1 and 8 byte)
 * len bytes are written (len may not be greater then the number of bytes
 * remaining of the requested memory page)
 * mem pointer contains data to be written (1..8 byte)
 * returns the number of bytes successfully written, -1 on failure
 */
int8_t
ow_ds2450_mempage_write(ow_rom_code_t * rom, int8_t mempage, uint8_t len,
                        uint8_t * mem)
{
  // FIXME: currently only on 1st bus
  uint8_t mask = 1 << (ONEWIRE_STARTPIN);
  int8_t ret;
  uint16_t seed = 0;

  if (mempage > OW_DS2450_LAST_MEMPAGE_ADDR)
    return -4;

  /* number of bytes from requested memory page that remain */
  uint8_t bytes_remaining = 8 - (mempage % 8);

  if (len == 0 || len > bytes_remaining)
    return -4;

  /* match ROM vs. skip ROM */
  ret = ow_match_skip_rom(rom);
  if (ret < 0)
  {
    return ret;
  }

  DS2450_CORE_DEBUG
    ("ow_ds2450_mempage_write: memory page starting addr: %02x, bytes "
     "remaining: %i, len: %i.\n", mempage, bytes_remaining, len);

  ow_write_byte(mask, OW_DS2450_WRITE_MEMORY);
  ow_crc16_seed_bytewise(OW_DS2450_WRITE_MEMORY, &seed);
  ow_write_byte(mask, mempage);
  ow_crc16_seed_bytewise(mempage, &seed);
  ow_write_byte(mask, OW_DS2450_SECOND_MEM_ADDR);
  ow_crc16_seed_bytewise(OW_DS2450_SECOND_MEM_ADDR, &seed);

  /* write data to memory page */
  ret = 0;
  for (uint8_t i = 0; i < len; ++i)
  {
    DS2450_CORE_DEBUG("ow_ds2450_mempage_write: addr: %02x, val: %02x.\n",
                      mempage + i, mem[i]);

    ow_write_byte(mask, mem[i]);

    /* reset seed and feed memory page address to CRC16 calculation only if
     * first run is passed */
    if (i != 0)
    {
      seed = 0;
      ow_crc16_seed_bytewise(mempage + i, &seed);
      ow_crc16_seed_bytewise(OW_DS2450_SECOND_MEM_ADDR, &seed);
    }

    /* feeding value to CRC16 calculation */
    ow_crc16_seed_bytewise(mem[i], &seed);

    /* CRC16 */
    ow_crc16_seed_bytewise(ow_read_byte(mask), &seed);
    ow_crc16_seed_bytewise(ow_read_byte(mask), &seed);

    /* check CRC16 */
    if (ow_crc16_check(&seed) < 0)
    {
      DS2450_CORE_DEBUG("ow_ds2450_mempage_write: CRC16 invalid!\n");
      return -2;
    }

    /* read-back for simple verification */
#ifdef DEBUG_OW_DS2450_CORE
    uint8_t b = ow_read_byte(mask);
    if (b != mem[i])
#else
    if (ow_read_byte(mask) != mem[i])
#endif
    {
      DS2450_CORE_DEBUG
        ("ow_ds2450_mempage_write: read-back verification failed: wrote: "
         "%02x, read-back: %02x!\n", mem[i], b);
      return -2;
    }

    ++ret;
  }

  return ret;
}
