/*
 * Copyright (c) 2013 by Frank Sautter <ethersix@sautter.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <string.h>
#include <avr/pgmspace.h>

#include "config.h"
#include "core/bit-macros.h"
#include "core/debug.h"
#include "protocols/pport/pport.h"

#include "protocols/ecmd/ecmd-base.h"


int16_t
parse_cmd_pport_stats(char *cmd, char *output, uint16_t len)
{
  int16_t chars = snprintf_P(output, len,
                             PSTR("by=%u, bf=%u, rt=%u, bsy=%u, stb=%u"),
                             pport_rx_bytes,
                             pport_rx_bufferfull,
                             pport_eth_retransmit,
                             PIN_HIGH(PPORT_BUSY) == 1,
                             PIN_HIGH(PPORT_STROBE) == 1
);
  return ECMD_FINAL(chars);
}

/*
  -- Ethersex META --
  block([[PPORT]] commands)
  ecmd_ifdef(DEBUG_PPORT)
    ecmd_feature(pport_stats, "pport stats",, Report statistic counters)
  ecmd_endif()
*/
