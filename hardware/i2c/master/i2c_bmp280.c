/*
* Copyright (c) 2018 by Erik Kunze <ethersex@erik-kunze.de>
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

/*
* BMP280 absolute barometric pressure sensor
*
* https://www.bosch-sensortec.com/bst/products/all_products/bmp280
* https://startingelectronics.org/tutorials/arduino/modules/pressure-sensor/
*/

#include <avr/io.h>
#include <util/twi.h>
#include <util/delay.h>

#include "config.h"
#include "core/debug.h"
#include "i2c_master.h"
#include "i2c_bmp280.h"


/* Chip IDs */
#define BMP280_CHIP_ID1	           0x56
#define BMP280_CHIP_ID2	           0x57
#define BMP280_CHIP_ID3	           0x58

/* I2C addresses */
#define BMP280_ADDR1               0x76 /* Default address */
#define BMP280_ADDR2               0x77 /* Alternative address */

/* Registers */
#define BMP280_DIG_T1_ADDR	   0x88
#define BMP280_CHIP_ID_ADDR	   0xD0
#define BMP280_SOFT_RESET_ADDR	   0xE0
#define BMP280_STATUS_ADDR	   0xF3
#define BMP280_CTRL_MEAS_ADDR	   0xF4
#define BMP280_CONFIG_ADDR	   0xF5
#define BMP280_PRESS_ADDR	   0xF7
#define BMP280_TEMP_ADDR	   0xFA

/* Soft reset command */
#define BMP280_SOFT_RESET_CMD	   0xB6


typedef struct
{
  uint16_t dig_t1;
  int16_t dig_t2;
  int16_t dig_t3;
  uint16_t dig_p1;
  int16_t dig_p2;
  int16_t dig_p3;
  int16_t dig_p4;
  int16_t dig_p5;
  int16_t dig_p6;
  int16_t dig_p7;
  int16_t dig_p8;
  int16_t dig_p9;
  int32_t t_fine;
} i2c_bmp280_calib;

typedef struct
{
  uint32_t temp;
  uint32_t press;
} i2c_bmp280_raw;


static struct
{
  uint8_t addr;
  uint8_t chip_id;
  i2c_bmp280_calib calib;
  i2c_bmp280_conf conf;
} i2c_bmp280_data =
{
  .conf =
  {
    .os_temp = BMP280_OS_4X,
    .os_pres = BMP280_OS_16X,
    .odr = BMP280_ODR_1000_MS,
    .filter = BMP280_FILTER_COEFF_2
  }
};


static uint8_t
i2c_bmp280_select(uint8_t twi_mode)
{
  if (i2c_bmp280_data.addr != 0)
    return i2c_master_select(i2c_bmp280_data.addr, twi_mode);

  if (i2c_master_select(BMP280_ADDR1, twi_mode) != 0)
  {
    I2CDEBUG("bmp280 addr=%02x\n", BMP280_ADDR1);
    i2c_bmp280_data.addr = BMP280_ADDR1;
    return 1;
  }

  if (i2c_master_select(BMP280_ADDR2, twi_mode) != 0)
  {
    I2CDEBUG("bmp280 addr=0x%02x\n", BMP280_ADDR2);
    i2c_bmp280_data.addr = BMP280_ADDR2;
    return 1;
  }

  return 0;
}


/* Read the data from the given register address of the sensor. */
static int8_t
i2c_bmp280_read(uint8_t addr, uint8_t len, uint8_t * data)
{
  int8_t result = BMP280_RESULT_NODEV;

  if (!i2c_bmp280_select(TW_WRITE))
    goto end;

  result = BMP280_RESULT_ERROR;

  TWDR = addr;
  if (i2c_master_transmit_with_ack() != TW_MT_DATA_ACK)
    goto end;

  /* Do a repeated start condition */
  if (i2c_master_start() != TW_REP_START)
    goto end;

  TWDR = (i2c_bmp280_data.addr << 1) | TW_READ;
  if (i2c_master_transmit() != TW_MR_SLA_ACK)
    goto end;

  while (--len > 0)
  {
    if (i2c_master_transmit_with_ack() != TW_MR_DATA_ACK)
      goto end;

    *data++ = TWDR;
  }

  if (i2c_master_transmit() != TW_MR_DATA_NACK)
    goto end;

  *data = TWDR;

  result = BMP280_RESULT_OK;

end:
  i2c_master_stop();
  return result;
}


/* Write the given data to the register addresses of the sensor. */
static int8_t
i2c_bmp280_write(uint8_t * addr, uint8_t len, uint8_t * data)
{
  int8_t result = BMP280_RESULT_NODEV;

  if (!i2c_bmp280_select(TW_WRITE))
    goto end;

  result = BMP280_RESULT_ERROR;

  for (uint8_t i = 0; i < len; i++)
  {
    TWDR = addr[i];
    if (i2c_master_transmit_with_ack() != TW_MT_DATA_ACK)
      break;

    TWDR = data[i];
    if (i2c_master_transmit_with_ack() != TW_MT_DATA_ACK)
      break;
  }

  result = BMP280_RESULT_OK;

end:
  i2c_master_stop();
  return result;
}


/* Trigger the soft reset of the sensor. */
static int8_t
i2c_bmp280_soft_reset(void)
{
  I2CDEBUG("bmp280 soft reset\n");

  uint8_t addr = BMP280_SOFT_RESET_ADDR;
  uint8_t cmd = BMP280_SOFT_RESET_CMD;

  int8_t result = i2c_bmp280_write(&addr, 1, &cmd);
  if (result == BMP280_RESULT_OK)
    _delay_ms(2);

  return result;
}


/* Read the calibration parameters. */
static int8_t
i2c_bmp280_read_calib(void)
{
  I2CDEBUG("bmp280 read calibration data\n");

  uint8_t temp[24];

  int8_t result = i2c_bmp280_read(BMP280_DIG_T1_ADDR, sizeof(temp), temp);
  if (result == BMP280_RESULT_OK)
  {
    i2c_bmp280_data.calib.dig_t1 = (uint16_t) (temp[1] << 8 | temp[0]);
    i2c_bmp280_data.calib.dig_t2 = (int16_t) (temp[3] << 8 | temp[2]);
    i2c_bmp280_data.calib.dig_t3 = (int16_t) (temp[5] << 8 | temp[4]);

    i2c_bmp280_data.calib.dig_p1 = (uint16_t) (temp[7] << 8 | temp[6]);
    i2c_bmp280_data.calib.dig_p2 = (int16_t) (temp[9] << 8 | temp[8]);
    i2c_bmp280_data.calib.dig_p3 = (int16_t) (temp[11] << 8 | temp[10]);
    i2c_bmp280_data.calib.dig_p4 = (int16_t) (temp[13] << 8 | temp[12]);
    i2c_bmp280_data.calib.dig_p5 = (int16_t) (temp[15] << 8 | temp[14]);
    i2c_bmp280_data.calib.dig_p6 = (int16_t) (temp[17] << 8 | temp[16]);
    i2c_bmp280_data.calib.dig_p7 = (int16_t) (temp[19] << 8 | temp[18]);
    i2c_bmp280_data.calib.dig_p8 = (int16_t) (temp[21] << 8 | temp[20]);
    i2c_bmp280_data.calib.dig_p9 = (int16_t) (temp[23] << 8 | temp[22]);

    I2CDEBUG("t1=%u, t2=%i, t3=%i\n", i2c_bmp280_data.calib.dig_t1,
             i2c_bmp280_data.calib.dig_t2, i2c_bmp280_data.calib.dig_t3);
    I2CDEBUG("p1=%u, p2=%i, p3=%i\n", i2c_bmp280_data.calib.dig_p1,
             i2c_bmp280_data.calib.dig_p2, i2c_bmp280_data.calib.dig_p3);
    I2CDEBUG("p4=%u, p5=%i, p6=%i\n", i2c_bmp280_data.calib.dig_p4,
             i2c_bmp280_data.calib.dig_p5, i2c_bmp280_data.calib.dig_p6);
    I2CDEBUG("p7=%u, p8=%i, p9=%i\n", i2c_bmp280_data.calib.dig_p7,
             i2c_bmp280_data.calib.dig_p8, i2c_bmp280_data.calib.dig_p9);
  }

  return result;
}


int8_t
i2c_bmp280_read_conf(i2c_bmp280_conf * conf)
{
  uint8_t temp[2];

  int8_t result = i2c_bmp280_read(BMP280_CTRL_MEAS_ADDR, sizeof(temp), temp);
  if (result == BMP280_RESULT_OK)
  {
    conf->os_temp = (temp[0] & 0xE0) >> 5;
    conf->os_pres = (temp[0] & 0x1C) >> 2;
    conf->odr = (temp[1] & 0xE0) >> 5;
    conf->filter = (temp[1] & 0x1C) >> 2;

    i2c_bmp280_data.conf = *conf;
  }

  return result;
}


/* Reset the sensor, restore/set conf, restore/set mode. */
int8_t
i2c_bmp280_write_conf(uint8_t mode, const i2c_bmp280_conf * conf)
{
  uint8_t temp[2];
  uint8_t addr[2] = { BMP280_CTRL_MEAS_ADDR, BMP280_CONFIG_ADDR };

  I2CDEBUG("bmp280 write conf\n");

  int8_t result = i2c_bmp280_read(BMP280_CTRL_MEAS_ADDR, sizeof(temp), temp);
  if (result < BMP280_RESULT_OK)
    goto end;

  result = i2c_bmp280_soft_reset();
  if (result < BMP280_RESULT_OK)
    goto end;

  temp[0] = (temp[0] & ~0xE0) | (conf->os_temp << 5);
  temp[0] = (temp[0] & ~0x1C) | (conf->os_pres << 2);
  temp[1] = (temp[1] & ~0xE0) | (conf->odr << 5);
  temp[1] = (temp[1] & ~0x1C) | (conf->filter << 2);

  result = i2c_bmp280_write(addr, 2, temp);
  if (result < BMP280_RESULT_OK)
    goto end;

  i2c_bmp280_data.conf = *conf;

  if (mode != BMP280_SLEEP_MODE)
  {
    /* Write only the power mode register in a separate write */
    temp[0] = (temp[0] & ~0x03) | mode;
    result = i2c_bmp280_write(addr, 1, temp);
  }

end:
  return result;
}


int8_t
i2c_bmp280_set_power_mode(uint8_t mode)
{
  return i2c_bmp280_write_conf(mode, &i2c_bmp280_data.conf);
}


static int8_t
i2c_bmp280_get_raw(i2c_bmp280_raw * raw)
{
  uint8_t temp[6];

  I2CDEBUG("bmp280 get raw\n");

  int8_t result = i2c_bmp280_read(BMP280_PRESS_ADDR, sizeof(temp), temp);
  if (result == BMP280_RESULT_OK)
  {
    uint32_t v = temp[0];
    v <<= 8;
    v |= temp[1];
    v <<= 8;
    v |= temp[2];
    v >>= 4;
    raw->press = v;

    v = temp[3];
    v <<= 8;
    v |= temp[4];
    v <<= 8;
    v |= temp[5];
    v >>= 4;
    raw->temp = v;

    I2CDEBUG("bmp280 press=%lu temp=%lu\n", raw->press, raw->temp);
  }

  return result;
}


/* Read the chip-id and calibration data from the sensor. */
void
i2c_bmp280_init(void)
{
  I2CDEBUG("bmp280 init start\n");

  if (i2c_bmp280_read(BMP280_CHIP_ID_ADDR, sizeof(i2c_bmp280_data.chip_id),
                      &i2c_bmp280_data.chip_id) == BMP280_RESULT_OK)
  {
    I2CDEBUG("bmp280 chip_id=%02x\n", i2c_bmp280_data.chip_id);

    if (i2c_bmp280_data.chip_id == BMP280_CHIP_ID1 ||
        i2c_bmp280_data.chip_id == BMP280_CHIP_ID2 ||
        i2c_bmp280_data.chip_id == BMP280_CHIP_ID3)
    {
      if (i2c_bmp280_soft_reset() == BMP280_RESULT_OK &&
          i2c_bmp280_read_calib() == BMP280_RESULT_OK &&
          i2c_bmp280_set_power_mode(BMP280_NORMAL_MODE) == BMP280_RESULT_OK)
      {
        I2CDEBUG("bmp280 init succeded\n");
        return;
      }
    }
  }

  I2CDEBUG("bmp280 init failed\n");
}


int8_t
i2c_bmp280_get_temp(int16_t * temp)
{
  i2c_bmp280_raw raw;
  int32_t var1, var2;

  int8_t result = i2c_bmp280_get_raw(&raw);
  if (result < BMP280_RESULT_OK)
    goto end;

  var1 = ((((raw.temp >> 3) -
            ((int32_t) i2c_bmp280_data.calib.dig_t1 << 1))) *
          ((int32_t) i2c_bmp280_data.calib.dig_t2)) >> 11;
  var2 = (((((raw.temp >> 4) -
             ((int32_t) i2c_bmp280_data.calib.dig_t1)) *
            ((raw.temp >> 4) -
             ((int32_t) i2c_bmp280_data.calib.dig_t1))) >> 12) *
          ((int32_t) i2c_bmp280_data.calib.dig_t3)) >> 14;

  i2c_bmp280_data.calib.t_fine = var1 + var2;

  *temp = (i2c_bmp280_data.calib.t_fine * 5 + 128) >> 8;

end:
  return result;
}


int8_t
i2c_bmp280_get_press(uint16_t * press)
{
  i2c_bmp280_raw raw;
  int32_t var1, var2;
  uint32_t p;

  int8_t result = i2c_bmp280_get_raw(&raw);
  if (result < BMP280_RESULT_OK)
    goto end;

  var1 = ((((int32_t) i2c_bmp280_data.calib.t_fine) >> 1) - (int32_t) 64000);
  var2 = ((((var1 >> 2) * (var1 >> 2)) >> 11) *
          ((int32_t) i2c_bmp280_data.calib.dig_p6));
  var2 = var2 + ((var1 * ((int32_t) i2c_bmp280_data.calib.dig_p5)) << 1);
  var2 = (var2 >> 2) + (((int32_t) i2c_bmp280_data.calib.dig_p4) << 16);
  var1 = (((i2c_bmp280_data.calib.dig_p3 *
            (((var1 >> 2) * (var1 >> 2)) >> 13)) >> 3) +
          ((((int32_t) i2c_bmp280_data.calib.dig_p2) * var1) >> 1)) >> 18;
  var1 = ((((32768 + var1)) *
           ((int32_t) i2c_bmp280_data.calib.dig_p1)) >> 15);
  p = (((uint32_t) (((int32_t) 1048576) - raw.press) - (var2 >> 12))) * 3125;

  if (var1 == 0)
  {
    *press = 0;
    goto end;
  }

  /* Check for overflows */
  if (p < 0x80000000)
    p = (p << 1) / ((uint32_t) var1);
  else
    p = (p / (uint32_t) var1) * 2;

  var1 = (((int32_t) i2c_bmp280_data.calib.dig_p9) *
          ((int32_t) (((p >> 3) * (p >> 3)) >> 13))) >> 12;
  var2 = (((int32_t) (p >> 2)) *
          ((int32_t) i2c_bmp280_data.calib.dig_p8)) >> 13;
  p = (uint32_t) ((int32_t) p +
                  ((var1 + var2 + i2c_bmp280_data.calib.dig_p7) >> 4));
  *press = p / 10;

end:
  return result;
}


/*
  -- Ethersex META --
  header(hardware/i2c/master/i2c_bmp280.h)
  init(i2c_bmp280_init)
 */
