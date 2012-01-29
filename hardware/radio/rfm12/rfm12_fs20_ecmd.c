/*
 *
 * Copyright (c) 2012 by Erik Kunze <ethersex@erik-kunze.de>
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
#include <avr/pgmspace.h>

#include "config.h"
#include "core/bit-macros.h"
#include "protocols/ecmd/ecmd-base.h"

#include "rfm12_fs20.h"

#ifdef RFM12_ASK_FS20_SUPPORT

int16_t parse_cmd_rfm12_fs20_send(char *cmd, char *output, uint16_t len)
{
    uint16_t hc, addr, c, c2;

    int ret = sscanf_P(cmd, PSTR("%x %x %x %x"), &hc, &addr, &c, &c2);
    if (ret == 3 || ret == 4) 
    {
        if (ret == 3) 
        {
            c2 = 0;
        }
        
        rfm12_fs20_send(hc, LO8(addr), LO8(c), LO8(c2));
        return ECMD_FINAL_OK;
    }

    return ECMD_ERR_PARSE_ERROR;

}
#endif

#ifdef RFM12_ASK_FHT_SUPPORT
int16_t parse_cmd_rfm12_fht_send(char *cmd, char *output, uint16_t len)
{
    uint16_t hc, addr, c, c2;

    int ret = sscanf_P(cmd, PSTR("%x %x %x %x"), &hc, &addr, &c, &c2);
    if (ret == 3 || ret == 4) 
    {
        if (ret == 3) 
        {
            c2 = 0;
        }
        
	rfm12_fht_send(hc, LO8(addr), LO8(c), LO8(c2));
        return ECMD_FINAL_OK;
    }

    return ECMD_ERR_PARSE_ERROR;

}
#endif

/*
-- Ethersex META --
  block([[RFM12_FS20]])
  ecmd_ifdef(RFM12_ASK_FS20_SUPPORT)
    ecmd_feature(rfm12_fs20_send, "fs20 send", , housecode addr command data)
  ecmd_endif()
  ecmd_ifdef(RFM12_ASK_FHT_SUPPORT)
    ecmd_feature(rfm12_fht_send, "fht send", , housecode addr command data)
  ecmd_endif()
*/
