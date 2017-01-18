/*
 * hd44780 driver library fur use of hd44780 with i2c expander
 *
 * (c) by Alexander Neumann <alexander@bumpern.de>
 * (c) by Alexander Tuschen <atuschen@web.de>
 * (c) by Mirko Taschenberger <mirkiway at gmx.de>
 * (c) 2014 by Daniel Lindner <daniel.lindner at gmx.de>
 * Copyright (c) 2016 Michael Brakemeier <michael@brakemeier.de>
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

#include "config.h"

#include "hd44780.h"

#include "core/debug.h"

/* include the appropriate header for the i2c port expander */
#if CONF_HD44780_I2C_PORTEXP == HD44780_I2C_PCF8574
#include "hardware/i2c/master/i2c_pcf8574x.h"
#elif CONF_HD44780_I2C_PORTEXP == HD44780_I2C_MCP23017
#include "hardware/i2c/master/i2c_mcp23017.h"
#else
#error "HD44780 - invalid value for I2C Port Expander!"
#endif

#if defined(DEBUG_HD44780_I2C) && !defined(TEENSY_SUPPORT)
#define DEBUG_LCD(fnc, s, msg...) printf_P(PSTR("D: LCD: " #fnc ": " s), ## msg)
#else
#define DEBUG_LCD(fnc, s, msg...)
#endif

/* global variables */
static uint8_t lcd_data;

#if CONF_HD44780_I2C_PORTEXP == HD44780_I2C_PCF8574
#define HD44780_I2C_ADR  HD44780_PCF8574x_ADR
#define HD44780_I2C_RS   HD44780_PCF8574x_RS
#define HD44780_I2C_WR   HD44780_PCF8574x_WR
#define HD44780_I2C_EN   HD44780_PCF8574x_EN
#define HD44780_I2C_EN1  HD44780_PCF8574x_EN1
#define HD44780_I2C_EN2  HD44780_PCF8574x_EN2
#define HD44780_I2C_DB4  HD44780_PCF8574x_DB4
#define HD44780_I2C_DB5  HD44780_PCF8574x_DB5
#define HD44780_I2C_DB6  HD44780_PCF8574x_DB6
#define HD44780_I2C_DB7  HD44780_PCF8574x_DB7
#define HD44780_I2C_BL   HD44780_PCF8574x_BL
#elif CONF_HD44780_I2C_PORTEXP == HD44780_I2C_MCP23017
#define HD44780_I2C_ADR  HD44780_MCP23017_ADR
#define HD44780_I2C_RS   HD44780_MCP23017_RS
#define HD44780_I2C_WR   HD44780_MCP23017_WR
#define HD44780_I2C_EN   HD44780_MCP23017_EN
#define HD44780_I2C_EN1  HD44780_MCP23017_EN1
#define HD44780_I2C_EN2  HD44780_MCP23017_EN2
#define HD44780_I2C_DB4  HD44780_MCP23017_DB4
#define HD44780_I2C_DB5  HD44780_MCP23017_DB5
#define HD44780_I2C_DB6  HD44780_MCP23017_DB6
#define HD44780_I2C_DB7  HD44780_MCP23017_DB7
#define HD44780_I2C_BL   HD44780_MCP23017_BL
#else
#error "HD44780 - invalid value for I2C Port Expander!"
#endif

/* Initialize connection circuitry */
void noinline
hd44780_hw_init(void)
{
  /* clear all outputs */
  lcd_data = 0;

#ifdef HD44780_BACKLIGHT_SUPPORT
  /* init backlight first */
#ifndef HD44780_MULTIEN_BACKLIGHT_SUPPORT
#ifdef HD44780_BACKLIGHT_INV
  if (hd44780_backlight_state)
    lcd_data &= ~(_BV(HD44780_I2C_BL));
  else
    lcd_data |= _BV(HD44780_I2C_BL);
#else
  if (hd44780_backlight_state)
    lcd_data |= _BV(HD44780_I2C_BL);
  else
    lcd_data &= ~(_BV(HD44780_I2C_BL));
#endif
#else /* HD44780_MULTIEN_BACKLIGHT_SUPPORT */
#if CONF_HD44780_I2C_PORTEXP == HD44780_I2C_MCP23017
  /* set backlight databus line to output and switch BL off */
  uint8_t curval = 0;
  i2c_mcp23017_output_state mcp_state;

#ifdef HD44780_BACKLIGHT_INV
  if (hd44780_backlight_state)
    mcp_state = OFF;
  else
    mcp_state = ON;
#else
  if (hd44780_backlight_state)
    mcp_state = ON;
  else
    mcp_state = OFF;
#endif

  /* set backlight pin to output */
  if (i2c_mcp23017_read_register
      (HD44780_MCP23017_BL_ADR, HD44780_MCP23017_BL_IODIR, &curval) > 0)
  {
    curval &= ~(_BV(HD44780_I2C_BL));
    if (i2c_mcp23017_write_register
        (HD44780_MCP23017_BL_ADR, HD44780_MCP23017_BL_IODIR, curval) > 0)
    {
      /* switch backlight off */
      if (i2c_mcp23017_modify_pin
          (HD44780_MCP23017_BL_ADR, HD44780_MCP23017_BL_OLAT, &curval,
           HD44780_I2C_BL, mcp_state) == 0)
      {
        DEBUG_LCD(hd44780_hw_init,
                  "failed to switch BL OLAT w/ data 0x%X [(0x%X)]\n", curval,
                  HD44780_MCP23017_BL_ADR);
      }
    }
  }
#else
#error "HD44780 - invalid value for BL I2C Port Expander!"
#endif /* CONF_HD44780_I2C_PORTEXP == HD44780_I2C_MCP23017 */
#endif /* HD44780_MULTIEN_BACKLIGHT_SUPPORT */
#endif /* HD44780_BACKLIGHT_SUPPORT */

#if CONF_HD44780_I2C_PORTEXP == HD44780_I2C_PCF8574
  /* write lcd_data to i2c port expander */
  i2c_pcf8574x_set(HD44780_PCF8574x_ADR, lcd_data);
#elif CONF_HD44780_I2C_PORTEXP == HD44780_I2C_MCP23017
  /* set all databus lines to output */
  if (i2c_mcp23017_write_register
      (HD44780_MCP23017_ADR, HD44780_MCP23017_IODIR, 0x00) > 0)
  {
    /* send lcd_data to MCP */
    if (i2c_mcp23017_write_register
        (HD44780_MCP23017_ADR, HD44780_MCP23017_OLAT, lcd_data) == 0)
    {
      DEBUG_LCD(hd44780_hw_init,
                "failed to write OLAT w/ data 0x%X [(0x%X)]\n", lcd_data,
                HD44780_I2C_ADR);
    }
  }
#else
#error "HD44780 - invalid value for I2C Port Expander!"
#endif
}

uint8_t noinline
hd44780_clock_rw(uint8_t read, uint8_t en)
{
  uint8_t data = 0;
  uint8_t enPin = 0;

  /* map number of en to actual pin */
#ifdef HD44780_MULTIEN_SUPPORT
  if (en == 1)
  {
    enPin = HD44780_I2C_EN1;
  }
  else if (en == 2)
  {
    enPin = HD44780_I2C_EN2;
  }
  else
  {
    DEBUG_LCD(hd44780_clock_rw, "#### unknown EN!\n");
  }
#else
  enPin = HD44780_I2C_EN;
#endif

#ifdef HD44780_READBACK_SUPPORT
  if (read)
  {
    /* pulse EN and read data */
    uint8_t i2c_read_data;

    lcd_data |=
      (_BV(HD44780_I2C_DB4) | _BV(HD44780_I2C_DB5) | _BV(HD44780_I2C_DB6) |
       _BV(HD44780_I2C_DB7));
#if CONF_HD44780_I2C_PORTEXP == HD44780_I2C_PCF8574
    /* set DBx high while reading, ref. PCF8574 datasheet */
    i2c_pcf8574x_set(HD44780_PCF8574x_ADR, lcd_data);
    /* set EN */
    lcd_data |= _BV(enPin);
    /* send data to PCF */
    i2c_pcf8574x_set(HD44780_PCF8574x_ADR, lcd_data);
    /* and read from PCF */
    i2c_read_data = i2c_pcf8574x_read(HD44780_PCF8574x_ADR);
#elif CONF_HD44780_I2C_PORTEXP == HD44780_I2C_MCP23017
    /* set direction of databus lines to input */
    uint8_t iodir =
      (_BV(HD44780_I2C_DB4) | _BV(HD44780_I2C_DB5) | _BV(HD44780_I2C_DB6) |
       _BV(HD44780_I2C_DB7));
    if (i2c_mcp23017_write_register
        (HD44780_MCP23017_ADR, HD44780_MCP23017_IODIR, iodir) > 0)
    {
      /* set EN */
      lcd_data |= _BV(enPin);
      /* send lcd_data to MCP */
      if (i2c_mcp23017_write_register
          (HD44780_MCP23017_ADR, HD44780_MCP23017_OLAT, lcd_data) > 0)
      {
        if (i2c_mcp23017_read_register
            (HD44780_MCP23017_ADR, HD44780_MCP23017_GPIO,
             &i2c_read_data) == 0)
        {
          DEBUG_LCD(hd44780_clock_rw, "failed to read GPIO [(0x%X)]\n",
                    HD44780_I2C_ADR);
        }
      }
      /* reset direction of databus lines to output again */
      if (i2c_mcp23017_write_register
          (HD44780_MCP23017_ADR, HD44780_MCP23017_IODIR, 0x00) == 0)
      {
        DEBUG_LCD(hd44780_clock_rw, "failed to write IODIR [(0x%X)]\n",
                  HD44780_I2C_ADR);
      }
    }
#else
#error "HD44780 - invalid value for I2C Port Expander!"
#endif

    if (i2c_read_data & _BV(HD44780_I2C_DB4))
      data |= _BV(0);
    if (i2c_read_data & _BV(HD44780_I2C_DB5))
      data |= _BV(1);
    if (i2c_read_data & _BV(HD44780_I2C_DB6))
      data |= _BV(2);
    if (i2c_read_data & _BV(HD44780_I2C_DB7))
      data |= _BV(3);

    DEBUG_LCD(hd44780_clock_rw,
              "I2C-LCD-R: read data 0x%X  raw(0x%X) [(%u) (0x%X)]\n", data,
              i2c_read_data, enPin, HD44780_I2C_ADR);
  }
  else
#endif /* HD44780_READBACK_SUPPORT */
  {
    /* write data any pulse EN */
    DEBUG_LCD(hd44780_clock_rw, "I2C-LCD-W: write data 0x%X [(%u) (0x%X)]\n",
              lcd_data, enPin, HD44780_I2C_ADR);

#if CONF_HD44780_I2C_PORTEXP == HD44780_I2C_PCF8574
    i2c_pcf8574x_set(HD44780_PCF8574x_ADR, lcd_data);
    /* set EN */
    lcd_data |= _BV(enPin);
    /* and send data */
    i2c_pcf8574x_set(HD44780_PCF8574x_ADR, lcd_data);
#elif CONF_HD44780_I2C_PORTEXP == HD44780_I2C_MCP23017
    if (i2c_mcp23017_write_register
        (HD44780_MCP23017_ADR, HD44780_MCP23017_OLAT, lcd_data) == 0)
    {
      DEBUG_LCD(hd44780_clock_rw,
                "failed to write OLAT w/ data 0x%X [(0x%X)]\n", lcd_data,
                HD44780_I2C_ADR);
    }
    /* set EN */
    lcd_data |= _BV(enPin);
    /* and send data */
    if (i2c_mcp23017_write_register
        (HD44780_MCP23017_ADR, HD44780_MCP23017_OLAT, lcd_data) == 0)
    {
      DEBUG_LCD(hd44780_clock_rw,
                "failed to write OLAT w/ data 0x%X [(0x%X)]\n", lcd_data,
                HD44780_I2C_ADR);
    }
#else
#error "HD44780 - invalid value for I2C Port Expander!"
#endif

  }

  /* clear EN */
  lcd_data &= ~(_BV(enPin));
#if CONF_HD44780_I2C_PORTEXP == HD44780_I2C_PCF8574
  i2c_pcf8574x_set(HD44780_PCF8574x_ADR, lcd_data);
#elif CONF_HD44780_I2C_PORTEXP == HD44780_I2C_MCP23017
  if (i2c_mcp23017_write_register
      (HD44780_MCP23017_ADR, HD44780_MCP23017_OLAT, lcd_data) == 0)
  {
    DEBUG_LCD(hd44780_clock_rw,
              "failed to write OLAT w/ data 0x%X [(0x%X)]\n", lcd_data,
              HD44780_I2C_ADR);
  }
#else
#error "HD44780 - invalid value for I2C Port Expander!"
#endif

  return data;
}

void noinline
hd44780_output_4bit(uint8_t rs, uint8_t nibble, uint8_t en)
{
  /* compute data bits */
  lcd_data = 0;

  if (nibble & _BV(0))
    lcd_data |= _BV(HD44780_I2C_DB4);
  if (nibble & _BV(1))
    lcd_data |= _BV(HD44780_I2C_DB5);
  if (nibble & _BV(2))
    lcd_data |= _BV(HD44780_I2C_DB6);
  if (nibble & _BV(3))
    lcd_data |= _BV(HD44780_I2C_DB7);

  /* set RS */
  if (rs)
    lcd_data |= _BV(HD44780_I2C_RS);

#if defined(HD44780_BACKLIGHT_SUPPORT) && !defined(HD44780_MULTIEN_BACKLIGHT_SUPPORT)
#ifdef HD44780_BACKLIGHT_INV
  if (hd44780_backlight_state == 0)
    lcd_data |= _BV(HD44780_I2C_BL);
#else
  if (hd44780_backlight_state)
    lcd_data |= _BV(HD44780_I2C_BL);
#endif
#endif

  /* let clock_rw transfer the data */
  hd44780_clock_write(en);
}

#ifdef HD44780_READBACK_SUPPORT
uint8_t noinline
hd44780_input_4bit(uint8_t rs, uint8_t en)
{
  /* set WR means read */
  lcd_data |= _BV(HD44780_I2C_WR);

  /* (re-)set RS */
  if (rs)                       /* set RS */
    lcd_data |= _BV(HD44780_I2C_RS);
  else                          /* clear RS */
    lcd_data &= ~(_BV(HD44780_I2C_RS));

  /* read data using clock_rw */
  uint8_t data = hd44780_clock_read(en);

  /* and clear WR again */
  lcd_data &= ~(_BV(HD44780_I2C_WR));

  return data;
}
#endif

#ifdef HD44780_BACKLIGHT_SUPPORT
void
hd44780_backlight(uint8_t state)
{
  hd44780_backlight_state = state;

#ifndef HD44780_MULTIEN_BACKLIGHT_SUPPORT
#ifdef HD44780_BACKLIGHT_INV
  if (hd44780_backlight_state)
    lcd_data &= ~(_BV(HD44780_I2C_BL));
  else
    lcd_data |= _BV(HD44780_I2C_BL);
#else
  if (hd44780_backlight_state)
    lcd_data |= _BV(HD44780_I2C_BL);
  else
    lcd_data &= ~(_BV(HD44780_I2C_BL));
#endif

  /* write lcd_data to i2c port expander */
#if CONF_HD44780_I2C_PORTEXP == HD44780_I2C_PCF8574
  i2c_pcf8574x_set(HD44780_PCF8574x_ADR, lcd_data);
#elif CONF_HD44780_I2C_PORTEXP == HD44780_I2C_MCP23017
  if (i2c_mcp23017_write_register
      (HD44780_MCP23017_ADR, HD44780_MCP23017_OLAT, lcd_data) == 0)
  {
    DEBUG_LCD(hd44780_backlight,
              "failed to write OLAT w/ data 0x%X [(0x%X)]\n", lcd_data,
              HD44780_I2C_ADR);
  }
#else
#error "HD44780 - invalid value for I2C Port Expander!"
#endif
#else /* HD44780_MULTIEN_BACKLIGHT_SUPPORT */
#if CONF_HD44780_I2C_PORTEXP == HD44780_I2C_MCP23017
  /* switch BL */
  uint8_t curval = 0;
  i2c_mcp23017_output_state mcp_state;

#ifdef HD44780_BACKLIGHT_INV
  if (hd44780_backlight_state)
    mcp_state = OFF;
  else
    mcp_state = ON;
#else
  if (hd44780_backlight_state)
    mcp_state = ON;
  else
    mcp_state = OFF;
#endif

  /* switch backlight */
  if (i2c_mcp23017_modify_pin
      (HD44780_MCP23017_BL_ADR, HD44780_MCP23017_BL_OLAT, &curval,
       HD44780_I2C_BL, mcp_state) == 0)
  {
    DEBUG_LCD(hd44780_hw_init,
              "failed to switch BL OLAT w/ data 0x%X [(0x%X)]\n", curval,
              HD44780_MCP23017_BL_ADR);
  }
#else
#error "HD44780 - invalid value for BL I2C Port Expander!"
#endif /* CONF_HD44780_I2C_PORTEXP == HD44780_I2C_MCP23017 */
#endif /* HD44780_MULTIEN_BACKLIGHT_SUPPORT */
}
#endif
