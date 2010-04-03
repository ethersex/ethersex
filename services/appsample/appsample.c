/*
 * Copyright (c) 2009 by Stefan Riepenhausen <rhn@gmx.net>
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
#include "appsample.h"

#include "protocols/ecmd/ecmd-base.h"

/*
  If enabled in menuconfig, this function is called during boot up of ethersex
*/
int16_t
app_sample_init(void)
{
  APPSAMPLEDEBUG ("init\n");
  // enter your code here

  return ECMD_FINAL_OK;
}

/*
  If enabled in menuconfig, this function is periodically called
  change "timer(100,app_sample_periodic)" if needed
*/
int16_t
app_sample_periodic(void)
{
  APPSAMPLEDEBUG ("periodic\n");
  // enter your code here

  return ECMD_FINAL_OK;
}

/*
  This function will be called on request by menuconfig, if wanted...
  You need to enable ECMD_SUPPORT for this.
  Otherwise you can use this function for anything you like 
*/
int16_t
app_sample_onrequest(char *cmd, char *output, uint16_t len){
  APPSAMPLEDEBUG ("main\n");
  // enter your code here

  return ECMD_FINAL_OK;
}

/*
  -- Ethersex META --
  header(services/appsample/appsample.h)
  ifdef(`conf_APP_SAMPLE_INIT_AUTOSTART',`init(app_sample_init)')
  ifdef(`conf_APP_SAMPLE_PERIODIC_AUTOSTART',`timer(100,app_sample_periodic())')
*/
