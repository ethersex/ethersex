/*
* Copyright (c) 2018,2024 by Erik Kunze <ethersex@erik-kunze.de>
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
* https://www.bosch-sensortec.com/products/environmental-sensors/pressure-sensors/bmp280/
* https://startingelectronics.org/tutorials/arduino/modules/pressure-sensor/
*
* BME280 humidity sensor
* https://www.bosch-sensortec.com/products/environmental-sensors/humidity-sensors-bme280/
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
#define BME280_CHIP_ID	           0x60

/* I2C addresses */
#define BMP280_ADDR1               0x76 /* Default address */
#define BMP280_ADDR2               0x77 /* Alternative address */

/* Registers */
#define BMP280_DIG_T1_ADDR	   0x88
#define BME280_DIG_H1_ADDR	   0xA1
#define BME280_DIG_H2_ADDR	   0xE1
#define BME280_DIG_H3_ADDR	   0xE3
#define BME280_DIG_H4_ADDR	   0xE4
#define BME280_DIG_H5_ADDR	   0xE5
#define BME280_DIG_H6_ADDR	   0xE7
#define BMP280_CHIP_ID_ADDR	   0xD0
#define BMP280_SOFT_RESET_ADDR	   0xE0
#define BMP280_CTRL_HUMID_ADDR	   0xF2
#define BMP280_STATUS_ADDR	   0xF3
#define BMP280_CTRL_MEAS_ADDR	   0xF4
#define BMP280_CONFIG_ADDR	   0xF5
#define BMP280_PRESS_ADDR	   0xF7
#define BMP280_TEMP_ADDR	   0xFA
#define BME280_HUMID_ADDR	   0xFD

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
#ifdef I2C_BME280_SUPPORT
  uint8_t dig_h1;
  int16_t dig_h2;
  uint8_t dig_h3;
  int16_t dig_h4;
  int16_t dig_h5;
  int8_t dig_h6;
#endif
  int32_t t_fine;
} i2c_bmp280_calib;

typedef struct
{
  uint32_t temp;
  uint32_t press;
#ifdef I2C_BME280_SUPPORT
  uint32_t humid;
#endif
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
#ifdef I2C_BME280_SUPPORT
    /* Datasheet 3.5.3 table 9 */
    .os_humid = BMP280_OS_1X,
    .os_temp = BMP280_OS_2X,
    .os_pres = BMP280_OS_16X,
    .odr = BMP280_ODR_500_MS,
    .filter = BMP280_FILTER_COEFF_16
#else
    .os_temp = BMP280_OS_4X,
    .os_pres = BMP280_OS_16X,
    .odr = BMP280_ODR_1000_MS,
    .filter = BMP280_FILTER_COEFF_2
#endif
  }
};


static uint8_t
i2c_bmp280_select(uint8_t twi_mode)
{
  if (i2c_bmp280_data.addr != 0)
    return i2c_master_select(i2c_bmp280_data.addr, twi_mode);

  if (i2c_master_select(BMP280_ADDR1, twi_mode) != 0)
  {
    I2CDEBUG("bmp280 addr=0x%02x\n", BMP280_ADDR1);
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

#ifdef I2C_BME280_SUPPORT
    if (i2c_bmp280_data.chip_id == BME280_CHIP_ID)
    {
      result = i2c_bmp280_read(BME280_DIG_H1_ADDR, 1, temp);
      if (result == BMP280_RESULT_OK)
      {
        i2c_bmp280_data.calib.dig_h1 = temp[0];
        result = i2c_bmp280_read(BME280_DIG_H2_ADDR, 7, temp);
        if (result == BMP280_RESULT_OK)
        {
          i2c_bmp280_data.calib.dig_h2 = (int16_t) (temp[1] << 8 | temp[0]);
          i2c_bmp280_data.calib.dig_h3 = temp[2];
          i2c_bmp280_data.calib.dig_h4 = (int16_t) (temp[3] << 4 | (temp[4] & 0xF));
          i2c_bmp280_data.calib.dig_h5 = (int16_t) (temp[5] << 4 | temp[4] >> 4);
          i2c_bmp280_data.calib.dig_h6 = (int8_t) temp[6];
          I2CDEBUG("h1=%u, h2=%i, h3=%u\n",
                   i2c_bmp280_data.calib.dig_h1,
                   i2c_bmp280_data.calib.dig_h2,
                   i2c_bmp280_data.calib.dig_h3);
          I2CDEBUG("h4=%u, h5=%i, h6=%i\n",
                   i2c_bmp280_data.calib.dig_h4,
                   i2c_bmp280_data.calib.dig_h5,
                   i2c_bmp280_data.calib.dig_h6);
        }
      }
    }
#endif
  }

  return result;
}


int8_t
i2c_bmp280_read_conf(i2c_bmp280_conf * conf)
{
  uint8_t temp[4];

  int8_t result = i2c_bmp280_read(BMP280_CTRL_HUMID_ADDR, sizeof(temp), temp);
  if (result == BMP280_RESULT_OK)
  {
    I2CDEBUG("bmp280 read conf = %02x %02x %02x\n", temp[0], temp[2], temp[3]);

#ifdef I2C_BME280_SUPPORT
    if (i2c_bmp280_data.chip_id == BME280_CHIP_ID)
        conf->os_humid = temp[0] & 0x07;
#endif
    conf->os_temp = (temp[2] & 0xE0) >> 5;
    conf->os_pres = (temp[2] & 0x1C) >> 2;
    conf->odr = (temp[3] & 0xE0) >> 5;
    conf->filter = (temp[3] & 0x1C) >> 2;

    i2c_bmp280_data.conf = *conf;
  }

  return result;
}


/* Reset the sensor, restore/set conf, restore/set mode. */
int8_t
i2c_bmp280_write_conf(uint8_t mode, const i2c_bmp280_conf * conf)
{
  uint8_t temp[4];
  uint8_t addr[2] = { BMP280_CTRL_MEAS_ADDR, BMP280_CONFIG_ADDR };

  I2CDEBUG("bmp280 write conf\n");

  int8_t result = i2c_bmp280_read(BMP280_CTRL_HUMID_ADDR, sizeof(temp), temp);
  if (result < BMP280_RESULT_OK)
    goto end;

  result = i2c_bmp280_soft_reset();  /* put device in sleep mode */
  if (result < BMP280_RESULT_OK)
    goto end;

#ifdef I2C_BME280_SUPPORT
  temp[0] = (temp[0] & ~0x07) | conf->os_humid;
#endif
  temp[2] = (temp[2] & ~0x03) | BMP280_SLEEP_MODE;
  temp[2] = (temp[2] & ~0xE0) | (conf->os_temp << 5);
  temp[2] = (temp[2] & ~0x1C) | (conf->os_pres << 2);
  temp[3] = (temp[3] & ~0xE0) | (conf->odr << 5);
  temp[3] = (temp[3] & ~0x1C) | (conf->filter << 2);

  I2CDEBUG("bmp280 write conf = %02x %02x %02x\n", temp[0], temp[2], temp[3]);

#ifdef I2C_BME280_SUPPORT
  if (i2c_bmp280_data.chip_id == BME280_CHIP_ID)
  {
    result = i2c_bmp280_write(BMP280_CTRL_HUMID_ADDR, 1, temp);
    if (result < BMP280_RESULT_OK)
      goto end;
  }
#endif

  result = i2c_bmp280_write(addr, 2, &temp[2]);
  if (result < BMP280_RESULT_OK)
    goto end;

  i2c_bmp280_data.conf = *conf;

  if (mode != BMP280_SLEEP_MODE)
  {
    /* Write only the power mode register in a separate write */
    temp[2] = (temp[2] & ~0x03) | mode;
    result = i2c_bmp280_write(addr, 1, &temp[2]);
  }

#ifdef DEBUG_I2C
  result = i2c_bmp280_read(BMP280_CTRL_HUMID_ADDR, sizeof(temp), temp);
  if (result < BMP280_RESULT_OK)
    goto end;

  I2CDEBUG("bmp280 read conf = %02x %02x %02x\n", temp[0], temp[2], temp[3]);
#endif

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
  uint8_t temp[8];

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

#ifdef I2C_BME280_SUPPORT
    if (i2c_bmp280_data.chip_id == BME280_CHIP_ID)
    {
      v = temp[6];
      v <<= 8;
      v |= temp[7];
      raw->humid = v;
      I2CDEBUG("bme280 press=%lu temp=%lu humid=%lu\n",
               raw->press, raw->temp, raw->humid);
    }
    else
#endif
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
    I2CDEBUG("bmp280 chip_id=0x%02x\n", i2c_bmp280_data.chip_id);

    if (i2c_bmp280_data.chip_id == BMP280_CHIP_ID1
        || i2c_bmp280_data.chip_id == BMP280_CHIP_ID2
        || i2c_bmp280_data.chip_id == BMP280_CHIP_ID3
#ifdef I2C_BME280_SUPPORT
        || i2c_bmp280_data.chip_id == BME280_CHIP_ID
#endif
       )
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

#ifdef I2C_BME280_SUPPORT

int8_t
i2c_bme280_get_humid(uint16_t * humid)
{
  if (i2c_bmp280_data.chip_id != BME280_CHIP_ID)
    return BMP280_RESULT_NODEV;

  i2c_bmp280_raw raw;

  int8_t result = i2c_bmp280_get_raw(&raw);
  if (result < BMP280_RESULT_OK)
    goto end;

  /* value in case humidity measurement was disabled */
  if (raw.humid == 0x8000)
    return BMP280_RESULT_NODATA;

  int32_t var;
  var = i2c_bmp280_data.calib.t_fine - (int32_t) 76800;
  var =
    (((((((int32_t) raw.humid) << 14) -
        (((int32_t) i2c_bmp280_data.calib.dig_h4) << 20) -
        (((int32_t) i2c_bmp280_data.calib.dig_h5) * var)) +
       ((int32_t) 16384)) >> 15) *
     (((((((var * ((int32_t) i2c_bmp280_data.calib.dig_h6)) >> 10) *
          (((var * ((int32_t) i2c_bmp280_data.calib.dig_h3)) >> 11) +
           ((int32_t) 32768))) >> 10) +
        ((int32_t) 2097152)) * ((int32_t) i2c_bmp280_data.calib.dig_h2) +
       8192) >> 14));
  var =
    (var -
     (((((var >> 15) * (var >> 15)) >> 7) *
       ((int32_t) i2c_bmp280_data.calib.dig_h1)) >> 4));
  var = (var < 0 ? 0 : var);
  var = (var > 419430400 ? 419430400 : var);
  I2CDEBUG("bmp280 humid=%lu\n", var);
  *humid = (uint16_t) (var >> 12);

end:
  return result;
}

#endif /* I2C_BME280_SUPPORT */

/*
  -- Ethersex META --
  header(hardware/i2c/master/i2c_bmp280.h)
  init(i2c_bmp280_init)
 */
