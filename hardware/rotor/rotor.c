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
#include <stdbool.h>

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
* nord = 0 , ost = 90 , sued = 180 , west = 270
* ad values from ham 4 ---> ham 4 mode needs -180 to 180 mode
* adc   50%        75%      0%/100%          25%
* enum {NO, CW, CCW};
*/

extern char* rtstr[];
volatile struct rot_t rot;
volatile struct job_t job;
uint32_t count;

/**
* periodic call
*/
void rotor_periodic(void)
{
  rot.az_value = get_adc(0);
  double az;
  
  if (rot.az_value > rot.az_min_store)
    az = (rot.az_value - rot.az_min_store);
  else
    az = 0;
  
  az *= 360;
  az /= (rot.az_max_store - rot.az_min_store);

#ifdef ROTOR_HAM4_SUPPORT
  az -= 180;
#endif

  rot.azimuth = az;

  //syslog_sendf("%s,%d,%d", rtstr[rot.az_movement], rot.azimuth, rot.az_value); syslog_flush();

#ifdef DEBUG_ROTOR_FINE
       debug_printf("ROTOR: %s,%d,%d\n", rtstr[rot.az_movement], rot.azimuth, rot.az_value);
#endif

  if (rot.is_auto) {
    uint8_t mv = get_az_movement();
    syslog_sendf("autorot:%s,%d,%d", rtstr[mv], rot.az_preset, rot.azimuth); syslog_flush();
    job.az_turn = true;
    job.az_haswork = true;
    job.az_finish = false;

    //if (rot == NO)
      //rotor_turn(NO, MANUEL);

  } else {
    if (rot.az_movement == CCW && rot.az_value <= rot.az_min_store) {
       job.az_stop_immediately = true;
       job.az_stop = true;
       job.az_haswork = true;
       job.az_finish = false;
#ifdef DEBUG_ROTOR
       debug_printf("ROTOR: stop\n");
#endif

    } else if (rot.az_movement == CW && rot.az_value >= rot.az_max_store) {
       job.az_stop_immediately = true;
       job.az_stop = true;
       job.az_haswork = true;
       job.az_finish = false;
#ifdef DEBUG_ROTOR
       debug_printf("ROTOR: stop\n");
#endif
    }
  }

  if (count >= 50000)
    count = 0;
  count++;
}

void rotor_loop(void)
{
  if (job.az_finish)
    return;
  //syslog_sendf("do my work"); syslog_flush();

  if (job.az_haswork && job.az_stop) {
    rotor_stop(job.az_stop_immediately);
    job.az_stop_immediately = false;
    job.az_stop = false;
    job.az_finish = true;
    job.az_haswork = false;

  } else if (job.az_haswork && job.az_turn) {
    if (rot.az_movement == NO) {
       rotor_stop(job.az_stop_immediately);
       job.az_finish = true;
       job.az_haswork = false;

    } else if (rot.az_movement == CCW) {
      if (rot.az_preset >= rot.azimuth) {
        rotor_stop(true);
	job.az_finish = true;
	job.az_haswork = false;
      }

    } else if (rot.az_movement == CW) {
      if (rot.az_preset <= rot.azimuth) {
        rotor_stop(true);
	job.az_finish = true;
	job.az_haswork = false;
      }
    }

  }

  
}

/**
* initial 
*/
void rotor_init(void)
{
  job.az_finish = true;
  job.az_haswork = false;
  job.az_stop_immediately = false;
  job.az_stop = false;

  /* set all to zero */
  rot.speed = 50;   /* 50 % */
  rot.az_movement  = NO;
  rot.el_movement  = NO;
  rot.is_auto = MANUEL;

  uint16_t min_store, max_store;
  eeprom_restore_int(rotor_azimuth_min, &min_store);
  eeprom_restore_int(rotor_azimuth_max, &max_store);
  rot.az_min_store = min_store;
  rot.az_max_store = max_store;
}

/**
* stop the rotor
*/
void rotor_stop(uint8_t immediately) {
  rot.az_movement = NO;
  if (immediately)
    rot.is_auto = MANUEL;

  PIN_CLEAR(ROTOR_CW);
  PIN_CLEAR(ROTOR_CCW);
  

  syslog_sendf("rotor_stop %d,%d", rot.azimuth, rot.az_value); syslog_flush();
  break_set();

}

/**
* park the rotor
*/
void rotor_park() {
  syslog_sendf("rotor_park"); syslog_flush();
  uint16_t az_park_pos, el_park_pos;
  eeprom_restore_int(rotor_azimuth_parkpos, &az_park_pos);
  eeprom_restore_int(rotor_elevation_parkpos, &el_park_pos);

  rot.az_preset = az_park_pos;
  rot.el_preset = el_park_pos;
  rotor_turn(get_az_movement(), AUTO);
}

/**
* open break (break ins free)
* @param delay in ms
*/
void break_free(uint16_t delay) {
  uint16_t delay250ms = 100;
  if(delay > 0)
    _delay_ms(delay250ms);
  PIN_SET(ROTOR_BREAK);
#ifdef DEBUG_ROTOR
       debug_printf("ROTOR: break free\n");
#endif
}

/**
* close break (break is in use)
* @param delay in ms
*/
void break_set() {
#ifdef ROTOR_HAM4_SUPPORT
  PIN_CLEAR(ROTOR_BREAK);
#endif

#ifdef DEBUG_ROTOR
       debug_printf("ROTOR: break set\n");
#endif
}

/**
* turn the rotor to cw/ccw
* @parameter rotation cw/ccw
* @parameter auto(matic)
*/
void rotor_turn(uint8_t rotation, uint8_t isauto)
{
  uint16_t delay250ms = 250;
  syslog_sendf("rotor_turn:%s a=%d", rtstr[rotation], isauto); syslog_flush();

  if (rotation == CW && isauto == MANUEL) {
      if (! PIN_HIGH(ROTOR_CW)) {
        if (PIN_HIGH(ROTOR_BREAK)) {
	  rotor_stop(false);
          break_free(500);
          _delay_ms(delay250ms);
	}
        break_free(0);
        PIN_SET(ROTOR_CW);
	rot.az_movement = rotation;  // wieder setzen weil durch stop entfallen
      }

  } else if (rotation == CCW && isauto == MANUEL) {
      if (! PIN_HIGH(ROTOR_CCW)) {
        if (PIN_HIGH(ROTOR_BREAK)) {
          rotor_stop(false);
          break_free(500);
          _delay_ms(delay250ms);
	}
        break_free(0);
        PIN_SET(ROTOR_CCW);
	rot.az_movement = rotation;
      }

  } else if (rotation == CW && isauto == AUTO) {
    rot.is_auto = AUTO;
    rotor_turn(rotation, MANUEL);

  } else if (rotation == CCW && isauto == AUTO) {
    rot.is_auto = AUTO;
    rotor_turn(rotation, MANUEL);

  } else {
    rotor_stop(false);
    rot.is_auto = MANUEL;
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
uint8_t  get_az_movement() {

  //syslog_sendf("move:az=%d ps=%d", rot.azimuth, rot.az_preset); syslog_flush();
  
  if (rot.azimuth >= rot.az_preset && rot.azimuth <= rot.az_preset) {
    return NO;
  } else if (rot.azimuth <= rot.az_preset && rot.azimuth >= rot.az_preset) {
    return NO;
  } else if (rot.azimuth < rot.az_preset) {
    return CW;
  } else if (rot.azimuth > rot.az_preset) {
    return CCW;
  }
  return rot.az_movement;
}




/*
  -- Ethersex META --
  header(hardware/rotor/rotor.h)
  init(rotor_init)
  timer(5, rotor_periodic())
  mainloop(rotor_loop)
*/



