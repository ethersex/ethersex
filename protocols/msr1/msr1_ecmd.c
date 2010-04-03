/*
 *
 * Copyright (c) 2009 by Christian Dietrich <stettberger@dokucode.de>
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


#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>
#include <stdio.h>
#include "config.h"
#include "msr1.h"
#include "protocols/ecmd/ecmd-base.h"

#ifdef DEBUG_MSR1
# include "core/debug.h"
# define MSR1_DEBUG(a...)  debug_printf("msr1: " a)
#else
# define MSR1_DEBUG(a...)
#endif

extern struct msr1_e8_info msr1_e8_data;
extern struct msr1_generic_info msr1_c0_data;
extern struct msr1_generic_info msr1_48_data;
extern struct msr1_generic_info msr1_50_data;

int16_t 
parse_cmd_msr1_get(char *cmd, char *output, uint16_t len) 
{
  uint8_t i;
  MSR1_DEBUG("called msr1 get ecmd");
  while(*cmd == ' ') cmd ++;
  if (*cmd == 0 || *cmd == '0') {
    for (i = 0; i < 21; i++) {
      output = output + sprintf(output, "%02x", msr1_e8_data.data[i]);
    }
    return ECMD_FINAL(21 * 2);
  } else {
    /* Here we do all 76 byte long messages */
    struct msr1_generic_info *info;
    if (*cmd == '1') 
      info = &msr1_c0_data;
    else if (*cmd == '2')
      info = &msr1_48_data;
    else if (*cmd == '3')
      info = &msr1_50_data;
    else 
      return ECMD_ERR_PARSE_ERROR;

    /* trick: use bytes on cmd as "connection specific static variables" */
    if (cmd[1] != 23) {		/* indicator flag: real invocation:  0 */
        cmd[1] = 23;		/*                 continuing call: 23 */
        cmd[2] = 0;		/* counter for data blocks */
    }
    for (i = 0; i < 20; i++) {
      sprintf(output, "%02x", info->data[cmd[2] * 20 + i]);
      output += 2;
    }
    cmd[2] ++;
    if (cmd[2] == 4) {
      return ECMD_FINAL(15 * 2);
    }
    else
      return ECMD_AGAIN(20 * 2 + 1); 
  } 
  return ECMD_ERR_PARSE_ERROR;
}


/*
  -- Ethersex META --
  block([[Dachs_MSR1_auslesen|MSR1]])
  ecmd_feature(msr1_get, "msr1 get",,Get data)
*/
