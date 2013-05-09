/*
 * Copyright (c) 2007,2008 by Stefan Siegl <stesie@brokenpipe.de>
 * Copyright (c) 2007,2008 by Christian Dietrich <stettberger@dokucode.de>
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

#include <stdint.h>
#include "config.h"
#include "core/global.h"
#include "protocols/uip/uip.h"
#include "protocols/ecmd/ecmd-base.h"

int16_t
parse_cmd_bootloader(char *cmd, char *output, uint16_t len)
{
  (void) cmd;
  (void) output;
  (void) len;

  status.request_bootloader = 1;
#   ifdef UIP_SUPPORT
  uip_close();
#   endif
  return ECMD_FINAL_OK;
}


/*
  -- Ethersex META --
  block(Resetting the controller)
  ecmd_feature(bootloader, "bootloader",,Call the bootloader.)
*/
