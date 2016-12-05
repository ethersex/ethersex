/*
 *
 * Copyright (c) by Alexander Neumann <alexander@bumpern.de>
 * Copyright (c) 2007 by Stefan Siegl <stesie@brokenpipe.de>
 * Copyright (c) 2007 by Christian Dietrich <stettberger@dokucode.de>
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

#include <string.h>
#include <stdio.h>
#include <avr/pgmspace.h>

#include "config.h"

#include "hd44780.h"

#include "core/bit-macros.h"
#include "core/debug.h"

#include "protocols/ecmd/ecmd-base.h"

/*
 * Prototypes
 */
int16_t parse_cmd_lcd_clear(char *cmd, char *output, uint16_t len);
int16_t parse_cmd_lcd_home(char *cmd, char *output, uint16_t len);
int16_t parse_cmd_lcd_write(char *cmd, char *output, uint16_t len);
int16_t parse_cmd_lcd_goto(char *cmd, char *output, uint16_t len);
#ifndef TEENSY_SUPPORT
int16_t parse_cmd_lcd_char(char *cmd, char *output, uint16_t len);
#endif
int16_t parse_cmd_lcd_init(char *cmd, char *output, uint16_t len);
int16_t parse_cmd_lcd_shift(char *cmd, char *output, uint16_t len);
#ifdef HD44780_BACKLIGHT_SUPPORT
int16_t parse_cmd_lcd_backlight(char *cmd, char *output, uint16_t len);
#endif

#if defined(DEBUG_HD44780_CHARCONV) && !defined(TEENSY_SUPPORT)
int16_t parse_cmd_lcd_print_charset(char *cmd, char *output, uint16_t len);
#endif

int16_t
parse_cmd_lcd_clear(char *cmd, char *output, uint16_t len)
{
  if (*cmd)
  {
    uint8_t line = (uint8_t) atoi(cmd);

    if (line > (LCD_LINES - 1))
      return ECMD_ERR_PARSE_ERROR;

    hd44780_goto(line, 0);
    for (uint8_t i = 0; i < LCD_CHAR_PER_LINE; i++)
      fputc(' ', &lcd);
    hd44780_goto(line, 0);

    return ECMD_FINAL_OK;
  }
  else
  {
    hd44780_clear();
    return ECMD_FINAL_OK;
  }
}

int16_t
parse_cmd_lcd_home(char *cmd, char *output, uint16_t len)
{
  hd44780_home();
  return ECMD_FINAL_OK;
}

int16_t
parse_cmd_lcd_write(char *cmd, char *output, uint16_t len)
{
  if (strlen(cmd) > 1)
  {
    fputs(cmd + 1, &lcd);
    return ECMD_FINAL_OK;
  }
  else
    return ECMD_ERR_PARSE_ERROR;
}

int16_t
parse_cmd_lcd_goto(char *cmd, char *output, uint16_t len)
{
#ifdef TEENSY_SUPPORT
  uint8_t line, pos = 0;

  /* Skip leading spaces. */
  while (*cmd == ' ')
    cmd++;

  /* Seek space (pos argument), chop and atoi to `pos'.  */
  char *p = cmd;
  while (*p && *p != ' ')
    p++;
  if (*p)
  {
    *p = 0;
    pos = atoi(p + 1);
  }

  line = atoi(cmd);
#else
  uint16_t line, pos = 0;

  int ret = sscanf_P(cmd,
                     PSTR("%u %u"),
                     &line, &pos);
  if (!ret)
    return ECMD_ERR_PARSE_ERROR;
#endif

  if (line > (LCD_LINES - 1))
    return ECMD_ERR_PARSE_ERROR;

  if (LO8(pos) > LCD_CHAR_PER_LINE)
    pos = LCD_CHAR_PER_LINE;

  hd44780_goto(LO8(line), LO8(pos));
  return ECMD_FINAL_OK;
}

int16_t
parse_cmd_lcd_char(char *cmd, char *output, uint16_t len)
{
  if (strlen(cmd) < 26)
    return ECMD_ERR_PARSE_ERROR;
  uint8_t n_char, data[8];
  int ret =
    sscanf_P(cmd, PSTR("%hhu %hhx %hhx %hhx %hhx %hhx %hhx %hhx %hhx"),
             &n_char,
             &data[0], &data[1], &data[2], &data[3],
             &data[4], &data[5], &data[6], &data[7]);

  if (ret == 9)
  {
    hd44780_define_char(n_char, data);
    return ECMD_FINAL_OK;
  }
  else
    return ECMD_ERR_PARSE_ERROR;
}

int16_t
parse_cmd_lcd_init(char *cmd, char *output, uint16_t len)
{
  uint8_t cursor, blink;

#ifdef TEENSY_SUPPORT
  /* Skip leading spaces. */
  while (*cmd == ' ')
    cmd++;

  /* Seek space (blink argument), chop and atoi to `blink'.  */
  char *p = cmd;
  while (*p && *p != ' ')
    p++;
  if (!*p)
    return ECMD_ERR_PARSE_ERROR;        /* Required argument `blink' missing. */

  *p = 0;
  blink = atoi(p + 1);
  cursor = atoi(cmd);
#else
  int ret = sscanf_P(cmd, PSTR("%hhu %hhu"), &cursor, &blink);
  if (ret != 2)
    return ECMD_ERR_PARSE_ERROR;
#endif

  hd44780_init();
  hd44780_config(cursor, blink);
  return ECMD_FINAL_OK;
}

int16_t
parse_cmd_lcd_shift(char *cmd, char *output, uint16_t len)
{
  if (strlen(cmd) < 1)
    return ECMD_ERR_PARSE_ERROR;

  if (!strncmp_P(cmd + 1, PSTR("right"), 5))
  {
    hd44780_shift(1);
  }
  else if (!strncmp_P(cmd + 1, PSTR("left"), 4))
  {
    hd44780_shift(0);
  }
  else
    return ECMD_ERR_PARSE_ERROR;

  return ECMD_FINAL_OK;
}

#ifdef HD44780_BACKLIGHT_SUPPORT
int16_t
parse_cmd_lcd_backlight(char *cmd, char *output, uint16_t len)
{
  if (strlen(cmd) < 1)
    return
      ECMD_FINAL(snprintf_P
                 (output, len,
                  hd44780_backlight_state ? PSTR("on") : PSTR("off")));

  if (strncmp_P(cmd + 1, PSTR("on"), 2) == 0)
    hd44780_backlight(1);
  else if (strncmp_P(cmd + 1, PSTR("off"), 3) == 0)
    hd44780_backlight(0);
  else
    return ECMD_ERR_PARSE_ERROR;

  return ECMD_FINAL_OK;
}
#endif

#if defined(DEBUG_HD44780_CHARCONV) && !defined(TEENSY_SUPPORT)
int16_t
parse_cmd_lcd_print_charset(char *cmd, char *output, uint16_t len)
{
  uint16_t from, to = 0;
  char c;

  int ret = sscanf_P(cmd,
                     PSTR("%u %u"),
                     &from, &to);
  if ((!ret) || (from > 255) || (to > 255))
    return ECMD_ERR_PARSE_ERROR;

  hd44780_clear();

  for (c = (from & 0xFF); c < to; c++)
  {
    /* \n and \r will be interpreted by hd44780_put() and may not be
     * used as characters, even if there are LCDs that support these
     * codepoints with printable characters.
     */
    if ((c != '\n') && (c != '\r'))
      hd44780_put(c, &lcd);
    else
      hd44780_put(' ', &lcd);

#if LCD_CHAR_PER_LINE >= 32
    if ((c + 1) % 32 == 0)
      hd44780_put('\n', &lcd);
#elif LCD_CHAR_PER_LINE >= 16
    if ((c + 1) % 16 == 0)
      hd44780_put('\n', &lcd);
#else /* LCD_CHAR_PER_LINE >= 8 */
    if ((c + 1) % 8 == 0)
      hd44780_put('\n', &lcd);
#endif
  }

  return ECMD_FINAL_OK;
}
#endif

/*
  -- Ethersex META --
  block(HD44780 [[LCD]])
  ecmd_feature(lcd_clear, "lcd clear", [LINE], Clear line LINE (0..3) or the whole display (if parameter is omitted))
  ecmd_feature(lcd_home, "lcd home", Cursor home (origin is 0/0)))
  ecmd_feature(lcd_write, "lcd write", TEXT, Write TEXT to the current cursor location)
  ecmd_feature(lcd_goto, "lcd goto", LINE COL, Move cursor to LINE and column COL (origin is 0/0))
  ecmd_ifndef(TEENSY_SUPPORT)
    ecmd_feature(lcd_char, "lcd char", N D1 D2 D3 D4 D5 D6 D7 D8, Define use-definable char N with data D1..D8 (provide DATA in hex))
  ecmd_endif()
  ecmd_feature(lcd_init, "lcd reinit", CURSOR BLINK, Reinitialize the display, set whether to show the cursor (CURSOR, 0 or 1) and whether the cursor shall BLINK)
  ecmd_feature(lcd_shift, "lcd shift", DIR, Shift the display to DIR (either ''left'' or ''right''))
  ecmd_ifdef(HD44780_BACKLIGHT_SUPPORT)
    ecmd_feature(lcd_backlight, "lcd backlight", [STATE], get or set the state of the lcd backlight)
  ecmd_endif()
  ecmd_ifdef(DEBUG_HD44780_CHARCONV)
    ecmd_feature(lcd_print_charset, "lcd print charset", FROM TO, Print char FROM up to char TO)
  ecmd_endif()
*/
