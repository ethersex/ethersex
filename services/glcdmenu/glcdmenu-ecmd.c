/*
 * Copyright (c) 2010 by Daniel Walter <fordprfkt@googlemail.com>
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

#include <avr/io.h>
#include <avr/pgmspace.h>

#include <stdio.h>
#include <stdlib.h>

#include "config.h"
#include "glcdmenu.h"
#include "protocols/ecmd/ecmd-base.h"
#include "menu-interpreter/menu-interpreter.h"

int16_t parse_cmd_glcdmenu_redraw(char *cmd, char *output, uint16_t len)
{
	glcdmenuRedraw();
	return ECMD_FINAL_OK;
}

int16_t parse_cmd_glcdmenu_key(char *cmd, char *output, uint16_t len)
{
	uint16_t value_ui16 = 0;

	if (cmd[0] != 0)
	{
		while (*cmd == ' ')
			cmd++;
		value_ui16 = atoi(cmd);

		if ((value_ui16 > 0) && (value_ui16))
		{
			glcdmenuKeypress((unsigned char) value_ui16);
			return ECMD_FINAL_OK;
		}
		else
			return ECMD_ERR_PARSE_ERROR;
	}
	else
		return ECMD_ERR_PARSE_ERROR;
}

/*
 -- Ethersex META --
 block([[GLCD_Menu]])
 ecmd_feature(glcdmenu_redraw, "glcdmenu update",, Update the menu)
 ecmd_feature(glcdmenu_key, "glcdmenu key ",VALUE, Send a keypress to the menu)
 */
