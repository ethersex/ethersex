/*
 *
 * Copyright (c) 2011 by Maximilian Güntner <maximilian.guentner@gmail.com>
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
#include <avr/io.h>
#include <stdlib.h>
#include <string.h>
#include "core/debug.h"
#include "config.h"
#include "dmx-effect.h"
#include "protocols/ecmd/ecmd-base.h"


#ifdef DMX_EFFECT_SUPPORT

#ifdef DMX_EFFECT_RAINBOW
int16_t parse_cmd_dmx_rainbow(char *cmd, char *output, uint16_t len)
{
   uint8_t ret=0;
   uint16_t selection=0;
   if(cmd[0]!=0) ret = sscanf_P(cmd, PSTR("%u"), &selection);
   if(ret == 1)
   {
	if(selection == DMX_EFFECT_ENABLED)
	{
		rainbow_enabled=DMX_EFFECT_ENABLED;
                return ECMD_FINAL_OK;
	}
	else if(selection == DMX_EFFECT_DISABLED)
	{
		rainbow_enabled=DMX_EFFECT_DISABLED;
                return ECMD_FINAL_OK;
	}
   	else
		return ECMD_ERR_PARSE_ERROR;
   } 
   return ECMD_ERR_PARSE_ERROR;

}
#endif /*Rainbow*/

#endif
/*
   -- Ethersex META --
   block([[DMX_Effect]] commands)
   ecmd_ifdef(DMX_EFFECT_RAINBOW)
	ecmd_feature(dmx_rainbow, "dmx rainbow",, switch rainbow on (1) or off (0)) 
   ecmd_endif
*/
     
