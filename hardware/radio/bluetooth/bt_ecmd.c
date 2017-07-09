/*
 * Copyright (c) 2013-2017 Erik Kunze <ethersex@erik-kunze.de>
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

#include <stdint.h>
#include <avr/pgmspace.h>

#include "config.h"
#include "protocols/ecmd/ecmd-base.h"

#include "bt.h"
#include "bt_usart.h"
#include "bt_ecmd.h"

int16_t
parse_cmd_bt_at(char *cmd, char *output, uint16_t len)
{
  while (*cmd == ' ')
    cmd++;

  /* send command to and return status from BT module */
  if (*cmd == '\0')
    return ECMD_ERR_PARSE_ERROR;

  return ECMD_FINAL(bt_send_with_response(output, len, cmd));
}

int16_t
parse_cmd_bt_stats(char *cmd, char *output, uint16_t len)
{
  return ECMD_FINAL(snprintf_P(output, len,
                             PSTR
                             ("rx fe=%u, ov=%u, pe=%u, bf=%u, #=%u, tx #=%u"),
                             bt_rx_frameerror,
                             bt_rx_overflow,
                             bt_rx_parityerror,
                             bt_rx_bufferfull,
                             bt_rx_count,
                             bt_tx_count));
}

/*
  -- Ethersex META --
  block(Bluetooth SPP Module ([[Bluetooth]]))
  ecmd_feature(bt_at, "bt cmd",,send an AT command)
  ecmd_feature(bt_stats, "bt stats",,report statistics counter)
*/
