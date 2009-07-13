/*
 * Copyright (c) 2009 by Stefan Riepenhausen <rhn@gmx.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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

#include "config.h"
#include "core/debug.h"
#include "pwm_servo.h"

#include "protocols/ecmd/ecmd-base.h"

#ifdef PWM_SERVO_SUPPORT

int16_t
parse_cmd_pwm_servo_set(char *cmd, char *output, uint16_t len)
{
  uint8_t servo = 0;
  uint8_t pos = 128;
  sscanf_P(cmd, PSTR("%i %i"), &servo, &pos);
  setservo(servo, pos);
  return ECMD_FINAL(snprintf_P(output, len, PSTR("PWM servo %i to %i"),servo, pos));
}

int16_t
parse_cmd_pwm_servo_init(char *cmd, char *output, uint16_t len)
{
  pwm_servo_init();
  return ECMD_FINAL(snprintf_P(output, len, PSTR("PWM init done")));
}


#endif  /* PWM_SUPPORT */

/*
  -- Ethersex META --
  header(hardware/pwm/pwm_servo.h)
  ecmd_feature(pwm_servo_set, "pwm servo_set",, Set servo position)
  ecmd_feature(pwm_servo_init, "pwm servo_init",, Initialise servo)
*/

