/* vim:fdm=marker ts=4 et ai
 * {{{
 *
 * Copyright (c) 2009 by Christian Dietrich <stettberger@dokucode.de>
 * Copyright (c) 2009 by Stefan Riepenhausen <rhn@gmx.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
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
 }}} */

#include <string.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>

#include "../config.h"
#include "../debug.h"

#include "../ecmd_parser/ecmd.h"
#include "aliascmd.h"

#ifdef ALIASCMD_SUPPORT

#include "alias_defs.c"

char
*aliascmd_decode(char *cmd)
{
	aliascmd_t alias;
    uint8_t alias_cmp_len = 0;
    int8_t alias_cmp_idx = -1;
	for (uint8_t i = 0; ; i++) {
		memcpy_P(&alias, &aliascmdlist[i], sizeof(aliascmd_t));
		if (alias.name == NULL) break;
#ifdef DEBUG_ECMD
    	debug_printf("test cmd %s vs. alias %S\n", cmd+1, alias.name);
#endif
		if(strncmp_P(cmd + 1, alias.name, strlen_P(alias.name)) == 0 
           && alias_cmp_len < strlen_P(alias.name)) {
          alias_cmp_len = strlen_P(alias.name);
          alias_cmp_idx = i;
        }
    }

    if (alias_cmp_idx != -1) {/* copy alias in cmd buffer */
		memcpy_P(&alias, &aliascmdlist[alias_cmp_idx], sizeof(aliascmd_t));
		uint8_t newlen = strlen_P(alias.cmd);


		memmove(cmd + newlen, cmd + alias_cmp_len + 1, 
                strlen(cmd + alias_cmp_len + 1) + 1);
        memcpy_P(cmd, alias.cmd, newlen);

#ifdef DEBUG_ECMD
       debug_printf("alias found at pos %i: %S -> %S\n", i, alias.name, alias.cmd);
#endif
       return cmd;
	}
#ifdef DEBUG_ECMD
    debug_printf("no alias found\n");
#endif
    return NULL;
}
#endif /*ALIASCMD_SUPPORT*/
