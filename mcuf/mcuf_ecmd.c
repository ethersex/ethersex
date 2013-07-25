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
#include "config.h"
#include "mcuf/mcuf.h"
#include "mcuf/mcuf_modul.h"
#include "core/debug.h"
#include "protocols/ecmd/ecmd-base.h"

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
  return ECMD_FINAL_OK;
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
    return ECMD_ERR_PARSE_ERROR;

  snprintf_P(buffer,34,PSTR("%s  "),cmd);
  mcuf_show_string(buffer);
  return ECMD_FINAL_OK;
}
#endif

#ifdef MCUF_MODUL_SUPPORT
int16_t parse_cmd_mcuf_modul(char *cmd, char *output, uint16_t len)
{
  uint8_t modul=0;
  char title[15];

  MCUF_PLAY_MODE mode = MCUF_MODUL_PLAY_MODE_SEQUENCE;
#ifdef MCUF_MODUL_DISPLAY_MODE_MANUAL
  mode = MCUF_MODUL_PLAY_MODE_MANUAL;
  sscanf_P (cmd, PSTR ("%hhu\n"),&modul);
#endif
#ifdef MCUF_MODUL_DISPLAY_MODE_RANDOM
  mode = MCUF_MODUL_PLAY_MODE_RANDOM;
#endif
  modul = mcuf_play_modul(mode, modul);
  mcuf_list_modul(title, modul);
  return ECMD_FINAL(snprintf_P(output, len, PSTR("%i=%s"), modul, title));
}

int16_t parse_cmd_mcuf_modul_list(char *cmd, char *output, uint16_t len)
{
  char title[15];

  if (cmd[0] != ECMD_STATE_MAGIC) {
    cmd[0] = ECMD_STATE_MAGIC;  //magic byte
    cmd[1] = 0x00;
 
    return ECMD_AGAIN(snprintf_P(output, len, PSTR("available modules:\n")));

  } else { 
    int i = cmd[1]++;
    if (mcuf_list_modul(title, i) == 0){
      return ECMD_FINAL_OK;
    }
    return ECMD_AGAIN(snprintf_P(output, len, PSTR("%i. %s"), i, title));
  }
}


#endif //MCUF_MODUL_SUPPORT

/*
-- Ethersex META --

  block([[Blinkenlights_MCUF|MCUF]])
  ecmd_ifdef(MCUF_CLOCK_SUPPORT)
    ecmd_feature(mcuf_show_clock, "mcuf showclock",, Show digital clock)
  ecmd_endif
  ecmd_ifdef(MCUF_SCROLLTEXT_SUPPORT)
   ecmd_feature(mcuf_show_string, "mcuf showstring",MESSAGE, Show scrolling MESSAGE on the display)
  ecmd_endif
  ecmd_ifdef(MCUF_MODUL_SUPPORT)
   ecmd_feature(mcuf_modul_list, "mcuf modul list",, List all modules)
   ecmd_feature(mcuf_modul, "mcuf modul",N, Select module N)
  ecmd_endif
*/
