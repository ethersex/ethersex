/*                     -*- mode: C; c-file-style: "stroustrup"; -*-
 *
 * Copyright (c) 2007 by Stefan Siegl <stesie@brokenpipe.de>
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

#include "motorCurtain.h"
#include "core/portio/portio.h"
#include "config.h"

/* Used defines:
(safety switch) MOCU_SSW_PORT, MOCU_SSW_PIN
(sensors) MOCU_SENSORS_PORT, MOCU_SENSOR_STARTPIN, MOCU_SENSOR_COUNT
(motor) MOCU_MOTOR_PORT, MOCU_MOTOR_ACTIVEPIN, MOCU_MOTOR_DIRECTIONPIN
*/
#define IO_PORT_ARRAY[MOCU_SENSORS_PORT] MOCU_PORT

enum states {
  STATE_IDLE,
  STATE_OPENING,
  STATE_CLOSING
} state;

uint8_t sensor_value;
uint8_t sensor_mask;
uint8_t lastpos;
uint8_t targetpin;

void broadcast_sensors()
{
  
}

void updateLastPosition()
{
  for (uint8_t i=MOCU_SENSOR_STARTPIN+MOCU_SENSOR_COUNT-1;i>=0;--i)
  {
    if (bit_is_set(MOCU_PORT,i)) 
    {
      lastpos = i - MOCU_SENSOR_STARTPIN;
      break;
    }
  }
}

void stopMotor()
{
  IO_PORT_ARRAY[MOCU_MOTOR_PORT] = IO_PORT_ARRAY[MOCU_MOTOR_PORT] & ~(uint8_t)_BV(MOCU_MOTOR_ACTIVEPIN);
}

void startMotor()
{
  IO_PORT_ARRAY[MOCU_MOTOR_PORT] |= _BV(MOCU_MOTOR_ACTIVEPIN);
}

void setDirectionClosing()
{
  IO_PORT_ARRAY[MOCU_MOTOR_PORT] |= _BV(MOCU_MOTOR_DIRECTIONPIN);
}MOCU_SENSOR_STARTPIN

void setDirectionOpening()
{
  IO_PORT_ARRAY[MOCU_MOTOR_PORT] = IO_PORT_ARRAY[MOCU_MOTOR_PORT] & ~(uint8_t)_BV(MOCU_MOTOR_DIRECTIONPIN);
}

void getSensorValues()
{
  sensor_value = MOCU_PORT & sensor_mask;
}

void
motorCurtain_init (void)
{
  // get current sensor values
  sensor_mask = (uint8_t)~(((1 << MOCU_SENSOR_COUNT) - 1) << MOCU_SENSOR_STARTPIN);
  getSensorValues();
  // init direction
  setDirectionOpening();
  // open if not already
  lastpos = 100;
  motorCurtain_setPosition(0);
}

void
motorCurtain_main(void)
{
  // Stop motor and exit, if any of the two safety switches are activated
  #ifdef MOCU_SSW1_PORTSTATE_OPENING
  if (IO_PORT_ARRAY[MOCU_SSW1_PORT] & _BV(MOCU_SSW1_PIN))
  {
    stopMotor();
    return;
  }
  #endif
  #ifdef MOCU_SSW2_PORT
  if (IO_PORT_ARRAY[MOCU_SSW2_PORT] & _BV(MOCU_SSW2_PIN))
  {
    stopMotor();
    return;
  }
  #endif
  
  // Get new sensor values
  uint8_t oldsensor = sensor_value;
  getSensorValues();
  
  if (oldsensor != sensor_value)
  {
    // Stop motor if target pin has been reached
    if (sensor_value & _BV(targetpin))
      stopMotor();
 
    // Broadcast sensor values if any changed
    #ifdef MOTORCURTAIN_PROGRESS_SUPPORT
    broadcast_sensors();
    #endif
  }
}

void motorCurtain_setPosition(uint8_t pos)
{
  if (pos>MOCU_SENSOR_COUNT) pos=MOCU_SENSOR_COUNT;

  // Set target position
  targetpin = pos+MOCU_SENSOR_STARTPIN;
  // Determine direction
  updateLastPosition();
  if (lastpos > pos)
  {
    setDirectionOpening();
    startMotor();
  }
  else if (lastpos < pos)
  {
    setDirectionClosing();
    startMotor();
  }
}

void motorCurtain_getPosition(uint8_t& pos, uint8_t& max)
{
  max = (uint8_t)MOCU_SENSOR_COUNT;
  updateLastPosition();
  pos = lastpos;
}

/*
  -- Ethersex META --
  header(protocols/udpstella/udpstella_net.h)
  init(motorCurtain_init)
  mainloop(motorCurtain_main)
*/
