/**********************************************************
 * Copyright(C) 2009 Dirk Pannenbecker <dp@sd-gp.de>
 *
 * @author      Dirk Pannenbecker
 * @date        21.02.2009

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software Foundation,
 Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA

*/
#include <stdint.h>
#include "../config.h"
#include "../mcuf/mcuf.h"
#include "../debug.h"

#ifdef MCUF_SUPPORT
#ifdef MCUF_CLOCK_SUPPORT
int16_t parse_cmd_mcuf_show_clock(char *cmd, char *output, uint16_t len)
{
  (void) output;
  (void) len;
  uint16_t buffer;
  uint8_t ret = sscanf_P (cmd, PSTR ("%u\n"), &buffer);
  if (ret != 1)
    buffer = 3;

  mcuf_show_clock(buffer);
  return 0;
}
#endif

#ifdef MCUF_SCROLLTEXT_SUPPORT
int16_t parse_cmd_mcuf_show_string(char *cmd, char *output, uint16_t len)
{
  (void) output;
  (void) len;
  char buffer[1];
  uint8_t ret = sscanf_P (cmd, PSTR ("%s\n"),buffer);
  if (ret != 1)
    return -1;

  mcuf_show_string(cmd);
  return 0;
}

#endif
#endif
