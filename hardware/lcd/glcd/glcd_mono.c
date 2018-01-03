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
#include <avr/interrupt.h>
#include <util/twi.h>

#include "config.h"
#include "core/debug.h"
#include "core/spi.h"
#include "hardware/i2c/master/i2c_master.h"

#include "glcd_mono.h"


#define GLCD_MONO_ROTATION_NONE    U8G2_R0
#define GLCD_MONO_ROTATION_90      U8G2_R1
#define GLCD_MONO_ROTATION_180     U8G2_R2
#define GLCD_MONO_ROTATION_270     U8G2_R3
#define GLCD_MONO_ROTATION_MIRROR  U8G2_MIRROR


#ifdef GLCD_I2C_SUPPORT

static uint8_t
glcd_u8x8_byte_hw_i2c(u8x8_t * u8x8, uint8_t msg, uint8_t arg, void *data)
{
  uint8_t *p;
  switch (msg)
  {
    case U8X8_MSG_BYTE_INIT:
      DEBUG_LCD(glcd_u8x8_byte_hw_i2c, "addr=%02x\n",
                u8x8_GetI2CAddress(u8x8));
      break;
    case U8X8_MSG_BYTE_START_TRANSFER:
      if (!i2c_master_select(u8x8_GetI2CAddress(u8x8) >> 1, TW_WRITE))
      {
        DEBUG_LCD(glcd_u8x8_byte_hw_i2c, "I2C master select failed\n");
        return 0;
      }
      break;
    case U8X8_MSG_BYTE_END_TRANSFER:
      i2c_master_stop();
      break;
    case U8X8_MSG_BYTE_SEND:
      p = (uint8_t *) data;
      while (arg > 0)
      {
        TWDR = *p++;
        uint8_t result = i2c_master_transmit_with_ack();
        if (result != TW_MT_DATA_ACK)
        {
          DEBUG_LCD(glcd_u8x8_byte_hw_i2c, "I2C transmit failed: %02x\n",
                    result);
          return 0;
        }
        arg--;
      }
      break;
  }

  return 1;
}

static uint8_t
glcd_u8x8_gpio_and_delay_hw_i2c(u8x8_t * _u8g2, uint8_t msg, uint8_t arg,
                                void *data)
{
  switch (msg)
  {
    case U8X8_MSG_DELAY_10MICRO:
      for (; arg >= 10; _delay_us(10), arg -= 10);
      break;
    case U8X8_MSG_DELAY_MILLI:
      for (; arg >= 100; _delay_ms(100), arg -= 100);
      for (; arg >= 10; _delay_ms(10), arg -= 10);
      for (; arg >= 1; _delay_ms(1), arg -= 1);
      break;
  }

  return 1;
}

#endif /* GLCD_I2C_SUPPORT */

#ifdef GLCD_SPI_SUPPORT

static uint8_t
glcd_u8x8_byte_hw_spi(u8x8_t * u8x8, uint8_t msg, uint8_t arg, void *data)
{
#ifdef GLCD_SPI_TRANSACTION
  static uint8_t sreg_save;
#endif
  uint8_t *p;

  switch (msg)
  {
    case U8X8_MSG_BYTE_INIT:
#ifdef HAVE_GLCD_RESET
      glcd_rst_configure();
#endif
      glcd_mode_configure();
      glcd_cs_configure();
      u8x8_gpio_SetCS(u8x8, u8x8->display_info->chip_disable_level);
      break;
    case U8X8_MSG_BYTE_SET_DC:
      u8x8_gpio_SetDC(u8x8, arg);
      break;
    case U8X8_MSG_BYTE_START_TRANSFER:
#ifdef GLCD_SPI_TRANSACTION
      sreg = SREG;
      cli();
#endif
      u8x8_gpio_SetCS(u8x8, u8x8->display_info->chip_enable_level);
      break;
    case U8X8_MSG_BYTE_END_TRANSFER:
      u8x8_gpio_SetCS(u8x8, u8x8->display_info->chip_disable_level);
#ifdef GLCD_SPI_TRANSACTION
      SREG = sreg;
#endif
      break;
    case U8X8_MSG_BYTE_SEND:
      p = (uint8_t *) data;
      while (arg > 0)
      {
        spi_send(*p++);
        arg--;
      }
      break;
  }

  return 1;
}

static uint8_t
glcd_u8x8_gpio_and_delay_hw_spi(u8x8_t * u8g2, uint8_t msg, uint8_t arg,
                                void *data)
{
  switch (msg)
  {
    case U8X8_MSG_DELAY_10MICRO:
      for (; arg >= 10; _delay_us(10), arg -= 10);
      break;
    case U8X8_MSG_DELAY_MILLI:
      for (; arg >= 100; _delay_ms(100), arg -= 100);
      for (; arg >= 10; _delay_ms(10), arg -= 10);
      for (; arg >= 1; _delay_ms(1), arg -= 1);
      break;
#ifdef HAVE_GLCD_RESET
    case U8X8_MSG_GPIO_RESET:
      if (arg)
        glcd_rst_high();
      else
        glcd_rst_low();
      break;
#endif
    case U8X8_MSG_GPIO_CS:
      if (arg)
        glcd_cs_high();
      else
        glcd_cs_low();
      break;
    case U8X8_MSG_GPIO_DC:
      if (arg)
        glcd_mode_data();
      else
        glcd_mode_cmd();
      break;
  }

  return 1;
}

#endif /* GLCD_SPI_SUPPORT */

#if CONF_GLCD_MONO_TYPE == GLCD_MONO_IL3820_296x128_SPI
#define U8G2_SETUP u8g2_Setup_il3820_296x128_1
#elif CONF_GLCD_MONO_TYPE == GLCD_MONO_IST3020_ERC19264_SPI
#define U8G2_SETUP u8g2_Setup_ist3020_erc19264_1
#elif CONF_GLCD_MONO_TYPE == GLCD_MONO_LD7032_60x32_I2C
#define U8G2_SETUP u8g2_Setup_ld7032_60x32_1
#elif CONF_GLCD_MONO_TYPE == GLCD_MONO_LD7032_60x32_SPI
#define U8G2_SETUP u8g2_Setup_ld7032_60x32_1
#elif CONF_GLCD_MONO_TYPE == GLCD_MONO_LS013B7DH03_128x128_SPI
#define U8G2_SETUP u8g2_Setup_ls013b7dh03_128x128_1
#elif CONF_GLCD_MONO_TYPE == GLCD_MONO_MAX7219_32x8_SPI
#define U8G2_SETUP u8g2_Setup_max7219_32x8_1
#elif CONF_GLCD_MONO_TYPE == GLCD_MONO_NT7534_TG12864R_SPI
#define U8G2_SETUP u8g2_Setup_nt7534_tg12864r_1
#elif CONF_GLCD_MONO_TYPE == GLCD_MONO_PCD8544_84x48_SPI
#define U8G2_SETUP u8g2_Setup_pcd8544_84x48_1
#elif CONF_GLCD_MONO_TYPE == GLCD_MONO_PCF8812_96x65_SPI
#define U8G2_SETUP u8g2_Setup_pcf8812_96x65_1
#elif CONF_GLCD_MONO_TYPE == GLCD_MONO_SH1106_128x64_I2C
#define U8G2_SETUP u8g2_Setup_sh1106_i2c_128x64_noname_1
#elif CONF_GLCD_MONO_TYPE == GLCD_MONO_SH1106_128x64_SPI
#define U8G2_SETUP u8g2_Setup_sh1106_128x64_noname_1
#elif CONF_GLCD_MONO_TYPE == GLCD_MONO_SSD1305_128x32_I2C
#define U8G2_SETUP u8g2_Setup_ssd1305_i2c_128x32_noname_1
#elif CONF_GLCD_MONO_TYPE == GLCD_MONO_SSD1305_128x32_SPI
#define U8G2_SETUP u8g2_Setup_ssd1305_128x32_noname_1
#elif CONF_GLCD_MONO_TYPE == GLCD_MONO_SSD1306_64x32_I2C
#define U8G2_SETUP u8g2_Setup_ssd1306_i2c_64x32_noname_1
#elif CONF_GLCD_MONO_TYPE == GLCD_MONO_SSD1306_64x32_SPI
#define U8G2_SETUPA u8g2_Setup_ssd1306_64x32_noname_1
#elif CONF_GLCD_MONO_TYPE == GLCD_MONO_SSD1306_64x48_I2C
#define U8G2_SETUP u8g2_Setup_ssd1306_i2c_64x48_er_1
#elif CONF_GLCD_MONO_TYPE == GLCD_MONO_SSD1306_64x48_SPI
#define U8G2_SETUP u8g2_Setup_ssd1306_64x48_er_1
#elif CONF_GLCD_MONO_TYPE == GLCD_MONO_SSD1306_96x16_I2C
#define U8G2_SETUP u8g2_Setup_ssd1306_i2c_96x16_er_1
#elif CONF_GLCD_MONO_TYPE == GLCD_MONO_SSD1306_96x16_SPI
#define U8G2_SETUP u8g2_Setup_ssd1306_96x16_er_1
#elif CONF_GLCD_MONO_TYPE == GLCD_MONO_SSD1306_128x64_I2C
#define U8G2_SETUP u8g2_Setup_ssd1306_i2c_128x64_noname_1
#elif CONF_GLCD_MONO_TYPE == GLCD_MONO_SSD1306_128x64_SPI
#define U8G2_SETUP u8g2_Setup_ssd1306_128x64_noname_1
#elif CONF_GLCD_MONO_TYPE == GLCD_MONO_SSD1309_128x64_I2C
#define U8G2_SETUP u8g2_Setup_ssd1309_i2c_128x64_noname0_1
#elif CONF_GLCD_MONO_TYPE == GLCD_MONO_SSD1309_128x64_SPI
#define U8G2_SETUP u8g2_Setup_ssd1309_128x64_noname2_1
#elif CONF_GLCD_MONO_TYPE == GLCD_MONO_SSD1322_256x64_NHD_SPI
#define U8G2_SETUP u8g2_Setup_ssd1322_nhd_256x64_1
#elif CONF_GLCD_MONO_TYPE == GLCD_MONO_SSD1325_128x64_I2C
#define U8G2_SETUP u8g2_Setup_ssd1325_i2c_nhd_128x64_1
#elif CONF_GLCD_MONO_TYPE == GLCD_MONO_SSD1325_128x64_SPI
#define U8G2_SETUP u8g2_Setup_ssd1325_nhd_128x64_1
#elif CONF_GLCD_MONO_TYPE == GLCD_MONO_SSD1326_256x32_ER_I2C
#define U8G2_SETUP u8g2_Setup_ssd1326_er_256x32_1
#elif CONF_GLCD_MONO_TYPE == GLCD_MONO_SSD1326_256x32_ER_SPI
#define U8G2_SETUP u8g2_Setup_ssd1326_er_256x32_1
#elif CONF_GLCD_MONO_TYPE == GLCD_MONO_SSD1327_96x96_I2C
#define U8G2_SETUP u8g2_Setup_ssd1327_i2c_seeed_96x96_1
#elif CONF_GLCD_MONO_TYPE == GLCD_MONO_SSD1327_96x96_SPI
#define U8G2_SETUP u8g2_Setup_ssd1327_seeed_96x96_1
#elif CONF_GLCD_MONO_TYPE == GLCD_MONO_SSD1327_128x128_SPI
#define U8G2_SETUP u8g2_Setup_ssd1327_midas_128x128_1
#elif CONF_GLCD_MONO_TYPE == GLCD_MONO_SSD1329_128x96_SPI
#define U8G2_SETUP u8g2_Setup_ssd1329_128x96_noname_1
#elif CONF_GLCD_MONO_TYPE == GLCD_MONO_SSD1606_172x72_SPI
#define U8G2_SETUP u8g2_Setup_ssd1606_172x72_1
#elif CONF_GLCD_MONO_TYPE == GLCD_MONO_SSD1607_200x200_SPI
#define U8G2_SETUP u8g2_Setup_ssd1607_200x200_1
#elif CONF_GLCD_MONO_TYPE == GLCD_MONO_ST75256_JLX172104_I2C
#define U8G2_SETUP u8g2_Setup_st75256_i2c_jlx172104_1
#elif CONF_GLCD_MONO_TYPE == GLCD_MONO_ST75256_JLX172104_SPI
#define U8G2_SETUP u8g2_Setup_st75256_jlx172104_1
#elif CONF_GLCD_MONO_TYPE == GLCD_MONO_ST75256_JLX256128_I2C
#define U8G2_SETUP u8g2_Setup_st75256_i2c_jlx256128_1
#elif CONF_GLCD_MONO_TYPE == GLCD_MONO_ST75256_JLX256128_SPI
#define U8G2_SETUP u8g2_Setup_st75256_jlx256128_1
#elif CONF_GLCD_MONO_TYPE == GLCD_MONO_ST75256_JLX25664_I2C
#define U8G2_SETUP u8g2_Setup_st75256_i2c_jlx25664_1
#elif CONF_GLCD_MONO_TYPE == GLCD_MONO_ST75256_JLX25664_SPI
#define U8G2_SETUP u8g2_Setup_st75256_jlx25664_1
#elif CONF_GLCD_MONO_TYPE == GLCD_MONO_ST7565_128x64_ZOLEN_SPI
#define U8G2_SETUP u8g2_Setup_st7565_zolen_128x64_1
#elif CONF_GLCD_MONO_TYPE == GLCD_MONO_ST7565_64128N_SPI
#define U8G2_SETUP u8g2_Setup_st7565_64128n_1
#elif CONF_GLCD_MONO_TYPE == GLCD_MONO_ST7565_EA_DOGM128_SPI
#define U8G2_SETUP u8g2_Setup_st7565_ea_dogm128_1
#elif CONF_GLCD_MONO_TYPE == GLCD_MONO_ST7565_EA_DOGM132_SPI
#define U8G2_SETUP u8g2_Setup_st7565_ea_dogm132_1
#elif CONF_GLCD_MONO_TYPE == GLCD_MONO_ST7565_ERC12864_SPI
#define U8G2_SETUP u8g2_Setup_st7565_erc12864_1
#elif CONF_GLCD_MONO_TYPE == GLCD_MONO_ST7565_LM6059_SPI
#define U8G2_SETUP u8g2_Setup_st7565_lm6059_1
#elif CONF_GLCD_MONO_TYPE == GLCD_MONO_ST7565_NHD_C12832_SPI
#define U8G2_SETUP u8g2_Setup_st7565_nhd_c12832_1
#elif CONF_GLCD_MONO_TYPE == GLCD_MONO_ST7565_NHD_C12864_SPI
#define U8G2_SETUP u8g2_Setup_st7565_nhd_c12864_1
#elif CONF_GLCD_MONO_TYPE == GLCD_MONO_ST7567_132x64_SPI
#define U8G2_SETUP u8g2_Setup_st7567_pi_132x64_1
#elif CONF_GLCD_MONO_TYPE == GLCD_MONO_ST7567_JLX12864_SPI
#define U8G2_SETUP u8g2_Setup_st7567_jlx12864_1
#elif CONF_GLCD_MONO_TYPE == GLCD_MONO_ST7588_JLX12864_I2C
#define U8G2_SETUP u8g2_Setup_st7588_i2c_jlx12864_1
#elif CONF_GLCD_MONO_TYPE == GLCD_MONO_ST7588_JLX12864_SPI
#define U8G2_SETUP u8g2_Setup_st7588_jlx12864_1
#elif CONF_GLCD_MONO_TYPE == GLCD_MONO_UC1601_128x32_I2C
#define U8G2_SETUP u8g2_Setup_uc1601_i2c_128x32_1
#elif CONF_GLCD_MONO_TYPE == GLCD_MONO_UC1601_128x32_SPI
#define U8G2_SETUP u8g2_Setup_uc1601_128x32_1
#elif CONF_GLCD_MONO_TYPE == GLCD_MONO_UC1604_JLX19264_I2C
#define U8G2_SETUP u8g2_Setup_uc1604_i2c_jlx19264_1
#elif CONF_GLCD_MONO_TYPE == GLCD_MONO_UC1604_JLX19264_SPI
#define U8G2_SETUP u8g2_Setup_uc1604_jlx19264_1
#elif CONF_GLCD_MONO_TYPE == GLCD_MONO_UC1608_240x128_I2C
#define U8G2_SETUP u8g2_Setup_uc1608_i2c_240x128_1
#elif CONF_GLCD_MONO_TYPE == GLCD_MONO_UC1608_240x128_SPI
#define U8G2_SETUP u8g2_Setup_uc1608_240x128_1
#elif CONF_GLCD_MONO_TYPE == GLCD_MONO_UC1608_ERC240120_I2C
#define U8G2_SETUP u8g2_Setup_uc1608_i2c_erc240120_1
#elif CONF_GLCD_MONO_TYPE == GLCD_MONO_UC1608_ERC240120_SPI
#define U8G2_SETUP u8g2_Setup_uc1608_erc240120_1
#elif CONF_GLCD_MONO_TYPE == GLCD_MONO_UC1608_ERC24064_I2C
#define U8G2_SETUP u8g2_Setup_uc1608_i2c_erc24064_1
#elif CONF_GLCD_MONO_TYPE == GLCD_MONO_UC1608_ERC24064_SPI
#define U8G2_SETUP u8g2_Setup_uc1608_erc24064_1
#elif CONF_GLCD_MONO_TYPE == GLCD_MONO_UC1610_EA_DOGXL160_I2C
#define U8G2_SETUP u8g2_Setup_uc1610_i2c_ea_dogxl160_1
#elif CONF_GLCD_MONO_TYPE == GLCD_MONO_UC1610_EA_DOGXL160_SPI
#define U8G2_SETUP u8g2_Setup_uc1610_ea_dogxl160_1
#elif CONF_GLCD_MONO_TYPE == GLCD_MONO_UC1611_EA_DOGM240_I2C
#define U8G2_SETUP u8g2_Setup_uc1611_i2c_ea_dogm240_1
#elif CONF_GLCD_MONO_TYPE == GLCD_MONO_UC1611_EA_DOGM240_SPI
#define U8G2_SETUP u8g2_Setup_uc1611_ea_dogm240_1
#elif CONF_GLCD_MONO_TYPE == GLCD_MONO_UC1611_EA_DOGXL240_I2C
#define U8G2_SETUP u8g2_Setup_uc1611_i2c_ea_dogxl240_1
#elif CONF_GLCD_MONO_TYPE == GLCD_MONO_UC1611_EA_DOGXL240_SPI
#define U8G2_SETUP u8g2_Setup_uc1611_ea_dogxl240_1
#elif CONF_GLCD_MONO_TYPE == GLCD_MONO_UC1611_EW50850_I2C
#define U8G2_SETUP u8g2_Setup_uc1611_i2c_ew50850_1
#elif CONF_GLCD_MONO_TYPE == GLCD_MONO_UC1611_EW50850_SPI
#define U8G2_SETUP u8g2_Setup_uc1611_ew50850_1
#elif CONF_GLCD_MONO_TYPE == GLCD_MONO_UC1638_160x128_SPI
#define U8G2_SETUP u8g2_Setup_uc1638_160x128_1
#elif CONF_GLCD_MONO_TYPE == GLCD_MONO_UC1701_EA_DOGS102_SPI
#define U8G2_SETUP u8g2_Setup_uc1701_ea_dogs102_1
#elif CONF_GLCD_MONO_TYPE == GLCD_MONO_UC1701_MINI12864_SPI
#define U8G2_SETUP u8g2_Setup_uc1701_mini12864_1
#endif

#if defined(GLCD_SPI_SUPPORT)
#define U8X8_COM glcd_u8x8_byte_hw_spi
#define U8G2_COM_CB glcd_u8x8_gpio_and_delay_hw_spi
#elif defined(GLCD_I2C_SUPPORT)
#define U8X8_COM glcd_u8x8_byte_hw_i2c
#define U8G2_COM_CB glcd_u8x8_gpio_and_delay_hw_i2c
#else
#error unsupported HW
#endif

void
glcd_init(void)
{

  U8G2_SETUP(&glcd, CONF_GLCD_MONO_ROTATION, U8X8_COM, U8G2_COM_CB);
  u8g2_InitDisplay(&glcd);
  u8g2_ClearDisplay(&glcd);
  u8g2_SetPowerSave(&glcd, 0);
}

int16_t
glcd_draw_string_P(int16_t x, int16_t y, const char *s)
{
  int16_t delta, w = 0;

  for (;;)
  {
    uint8_t encoding = pgm_read_byte(s++);
    if (encoding == 0)
      break;

    delta = glcd_draw_glyph(x, y, encoding);

    switch (tdir)
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
