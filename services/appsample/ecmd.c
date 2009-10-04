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

int16_t parse_cmd_app_sample_command(char *cmd, char *output, uint16_t len) 
{
  // enter your commands here

  return ECMD_FINAL_OK;
}

int16_t parse_cmd_app_sample_init(char *cmd, char *output, uint16_t len) 
{
  app_sample_init();
  return ECMD_FINAL_OK;
}

int16_t parse_cmd_app_sample_periodic(char *cmd, char *output, uint16_t len) 
{
  app_sample_periodic();
  return ECMD_FINAL_OK;
}

/*
-- Ethersex META --
block([[Application_Sample]])
ecmd_feature(app_sample_command, "sample ",, Manually call application sample commands)
ecmd_feature(app_sample_init, "sample_init",, Manually call application sample init method)
ecmd_feature(app_sample_periodic, "sample_periodic",, Manually call application sample periodic method)
*/
