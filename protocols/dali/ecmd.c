/*
* ECMD-commands for DALI light control
*
* Copyright (c) 2011 by Gerd v. Egidy <gerd@egidy.de>
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

#include <string.h>
#include <stdlib.h>
#include <avr/pgmspace.h>

#include "config.h"
#include "core/debug.h"
#include "protocols/ecmd/ecmd-base.h"

#include "protocols/dali/dali.h"

int16_t parse_cmd_dali_raw(char *cmd, char *output, uint16_t len)
{
    uint16_t data=0;

    while(*cmd == ' ') cmd++;

    if (sscanf_P(cmd, PSTR("%x %x"), &data, (uint8_t*)(&data)+1) != 2)
        return ECMD_ERR_PARSE_ERROR;

    dali_send(&data);
    
    return ECMD_FINAL_OK;
}


/*
  -- Ethersex META --
  block([[DALI]])
  ecmd_ifdef(DALI_SUPPORT)
    ecmd_feature(dali_raw, "dali raw", `[BYTE1] [BYTE2], send a raw command sequence (two bytes, given in hex) over the DALI bus')
  ecmd_endif()
*/
