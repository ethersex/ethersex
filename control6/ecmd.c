/*
 *
 * Copyright (c) 2009 by Jochen Roessner <jochen@lugrot.de>
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
#include <stdlib.h>
#include <avr/pgmspace.h>

#include "config.h"
#include "core/debug.h"
#include "control6.h"

#include "protocols/ecmd/ecmd-base.h"

int16_t
parse_cmd_c6_get(char *cmd, char *output, uint16_t len)
{
  struct c6_vario_type varvalue;

  if (control6_get(cmd, &varvalue))
  {
    char *ptr;
    ptr = output + snprintf_P(output, len, PSTR("%s "), cmd);
    len -= (ptr - output);
    switch (varvalue.type)
    {
      case C6_TYPE_uint8_t:
        ptr += snprintf_P(ptr, len, PSTR("%hhu"), varvalue.data.d_uint8_t);
        break;

      case C6_TYPE_int8_t:
        ptr += snprintf_P(ptr, len, PSTR("%hhi"), varvalue.data.d_int8_t);
        break;

      case C6_TYPE_uint16_t:
        ptr += snprintf_P(ptr, len, PSTR("%u"), varvalue.data.d_uint16_t);
        break;

      case C6_TYPE_int16_t:
        ptr += snprintf_P(ptr, len, PSTR("%i"), varvalue.data.d_int16_t);
        break;

      case C6_TYPE_uint32_t:
        ptr += snprintf_P(ptr, len, PSTR("%lu"), varvalue.data.d_uint32_t);
        break;
      case C6_TYPE_int32_t:
        ptr += snprintf_P(ptr, len, PSTR("%li"), varvalue.data.d_int32_t);
        break;




    }
    return ECMD_FINAL(ptr - output);
  }
  else
    return ECMD_ERR_PARSE_ERROR;
}

int16_t
parse_cmd_c6_set(char *cmd, char *output, uint16_t len)
{
  char *buf;
  buf = strrchr(cmd, ' ');
  if (!buf)
    return ECMD_ERR_PARSE_ERROR;

  *(buf++) = 0;

  struct c6_vario_type varvalue;

  if (!control6_get(cmd, &varvalue))
    return ECMD_ERR_PARSE_ERROR;

  if (varvalue.type == C6_TYPE_int8_t || varvalue.type == C6_TYPE_int16_t)
  {
    /* signed */
    int32_t nv = strtol(buf, NULL, 0);

    if (varvalue.type == C6_TYPE_int8_t)
    {
      if (nv < INT8_MIN || nv > INT8_MAX)
      range_error:
        return ECMD_FINAL(snprintf_P(output, len, PSTR("range error.")));
      varvalue.data.d_int8_t = (int8_t) nv;
    }
    else if (varvalue.type == C6_TYPE_int16_t)
    {
      if (nv < INT16_MIN || nv > INT16_MAX)
        goto range_error;
      varvalue.data.d_int16_t = (uint32_t) nv;
    }

    else
    {
      varvalue.data.d_int32_t = nv;
    }

  }
  else
  {
    /* unsigned */
    if (*buf == '-')
      goto range_error;

    uint32_t nv = strtoul(buf, NULL, 0);

    if (varvalue.type == C6_TYPE_uint8_t)
    {
      if (nv > UINT8_MAX)
        goto range_error;
      varvalue.data.d_uint8_t = (uint8_t) nv;
    }
    else if (varvalue.type == C6_TYPE_uint16_t)
    {
      if (nv > UINT16_MAX)
        goto range_error;
      varvalue.data.d_uint16_t = (uint16_t) nv;
    }

    else
    {
      varvalue.data.d_uint32_t = nv;
    }
  }

  control6_set(cmd, varvalue);
  return ECMD_FINAL_OK;
}
