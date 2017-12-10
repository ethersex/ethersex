/*
 * Copyright (c) 2017 Erik Kunze <ethersex@erik-kunze.de>
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
#include <util/atomic.h>

#include "config.h"
#include "core/debug.h"
#include "core/spi.h"

#include "glcd_color.h"


#ifdef GLCD_SPI_SUPPORT

#ifdef GLCD_SPI_TRANSACTION
#define glcd_prolog()    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
#define glcd_epilogue()  }
#else
#define glcd_prolog()
#define glcd_epilogue()
#endif

static int16_t
glcd_ucg_com_4wire_spi(ucg_t * ucg, int16_t msg, uint16_t arg, uint8_t * data)
{
  switch (msg)
  {
    case UCG_COM_MSG_POWER_UP:
#ifdef HAVE_GLCD_RESET
      glcd_rst_configure();
#endif
      glcd_mode_configure();
      glcd_cs_configure();
      break;
    case UCG_COM_MSG_POWER_DOWN:
      break;
    case UCG_COM_MSG_DELAY:
      for (; arg >= 1000; _delay_ms(1), arg -= 1000);
      for (; arg >= 100; _delay_us(100), arg -= 100);
      for (; arg >= 10; _delay_us(10), arg -= 10);
      for (; arg >= 1; _delay_us(1), arg -= 1);
      break;
#ifdef HAVE_GLCD_RESET
    case UCG_COM_MSG_CHANGE_RESET_LINE:
      if (arg)
        glcd_rst_high();
      else
        glcd_rst_low();
      break;
#endif
    case UCG_COM_MSG_CHANGE_CS_LINE:
      if (arg)
        glcd_cs_high();
      else
        glcd_cs_low();
      break;
    case UCG_COM_MSG_CHANGE_CD_LINE:
      if (arg)
        glcd_mode_data();
      else
        glcd_mode_cmd();
      break;
    case UCG_COM_MSG_SEND_BYTE:
      glcd_prolog();
      spi_send(arg);
      glcd_epilogue();
      break;
    case UCG_COM_MSG_REPEAT_1_BYTE:
      glcd_prolog();
      while (arg > 0)
      {
        spi_send(data[0]);
        arg--;
      }
      glcd_epilogue();
      break;
    case UCG_COM_MSG_REPEAT_2_BYTES:
      glcd_prolog();
      while (arg > 0)
      {
        spi_send(data[0]);
        spi_send(data[1]);
        arg--;
      }
      glcd_epilogue();
      break;
    case UCG_COM_MSG_REPEAT_3_BYTES:
      glcd_prolog();
      while (arg > 0)
      {
        spi_send(data[0]);
        spi_send(data[1]);
        spi_send(data[2]);
        arg--;
      }
      glcd_epilogue();
      break;
    case UCG_COM_MSG_SEND_STR:
      glcd_prolog();
      while (arg > 0)
      {
        spi_send(*data++);
        arg--;
      }
      glcd_epilogue();
      break;
    case UCG_COM_MSG_SEND_CD_DATA_SEQUENCE:
      glcd_prolog();
      while (arg > 0)
      {
        if (*data != 0)
        {
          /* set the data line directly, ignore the setting from UCG_CFG_CD */
          if (*data == 1)
          {
            glcd_mode_data();
          }
          else
          {
            glcd_mode_cmd();
          }
        }
        data++;
        spi_send(*data);
        data++;
        arg--;
      }
      glcd_epilogue();
      break;
  }

  return 1;
}

#endif /* GLCD_SPI_SUPPORT */

#ifdef DEBUG_GLCD_HW_COM

static int16_t
glcd_ucg_com_debug(ucg_t * ucg, int16_t msg, uint32_t arg, uint8_t * data)
{
  const char *m;

  switch (msg)
  {
    case UCG_COM_MSG_POWER_UP:
      m = PSTR("UCG_COM_MSG_POWER_UP");
      break;
    case UCG_COM_MSG_POWER_DOWN:
      m = PSTR("UCG_COM_MSG_POWER_DOWN");
      break;
    case UCG_COM_MSG_DELAY:
      m = PSTR("UCG_COM_MSG_DELAY");
      break;
    case UCG_COM_MSG_CHANGE_RESET_LINE:
      m = PSTR("UCG_COM_MSG_CHANGE_RESET_LINE");
      break;
    case UCG_COM_MSG_CHANGE_CS_LINE:
      m = PSTR("UCG_COM_MSG_CHANGE_CS_LINE");
      break;
    case UCG_COM_MSG_CHANGE_CD_LINE:
      m = PSTR("UCG_COM_MSG_CHANGE_CD_LINE");
      break;
    case UCG_COM_MSG_SEND_BYTE:
      m = PSTR("UCG_COM_MSG_SEND_BYTE");
      break;
    case UCG_COM_MSG_REPEAT_1_BYTE:
      m = PSTR("UCG_COM_MSG_REPEAT_1_BYTE");
      break;
    case UCG_COM_MSG_REPEAT_2_BYTES:
      m = PSTR("UCG_COM_MSG_REPEAT_2_BYTES");
      break;
    case UCG_COM_MSG_REPEAT_3_BYTES:
      m = PSTR("UCG_COM_MSG_REPEAT_3_BYTES");
      break;
    case UCG_COM_MSG_SEND_STR:
      m = PSTR("UCG_COM_MSG_SEND_STR");
      break;
    case UCG_COM_MSG_SEND_CD_DATA_SEQUENCE:
      m = PSTR("UCG_COM_MSG_SEND_CD_DATA_SEQUENCE");
      break;
  }

  DEBUG_LCD(glcd_ucg_com_debug, "%S com_status=0x%02x arg=%lu\n", m,
            ucg->com_status, arg);

  return 1;
}

#endif /* DEBUG_GLCD_HW_COM */

#if CONF_GLCD_COLOR_TYPE == GLCD_COLOR_ILI9163_128x128
#define UCG_DEV_CB ucg_dev_ili9163_18x128x128
#define UCG_EXT_CB ucg_ext_ili9163_18
#elif CONF_GLCD_COLOR_TYPE ==  GLCD_COLOR_ILI9341_240x320
#define UCG_DEV_CB ucg_dev_ili9341_18x240x320
#define UCG_EXT_CB ucg_ext_ili9341_18
#elif CONF_GLCD_COLOR_TYPE == GLCD_COLOR_SEPS225_128x128_UNIVISION
#define UCG_DEV_CB ucg_dev_seps225_16x128x128_univision
#define UCG_EXT_CB ucg_ext_seps225_16
#elif CONF_GLCD_COLOR_TYPE == GLCD_COLOR_SSD1331_96x64_UNIVISION
#define UCG_DEV_CB ucg_dev_ssd1331_18x96x64_univision
#define UCG_EXT_CB ucg_ext_ssd1331_18
#elif CONF_GLCD_COLOR_TYPE == GLCD_COLOR_SSD1351_128x128
#define UCG_DEV_CB ucg_dev_ssd1351_18x128x128_ilsoft
#define UCG_EXT_CB ucg_ext_ssd1351_18
#elif CONF_GLCD_COLOR_TYPE == GLCD_COLOR_SSD1351_128x128_FT
#define UCG_DEV_CB ucg_dev_ssd1351_18x128x128_ft
#define UCG_EXT_CB ucg_ext_ssd1351_18
#elif CONF_GLCD_COLOR_TYPE == GLCD_COLOR_ST7735_128x160
#define UCG_DEV_CB ucg_dev_st7735_18x128x160
#define UCG_EXT_CB ucg_ext_st7735_18
#endif

#ifdef DEBUG_GLCD_HW_COM
#define UCG_COM_CB glcd_ucg_com_debug
#elif defined(GLCD_SPI_SUPPORT)
#define UCG_COM_CB glcd_ucg_com_4wire_spi
#else
#error unsupported HW
#endif

void
glcd_init(void)
{
  ucg_Init(&glcd, UCG_DEV_CB, UCG_EXT_CB, UCG_COM_CB);
  ucg_SetFontMode(&glcd, UCG_FONT_MODE_TRANSPARENT);
  ucg_ClearScreen(&glcd);
}

int16_t
glcd_draw_string_P(int16_t x, int16_t y, uint8_t dir, const char *s)
{
  int16_t delta, w = 0;

  for (;;)
  {
    uint8_t encoding = pgm_read_byte(s++);
    if (encoding == 0)
      break;

    delta = glcd_draw_glyph(x, y, dir, encoding);

    switch (dir)
    {
      case GLCD_DIR_LTR:
        x += delta;
        break;
      case GLCD_DIR_RTL:
        x -= delta;
        break;
      case GLCD_DIR_DOWN:
        y += delta;
        break;
      case GLCD_DIR_UP:
        y -= delta;
        break;
    }

    w += delta;
  }

  return w;
}
