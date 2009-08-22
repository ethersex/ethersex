/*
 * Copyright (c) 2009 by Stefan Siegl <stesie@brokenpipe.de>
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

#include "clock.h"
#include "protocols/soap/soap.h"

uint8_t soap_rpc_time(uint8_t len, soap_data_t *args, soap_data_t *result)
{
  if (len != 0) return 1;	/* we don't want args. */

  result->type = SOAP_TYPE_UINT32;
  result->u.d_uint32 = clock_get_time();
  return 0;
}

/*
  -- Ethersex META --
  soap_rpc(soap_rpc_time, "time")
*/
