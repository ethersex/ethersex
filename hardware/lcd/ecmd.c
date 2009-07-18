/*
 *
 * Copyright (c) by Alexander Neumann <alexander@bumpern.de>
 * Copyright (c) 2007 by Stefan Siegl <stesie@brokenpipe.de>
 * Copyright (c) 2007 by Christian Dietrich <stettberger@dokucode.de>
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
#include <avr/pgmspace.h>

#include "config.h"
#include "core/bit-macros.h"
#include "core/debug.h"
#include "hardware/lcd/hd44780.h"

#include "protocols/ecmd/ecmd-base.h"


int16_t parse_cmd_lcd_clear(char *cmd, char *output, uint16_t len)
{
    uint16_t line;

    int ret = sscanf_P(cmd,
            PSTR("%u"),
            &line);

    if (ret == 1) {
        if (line > 3)
            return ECMD_ERR_PARSE_ERROR;

        hd44780_goto(LO8(line), 0);
        for (uint8_t i = 0; i < 20; i++)
            fputc(' ', lcd);
        hd44780_goto(LO8(line), 0);

        return ECMD_FINAL_OK;
    } else {
        hd44780_clear();
        hd44780_goto(0, 0);
        return ECMD_FINAL_OK;
    }
}

int16_t parse_cmd_lcd_write(char *cmd, char *output, uint16_t len)
{
    if (strlen(cmd) > 1) {
        fputs(cmd+1, lcd);
        return ECMD_FINAL_OK;
    } else
        return ECMD_ERR_PARSE_ERROR;
}

int16_t parse_cmd_lcd_goto(char *cmd, char *output, uint16_t len)
{
    uint16_t line, pos = 0;

    int ret = sscanf_P(cmd,
            PSTR("%u %u"),
            &line, & pos);

    if (ret >= 1 && line < 4) {
        if (ret == 2 && pos >= 20) {
            pos = 20;
        } else if (ret == 1)
            pos = 0;

        debug_printf("going to line %u, pos %u\n", line, pos);

        hd44780_goto(LO8(line), LO8(pos));
        return ECMD_FINAL_OK;
    } else
        return ECMD_ERR_PARSE_ERROR;

}

int16_t parse_cmd_lcd_char(char *cmd, char *output, uint16_t len)
{
  if (strlen(cmd) < 26) 
    return ECMD_ERR_PARSE_ERROR;
  uint8_t n_char, data[8];
  int ret = sscanf_P(cmd, PSTR("%u %x %x %x %x %x %x %x %x"), &n_char,
                     &data[0], &data[1], &data[2], &data[3],
                     &data[4], &data[5], &data[6], &data[7]);

  if (ret == 9) {
    hd44780_define_char(n_char, data);
    return ECMD_FINAL_OK;
  } else
    return ECMD_ERR_PARSE_ERROR;
}

int16_t parse_cmd_lcd_init(char *cmd, char *output, uint16_t len)
{
  uint8_t cursor, blink;
  int ret = sscanf_P(cmd, PSTR("%u %u"), &cursor, &blink);
  if ( ret == 2 ) {
    hd44780_init();
    hd44780_config(cursor, blink);
    return ECMD_FINAL_OK;
  } else
    return ECMD_ERR_PARSE_ERROR;
}

int16_t parse_cmd_lcd_shift(char *cmd, char *output, uint16_t len)
{
  if (strlen(cmd) < 1) 
    return ECMD_ERR_PARSE_ERROR;

  if (!strncmp_P(cmd + 1, PSTR("right"), 5))
    hd44780_shift(1);
  else if (!strncmp_P(cmd + 1, PSTR("left"), 4)) 
    hd44780_shift(0);
  else
    return ECMD_ERR_PARSE_ERROR;

  return ECMD_FINAL_OK;
}

/*
  -- Ethersex META --
  block(HD44780 LCD Display)
  ecmd_feature(lcd_clear, "lcd clear", [LINE], Clear line LINE (0..3) or the whole display (if parameter is omitted))
  ecmd_feature(lcd_write, "lcd write", TEXT, Write TEXT to the current cursor location)
  ecmd_feature(lcd_goto, "lcd goto", LINE COL, Move cursor to LINE and column COL (origin is 0/0))
  ecmd_feature(lcd_char, "lcd char", N D1 D2 D3 D4 D5 D6 D7 D8, Define use-definable char N with data D1..D8 (provide DATA in hex))
  ecmd_feature(lcd_init, "lcd reinit", CURSOR BLINK, Reinitialize the display, set whether to show the cursor (CURSOR, 0 or 1) and whether the cursor shall BLINK)
  ecmd_feature(lcd_shift, "lcd shift", DIR, Shift the display to DIR (either ''left'' or ''right''))
*/
