/*
 * Copyright (c) 2009 by Christian Dietrich <stettberger@dokucode.de>
 * Copyright (c) 2010 by Justin Otherguy <justin@justinotherguy.org>
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

// httplog_ecmd.c
//
// this is a literal copy of twitter_ecmd.c with "twitter" replaced by "httplog"

#include <avr/pgmspace.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "ws2801.h"

#include "protocols/ecmd/ecmd-base.h"

int16_t parse_cmd_ws2801_universe(char *cmd, char *output, uint16_t len)
{
  if (cmd[0])
  {
    ws2801_outputUniverse = atoi(cmd);
    return ECMD_FINAL_OK;
  }
  else
  {
    itoa(ws2801_outputUniverse, output, 10);
    return ECMD_FINAL(strlen(output));
  }
}

int16_t parse_cmd_ws2801_setall(char *cmd, char *output, uint16_t len)
{
  if (cmd[0])
  {
    ws2801_setall(atoi(cmd));
    return ECMD_FINAL_OK;
  }
  else
  {
    ws2801_setall(0);
    return ECMD_FINAL_OK;
  }
}

int16_t parse_cmd_ws2801_settemp(char *cmd, char *output, uint16_t len)
{
  uint16_t ret=0, k=0, d=0;
  ret = sscanf_P(cmd, PSTR("%u %hhu"), &k, &d);
  WS2801_DEBUG("input: %d, Dim:%d \r\n",k,d);
  if (ret==2)
  {
    if (d<0){
	d=0;
    }
    if (d>100){
	d=100;
    }
    ws2801_setColorTemp(k,d);
    ws2801_colortemp = k;
    return ECMD_FINAL_OK;
  }
  else if (ret==1)
  {
    ws2801_setColorTemp(k,100);
    return ECMD_FINAL_OK;
  }
  else
  {
    itoa(ws2801_colortemp, output, 10);
    return ECMD_FINAL(strlen(output));
  }
}

int16_t parse_cmd_ws2801_set_pixel_rgb(char *cmd, char *output, uint16_t len)
{
    uint8_t ret=0, pixel=0, r=0, g=0, b=0;
    ret = sscanf_P(cmd, PSTR("%hhu %hhu %hhu %hhu"), &pixel, &r, &g, &b);
    if(ret == 4)
	{
		if (pixel >= 170)
            return ECMD_ERR_PARSE_ERROR;
        
        ws2801_setPixelColor(pixel, r, g, b);
        return ECMD_FINAL_OK;
    }
    else
		return ECMD_ERR_PARSE_ERROR;
    
}

int16_t parse_cmd_ws2801_set_pixels(char *cmd, char *output, uint16_t len)
{
    uint8_t ret=0;
    uint16_t val1=0,val2=0,val3=0,pix;
    ret = sscanf_P(cmd, PSTR(" %2x%2x%2x"),&val1, &val2, &val3);
    if(ret == 3) {
        /*
   	ws2801 Datenausgabe start
   	*/
	for(pix = 0; pix < 171; pix++)
		{
			ws2801_writeByte(val1);  //r
			ws2801_writeByte(val2);  //g
			ws2801_writeByte(val3);  //b
		}
    	ws2801_showPixel();
   	/*
   	ws2801 Datenausgabe ende
   	*/
		return ECMD_FINAL_OK;
	}
	else
		return ECMD_ERR_PARSE_ERROR;

}


/*
  -- Ethersex META --
  block([[WS2801]] commands)
  ecmd_feature(ws2801_universe, "ws2801 universe",UNIVERSE, set/get Universe to show)
  ecmd_feature(ws2801_setall, "ws2801 setall",, set all channels)
  ecmd_feature(ws2801_settemp, "ws2801 colortemp",Temperature Dimm, set Temperature)
  ecmd_feature(ws2801_set_pixels, "ws2801 set",, Set pixel(s) values) 
  ecmd_feature(ws2801_set_pixel_rgb, "ws2801 rgb", PIXEL R G B, Set one pixel with rgb values)
*/
