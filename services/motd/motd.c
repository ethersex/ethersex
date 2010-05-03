/*
 *
 * Copyright (c) 2010 by Stefan Riepenhausen <rhn@gmx.net>
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

#include <avr/pgmspace.h>
#include <string.h>
#include "motd.h"
#include "core/eeprom.h"
#include "protocols/ecmd/ecmd-base.h"
#include "config.h"

int16_t parse_cmd_motd (char *cmd, char *output, uint16_t len) 
{
	while (*cmd == ' ')
	cmd++;

    if (*cmd != '\0') {
		eeprom_save(motd_text, cmd, strlen(cmd) + 1);
		eeprom_update_chksum();
		return ECMD_FINAL_OK;
    } 
    char motd[MOTD_VALUESIZE];
    memset(motd,0,MOTD_VALUESIZE);
    eeprom_restore(motd_text, &motd, MOTD_VALUESIZE);
	return ECMD_FINAL(snprintf_P(output, len, PSTR("%s"), motd));
}

/*
  -- Ethersex META --
  block(Miscelleanous)
  ecmd_feature(motd, "motd",[MESSAGE],Save MESSAGE as new message of the day, otherwise just show current message)
*/
