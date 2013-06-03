/*
 * Copyright (c) 2009 by David Gräff <david.graeff@web.de>
 * Copyright (c) 2011 by Maximilian Güntner <maximilian.guentner@gmail.com>
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

#include <stdlib.h>
#include "config.h"

#include "sanyoz700.h"
#include "protocols/ecmd/ecmd-base.h"
#include "core/debug.h"

int16_t parse_cmd_sanyoz700 (char *cmd, char *output, uint16_t len)
{
	char b[2] = {0,0};

	while(*cmd && *cmd == ' ') cmd++; //skip whitespace
	if (!*cmd)
		return ECMD_ERR_PARSE_ERROR;
	b[0] = *cmd;
	while(*cmd && *cmd != ' ') cmd++; //skip value
	while(*cmd && *cmd == ' ') cmd++; //skip whitespace
	if (!*cmd)
		return ECMD_ERR_PARSE_ERROR;
	b[1] = *cmd;

	sanyoZ700cmd(b);
	return ECMD_FINAL_OK;
}

/*
-- Ethersex META --
block([[Sanyo Z700]] commands)
ecmd_feature(sanyoz700, "sanyoz700", CMD, Send command to projector)
*/
