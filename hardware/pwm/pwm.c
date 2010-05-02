/*
 * Copyright (c) 2010 by Stefan Riepenhausen <rhn@gmx.net>
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

#include <stdlib.h>

#include "config.h"
#include "pwm.h"
#include "protocols/ecmd/ecmd-base.h"

#ifdef CH_A_PWM_GENERAL_SUPPORT
  uint8_t channelAval=0xFF;
#endif /* CH_A_PWM_GENERAL_SUPPORT */
#ifdef CH_B_PWM_GENERAL_SUPPORT
  uint8_t channelBval=0xFF;
#endif /* CH_B_PWM_GENERAL_SUPPORT */
#ifdef CH_C_PWM_GENERAL_SUPPORT
  uint8_t channelCval=0xFF;
#endif /* CH_C_PWM_GENERAL_SUPPORT */


void
pwm_init(){
  TCNT1=0x00FF; //set the timer counter
#ifdef CH_A_PWM_GENERAL_SUPPORT
  DDR_CONFIG_OUT(CHANNEL_A_PWM); 		// PWM OUTPUT
  OCR1A=channelAval;
  TCCR1A|=_BV(COM1A1)|_BV(COM1A0); 		// Set OCnA on compare match
#endif /* CH_A_PWM_GENERAL_SUPPORT */
#ifdef CH_B_PWM_GENERAL_SUPPORT
  DDR_CONFIG_OUT(CHANNEL_B_PWM); 		// PWM OUTPUT
  OCR1B=channelBval;
  TCCR1A|=_BV(COM1B1)|_BV(COM1B0); 		// Set OCnB on compare match
#endif /* CH_B_PWM_GENERAL_SUPPORT */
#ifdef CH_C_PWM_GENERAL_SUPPORT
  DDR_CONFIG_OUT(CHANNEL_C_PWM); 		// PWM OUTPUT
  OCR1C=channelCval;
  TCCR1A|=_BV(COM1C1)|_BV(COM1C0); 		// Set OCnC on compare match
#endif /* CH_C_PWM_GENERAL_SUPPORT */

  TCCR1A|=_BV(WGM10);  					// PWM, Phase Correct, 8-bit

  TCCR1B|=_BV(WGM12); 					// waveform generation mode: CTC, 
  TCCR1B|=_BV(CS10); 					// clockselect: clkI/O/1 (No prescaling)

  // activate PWM outports OC1C
#ifdef CH_A_PWM_GENERAL_SUPPORT
  #if defined(_ATMEGA128)
  TCCR1C|=1<<FOC1A;					// with atmega128
  #else
  TCCR1A|=1<<FOC1A;  					// with atmega32
  #endif
#endif /* CH_A_PWM_GENERAL_SUPPORT */
#ifdef CH_B_PWM_GENERAL_SUPPORT
  #if defined(_ATMEGA128)
  TCCR1C|=1<<FOC1B;					// with atmega128
  #else
  TCCR1A|=1<<FOC1B;						// with atmega 32
  #endif
#endif /* CH_B_PWM_GENERAL_SUPPORT */
#ifdef CH_C_PWM_GENERAL_SUPPORT
  #if defined(_ATMEGA128)
  TCCR1C|=1<<FOC1C;  					// with atmega128
  #else
  TCCR1A|=1<<FOC1C; 					// with atmega 32
  #endif
#endif /* CH_C_PWM_GENERAL_SUPPORT */

}

void
setpwm(char channel, uint8_t value){
  PWMDEBUG ("channel %c, values: %i\n",channel, value);
  switch (channel){
#ifdef CH_A_PWM_GENERAL_SUPPORT
    case 'a': 
      OCR1A=value;
	  channelAval=value;
	  break;
#endif /* CH_A_PWM_GENERAL_SUPPORT */
#ifdef CH_B_PWM_GENERAL_SUPPORT
    case 'b': 
      OCR1B=value;
	  channelBval=value;
	  break;
#endif /* CH_C_PWM_GENERAL_SUPPORT */
#ifdef CH_C_PWM_GENERAL_SUPPORT
    case 'c': 
      OCR1C=value;
	  channelCval=value;
	  break;
#endif /* CH_A_PWM_GENERAL_SUPPORT */
    default:
    PWMDEBUG ("channel %c unsupported\n",channel);
  }
}

int16_t parse_cmd_pwm_command(char *cmd, char *output, uint16_t len) 
{
  PWMDEBUG ("call: \n");

  if (cmd[0]=='\0') {
#ifdef CH_A_PWM_GENERAL_SUPPORT
    PWMDEBUG ("a: %i\n",channelAval);
#endif /* CH_A_PWM_GENERAL_SUPPORT */
#ifdef CH_B_PWM_GENERAL_SUPPORT
    PWMDEBUG ("b: %i\n",channelBval);
#endif /* CH_B_PWM_GENERAL_SUPPORT */
#ifdef CH_C_PWM_GENERAL_SUPPORT
    PWMDEBUG ("c: %i\n",channelCval);
#endif /* CH_C_PWM_GENERAL_SUPPORT */
    return ECMD_FINAL_OK;
  }
  uint8_t channel=cmd[1];
  uint8_t value=atoi(cmd+3);
  setpwm(channel,value);

  return ECMD_FINAL_OK;
}

/*
  -- Ethersex META --
  header(hardware/pwm/pwm.h)
  init(pwm_init)
  block([[PWM]])
  ecmd_feature(pwm_command, "pwm", [channel int], Set channel to value)
*/
