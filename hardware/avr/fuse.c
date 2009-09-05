/*
 * Copyright (c) 2009 by Stefan Riepenhausen <rhn@gmx.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
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

#include "protocols/ecmd/ecmd-base.h"

int16_t parse_cmd_fuse(char *cmd, char *output, uint16_t len)
{
    (void) cmd;
#ifdef _SPMCR
    _SPMCR = 1<<BLBSET | 1<<SPMEN;
    uint8_t lo = pgm_read_byte(0);
    _SPMCR = 1<<BLBSET | 1<<SPMEN;
    uint8_t hi = pgm_read_byte(3);

    return ECMD_FINAL(snprintf_P(output, len, PSTR("Fuses: low=%02X high=%02X"), lo, hi));
#else
    return ECMD_FINAL(snprintf_P(output, len, PSTR("Fuses: unsupported")));
#endif
}

/*
  -- Ethersex META --
  ecmd_feature(fuse, "fuse",,Display the fuse settings.)
*/
