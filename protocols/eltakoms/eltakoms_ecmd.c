/*
 * Copyright (c) 2021 by Frank Sautter <ethersix@sautter.com>
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

#include "config.h"

#ifdef ELTAKOMS_SUPPORT

#include <avr/pgmspace.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "protocols/ecmd/ecmd-base.h"
#include "eltakoms.h"


int16_t
parse_cmd_eltakoms_get(char *cmd, char *output, uint16_t len)
{
  memcpy(output, eltakoms_data.buffer, 40);

  return ECMD_FINAL(40);
}
#endif /* ELTAKOMS_H */

/*
  -- Ethersex META --
  block([[Eltako]])
  ecmd_feature(eltakoms_get, "eltakoms get",,Get weather data)
*/
