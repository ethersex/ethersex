/*
 *
 * Copyright (c) 2011 by Jonas Eickhoff <jonas02401@googlemail.com>
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
#include "dmx-fxslot.h"
#include "protocols/ecmd/ecmd-base.h"

#ifdef DMX_FXSLOT_SUPPORT


int16_t parse_cmd_dmx_fxslot_effect(char *cmd, char *output, uint16_t len)
{
	uint16_t speed=0;
	uint8_t fxslot_number=0, active=0, effect=0, ret=0;
	if (cmd[0]!=0) {
		ret =sscanf_P(cmd, PSTR("%hhu %hhu %hhu %u"),&fxslot_number, &active, &effect, &speed);
		if (fxslot_number >= DMX_FXSLOT_AMOUNT)
			return ECMD_ERR_PARSE_ERROR;	  
		switch(ret)
		{
			case 4:                                                     //sets speed of fxslot
				fxslot[fxslot_number].speed=speed;            

			case 3:
				if(fxslot[fxslot_number].effect!=effect)              //inits effect on fxslot everytime effect is changed
				{   
					dmx_fxslot_init(fxslot_number,effect);
					fxslot[fxslot_number].effect=effect;       
				}

			case 2:
				fxslot[fxslot_number].active=active;                  //sets fxslot as active(1) / inactive(0)
				break;

			case 1:
			case 0:
				return ECMD_ERR_PARSE_ERROR;

		}  	    
		return ECMD_FINAL_OK;
	}
	else
		return ECMD_ERR_PARSE_ERROR;
}
int16_t parse_cmd_dmx_fxslot_setdevices(char *cmd, char *output, uint16_t len)
{
	uint16_t channel=0;
	uint8_t fxslot_number, devices, margin, universe, ret=0;
	if (cmd[0]!=0) {
		ret = sscanf_P(cmd, PSTR("%hhu %hhu %hhu %u %hhu"),&fxslot_number, &devices, &margin, &channel, &universe);

		if (ret<5)
			return ECMD_ERR_PARSE_ERROR;

		if (universe >= DMX_STORAGE_UNIVERSES)
			return ECMD_ERR_PARSE_ERROR;

		if (channel >= DMX_STORAGE_CHANNELS)
			return ECMD_ERR_PARSE_ERROR;

		fxslot[fxslot_number].universe=universe;    
		fxslot[fxslot_number].startchannel=channel;   
		fxslot[fxslot_number].devices=devices;
		fxslot[fxslot_number].margin=margin;
		return ECMD_FINAL_OK;
	}
	else
		return ECMD_ERR_PARSE_ERROR;
}


#endif
/*
   -- Ethersex META --
   block([[DMX_FXSlot]] commands)
   ecmd_feature(dmx_fxslot_effect, "dmx fxslot effect",,set the effect settings)
   ecmd_feature(dmx_fxslot_setdevices, "dmx fxslot devices",,set the device settings) 
 */

