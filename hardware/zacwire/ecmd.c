/*
* ECMD-commands to handle ZACwire reads (for TSic temp-sensors)
*
* Copyright (c) 2009 by Gerd v. Egidy <gerd@egidy.de>
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
#include <avr/interrupt.h>

#include "config.h"
#include "core/debug.h"
#include "core/bit-macros.h"
#include "core/util/fixedpoint.h"

#include "hardware/zacwire/zacwire.h"

#include "protocols/ecmd/ecmd-base.h"

static int8_t zacwire_parse_arguments(char *cmd, uint8_t **pin_ptr, uint8_t *bitno)
{
    // skip spaces
    while (*cmd == ' ')
        cmd++;

    // first parameter: port number
    switch (*cmd)
    {
#ifdef PINA
        case '0': case 'A': *pin_ptr=(uint8_t*)&PINA; break;
        case '1': case 'B': *pin_ptr=(uint8_t*)&PINB; break;
        case '2': case 'C': *pin_ptr=(uint8_t*)&PINC; break;
        case '3': case 'D': *pin_ptr=(uint8_t*)&PIND; break;
#else
        case '0': case 'B': *pin_ptr=(uint8_t*)&PINB; break;
        case '1': case 'C': *pin_ptr=(uint8_t*)&PINC; break;
        case '2': case 'D': *pin_ptr=(uint8_t*)&PIND; break;
#endif
        default:
            return ECMD_ERR_PARSE_ERROR;
    }

    // skip spaces (but the space is optional)
    while (*(++cmd) == ' ');

    // second parameter: bit number
    if (*cmd < '0' || *cmd > '7')
        return ECMD_ERR_PARSE_ERROR;

    *bitno=(*cmd)-'0';

    if (*(cmd+1) != 0)
        return ECMD_ERR_PARSE_ERROR;

    return ECMD_FINAL_OK;
}

// output error messages to ECMDs output buffer
static int16_t output_zac_error(int8_t returncode, char *output)
{
    switch(returncode)
    {
        case ZACWIRE_ERR_PARITY:
            strcpy_P(output, PSTR("parity error"));
            return ECMD_FINAL(12);  // length of "parity error"
        case ZACWIRE_ERR_TIMEOUT:
        case ZACWIRE_ERR_PROTOCOL:
        default:
            strcpy_P(output, PSTR("no zacwire"));
            return ECMD_FINAL(10);  // lenght of "no zacwire"
    }
}

int16_t parse_cmd_zacwire_raw(char *cmd, char *output, uint16_t len)
{
    uint16_t raw_temp=0;
    uint8_t *pin_ptr;
    uint8_t bitno;
    int8_t ret;

    // the longest error message is 12 bytes + \0, all possible temp/hex values fit
    if (len < 13)
        return ECMD_FINAL(ECMD_ERR_PARSE_ERROR);

    if (zacwire_parse_arguments(cmd, &pin_ptr, &bitno) != ECMD_FINAL_OK)
        return ECMD_FINAL(ECMD_ERR_PARSE_ERROR);

    ret=zacwire_get(&raw_temp,pin_ptr,bitno);

    if (ret == ZACWIRE_OK)
        return ECMD_FINAL(strlen(itoa(raw_temp,output,16)));
    else
        return output_zac_error(ret,output);
}

int16_t parse_cmd_zacwire_306(char *cmd, char *output, uint16_t len)
{
    uint16_t raw_temp;
    uint8_t *pin_ptr;
    uint8_t bitno;
    int8_t ret;

    // the longest error message is 12 bytes + \0, all possible temp/hex values fit
    if (len < 13)
        return ECMD_FINAL(ECMD_ERR_PARSE_ERROR);

    if (zacwire_parse_arguments(cmd, &pin_ptr, &bitno) != ECMD_FINAL_OK)
        return ECMD_FINAL(ECMD_ERR_PARSE_ERROR);

    ret=zacwire_get(&raw_temp,pin_ptr,bitno);

    if (ret == ZACWIRE_OK)
        return ECMD_FINAL(itoa_fixedpoint(convert_tsic306(raw_temp),1,output,len));
    else
        return output_zac_error(ret,output);
}

int16_t parse_cmd_zacwire_506(char *cmd, char *output, uint16_t len)
{
    uint16_t raw_temp;
    uint8_t *pin_ptr;
    uint8_t bitno;
    int8_t ret;

    // the longest error message is 12 bytes + \0, all possible temp/hex values fit
    if (len < 13)
        return ECMD_FINAL(ECMD_ERR_PARSE_ERROR);

    if (zacwire_parse_arguments(cmd, &pin_ptr, &bitno) != ECMD_FINAL_OK)
        return ECMD_FINAL(ECMD_ERR_PARSE_ERROR);

    ret=zacwire_get(&raw_temp,pin_ptr,bitno);

    if (ret == ZACWIRE_OK)
        return ECMD_FINAL(itoa_fixedpoint(convert_tsic506(raw_temp),2,output,len));
    else
        return output_zac_error(ret,output);
}

/*
  -- Ethersex META --
  block([[ZACwire]])
  ecmd_ifdef(ZACWIRE_RAW_SUPPORT)
    ecmd_feature(zacwire_raw, "zac raw", PORT BIT, Return raw hex temperature value of zacwire at BIT of PORT)
  ecmd_endif()
  ecmd_ifdef(ZACWIRE_CONVERT306_SUPPORT)
    ecmd_feature(zacwire_306, "zac 306", PORT BIT, Return temperature of TSic 306 at BIT of PORT)
  ecmd_endif()
  ecmd_ifdef(ZACWIRE_CONVERT506_SUPPORT)
    ecmd_feature(zacwire_506, "zac 506", PORT BIT, Return temperature of TSic 506 at BIT of PORT)
  ecmd_endif()
*/
