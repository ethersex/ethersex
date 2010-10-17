/*
 * Infrared-Multiprotokoll-Decoder 
 *
 * for additional information please
 * see http://www.mikrocontroller.net/articles/IRMP
 *
 * Copyright (c) 2010 by Erik Kunze <ethersex@erik-kunze.de>
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
#include <avr/pgmspace.h>

#include "config.h"
#include "core/bit-macros.h"
#include "core/debug.h"
#include "hardware/ir/irmp/irmp.h"

#include "protocols/ecmd/ecmd-base.h"


int16_t
parse_cmd_irmp_receive (char *cmd, char *output, uint16_t len)
{
  irmp_data_t irmp_data;
  return (irmp_read (&irmp_data)
	  ? ECMD_FINAL (sprintf_P (output, PSTR ("%d:%04X:%04X:%02X\n"),
				   irmp_data.protocol,
				   irmp_data.address,
				   irmp_data.command,
				   irmp_data.flags)) : ECMD_FINAL_OK);
}

#ifdef IRSND_SUPPORT

int16_t
parse_cmd_irmp_send (char *cmd, char *output, uint16_t len)
{
  int16_t ret;
  irmp_data_t irmp_data;

  ret = sscanf_P (cmd, PSTR ("%hhd %x %x %hhd"),
		  &irmp_data.protocol,
		  &irmp_data.address,
		  &irmp_data.command,
		  &irmp_data.flags);

  /* check if two values have been given */
  if (ret != 4)
    return ECMD_ERR_PARSE_ERROR;

  irmp_write (&irmp_data);

  return ECMD_FINAL_OK;
}

#endif

/*
  -- Ethersex META --
  block(Infrared Send/Receive ([[IRMP]]))
  ecmd_feature(irmp_receive, "irmp receive",,receive an IR command)
  ecmd_ifdef(IRSND_SUPPORT)
    ecmd_feature(irmp_send, "irmp send", PROTOCOL DEVICE COMMAND REPEAT, send COMMAND with REPEAT flag to DEVICE using PROTOCOL)
  ecmd_endif()
*/
