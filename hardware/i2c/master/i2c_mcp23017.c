/*
 *
 * Copyright (c) 2015 Michael Brakemeier <michael@brakemeier.de>
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

#include <util/delay.h>
#include <util/twi.h>

#include "config.h"

#include "core/debug.h"
#include "i2c_master.h"

#include "i2c_mcp23017.h"

#if defined(DEBUG_I2C) && !defined(TEENSY_SUPPORT)
#define DEBUG_MCP23017(fnc, s, msg...) printf_P(PSTR("D: I2C: " #fnc ": " s), ## msg)
#else
#define DEBUG_MCP23017(fnc, s, msg...)
#endif


/**
 * Read data from register reg.
 */
uint8_t
i2c_mcp23017_read_register(uint8_t address, uint8_t reg, uint8_t * data)
{
  uint8_t result = 0;

  *data = 0;

  DEBUG_MCP23017(i2c_mcp23017_read_register,
                 "addr 0x%02X (%d) reg 0x%02X (%d)\n", address, address, reg,
                 reg);

  /* select slave in write mode */
  if (i2c_master_select(address, TW_WRITE))
  {
    /* send the register address */
    TWDR = reg;
    if (i2c_master_transmit_with_ack() == TW_MT_DATA_ACK)
    {
      /* repeated start condition */
      if (i2c_master_start() == TW_REP_START)
      {
        /* select slave in read mode */
        TWDR = (uint8_t) (address << 1) | TW_READ;
        if (i2c_master_transmit() == TW_MR_SLA_ACK)
        {
          /* read register */
          if (i2c_master_transmit() == TW_MR_DATA_NACK)
          {
            *data = TWDR;
            result = 1;
          }
        }
      }
    }
  }

  /* always send stop */
  i2c_master_stop();

  DEBUG_MCP23017(i2c_mcp23017_read_register,
                 "result: %d, data: 0x%02X (%d)\n", result, *data, *data);

  return result;
}


/**
 * Write data to register reg.
 */
uint8_t
i2c_mcp23017_write_register(uint8_t address, uint8_t reg, uint8_t data)
{
  uint8_t result = 0;

  DEBUG_MCP23017(i2c_mcp23017_write_register,
                 "addr 0x%02X (%d) reg 0x%02X (%d) data 0x%02X (%d)\n",
                 address, address, reg, reg, data, data);

  /* select slave in write mode */
  if (i2c_master_select(address, TW_WRITE))
  {
    /* send the register address */
    TWDR = reg;
    if (i2c_master_transmit_with_ack() == TW_MT_DATA_ACK)
    {
      /* send data */
      TWDR = data;
      if (i2c_master_transmit_with_ack() == TW_MT_DATA_ACK)
      {
        result = 1;
      }
    }
  }

  i2c_master_stop();

  DEBUG_MCP23017(i2c_mcp23017_write_register, "result: %d\n", result);

  return result;
}


/**
 * Set or clear pin.
 */
uint8_t
i2c_mcp23017_modify_pin(uint8_t address, uint8_t reg, uint8_t * data,
                        uint8_t bit, i2c_mcp23017_output_state state)
{
  uint8_t tmp;

  *data = 0;

  /* read-modify-write */
  if (i2c_mcp23017_read_register(address, reg, &tmp) > 0)
  {
    /* bit set, clear or toggle */
    switch (state)
    {
      case ON:
        tmp |= (uint8_t) (1 << (bit));
        break;
      case OFF:
        tmp &= (uint8_t) ~ (1 << (bit));
        break;
      case TOGGLE:
        tmp ^= (uint8_t) (1 << (bit));
        break;
    }

    if (i2c_mcp23017_write_register(address, reg, tmp) > 0)
    {
      *data = tmp;
      return 1;
    }
  }

  return 0;
}


/**
 * Toggle pin to create a pulse with duration time.
 */
uint8_t
i2c_mcp23017_pulse_pin(uint8_t address, uint8_t reg, uint8_t * data,
                       uint8_t bit, uint16_t time)
{
  uint8_t tmp;

  *data = 0;

  /* read-modify-write twice */
  if (i2c_mcp23017_read_register(address, reg, &tmp) > 0)
  {
    /* bit flip */
    tmp ^= (uint8_t) (1 << (bit));
    if (i2c_mcp23017_write_register(address, reg, tmp) > 0)
    {
      /* and delay... */
      while (time--)
        _delay_ms(1);
      if (i2c_mcp23017_read_register(address, reg, &tmp) > 0)
      {
        /* flip bit back */
        tmp ^= (uint8_t) (1 << (bit));
        if (i2c_mcp23017_write_register(address, reg, tmp) > 0)
        {
          *data = tmp;
          return 1;
        }
      }
    }
  }

  return 0;
}
