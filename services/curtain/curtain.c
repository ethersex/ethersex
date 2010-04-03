/*
 * Copyright (c) 
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

#include "core/portio/portio.h"
#include "protocols/uip/uip.h"
#include "protocols/uip/uip_router.h"
#include "config.h"
#include "curtain.h"

/* Used defines:
(safety switches) MOCU_SAFETYSWITCH
(sensors) MOCU_SENSORS_PORT, MOCU_SENSOR_STARTPIN, MOCU_SENSOR_COUNT
(motor) MOCU_MOTOR_ENABLE, MOCU_MOTOR_DIRECTION
*/

enum states {
  STATE_IDLE,
  STATE_OPENING,
  STATE_CLOSING
} state;

uint8_t sensor_value;
uint8_t sensor_mask;
uint8_t lastpos;
uint8_t targetpin;
uint8_t sensors_changed = 0;
struct uip_udp_conn * new_conn = 0;

#ifdef MOTORCURTAIN_PROGRESS_SUPPORT
void broadcast_sensors()
{
	if(!sensors_changed || !uip_poll()) return;
	
	char* data = uip_appdata;
	data[0] = 'c';
	data[1] = 'u';
	data[2] = 'r';
	data[3] = 't';
	data[4] = 'a';
	data[5] = 'i';
	data[6] = 'n';
	data[7] = (char)lastpos;
	data[8] = (char)MOCU_SENSOR_COUNT;

	uip_udp_send(9);
	sensors_changed = 0;
}
#endif

void readSensorValues()
{
  sensor_value = MOCU_SENSORS_PIN_PORT & sensor_mask;
  #ifdef MOTORCURTAIN_INVERTED_SENSORS
	sensor_value = ~sensor_value;
  #endif
}

void updateLastPosition()
{
  for (int8_t i=MOCU_SENSOR_STARTPIN;i<MOCU_SENSOR_STARTPIN+MOCU_SENSOR_COUNT;++i)
  {
    if (bit_is_set(sensor_value,i))
    {
      lastpos = i - MOCU_SENSOR_STARTPIN;
      break;
    }
  }
}

void stopMotor()
{
	#ifdef MOTORCURTAIN_INVERTED_MOTOR
		PIN_SET(MOCU_MOTOR_ENABLE);
	#else
		PIN_CLEAR(MOCU_MOTOR_ENABLE);
	#endif
}

void startMotor()
{
	#ifdef MOTORCURTAIN_INVERTED_MOTOR
		PIN_CLEAR(MOCU_MOTOR_ENABLE);
	#else
		PIN_SET(MOCU_MOTOR_ENABLE);
	#endif
}

void setDirectionClosing()
{
	#ifdef MOTORCURTAIN_INVERTED_DIRECTION
  PIN_SET(MOCU_MOTOR_DIRECTION);
	#else
  PIN_CLEAR(MOCU_MOTOR_DIRECTION);
	#endif
}

void setDirectionOpening()
{
	#ifdef MOTORCURTAIN_INVERTED_DIRECTION
		PIN_CLEAR(MOCU_MOTOR_DIRECTION);
	#else
		PIN_SET(MOCU_MOTOR_DIRECTION);
	#endif
}

void
motorCurtain_init (void)
{
	sensor_mask = ((1 << MOCU_SENSOR_COUNT) - 1) << MOCU_SENSOR_STARTPIN;
	// set to input
	MOCU_SENSORS_DDR_PORT = MOCU_SENSORS_DDR_PORT & (uint8_t)~sensor_mask;
	// activate pullup if necessary
	#ifdef MOTORCURTAIN_SENSORS_PULLUP
		MOCU_SENSORS_PORT |= sensor_mask;
	#endif

	// activate pullup for the safty switch if necessary
	#ifdef MOCU_SAFETYSWITCH
	#ifdef MOTORCURTAIN_SAFETYSWITCH_PULLUP
		PIN_SET(MOCU_SAFETYSWITCH)
	#endif
	#endif
	
	// init direction
	setDirectionOpening();
	// define last position, overwrite if sensor value available
	lastpos = 0;
	// get sensor values + get last position (if available) + set position
	motorCurtain_setPosition(0);

	#ifdef MOTORCURTAIN_PROGRESS_SUPPORT
	// Set remote IP address to MOTORCURTAIN_REPORT_IP and remote port to MOTORCURTAIN_REPORT_PORT
  	uip_ipaddr_t ip_addr;
	if (uiplib_ipaddrconv(MOTORCURTAIN_REPORT_IP, (unsigned char *)&ip_addr)!=0)
	{
		//uip_ipaddr(ip_addr, 192, 168, 0, 1);
		new_conn = uip_udp_new(&ip_addr, HTONS(MOTORCURTAIN_REPORT_PORT), broadcast_sensors);
		//choose an arbitrary port for receiving (we only want to send with this connection)
		if (new_conn) uip_udp_bind(new_conn, HTONS(12344));

		// Force uIP to poll the UDP application for outgoing data
		uip_udp_periodic_conn(new_conn);
	}
	#endif
}

void
motorCurtain_main(void)
{
  // Stop motor and exit, if the safety switch is activated
  #ifdef MOCU_SAFETYSWITCH
  #ifdef MOTORCURTAIN_INVERTED_SAFETYSWITCH
  if (!PIN_HIGH(MOCU_SAFETYSWITCH) )
  #else
  if (PIN_HIGH(MOCU_SAFETYSWITCH) )
  #endif
  {
    stopMotor();
    return;
  }
  #endif

  // Get new sensor values
  uint8_t oldsensor = sensor_value;
  readSensorValues();
  
  if (oldsensor != sensor_value)
  {
    updateLastPosition();
    // Stop motor if target pin has been reached
    if (sensor_value & _BV(targetpin))
	{
      stopMotor();
	}
 
    // Broadcast sensor values if any changed
    sensors_changed = 1;
  }
}

void motorCurtain_setPosition(uint8_t pos)
{
  if (pos>=MOCU_SENSOR_COUNT) pos=MOCU_SENSOR_COUNT-1;
  // stop motor
  stopMotor();

  // Set target position
  targetpin = pos+MOCU_SENSOR_STARTPIN;
  readSensorValues();
  updateLastPosition();
  // Determine direction
  if (lastpos > pos) // close
  {
    setDirectionClosing();
    startMotor();
  } else
  if (lastpos < pos) // open
  {
    setDirectionOpening();
    startMotor();
  }
}


uint8_t motorCurtain_getMax()
{
  return (uint8_t)MOCU_SENSOR_COUNT;
}

uint8_t motorCurtain_getPosition()
{
  updateLastPosition();
  return lastpos;
}

void motorCurtain_off()
{
	stopMotor();
}

/*
  -- Ethersex META --
  header(services/curtain/curtain.h)
  init(motorCurtain_init)
  mainloop(motorCurtain_main)
*/
