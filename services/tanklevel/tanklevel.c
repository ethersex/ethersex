/*
 *
 * Copyright(c) 2012 by Sascha Ittner <sascha.ittner@modusoft.de>
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

#include <avr/io.h>
#include <util/atomic.h>

#include "config.h"
#include "core/eeprom.h"
#include "core/debug.h"
#include "services/clock/clock.h"
#include "tanklevel.h"

#define TANKLEVEL_STATE_IDLE     0
#define TANKLEVEL_STATE_REQ      1
#define TANKLEVEL_STATE_BUSY     2

#ifdef TANKLEVEL_PUMP_ACTIVE_LOW
#define TANKLEVEL_PUMP_ON  PIN_CLEAR(TANKLEVEL_PUMP)
#define TANKLEVEL_PUMP_OFF PIN_SET(TANKLEVEL_PUMP)
#else
#define TANKLEVEL_PUMP_ON  PIN_SET(TANKLEVEL_PUMP)
#define TANKLEVEL_PUMP_OFF PIN_CLEAR(TANKLEVEL_PUMP)
#endif

#ifdef TANKLEVEL_LOCK_SUPPORT
#ifdef TANKLEVEL_LOCK_ACTIVE_LOW
#define TANKLEVEL_IS_LOCKED (locked || PIN_LOW(TANKLEVEL_LOCK))
#else
#define TANKLEVEL_IS_LOCKED (locked || PIN_HIGH(TANKLEVEL_LOCK))
#endif
#else
#define TANKLEVEL_IS_LOCKED (locked)
#endif

tanklevel_params_t tanklevel_params_ram;

float tanklevel_factor;

volatile uint8_t state = TANKLEVEL_STATE_IDLE;
volatile uint8_t locked;
volatile uint16_t capture_adc;
volatile timestamp_t capture_ts;
volatile uint16_t raise_timer;
volatile uint16_t hold_timer;
volatile timestamp_t get_ts;

void
tanklevel_init(void)
{
  /* init pins */
  TANKLEVEL_PUMP_OFF;

  /* restore parameters */
  eeprom_restore(tanklevel_params, &tanklevel_params_ram, sizeof(tanklevel_params_t));
  tanklevel_update_factor();

#ifdef TANKLEVEL_STARTUP
  tanklevel_start();
#endif
}

void
tanklevel_update_factor(void)
{
  float mul = 1000.0F * (float)tanklevel_params_ram.ltr_per_m;
  float div = (float)tanklevel_params_ram.med_density * TANKLEVEL_SENSOR_SENS * TANKLEVEL_GRAVITY;

  if (mul > 0.0F && div > 0.0F) {
    tanklevel_factor = mul / div;
  } else {
    tanklevel_factor = 0.0F;
  }
}

void
tanklevel_periodic(void)
{
  switch(state) {
    case TANKLEVEL_STATE_IDLE:
      return;

    case TANKLEVEL_STATE_REQ:
      /* initaialize timers */
      raise_timer = tanklevel_params_ram.raise_time;
      hold_timer = tanklevel_params_ram.hold_time;

      /* check lock */
      if (TANKLEVEL_IS_LOCKED) {
        return;
      }

      /* pump on */
      TANKLEVEL_PUMP_ON;

      /* next state */
      state++;
      return;

    case TANKLEVEL_STATE_BUSY:
      /* wait for pressure raise */
      if (raise_timer > 0) {
        raise_timer--;
        return;
      }

      /* pump off */
      TANKLEVEL_PUMP_OFF;

      /* wait for stable pressure */
      if (hold_timer > 0) {
        hold_timer--;
        return;
      }

      /* read timestamp and adc */
      ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        capture_ts = clock_get_time();
        capture_adc = adc_get(TANKLEVEL_ADC_CHANNEL);
      }

    default:
      state = TANKLEVEL_STATE_IDLE;
  }
}

void
tanklevel_start(void)
{
  if (state == TANKLEVEL_STATE_IDLE) {
    state = TANKLEVEL_STATE_REQ;
  }
}

int8_t
tanklevel_check_busy(void)
{
  return (state != TANKLEVEL_STATE_IDLE);
}

uint16_t
tanklevel_get(void)
{
  uint16_t adc;
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    get_ts = capture_ts;
    adc = capture_adc;
  }

  uint16_t adc_volt = adc_raw_to_voltage(adc);
  if (adc_volt < tanklevel_params_ram.sensor_offset) {
    return 0;
  }
  adc_volt -= tanklevel_params_ram.sensor_offset;

  return (uint16_t)((float)adc_volt * tanklevel_factor);
}

timestamp_t
tanklevel_get_ts(void)
{
  return get_ts;
}

void
tanklevel_set_lock(uint8_t val)
{
  locked = val;
}

/*
  -- Ethersex META --
  header(services/tanklevel/tanklevel.h)
  init(tanklevel_init)
  timer(1, tanklevel_periodic())
*/
