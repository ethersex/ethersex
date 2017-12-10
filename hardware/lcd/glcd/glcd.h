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

#ifndef __GLCD_H
#define __GLCD_H

#include <stdio.h>
#include <stdint.h>

#include "config.h"

/*
 * GLCD size/type
 */
#define GLCD_MONO_NONE                       100
#define GLCD_MONO_IL3820_296x128_SPI         101
#define GLCD_MONO_IST3020_ERC19264_SPI       102
#define GLCD_MONO_LD7032_60x32_I2C           103
#define GLCD_MONO_LD7032_60x32_SPI           104
#define GLCD_MONO_LS013B7DH03_128x128_SPI    105
#define GLCD_MONO_MAX7219_32x8_SPI           106
#define GLCD_MONO_NT7534_TG12864R_SPI        107
#define GLCD_MONO_PCD8544_84x48_SPI          108
#define GLCD_MONO_PCF8812_96x65_SPI          109
#define GLCD_MONO_S1D1305                    110
#define GLCD_MONO_SH1106_128x64_I2C          111
#define GLCD_MONO_SH1106_128x64_SPI          112
#define GLCD_MONO_SSD1305_128x32_I2C         113
#define GLCD_MONO_SSD1305_128x32_SPI         114
#define GLCD_MONO_SSD1306_64x32_SPI          115
#define GLCD_MONO_SSD1306_64x32_I2C          116
#define GLCD_MONO_SSD1306_64x48_SPI          117
#define GLCD_MONO_SSD1306_64x48_I2C          118
#define GLCD_MONO_SSD1306_96x16_SPI          119
#define GLCD_MONO_SSD1306_96x16_I2C          120
#define GLCD_MONO_SSD1306_128x64_I2C         121
#define GLCD_MONO_SSD1306_128x64_SPI         122
#define GLCD_MONO_SSD1309_128x64_I2C         123
#define GLCD_MONO_SSD1309_128x64_SPI         124
#define GLCD_MONO_SSD1322_256x64_NHD_SPI     125
#define GLCD_MONO_SSD1325_128x64_I2C         126
#define GLCD_MONO_SSD1325_128x64_SPI         127
#define GLCD_MONO_SSD1326_256x32_ER_I2C      128
#define GLCD_MONO_SSD1326_256x32_ER_SPI      129
#define GLCD_MONO_SSD1327_96x96_I2C          130
#define GLCD_MONO_SSD1327_96x96_SPI          131
#define GLCD_MONO_SSD1327_128x128_SPI        132
#define GLCD_MONO_SSD1329_128x96_SPI         133
#define GLCD_MONO_SSD1606_172x72_SPI         134
#define GLCD_MONO_SSD1607_200x200_SPI        135
#define GLCD_MONO_ST75256_JLX172104_I2C      136
#define GLCD_MONO_ST75256_JLX172104_SPI      137
#define GLCD_MONO_ST75256_JLX256128_I2C      138
#define GLCD_MONO_ST75256_JLX256128_SPI      139
#define GLCD_MONO_ST75256_JLX25664_I2C       140
#define GLCD_MONO_ST75256_JLX25664_SPI       141
#define GLCD_MONO_ST7565_128x64_ZOLEN_SPI    142
#define GLCD_MONO_ST7565_64128N_SPI          143
#define GLCD_MONO_ST7565_EA_DOGM128_SPI      144
#define GLCD_MONO_ST7565_EA_DOGM132_SPI      145
#define GLCD_MONO_ST7565_ERC12864_SPI        146
#define GLCD_MONO_ST7565_LM6059_SPI          147
#define GLCD_MONO_ST7565_NHD_C12832_SPI      148
#define GLCD_MONO_ST7565_NHD_C12864_SPI      149
#define GLCD_MONO_ST7567_132x64_SPI          150
#define GLCD_MONO_ST7567_JLX12864_SPI        151
#define GLCD_MONO_ST7588_JLX12864_I2C        152
#define GLCD_MONO_ST7588_JLX12864_SPI        153
#define GLCD_MONO_UC1601_128x32_I2C          154
#define GLCD_MONO_UC1601_128x32_SPI          155
#define GLCD_MONO_UC1604_JLX19264_I2C        156
#define GLCD_MONO_UC1604_JLX19264_SPI        157
#define GLCD_MONO_UC1608_240x128_I2C         158
#define GLCD_MONO_UC1608_240x128_SPI         159
#define GLCD_MONO_UC1608_ERC240120_I2C       160
#define GLCD_MONO_UC1608_ERC240120_SPI       161
#define GLCD_MONO_UC1608_ERC24064_I2C        162
#define GLCD_MONO_UC1608_ERC24064_SPI        163
#define GLCD_MONO_UC1610_EA_DOGXL160_I2C     164
#define GLCD_MONO_UC1610_EA_DOGXL160_SPI     165
#define GLCD_MONO_UC1611_EA_DOGM240_I2C      166
#define GLCD_MONO_UC1611_EA_DOGM240_SPI      167
#define GLCD_MONO_UC1611_EA_DOGXL240_I2C     168
#define GLCD_MONO_UC1611_EA_DOGXL240_SPI     169
#define GLCD_MONO_UC1611_EW50850_I2C         170
#define GLCD_MONO_UC1611_EW50850_SPI         171
#define GLCD_MONO_UC1638_160x128_SPI         172
#define GLCD_MONO_UC1701_EA_DOGS102_SPI      173
#define GLCD_MONO_UC1701_MINI12864_SPI       174

#define GLCD_COLOR_NONE                      200
#define GLCD_COLOR_S1D15G10_130x130          201
#define GLCD_COLOR_ST7626                    202
#define GLCD_COLOR_ST7735_128x160            203
#define GLCD_COLOR_ILI9341_240x320           204
#define GLCD_COLOR_ILI9163_128x128           205
#define GLCD_COLOR_SSD1351_128x128           206
#define GLCD_COLOR_SSD1351_128x128_FT        207
#define GLCD_COLOR_SSD1331_96x64_UNIVISION   208
#define GLCD_COLOR_SEPS225_128x128_UNIVISION 209

#define GLCD_DIR_LTR  0
#define GLCD_DIR_DOWN 1
#define GLCD_DIR_RTL  2
#define GLCD_DIR_UP   3

#ifdef GLCD_MONO_SUPPORT
#include "glcd_mono.h"
#endif

#ifdef GLCD_COLOR_SUPPORT
#include "glcd_color.h"
#endif

extern glcd_t glcd;
extern FILE glcd_stream;

void glcd_init(void);
void glcd_set_print_pos(uint16_t x, uint16_t y);
void glcd_set_print_dir(uint8_t dir);
uint16_t glcd_get_x(void);
uint16_t glcd_get_y(void);
uint8_t glcd_get_dir(void);
int16_t glcd_get_str_width_P(const char *s);

#endif /* !__GLCD_H */
