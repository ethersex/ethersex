/*
 *
 * Copyright (c) 2009 by Stefan Siegl <stesie@brokenpipe.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.

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
#include "mysql.h"

#include "protocols/ecmd/ecmd-base.h"

int16_t
parse_cmd_mysql_query (char *cmd, char *output, uint16_t len)
{
  (void) output;
  (void) len;

  return mysql_send_message (cmd) ? ECMD_ERR_PARSE_ERROR : ECMD_FINAL_OK;
}

/*
  -- Ethersex META --
  block([[MySQL]])
  ecmd_feature(mysql_query, "mysql query ",QUERY, Send specified MySQL query to the configured server)
*/
