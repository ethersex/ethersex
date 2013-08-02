/*
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
#include "protocols/yport/yport.h"

#include "protocols/ecmd/ecmd-base.h"


int16_t
parse_cmd_yport_stats(char *cmd, char *output, uint16_t len)
{
  int16_t chars = snprintf_P(output, len,
                             PSTR("rx fe=%u, ov=%u, pe=%u, bf=%u, rt=%u"),
                             yport_rx_frameerror,
                             yport_rx_overflow,
                             yport_rx_parityerror,
                             yport_rx_bufferfull,
                             yport_eth_retransmit);
  return ECMD_FINAL(chars);
}

/*
  -- Ethersex META --
  block([[YPORT]] commands)
  ecmd_ifdef(DEBUG_YPORT)
    ecmd_feature(yport_stats, "yport stats",, Report statistic counters)
  ecmd_endif()
*/
