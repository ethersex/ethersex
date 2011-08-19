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

#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <avr/pgmspace.h>

#include "config.h"
#include "core/debug.h"
#include "core/eeprom.h"

#include "protocols/ecmd/ecmd-base.h"

#include "hardware/rotor/rotor.h"

/**
* rotor status
* returns String cw,speed=75, preset=100, alpha=89
*/
int16_t parse_cmd_rotor_status(char *cmd, char *output, uint16_t len)
{
  char* rtstr[] = {"hold", "cw", "ccw"};

  return ECMD_FINAL(snprintf_P(output, len, PSTR("%s,speed=%d, preset=%d, alpha(%d)=%d"),
                               rtstr[rotation],
                               speed,
                               preset,
                               get_adc(0),
                               alpha ));
}

/**
* rotor preset DIRECTION SPEED
* @param direction in degree
* @param speed in percent
*/
int16_t parse_cmd_rotor_azimuth(char *cmd, char *output, uint16_t len)
{
  uint16_t direction;

  while(*cmd == ' ') cmd++;

  if (sscanf_P(cmd, PSTR("%d %d"), &direction, &speed) != 2)
    return ECMD_ERR_PARSE_ERROR;

  preset = direction;
  uint8_t rot = get_rotation();
  char* rtstr[] = {"hold", "cw", "ccw"};
  rotation = rot;
  rotor_turn(rot, AUTO);
  
  return ECMD_FINAL(snprintf_P(output, len, PSTR("OK angle=%d speed=%d turn %s"),
                               direction, speed, rtstr[rot]));
  //return ECMD_FINAL_OK;
}


/**
* rotor cw
* @params [speed]  in percent
*/
int16_t parse_cmd_rotor_cw(char *cmd, char *output, uint16_t len)
{
  while(*cmd == ' ') cmd++;
    if(strlen(cmd) > 0) {
      speed = atol(cmd);
    }
  
  rotation = CW;
  rotor_turn(rotation, MANUEL);
  return ECMD_FINAL(snprintf_P(output, len, PSTR("OK speed=%d"), speed));

}


/**
* rotor ccw
* @params [speed]  in percent
*/
int16_t parse_cmd_rotor_ccw(char *cmd, char *output, uint16_t len)
{
  while(*cmd == ' ') cmd++;
    if(strlen(cmd) > 0) {
      speed = atol(cmd);
     }

  rotation = CCW;
  rotor_turn(rotation, MANUEL);
  return ECMD_FINAL(snprintf_P(output, len, PSTR("OK speed=%d"), speed));
}

/**
* rotor stop
*/
int16_t parse_cmd_rotor_stop(char *cmd, char *output, uint16_t len)
{
  rotation = NO;
  rotor_stop();
  return ECMD_FINAL_OK;
}


/**
* rotor calibrate
* @param min max
*/
int16_t parse_cmd_rotor_cal_azimuth(char *cmd, char *output, uint16_t len)
{
  uint16_t min;
  uint16_t max;
  while(*cmd == ' ') cmd++;

  if (sscanf_P(cmd, PSTR("%d %d"), &min, &max) != 2)
    return ECMD_ERR_PARSE_ERROR;

  eeprom_save_int(rotor_azimuth_min, min);
  eeprom_save_int(rotor_azimuth_max, max);
  eeprom_update_chksum();
  return ECMD_FINAL_OK;
}

/**
* rotor get calibrate
* returns String min:0 max:1023
*/
int16_t parse_cmd_rotor_getcal_azimuth(char *cmd, char *output, uint16_t len)
{
  uint16_t min_store, max_store;
  eeprom_restore_int(rotor_azimuth_min, &min_store);
  eeprom_restore_int(rotor_azimuth_max, &max_store);
 
  return ECMD_FINAL(snprintf_P(output, len, PSTR("min:%d max:%d"),
                               min_store, max_store ));
}



/*
  -- Ethersex META --
  header(hardware/rotor/rotor.h)
  timer(5, rotor_periodic())
  init(rotor_init)
  block([[Rotor Interface]]))
  ecmd_feature(rotor_azimuth, "rotor preset", [DIRECTION] [SPEED], 'Set Geographic Direction (Angle: 0-359) Speed in % : 1-100')
  ecmd_feature(rotor_status, "rotor status",, Display the current rotor status.)
  ecmd_feature(rotor_cw,  "rotor cw" , [SPEED], Turn Direction.)
  ecmd_feature(rotor_ccw, "rotor ccw", [SPEED], Turn Direction.)
  ecmd_feature(rotor_stop, "rotor stop", [SPEED], Stop Turn Direction.)
  ecmd_feature(rotor_cal_azimuth, "rotor calibrate", [MIN] [MAX], Calibrate azimuth min and max.)
  ecmd_feature(rotor_getcal_azimuth, "rotor get calibrate",, Get calibrate azimuth min and max.)
*/

