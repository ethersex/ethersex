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
#include "../mcuf/mcuf_modul.h"
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
  char buffer[36];

  uint8_t ret = sscanf_P (cmd, PSTR (" %s \n"),buffer);
  if (ret != 1)
    return -1;

  snprintf_P(buffer,34,PSTR("%s  "),cmd);
  mcuf_show_string(buffer);
  return 0;
}
#endif

#ifdef MCUF_MODUL_SUPPORT
int16_t parse_cmd_mcuf_modul(char *cmd, char *output, uint16_t len)
{
  uint8_t modul;

  sscanf_P (cmd, PSTR ("%i\n"),&modul);
  switch (modul)
  {
#ifdef MCUF_CHESS_SUPPORT
    case 0:
      mcuf_chess();
    break;
#endif //MCUF_CHESS_SUPPORT
#ifdef MCUF_BOX_SUPPORT
    case 1:
      draw_box(0, 0, MCUF_MAX_SCREEN_WIDTH, MCUF_SPLIT_SCREEN_HEIGHT, mcuf_scrolltext_buffer.bcolor, mcuf_scrolltext_buffer.bcolor);
      break;
#endif //MCUF_BOX_SUPPORT
#ifdef MCUF_SCROLLTEXT_SUPPORT
    case 2:
       mcuf_show_string("ethersex rules");
     break;
#endif //MCUF_SCROLLTEXT_SUPPORT
#ifdef MCUF_CLEAN_SUPPORT
    case 3:
      mcuf_clean(5);
    break;
    case 4:
      mcuf_clean(50);
    break;
#endif //MCUF_CLEAN_SUPPORT
#ifdef MCUF_SPIRAL_SUPPORT
    case 5:
      mcuf_clean(0);
      mcuf_spiral(5);
    break;
    case 6:
      mcuf_clean(0);
      mcuf_spiral(30);
    break;
#endif //MCUF_SPIRAL_SUPPORT
    default:
      debug_printf("not found\n");
  }

  return 0;
}

#endif //MCUF_MODUL_SUPPORT

#endif
