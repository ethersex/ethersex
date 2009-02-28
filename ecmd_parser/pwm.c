/* 
 * Copyright (c) 2009 by Christian Dietrich <stettberger@dokucode.de>
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

#include <avr/io.h>
#include <avr/interrupt.h>
#include "../config.h"
#include "../debug.h"
#include "ecmd.h"

#ifdef PWM_SUPPORT

#include "../pwm/pwm.h"

int16_t
parse_cmd_pwm(char *cmd, char *output, uint16_t len)
{
  while (*cmd==' ') cmd++;

#ifdef PWM_WAV_SUPPORT
  if(*cmd == '1'){
    pwm_wav_init();
    return snprintf_P(output, len, PSTR("PWM wav init"));
  }
#endif //PWM_WAV_SUPPORT

#ifdef PWM_MELODY_SUPPORT
  if(*cmd == '2'){
    pwm_melody_init();
    return snprintf_P(output, len, PSTR("PWM melody init"));
  }
#endif //PWM_MELODY_SUPPORT


  if(cmd[0] == '0'){
#ifdef PWM_WAV_SUPPORT
    pwm_stop();
#endif //PWM_WAV_SUPPORT
    return snprintf_P(output, len, PSTR("PWM stop"));
  }

  return snprintf_P(output, len, PSTR("PWM any"));
}

#endif  /* PWM_SUPPORT */
