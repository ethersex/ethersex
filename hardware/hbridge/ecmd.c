/*
 * Copyright (c) 2010 by Stefan Riepenhausen <rhn@gmx.net>
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
#include <string.h>

#include "config.h"
#include "hbridge.h"
#include "protocols/ecmd/ecmd-base.h"

#ifdef HBRIDGE_PWM_SUPPORT
int16_t parse_cmd_hbridge_pwm_command(char *cmd, char *output, uint16_t len) 
{
  uint8_t pwm = atoi(cmd);
  HBRIDGEDEBUG ("pwm set: %i\n", pwm);
  hbridge_pwm(HBRIDGE_1_SELECT, pwm);
  hbridge_pwm(HBRIDGE_2_SELECT, pwm);
  
  return ECMD_FINAL_OK;
}
#endif /* HBRIDGE_PWM_SUPPORT */

#ifdef HBRIDGE_SUPPORT
int16_t parse_cmd_hbridge_command(char *cmd, char *output, uint16_t len) 
{
#ifdef DUAL_HBRIDGE_SUPPORT
  uint8_t action = DUAL_HBRIDGE_ACTION_FREE;
  switch (cmd[0]) {
	case 'f': action = DUAL_HBRIDGE_ACTION_FORWARD; break;
	case 'b': action = DUAL_HBRIDGE_ACTION_BACKWARD; break;
	case 's': action = DUAL_HBRIDGE_ACTION_BRAKE; break;
	case 'r': action = DUAL_HBRIDGE_ACTION_RIGHT; break;
	case 'l': action = DUAL_HBRIDGE_ACTION_LEFT; break;
	case 'R': action = DUAL_HBRIDGE_ACTION_RIGHT_ONLY; break;
	case 'L': action = DUAL_HBRIDGE_ACTION_LEFT_ONLY; break;
  }
  dual_hbridge(action);
#endif /* DUAL_HBRIDGE_SUPPORT */

  return ECMD_FINAL_OK;
}
#endif /* HBRIDGE_SUPPORT */

/*
  -- Ethersex META --
  block([[H-Bridge]])
  header(hardware/hbridge/hbridge.h)
  ecmd_feature(hbridge_pwm_command, "hbridge pwm", int, Set H-Bridge enable line valueeg. speed)
  ecmd_feature(hbridge_command, "hbridge ", [action] [enable_l] [enable_r], Set H-Bridge command)
*/
