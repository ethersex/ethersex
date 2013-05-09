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

#include <stdio.h>
#include <stdlib.h>
#include <avr/pgmspace.h>

#include "pwm_common.h"
#include "pwm.h"
#include "config.h"
#include "protocols/ecmd/ecmd-base.h"

#ifdef CH_A_PWM_GENERAL_SUPPORT
  uint8_t channelAval=PWM_MIN_VALUE;
#endif /* CH_A_PWM_GENERAL_SUPPORT */
#ifdef CH_B_PWM_GENERAL_SUPPORT
  uint8_t channelBval=PWM_MIN_VALUE;
#endif /* CH_B_PWM_GENERAL_SUPPORT */
#ifdef CH_C_PWM_GENERAL_SUPPORT
  uint8_t channelCval=PWM_MIN_VALUE;
#endif /* CH_C_PWM_GENERAL_SUPPORT */

#ifdef PWM_GENERAL_FADING_SUPPORT
  int8_t fadingAspeed=0;  // 0 = disable
  int8_t fadingBspeed=0;
  int8_t fadingCspeed=0;
#endif /* PWM_GENERAL_FADING_SUPPORT */

// init DDR, waveform and timer
void
pwm_init(){
  TC1_COUNTER_CURRENT=0x00FF; //set the timer counter
#ifdef CH_A_PWM_GENERAL_SUPPORT
  DDR_CONFIG_OUT(CHANNEL_A_PWM); 		// PWM OUTPUT
  TC1_COUNTER_COMPARE=channelAval;
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

// return current pwm value
uint8_t
getpwm(char channel){
  uint8_t ret=0;
  switch (channel){
#ifdef CH_A_PWM_GENERAL_SUPPORT
    case 'a': ret=channelAval;
	  break;
#endif /* CH_A_PWM_GENERAL_SUPPORT */
#ifdef CH_B_PWM_GENERAL_SUPPORT
    case 'b': ret=channelBval;
	  break;
#endif /* CH_C_PWM_GENERAL_SUPPORT */
#ifdef CH_C_PWM_GENERAL_SUPPORT
    case 'c': ret=channelCval;
	  break;
#endif /* CH_A_PWM_GENERAL_SUPPORT */
    default:
    PWMDEBUG ("channel %c unsupported\n",channel);
  }
#ifdef PWM_GENERAL_INVERT_SUPPORT
  return 255-ret;
#else
  return ret;
#endif /* PWM_GENERAL_INVERT_SUPPORT */
}

// set pwm value
void
setpwm(char channel, uint8_t setval){
  PWMDEBUG ("channel %c, values: %i\n",channel, setval);
#ifdef PWM_GENERAL_INVERT_SUPPORT
  setval=255-setval;
#endif /* PWM_GENERAL_INVERT_SUPPORT */
  switch (channel){
#ifdef CH_A_PWM_GENERAL_SUPPORT
    case 'a': 
      TC1_COUNTER_COMPARE=setval;
	  channelAval=setval;
	  break;
#endif /* CH_A_PWM_GENERAL_SUPPORT */
#ifdef CH_B_PWM_GENERAL_SUPPORT
    case 'b': 
      OCR1B=setval;
	  channelBval=setval;
	  break;
#endif /* CH_C_PWM_GENERAL_SUPPORT */
#ifdef CH_C_PWM_GENERAL_SUPPORT
    case 'c': 
      OCR1C=setval;
	  channelCval=setval;
	  break;
#endif /* CH_A_PWM_GENERAL_SUPPORT */
    default:
    PWMDEBUG ("channel %c unsupported\n",channel);
  }
}

// set pwm via ecmdA
int16_t parse_cmd_pwm_command(char *cmd, char *output, uint16_t len) 
{
  uint8_t channel=cmd[1];
  uint8_t value=atoi(cmd+3);

  if (cmd[0]=='\0') {
#ifdef CH_A_PWM_GENERAL_SUPPORT
    PWMDEBUG ("a: %i\n",getpwm('a'));
#endif /* CH_A_PWM_GENERAL_SUPPORT */
#ifdef CH_B_PWM_GENERAL_SUPPORT
    PWMDEBUG ("b: %i\n",getpwm('b'));
#endif /* CH_B_PWM_GENERAL_SUPPORT */
#ifdef CH_C_PWM_GENERAL_SUPPORT
    PWMDEBUG ("c: %i\n",getpwm('c'));
#endif /* CH_C_PWM_GENERAL_SUPPORT */
    return ECMD_FINAL_OK;
  }
  if (cmd[2]=='\0') {
      return ECMD_FINAL(snprintf_P(output, len, PSTR("%i"), getpwm(channel)));
  }
  setpwm(channel,value);

  return ECMD_FINAL_OK;
}

#ifdef PWM_GENERAL_FADING_SUPPORT
// set fading for channel
int16_t parse_cmd_pwm_fade_command(char *cmd, char *output, uint16_t len) 
{
  uint8_t channel=cmd[1];
  int8_t diff=atoi(cmd+3);
  uint8_t startvalue=atoi(cmd+8);

  PWMDEBUG ("set ch: %c, diff %i, start %i\n",channel, diff,startvalue);
  setpwm(channel,startvalue);

  switch (channel){
#ifdef CH_A_PWM_GENERAL_SUPPORT
	case 'a': fadingAspeed=diff; break;
#endif /* CH_A_PWM_GENERAL_SUPPORT */
#ifdef CH_B_PWM_GENERAL_SUPPORT
	case 'b': fadingBspeed=diff; break;
#endif /* CH_B_PWM_GENERAL_SUPPORT */
#ifdef CH_C_PWM_GENERAL_SUPPORT
	case 'c': fadingCspeed=diff; break;
#endif /* CH_C_PWM_GENERAL_SUPPORT */
  }

  return ECMD_FINAL_OK;
}
#endif /* PWM_GENERAL_FADING_SUPPORT */

void
pwm_periodic()
{
#ifdef PWM_GENERAL_FADING_SUPPORT

 #ifdef CH_A_PWM_GENERAL_SUPPORT
  if (fadingAspeed!=0){
    int16_t chAdiff = getpwm('a')+fadingAspeed;
    if (chAdiff >= PWM_MIN_VALUE) {
      fadingAspeed=0;
      setpwm('a',PWM_MIN_VALUE);
    } else if (chAdiff<=0) {
      fadingAspeed=0;
      setpwm('a',PWM_MAX_VALUE);
    } else
      setpwm('a',chAdiff);
  }
 #endif /* CH_A_PWM_GENERAL_SUPPORT */
 #ifdef CH_B_PWM_GENERAL_SUPPORT
  if (fadingBspeed!=0){
    int16_t chBdiff = getpwm('b')+fadingBspeed;
    if (chBdiff >= PWM_MIN_VALUE) {
      fadingBspeed=0;
      setpwm('b',PWM_MIN_VALUE);
    } else if (chBdiff<=0) {
      fadingBspeed=0;
      setpwm('b',PWM_MAX_VALUE);
    } else
      setpwm('b',chBdiff);
  }
 #endif /* CH_B_PWM_GENERAL_SUPPORT */
 #ifdef CH_C_PWM_GENERAL_SUPPORT
  if (fadingCspeed!=0){
    int16_t chCdiff = getpwm('c')+fadingCspeed;
    if (chCdiff >= PWM_MIN_VALUE) {
      fadingCspeed=0;
      setpwm('c',PWM_MIN_VALUE);
    } else if (chCdiff<=0) {
      fadingCspeed=0;
      setpwm('c',PWM_MAX_VALUE);
    } else
      setpwm('c',chCdiff);
  }
 #endif /* CH_C_PWM_GENERAL_SUPPORT */

#endif /* PWM_GENERAL_FADING_SUPPORT */
}



/*
  -- Ethersex META --
  header(hardware/pwm/pwm.h)
  init(pwm_init)
  timer(5, pwm_periodic())
  block([[PWM]])
  ecmd_ifdef(PWM_GENERAL_FADING_SUPPORT)
    ecmd_feature(pwm_fade_command, "pwm fade", [channel +-diff startvalue], Set fading at channel with startvalue and change each stepp to diff (must be signed 3 digit))
  ecmd_endif()
  ecmd_feature(pwm_command, "pwm set", [channel value], Set channel to value)
*/
