/*
 * Copyright (c) 2010 by Stefan Riepenhausen <rhn@gmx.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
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

#include "config.h"
#include "string.h"
#include "fnordlicht.h"
#include "fnordlicht_servo.h"
#include "remote-proto.h"
#include "hardware/pwm/pwm_servo.h"

#define USE_USART FNORDLICHT_USE_USART
#define BAUD 19200
#include "core/usart.h"

/* We generate our own usart init module, for our usart port */
generate_usart_init()

// flm buffer for data
uint8_t flmbuf[REMOTE_MSG_LEN];

// servo values
uint8_t flmmin[PWM_SERVOS];
uint8_t flmmax[PWM_SERVOS];
uint8_t flminit[PWM_SERVOS]; // todo set init values for servos

uint8_t flm_addr=0;

uint8_t ptr=0;
uint8_t syncbytes=0;
uint8_t flm_is_synced=0;

void
fnordlicht_servo_init(void)
{
  /* Initialize the usart module */
  usart_init();
  memset(flmmin,0x00, PWM_SERVOS);
  memset(flmmax,0xff, PWM_SERVOS);
}

ISR(usart(USART,_RX_vect))
{
  uint8_t data = usart(UDR);

  if (syncbytes==REMOTE_MSG_LEN) { // 15x sync + 0x00 found?
	flm_addr=data;  // we got now our address!
    ptr=0;  // reset pointer
    memset(flmbuf,0x00, REMOTE_MSG_LEN); // clear buffer
    flm_is_synced=1;  // now we are synced
    syncbytes=0;
	return;
  } 
  // search sync
  if (data == REMOTE_CMD_RESYNC) { // we have syncvalue?
    syncbytes++;
  } else {
    syncbytes=0;
  }

  if (flm_is_synced) {
     // add each byte to buffer 
     flmbuf[ptr++]=data;
     if (ptr==REMOTE_MSG_LEN) {  // buffer end?
       parseBuffer();
       ptr=0;
     }
   }
}

void flm_setservo(uint8_t s, uint8_t pos){
	if (pos < flmmin[s])
		pos=flmmin[s];
	if (pos > flmmax[s])
		pos=flmmax[s];
	setservo(s, pos);
}

void parseBuffer(){

  uint8_t addr=flmbuf[0];
  uint8_t cmd=flmbuf[1];
  uint8_t i=0;

  if ((addr!=REMOTE_ADDR_BROADCAST) && (addr!=flm_addr)) {  // is the command for us?
	return; 
  }

  switch (cmd){
    case REMOTE_CMD_RESYNC:
		break;
    case SERVO_ENABLE:
		pwm_servo_enable(1);
		break;
	case SERVO_DISABLE:
		pwm_servo_enable(0);
		break;
	case SERVO_SET_SINGLE:
		flm_setservo(flmbuf[2], flmbuf[3]);
		break;
	case SERVO_SET_ALL:
		for (i=0;i<PWM_SERVOS;i++) {
			flm_setservo(i, flmbuf[2+i]);
		}
		break;
	case SERVO_SET_MIN_ALL:
		for (i=0;i<PWM_SERVOS;i++) {
			uint8_t pos=flmbuf[2+i];
			flmmin[i]=pos;
		}
		break;
	case SERVO_SET_MAX_ALL:
		for (i=0;i<PWM_SERVOS;i++) {
			uint8_t pos=flmbuf[2+i];
			flmmax[i]=pos;
		}
		break;
	case SERVO_SET_INIT:
		for (i=0;i<PWM_SERVOS;i++) {
			setservo(i,flminit[i]);
        }
		break;
	case SERVO_SAVE:
		break;
  }
}

/*
  -- Ethersex META --
  header(protocols/fnordlicht/fnordlicht_servo.h)
  init(fnordlicht_servo_init)
*/

