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
 * Ambient Light Sensor IC BH1750FVI
 *
 * http://www.rohm.com/
 * http://s6z.de/cms/index.php/arduino/sensoren/15-umgebungslichtsensor-bh1750
 * https://github.com/hexenmeister/AS_BH1750
 */

#include <avr/io.h>
#include <util/twi.h>
#include <util/delay.h>

#include "config.h"
#include "core/debug.h"
#include "i2c_master.h"
#include "i2c_bh1750.h"


#define BH1750_ADDR1        0x23  /* Default address */
#define BH1750_ADDR2        0x5C  /* Alternative address */

#define BH1750_POWER_DOWN   0x00
#define BH1750_POWER_ON     0x01
#define BH1750_RESET        0x07

typedef enum
{
  UNCONFIGURED = 0,
  CONTINUOUS_HIGH_RES_MODE  = 0x10,
  CONTINUOUS_HIGH_RES_MODE2 = 0x11,
  CONTINUOUS_LOW_RES_MODE   = 0x13,
  ONE_TIME_HIGH_RES_MODE    = 0x20,
  ONE_TIME_HIGH_RES_MODE2   = 0x21,
  ONE_TIME_LOW_RES_MODE     = 0x23
} i2c_bh1750_mode;

#ifdef I2C_BH1750_AUTO_RESOLUTION
#define SENSITIVITY_DEFAULT  69U
#define SENSITIVITY_MIN      31U
#define SENSITIVITY_MAX     254U

typedef uint8_t i2c_bh1750_sensitivity;
#endif /* I2C_BH1750_AUTO_RESOLUTION */


static struct
{
  uint8_t addr;
  uint8_t auto_power_down;
  i2c_bh1750_resolution resolution;
  i2c_bh1750_mode mode;
#ifdef I2C_BH1750_AUTO_RESOLUTION
  uint8_t sensitivity;
#endif
} i2c_bh1750_data =
{
#ifdef I2C_BH1750_AUTO_RESOLUTION
  .sensitivity = SENSITIVITY_DEFAULT
#endif
};


static uint8_t
i2c_bh1750_select(uint8_t twi_mode)
{
  if (i2c_bh1750_data.addr != 0)
    return i2c_master_select(i2c_bh1750_data.addr, twi_mode);

  if (i2c_master_select(BH1750_ADDR1, twi_mode) != 0)
  {
    I2CDEBUG("bh1750 addr=%02x\n", BH1750_ADDR1);
    i2c_bh1750_data.addr = BH1750_ADDR1;
    return 1;
  }

  if (i2c_master_select(BH1750_ADDR2, twi_mode) != 0)
  {
    I2CDEBUG("bh1750 addr=0x%02x\n", BH1750_ADDR2);
    i2c_bh1750_data.addr = BH1750_ADDR2;
    return 1;
  }

  return 0;
}


static int8_t
i2c_bh1750_set_mode(const i2c_bh1750_mode mode)
{
  int8_t result = BH1750_RESULT_NODEV;

  if (!i2c_bh1750_select(TW_WRITE))
    goto end;

  result = BH1750_RESULT_ERROR;

  TWDR = mode;
  if (i2c_master_transmit_with_ack() != TW_MT_DATA_ACK)
    goto end;

  result = BH1750_RESULT_OK;
  i2c_bh1750_data.mode = mode;

end:
  i2c_master_stop();
  return result;
}


/*
 * Sensor resolution mode:
 *
 * RESOLUTION_LOW:
 *  Physical sensor mode with 4 lx resolution. Measuring time approx. 16ms. Range 0-54612.
 * RESOLUTION_NORMAL:
 *  Physical sensor mode with 1 lx resolution. Measuring time approx. 120ms. Range 0-54612.
 * RESOLUTION_HIGH:
 *  Physical sensor mode with 0.5 lx resolution. Measuring time approx. 120ms. Range 0-54612.
 *  (The measuring ranges can be shifted by changing the MTreg.)
 * RESOLUTION_AUTO_HIGH:
 *  The values in the MTreg are automatically adjusted according to the brightness
 *  that a maximum possible resolution and measuring range are achieved.
 *  The measurable values start from 0.11 lx and go up to over 100000 lx.
 *  Lower resolution 0.13 lx, middle 0.5 lx, upper 1-2 lx.
 *
 * The measuring times are extended by multiple measurements and
 * the changes from Measurement Time (MTreg) to max. about 500 ms.
 */
int8_t
i2c_bh1750_set_operating_mode(const i2c_bh1750_resolution resolution,
                              const uint8_t auto_power_down)
{
  i2c_bh1750_mode mode = UNCONFIGURED;

  switch (resolution)
  {
    case RESOLUTION_LOW:
      mode = auto_power_down ? ONE_TIME_LOW_RES_MODE
                             : CONTINUOUS_LOW_RES_MODE;
      break;
    case RESOLUTION_NORMAL:
      mode = auto_power_down ? ONE_TIME_HIGH_RES_MODE
                             : CONTINUOUS_HIGH_RES_MODE;
      break;
    case RESOLUTION_HIGH:
      mode = auto_power_down ? ONE_TIME_HIGH_RES_MODE2
                             : CONTINUOUS_HIGH_RES_MODE2;
      break;
#ifdef I2C_BH1750_AUTO_RESOLUTION
    case RESOLUTION_AUTO_HIGH:
      mode = CONTINUOUS_LOW_RES_MODE;
      break;
#endif
    default:
      return BH1750_RESULT_INVAL;
  }

  I2CDEBUG("bh1750 mode=0x%02x\n", mode);

  int8_t result = i2c_bh1750_set_mode(mode);
  if (result == BH1750_RESULT_OK)
  {
    i2c_bh1750_data.resolution = resolution;
    i2c_bh1750_data.auto_power_down = auto_power_down;
  }

  return result;
}


#ifdef I2C_BH1750_AUTO_RESOLUTION
/*
 * Sensitivity of the sensor:
 *
 * SENSITIVITY_MIN:
 *  Sensitivity: default * 0.45
 * SENSITIVITY_MAX:
 *  Sensitivity: default * 3.68
 * SENSITIVITY_DEFAULT:
 *  Sensitivity: 69
 *
 * Sensitivity changes the reading time (higher Sensitivity means
 * longer time span).
 */
static int8_t
i2c_bh1750_set_sensitivity(const i2c_bh1750_sensitivity sensitivity)
{
  if (sensitivity == i2c_bh1750_data.sensitivity)
    return BH1750_RESULT_OK;

  if (sensitivity < SENSITIVITY_MIN || sensitivity > SENSITIVITY_MAX)
    return BH1750_RESULT_INVAL;

  I2CDEBUG("bh1750 sensitivity=%u\n", sensitivity);

  /* Change measurement time
   * Two-step transfer: 3 bits and 5 bits, each with a prefix.
   *   High bit: 01000_MT[7,6,5]
   *   Low bit:  011_MT[4,3,2,1,0]
   */

  int8_t result = BH1750_RESULT_NODEV;

  if (!i2c_bh1750_select(TW_WRITE))
    goto end;

  TWDR = 0x40 | (sensitivity >> 5);
  if (i2c_master_transmit_with_ack() != TW_MT_DATA_ACK)
    goto end;

  TWDR = 0x60 | (sensitivity & 0x1f);
  if (i2c_master_transmit_with_ack() != TW_MT_DATA_ACK)
    goto end;

  i2c_bh1750_data.sensitivity = sensitivity;
  result = BH1750_RESULT_OK;

end:
  i2c_master_stop();
  return result;
}
#endif /* I2C_BH1750_AUTO_RESOLUTION */


static int32_t
i2c_bh1750_get_raw(void)
{
  int32_t result = BH1750_RESULT_NODEV;

  if (!i2c_bh1750_select(TW_WRITE))
    goto end;

  result = BH1750_RESULT_ERROR;

  /* Do a repeated start condition */
  if (i2c_master_start() != TW_REP_START)
    goto end;

  TWDR = (i2c_bh1750_data.addr << 1) | TW_READ;
  if (i2c_master_transmit() != TW_MR_SLA_ACK)
    goto end;

  uint8_t val[2];

  if (i2c_master_transmit_with_ack() != TW_MR_DATA_ACK)
    goto end;

  val[0] = TWDR;

  if (i2c_master_transmit() != TW_MR_DATA_NACK)
    goto end;

  val[1] = TWDR;

  result = (int16_t) ((val[0] << 8) | val[1]);

end:
  i2c_master_stop();
  return result;
}


int32_t
i2c_bh1750_get_lux(void)
{
  if (UNCONFIGURED == i2c_bh1750_data.mode)
  {
    I2CDEBUG("bh1750 lux: mode not set\n");
    return BH1750_RESULT_INVAL;
  }

  int8_t result;
  i2c_bh1750_mode mode;

#ifdef I2C_BH1750_AUTO_RESOLUTION
  /* The automatic mode requires special treatment.
   * First the brightness is read in LowRes mode,
   * depending on the area (dark, normal, very bright) the values of
   * MTreg is set and then the actual measurement is made.
   *
   * The fixed limits may cause a 'jump' in
   * the trace. In this case, an ongoing adjustment would be from MTreg
   * probably better in borderline areas.
   * For our purposes, this is irrelevant.
   */
  if (i2c_bh1750_data.resolution == RESOLUTION_AUTO_HIGH)
  {
    result = i2c_bh1750_set_sensitivity(SENSITIVITY_DEFAULT);
    if (result < BH1750_RESULT_OK)
      return result;

    result = i2c_bh1750_set_mode(CONTINUOUS_LOW_RES_MODE);
    if (result < BH1750_RESULT_OK)
      return result;

    i2c_bh1750_sensitivity sensitivity;
    uint8_t auto_power_down = i2c_bh1750_data.auto_power_down;

    int32_t level = i2c_bh1750_get_raw();
    if (level < BH1750_RESULT_OK)
      return result;

    if (level < 10)
    {
      /* Dark, sensitivity to maximum.
       * The value is random. From about 16000 this approach would be possible.
       * You only need this accuracy in the dark areas
       * (to recognize when it is really dark).
       */
      I2CDEBUG("bh1750 level 0: dark\n");
      sensitivity = SENSITIVITY_MAX;
      mode = auto_power_down ? ONE_TIME_HIGH_RES_MODE2
                             : CONTINUOUS_HIGH_RES_MODE2;
    }
    else if (level < 32767)
    {
      /* So far, the 0.5 lx mode is enough. Normal sensitivity. */
      I2CDEBUG("bh1750 level 1: normal\n");
      sensitivity = SENSITIVITY_DEFAULT;
      mode = auto_power_down ? ONE_TIME_HIGH_RES_MODE2
                             : CONTINUOUS_HIGH_RES_MODE2;
    }
    else if (level < 60000)
    {
      /* High range, 1 lx mode, normal sensitivity. The value of
       * 60000 is more or less random, it just has to be a high one
       * Value to be close to the limit.
       */
      I2CDEBUG("bh1750 level 2: bright\n");
      sensitivity = SENSITIVITY_DEFAULT;
      mode = auto_power_down ? ONE_TIME_HIGH_RES_MODE
                             : CONTINUOUS_HIGH_RES_MODE;
    }
    else
    {
      /* Very high range, reduce sensitivity
       * Min + 1, at the minimum of docu the sensor goes crazy:
       * The values are about 1/10 of the expected.
       */
      I2CDEBUG("bh1750 level 3: very bright\n");
      sensitivity = SENSITIVITY_MIN + 1;
      mode = auto_power_down ? ONE_TIME_HIGH_RES_MODE
                             : CONTINUOUS_HIGH_RES_MODE;
    }

    result = i2c_bh1750_set_sensitivity(sensitivity);
    if (result < BH1750_RESULT_OK)
      return result;

    result = i2c_bh1750_set_mode(mode);
    if (result < BH1750_RESULT_OK)
      return result;
  }
#else
  if (i2c_bh1750_data.auto_power_down)
  {
    /* power up sensor */
    mode = i2c_bh1750_data.mode;
    result = i2c_bh1750_set_mode(mode);
    if (result < BH1750_RESULT_OK)
      return result;
  }
#endif /* I2C_BH1750_AUTO_RESOLUTION */

  /* Give the sensor some time to wake up from the power down. */
  switch (mode)
  {
    case ONE_TIME_HIGH_RES_MODE:
    case ONE_TIME_HIGH_RES_MODE2:
      _delay_ms(120);
      break;
    case ONE_TIME_LOW_RES_MODE:
      _delay_ms(16);
      break;
    default:
      break;
  }

  int32_t raw = i2c_bh1750_get_raw();
  if (raw < BH1750_RESULT_OK)
    return raw;

  /* Base conversion. */
  int32_t lux = (raw * 5) / 6;
  I2CDEBUG("bh1750 lux(1)=%li\n", lux);

#ifdef I2C_BH1750_AUTO_RESOLUTION
  /* Take sensitivity into account. */
  if (i2c_bh1750_data.sensitivity != SENSITIVITY_DEFAULT)
    lux = (lux * SENSITIVITY_DEFAULT) / i2c_bh1750_data.sensitivity;
#endif
  I2CDEBUG("bh1750 lux(2)=%li\n", lux);

  /* Depending on the mode, a further conversion is necessary. */
  if ((i2c_bh1750_data.mode == ONE_TIME_HIGH_RES_MODE2)
      || (i2c_bh1750_data.mode == CONTINUOUS_HIGH_RES_MODE2))
    lux /= 2;
  I2CDEBUG("bh1750 lux(3)=%li\n", lux);

  return lux;
}
