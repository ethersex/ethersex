/*
 * Copyright (c) 2012 by Gerd v. Egidy <gerd@egidy.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by 
 * the Free Software Foundation; either version 2 of the License, or
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


#include <string.h>
#include <stdlib.h>
#include <avr/pgmspace.h>
#include <util/crc16.h>

#include "config.h"
#include "core/debug.h"

#include "protocols/ecmd/ecmd-base.h"

#define CRC_BYTE_POS 0x37FE

int16_t parse_cmd_crc_calc(char *cmd, char *output, uint16_t len)
{
    char *p;
    uint16_t crc=0xffff;
    
    for(p=0; p < (char*)CRC_BYTE_POS; p++)
        crc=_crc16_update(crc,pgm_read_byte(p));

    return ECMD_FINAL(sprintf_P(output, PSTR("%.4X"), crc));
}

int16_t parse_cmd_crc_read(char *cmd, char *output, uint16_t len)
{
    uint16_t crc;

    crc=pgm_read_word(CRC_BYTE_POS);
    
    return ECMD_FINAL(sprintf_P(output, PSTR("%.4X"), crc));
}

/*
  -- Ethersex META --
  block([[CRC]])
  ecmd_ifdef(CRC_VERIFY_SUPPORT)
    ecmd_feature(crc_calc, "crc calc", , "read the ethersex program code and calc a crc16 of it")
    ecmd_feature(crc_read, "crc read", , "read out the crc16 value at the end of the program space")
  ecmd_endif()
*/
