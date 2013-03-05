/*
* ECMD-commands to handle reading DHT humidity & temp sensors
*
* Copyright (c) 2013 Erik Kunze <ethersex@erik-kunze.de>
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

#include "config.h"
#include "core/util/fixedpoint.h"
#include "protocols/ecmd/ecmd-base.h"

#include "dht.h"
#include "dht_ecmd.h"

int16_t parse_cmd_dht_temp(char *cmd, char *output, uint16_t len)
{
  return ECMD_FINAL(itoa_fixedpoint(dht_global.temp,1,output));
}

int16_t parse_cmd_dht_humid(char *cmd, char *output, uint16_t len)
{
  return ECMD_FINAL(itoa_fixedpoint(dht_global.humid,1,output));
}

/*
  -- Ethersex META --
  block([[DHT]])
  ecmd_feature(dht_temp, "dht temp",, Return temperature of DHT sensor)
  ecmd_feature(dht_humid, "dht humid",, Return humidity of DHT sensor)
*/
