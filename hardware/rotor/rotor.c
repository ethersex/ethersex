/*
*
* Copyright (c) 2009 by Jonny Roeker <dg9oaa@darc.de>
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
#include <avr/pgmspace.h>
#include <stdio.h>
#include <stdlib.h>  
#include <string.h>
#include <util/delay.h>

#include "config.h"
#include "protocols/ecmd/ecmd-base.h"
#include "core/eeprom.h"
#include "protocols/syslog/syslog.h"

#include "hardware/rotor/rotor.h"


#ifndef ADC_REF
#define ADC_REF 0
#endif

/*
* CW  bedeutet clockwise        -> Rechtsdrehung
* CCW bedeutet counterclockwise -> Linksdrehung
*/

/**
* periodic call
*/
void rotor_periodic(void)
{
  uint16_t direction_current = get_adc(0);

  uint16_t min_store, max_store;
  eeprom_restore_int(rotor_azimuth_min, &min_store);
  eeprom_restore_int(rotor_azimuth_max, &max_store);
  
  alpha = (direction_current - min_store);
  alpha *= 360;
  alpha /= (max_store - min_store);

  if (is_auto) {
     uint8_t rot = get_rotation();
     //syslog_sendf("autorot:%d", rot);
     //syslog_flush();
     if (rot == NO)
       rotor_turn(NO, MANUEL);
  }
}


/**
* initial 
*/
void rotor_init(void)
{
  /* set all to zero */
  speed = 50;   /* 50 % */
  rotation  = NO;
}

/**
* stop the rotor
*/
void rotor_stop() {
  PIN_CLEAR(ROTOR_CW);
  PIN_CLEAR(ROTOR_CCW);
  break_set(500);
  rotation = NO;
}

/**
* open break (break ins free)
* @param delay in ms
*/
void break_free(uint16_t delay) {
  if(delay > 0)
    _delay_ms(delay);
  PIN_SET(ROTOR_BREAK);
}

/**
* close break (break is in use)
* @param delay in ms
*/
void break_set(uint16_t delay) {
  if(delay > 0)
    _delay_ms(delay);
  PIN_CLEAR(ROTOR_BREAK);
}

/**
* turn the rotor to cw/ccw
* @parameter rotation cw/ccw
* @parameter auto(matic)
*/
void rotor_turn(uint8_t rotation, uint8_t isauto)
{
  if (rotation == CW && isauto == MANUEL) {
      if (! PIN_HIGH(ROTOR_CW)) {
        rotor_stop();
        break_free(0);
        _delay_ms(250);
        PIN_SET(ROTOR_CW);
      }

  } else if (rotation == CCW && isauto == MANUEL) {
      if (! PIN_HIGH(ROTOR_CCW)) {
        rotor_stop();
        break_free(0);
        _delay_ms(250);
        PIN_SET(ROTOR_CCW);
      }
  } else if (rotation == CW && isauto == AUTO) {
    is_auto = AUTO;
    rotor_turn(rotation, MANUEL);
  } else if (rotation == CCW && isauto == AUTO) {
    is_auto = AUTO;
    rotor_turn(rotation, MANUEL);
  } else {
    rotor_stop();
    is_auto = MANUEL;
  }
}

/**
* get analog value from channel n
*/
uint16_t get_adc(int channel)
{
  uint16_t adc;

  ADMUX = (ADMUX & 0xF0) | channel | ADC_REF;
  /* Start adc conversion */
  ADCSRA |= _BV(ADSC);
  /* Wait for completion of adc */
  while (ADCSRA & _BV(ADSC)) {}
  adc = ADC;

  return adc;
}

/**
* returns the rotation dependent on alpha and preset
*/
uint8_t  get_rotation() {
  uint8_t rot = rotation;
  if (alpha >= preset && alpha <= preset + HYSTERESIS) {
    return NO;
  } else if (alpha <= preset && alpha >= preset - HYSTERESIS) {
    return NO;
  } else if (alpha < preset - HYSTERESIS) {
    return CW;
  } else if (alpha > preset + HYSTERESIS) {
    return CCW;
  }
  return rot;
}






