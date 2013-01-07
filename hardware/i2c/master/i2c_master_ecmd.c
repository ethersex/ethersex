/*
 * Copyright (c) 2009 by Christian Dietrich <stettberger@dokucode.de>
 * Copyright (c) 2009 by Stefan Riepenhausen <rhn@gmx.net>
 * Copyright (c) 2012 by Erik Kunze <ethersex@erik-kunze.de>
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

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/twi.h>
#include <string.h>

#include "autoconf.h"
#include "config.h"
#include "core/debug.h"
#include "hardware/i2c/master/i2c_master.h"
#include "protocols/ecmd/ecmd-base.h"

#ifdef I2C_DETECT_SUPPORT

int16_t
parse_cmd_i2c_detect(char *cmd, char *output, uint16_t len)
{
  /* First call, we initialize our magic bytes */
  if (cmd[0] != ECMD_STATE_MAGIC)
  {
    cmd[0] = ECMD_STATE_MAGIC;
    cmd[1] = 0;
  }
  uint8_t next_address = i2c_master_detect(cmd[1], 127);
  cmd[1] = next_address + 1;

  if (next_address > 127)       /* End of scaning */
    return ECMD_FINAL_OK;
  else
    return
      ECMD_AGAIN(snprintf_P
                 (output, len, PSTR("detected at: 0x%x (%d)"), next_address,
                  next_address));
}

#endif /* I2C_DETECT_SUPPORT */

/*
-- Ethersex META --

  block([[I2C]] (TWI))
  ecmd_ifdef(I2C_DETECT_SUPPORT)
    ecmd_feature(i2c_detect, "i2c detect",,list detected I2C Chips)
  ecmd_endif
*/
