/*
 * Copyright (c) 2009 by Stefan Siegl <stesie@brokenpipe.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
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

#include <stdlib.h>
#include <string.h>

#include "hr20.h"
#include "protocols/ecmd/ecmd-base.h"
#include "config.h"

int16_t parse_cmd_hr20_toggle(char *cmd, char *output, uint16_t len)
{
  uint8_t i = atoi (cmd);
  if (i >= 160)
    return ECMD_ERR_PARSE_ERROR;

  LCD_SEG_TOGGLE (i);
  return ECMD_FINAL_OK;
}

int16_t parse_cmd_hr20_hourbar(char *cmd, char *output, uint16_t len)
{
  /* skip leading extra spaces */
  while (*cmd == ' ') cmd ++;

  char *ptr = strchr (cmd, ' ');
  if (!ptr) return ECMD_ERR_PARSE_ERROR;

  *(ptr ++) = 0;
  uint8_t start = atoi(cmd);
 uint8_t stop = atoi(ptr);

  if (start > 24 || stop > 24)
    return ECMD_ERR_PARSE_ERROR;

  hr20_lcd_hourbar (start, stop);
  return ECMD_FINAL_OK;
}


/*
  -- Ethersex META --
  ecmd_feature(hr20_toggle, "hr20 toggle ", SEG, Toggle segment SEG (a number, not a symbolic name!))
  ecmd_feature(hr20_hourbar, "hr20 hourbar ", START STOP, Update hourbar to show ticks between START and STOP (range 0..23))
*/
