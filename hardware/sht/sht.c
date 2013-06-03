/*
* Read and convert data from a SHT hygro & temp sensor
*
* Copyright (c) 2010 Gerd v. Egidy <gerd@egidy.de>
* Copyright (c) 2013 Erik Kunze <ethersex@erik-kunze.de>
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 3
* of the License, or (at your option) any later version.
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

#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include <util/crc16.h>

#include "config.h"
#include "core/debug.h"

#include "sht.h"

enum firstbit_type
{
  MSB, LSB
};

static void
sht_init()
{
  uint8_t i;

  DDR_CONFIG_OUT(SHT_SCK);
  DDR_CONFIG_OUT(SHT_DATA);
  PIN_CLEAR(SHT_SCK);
  PIN_SET(SHT_DATA);

  // when sensor was just powered or reset up we need to wait first
  _delay_ms(11);

  // reset sequence to make sure the sensor really listens to us
  for (i = 0; i < 10; i++)
  {
    // double delays to keep our regular pace
    PIN_SET(SHT_SCK);
    _delay_us(2 * SHT_DELAY_US);
    PIN_CLEAR(SHT_SCK);
    _delay_us(2 * SHT_DELAY_US);
  }

  // start sequence
  PIN_SET(SHT_SCK);
  _delay_us(SHT_DELAY_US);

  PIN_CLEAR(SHT_DATA);
  _delay_us(SHT_DELAY_US);

  PIN_CLEAR(SHT_SCK);
  _delay_us(2 * SHT_DELAY_US);

  PIN_SET(SHT_SCK);
  _delay_us(SHT_DELAY_US);

  PIN_SET(SHT_DATA);
  _delay_us(SHT_DELAY_US);

  PIN_CLEAR(SHT_SCK);
  _delay_us(SHT_DELAY_US);
}

static int8_t
sht_send_byte(uint8_t data)
{
  uint8_t i;
  int8_t ret;

  // SCK must be low

  // most significant bit first
  for (i = 0; i < 8; i++, data <<= 1)
  {
    // set DATA, but only drive it low
    if (data & 0x80)
    {
      // high, let the external pullup do its job
      DDR_CONFIG_IN(SHT_DATA);
    }
    else
    {
      DDR_CONFIG_OUT(SHT_DATA);
      PIN_CLEAR(SHT_DATA);
    }

    _delay_us(SHT_DELAY_US);

    PIN_SET(SHT_SCK);

    // double delay to keep our regular pace
    _delay_us(2 * SHT_DELAY_US);

    PIN_CLEAR(SHT_SCK);
    _delay_us(SHT_DELAY_US);
  }

  // we await the ACK bit on DATA from the sensor
  DDR_CONFIG_IN(SHT_DATA);
  _delay_us(SHT_DELAY_US);

  PIN_SET(SHT_SCK);
  _delay_us(2 * SHT_DELAY_US);

  // DATA low = ACK OK
  if (PIN_HIGH(SHT_DATA))
    ret = SHT_ERR_PROTOCOL;
  else
    ret = SHT_OK;

  PIN_CLEAR(SHT_SCK);
  _delay_us(SHT_DELAY_US);

  return ret;
}

static void
sht_read_byte(uint8_t * data, enum firstbit_type fb)
{
  uint8_t i;

  // SCK must be low

  // should already be in, but be sure
  DDR_CONFIG_IN(SHT_DATA);

  // we read all 8 bits so clear target first
  *data = 0;

  for (i = 0; i < 8; i++)
  {
    PIN_SET(SHT_SCK);
    _delay_us(2 * SHT_DELAY_US);

    if (fb == MSB)
    {
      // most significant bit first
      // shift before reading: don't shift last bit
      *data <<= 1;

      if (PIN_HIGH(SHT_DATA))
        *data |= 1;
    }
    else
    {
      // least significant bit first
      // shift before reading: don't shift last bit
      *data >>= 1;

      if (PIN_HIGH(SHT_DATA))
        *data |= 0x80;
    }

    PIN_CLEAR(SHT_SCK);
    // double delay to keep our regular pace
    _delay_us(2 * SHT_DELAY_US);
  }

  // we need to pull DATA low to ACK
  DDR_CONFIG_OUT(SHT_DATA);
  PIN_CLEAR(SHT_DATA);
  _delay_us(SHT_DELAY_US);

  PIN_SET(SHT_SCK);
  _delay_us(2 * SHT_DELAY_US);

  PIN_CLEAR(SHT_SCK);
  _delay_us(SHT_DELAY_US);

  // ACK done, let DATA go high again
  DDR_CONFIG_IN(SHT_DATA);

  return;
}

// this is a variant of crc8, not compatible e.g. with
// _crc_ibutton_update() from <util/crc16.h>
static void
sht_crc8(uint8_t * crc_value, uint8_t data)
{
  uint8_t i;
  for (i = 0; i < 8; i++)
  {
    if ((*crc_value ^ data) & 0x80)
    {
      *crc_value <<= 1;
      *crc_value ^= 0x31;
    }
    else
    {
      *crc_value <<= 1;
    }

    data <<= 1;
  }
}

// timeout is in 10us steps + 5 msec
static int8_t
sht_sensor_command(uint8_t command, uint16_t * raw_data, uint16_t timeout)
{
  uint16_t i;
  int8_t ret;
  uint8_t crc_calc = 0;
  uint8_t crc_read;

  sht_init();

  ret = sht_send_byte(command);
  if (ret != SHT_OK)
    return ret;

  // the ACK bit of the write should be gone by now
  _delay_ms(5);

  // DATA should be high, when it goes low the data is ready
  for (i = 0; i < timeout; i++)
  {
    if (!PIN_HIGH(SHT_DATA))
      break;
    _delay_us(10);
  }

  if (PIN_HIGH(SHT_DATA))
    return SHT_ERR_TIMEOUT;

  // read the most sigificant byte
  sht_read_byte(((uint8_t *) raw_data) + 1, MSB);

  // read the least sigificant byte
  sht_read_byte((uint8_t *) raw_data, MSB);

  // read crc: crc byte must be reversed:
  // so just read it LSB first
  sht_read_byte(&crc_read, LSB);

  // crc check over complete sequence
  sht_crc8(&crc_calc, command);
  sht_crc8(&crc_calc, *(((uint8_t *) raw_data) + 1));
  sht_crc8(&crc_calc, *((uint8_t *) raw_data));

  if (crc_calc != crc_read)
    return SHT_ERR_CRC;

  return SHT_OK;
}

int8_t
sht_get(uint16_t * raw_temp, uint16_t * raw_humid)
{
  int8_t ret;

  sht_init();

  // reset first to make sure all registers are at default etc.
  ret = sht_send_byte(SHT_CMD_RESET);
  if (ret != SHT_OK)
    return ret;

  if (raw_temp != NULL)
  {
    ret = sht_sensor_command(SHT_CMD_TEMP, raw_temp, 41500);
    if (ret != SHT_OK)
      return ret;
  }

  if (raw_humid != NULL)
  {
    ret = sht_sensor_command(SHT_CMD_HUMID, raw_humid, 10000);
    if (ret != SHT_OK)
      return ret;
  }

  return SHT_OK;
}

// temp is returned in two-digit fixedpoint (celsius*100)
int16_t
sht_convert_temp(uint16_t const *raw_temp)
{
  return (int16_t) (*raw_temp +
                    (((int16_t) SHT_VOLTAGE_COMPENSATION_D1) * 100));
}

// humidity is returned in one-digit fixedpoint (RH percent*10)
uint16_t
sht_convert_humid(uint16_t const *raw_temp, uint16_t const *raw_humid)
{
  uint32_t humid;

    /**** first step: relative humidity ****/

  // constants from datasheet (values for V4 sensors), shifted and adapted
  const uint32_t c1 = (uint32_t) ((2.0468 * 100 * (1L << 18)) + 0.5);
  const uint32_t c2 = (uint32_t) ((0.0367 * 100 * (1L << 18)) + 0.5);
  const uint32_t c3 = (uint32_t) ((1.5955e-6 * 100 * (1L << 27)) + 0.5);

  // uncompensated = c2*raw - (c3 * raw * raw) - c1
  humid = (c2 * (uint32_t) * raw_humid)
    - (((((uint32_t) * raw_humid * (uint32_t) * raw_humid) >> 7) * c3) >> 2)
    - c1;

  // humid >> 18 would now be the uncompensated humidity in RH% * 100

    /**** second step: temperature compensation ****/

  // constants from datasheet, shifted and adapted
  const int32_t t1 = (int32_t) ((0.01 * 100 * (1L << 18)) + 0.5);
  const int32_t t2 = (int32_t) ((0.00008 * 100 * (1L << 18)) + 0.5);

  // humid += (temp-25) * (t1 + t2*raw_temp)
  humid += (int32_t) ((sht_convert_temp(raw_temp) / 100) - 25)
    * (t1 + (t2 * (int32_t) * raw_humid));

  // humid >> 18 now is the real humidity in RH% * 100

  // shift it back and round to RH% * 10
  // (the sensor doesn't really provide enough accuracy to justify more resolution)
  return (uint16_t) (((humid >> 18) + 5) / 10);
}

/*
  -- Ethersex META --
  header(hardware/sht/sht.h)
*/
