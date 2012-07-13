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
  uint8_t pos = SERVO_STARTVALUE;
  sscanf_P(cmd, PSTR("%hhu %hhu"), &servo, &pos);
  if (servo >= PWM_SERVOS)
    return ECMD_FINAL(snprintf_P(output, len, PSTR("%i invalid"),servo));

  setservo(servo, pos);
  return ECMD_FINAL(snprintf_P(output, len, PSTR("PWM servo %i to %i"),servo, pos));
}

int16_t
parse_cmd_pwm_servo_inc(char *cmd, char *output, uint16_t len)
{
  uint8_t servo = 0;
  sscanf_P(cmd, PSTR("%hhu"), &servo);
  if (servo >= PWM_SERVOS)
    return ECMD_FINAL(snprintf_P(output, len, PSTR("%i invalid"),servo));

  servoinc(servo);
  return ECMD_FINAL(snprintf_P(output, len, PSTR("PWM servo %i inc"),servo));
}

int16_t
parse_cmd_pwm_servo_dec(char *cmd, char *output, uint16_t len)
{
  uint8_t servo = 0;
  sscanf_P(cmd, PSTR("%hhu"), &servo);
  if (servo >= PWM_SERVOS)
    return ECMD_FINAL(snprintf_P(output, len, PSTR("%i invalid"),servo));

  servodec(servo);
  return ECMD_FINAL(snprintf_P(output, len, PSTR("PWM servo %i dec"),servo));
}

#endif  /* PWM_SUPPORT */

/*
  -- Ethersex META --
  header(hardware/pwm/pwm_servo.h)
  block([[Servo_Ansteuerung|PWM Servo Control]])
  ecmd_feature(pwm_servo_set, "pwm servo_set",SERVONR POSITION, Set servo with SERVONR to POSITION)
  ecmd_feature(pwm_servo_inc, "pwm servo_inc",SERVONR, Increment position of servo SERVONR)
  ecmd_feature(pwm_servo_dec, "pwm servo_dec",SERVONR, Decrement position of servo SERVONR)
*/

