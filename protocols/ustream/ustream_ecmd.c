/*
 * Copyright (c) 2009 by Sylwester Sosnowski <esc@ext.no-route.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "ustream.h"
#include "vs1053.h"

#include "protocols/ecmd/ecmd-base.h"

int16_t parse_cmd_ustream_init(char *cmd, char *output, uint16_t len) 
{
  ustream_init();
  return ECMD_FINAL_OK;
}

int16_t parse_cmd_ustream_test(char *cmd, char *output, uint16_t len)
{
  cs_low();
  sci_write(0x00, (1<<SM_TESTS)|(1<<SM_SDISHARE)|(1<<SM_STREAM)|(1<<SM_SDINEW));
  cs_high();

  vs1053_sinetest(120);
  return ECMD_FINAL_OK;
}


/*
  -- Ethersex META --
  block(Ustream Client)
  ecmd_feature(ustream_init, ``"ustream init"'',,ustream service re-initialization)
  ecmd_feature(ustream_test, ``"ustream test"'',,test ustream service)
*/
