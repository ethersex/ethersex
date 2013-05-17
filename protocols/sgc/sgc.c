/*
 *
 * Copyright (c) 2008 by Christian Dietrich <stettberger@dokucode.de>
 * Copyright (c) 2013 by Nico Dziubek <hundertvolt@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
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
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <string.h>
#include "core/eeprom.h"
#include "config.h"
#include "sgc.h"

#define USE_USART SGC_USE_USART
#define BAUD SGC_BAUDRATE
#include "core/usart.h"

generate_usart_init() /* We generate our own usart init module, for our usart port */

struct sgc_buffer sgc_uart_buffer;
struct sgc_state sgc_power_state;

void sgc_init(void)
{
 PIN_SET(SGC_RESET);      /* hold display in reset */
 usart_init();         /* Initialize the usart module */
 sgc_uart_buffer.rxenable = 0; /* ignore anything on the RX line */
}

uint8_t sgc_setpowerstate (uint8_t soll)
{
#ifdef SGC_TIMEOUT_COUNTER
sgc_power_state.mincount = 0;	/* any command from ECMD resets the counter */
sgc_power_state.timeout = 0;
#endif /* SGC_TIMEOUT_COUNTER */

 if ((sgc_power_state.ist != SHUTDOWN) && (sgc_power_state.ist != POWERUP))
  return 1;            /* state machine busy */
 if (soll == 0)
 	{
 	sgc_power_state.ist = BEGIN_SHUTDOWN;
 	return 0;
	}
 sgc_power_state.ist = BEGIN_POWERUP;
 return 0;  
}

uint8_t sgc_getpowerstate (void)
{
#ifdef SGC_TIMEOUT_COUNTER
sgc_power_state.mincount = 0;	/* any command from ECMD resets the counter */
sgc_power_state.timeout = 0;
#endif /* SGC_TIMEOUT_COUNTER */
 return sgc_power_state.ist;
}

uint8_t sgc_sendcommand (uint8_t cmdlen, uint8_t *data, uint8_t from_sequence)
{
 if ((sgc_uart_buffer.pos != 0) || (((sgc_power_state.ist <= SHUTDOWN) || (sgc_uart_buffer.busy == 1) || (sgc_uart_buffer.ack > NACK) ) && (from_sequence == 0)))
  return 1;           /* Waiting for response, Display TX busy, Display in shutdown, not yet initialized, reserved */
 if (from_sequence == 0)
   {
   sgc_power_state.from_reset = 0; 
   #ifdef SGC_TIMEOUT_COUNTER
   sgc_power_state.mincount = 0;	/* any command from ECMD resets the counter */
   sgc_power_state.timeout = 0;
   #endif /* SGC_TIMEOUT_COUNTER */
   }
 memcpy(sgc_uart_buffer.txdata, data, cmdlen); /* copy send buffer */
 sgc_uart_buffer.len = cmdlen;         /* copy command length */
 sgc_uart_buffer.pos = 1;           /* set TX buffer pointer */
 sgc_uart_buffer.ack = SENDING;
 usart(UCSR,B) |= _BV(usart(TXCIE));      /* activate TX interrupt */
 usart(UDR) = sgc_uart_buffer.txdata[0];    /* transmit first byte */
 return 0;                   /* success */
}

uint8_t sgc_getcommandresult (void)
{
#ifdef SGC_TIMEOUT_COUNTER
sgc_power_state.mincount = 0;	/* any command from ECMD resets the counter */
sgc_power_state.timeout = 0;
#endif /* SGC_TIMEOUT_COUNTER */
 if ((sgc_uart_buffer.busy == 1) || (sgc_uart_buffer.ack > NONE)) /* not valid while in a command sequence or timeout*/
  return BUSY;
 if (sgc_power_state.from_reset == 1)
    return FROM_RESET;
 return sgc_uart_buffer.ack;   /* send last result */
}

uint8_t sgc_setcontrast (uint8_t contrast)
{
 uint8_t command[3];
#ifdef SGC_TIMEOUT_COUNTER
sgc_power_state.mincount = 0;	/* any command from ECMD resets the counter */
sgc_power_state.timeout = 0;
#endif /* SGC_TIMEOUT_COUNTER */
 if (sgc_power_state.ist == POWERUP)
  {
  command[0] = 0x59;   /* "Control" Command */
  command[1] = 0x02;   /* "Contrast" Command */
  command[2] = contrast; /* Value */
  if (sgc_sendcommand(3, command, 0) != 0) /* busy with some other command */
     return 1;
  }
 sgc_power_state.contrast = contrast;  /* save the contrast value for next powerup */
   return 0;                /* success */   
}

uint8_t rgb2sgc (uint8_t red, uint8_t green, uint8_t blue, uint8_t *ret)
{
 uint8_t green6;
 if ((red > 0x1F) || (green > 0x1F) || (blue > 0x1F))    /* value out of range? */
  return 1; /* error */
 green6 = (((green & 0x1F) << 1) | (green & 0x01));     /* convert from 5bit to 6bit, copy LSB */
 ret[0] = (((green6 & 0x38) >> 5) | ((red & 0x1F) << 3));  /* convert to High Byte */
 ret[1] = ((blue & 0x1F) | ((green6 & 0x07) << 5));     /* convert to Low Byte */
 return 0; /* success */
}

#ifdef SGC_TIMEOUT_COUNTER
void sgc_reset_timeout (void)
{
sgc_power_state.mincount = 0;
sgc_power_state.timeout = 0;
}
#endif /* SGC_TIMEOUT_COUNTER */

void sgc_pwr_periodic (void) /* Ausführungsrate 20ms */
{
 uint8_t pwr_command[3];
 
 if (sgc_uart_buffer.ack == NONE)
  sgc_uart_buffer.acktimer++;
 if (sgc_uart_buffer.acktimer > 20) /* 400ms Timeout for ACK */
  {
  sgc_uart_buffer.ack = TIMEOUT;
  sgc_uart_buffer.acktimer = 0;
  sgc_uart_buffer.rxenable = 0; /* disable reception */
  }

#ifdef SGC_TIMEOUT_COUNTER
if (sgc_power_state.ist == POWERUP)
  {
  if (++sgc_power_state.mincount == 3000) /* 1 minute */
    {
    sgc_power_state.mincount = 0;
    if (++sgc_power_state.timeout == SGC_TIMEOUT)
      {
      sgc_power_state.ist = BEGIN_SHUTDOWN;  /* go into shutdown if timed out */
      }
    }
  }
#endif /* SGC_TIMEOUT_COUNTER */

    switch (sgc_power_state.ist) 
     {
     case DISP_RESET:         /* STATE 0: Begin Reset Sequence */
     	sgc_uart_buffer.busy = 1;    /* block UART for command sequence */
     	sgc_uart_buffer.rxenable = 0;  /* ignore anything on the RX line */
      if (sgc_uart_buffer.pos != 0)  /* wait for UART to finish sending if necessary */
       break;
      PIN_SET(SGC_RESET);      /* put display in reset */
      sgc_power_state.baudcounter = 0;        /* reset autobaud command counter */
      sgc_uart_buffer.ack = ACK;   /* only for the first time to enable sending*/
      sgc_power_state.contrast = 0x0F;    /* default value */
      sgc_power_state.timer = 0;      /* start reset timer */
      sgc_power_state.ist = 1;    /* proceed to next state */
      sgc_power_state.from_reset = 1;
      break;
     
     case 1:              /* STATE 1: Wait for Reset to time out */
      sgc_power_state.timer++;       /* periodically count timer up */
      if (sgc_power_state.timer == 5)    /* hold reset for 100 ms */
       {
        PIN_CLEAR(SGC_RESET);   /* release reset */
        sgc_power_state.timer = 0;    /* clear for boot timer */
        sgc_power_state.ist = 2;  /* proceed to next state */
       }
      break;
     
     case 2:              /* STATE 2: Wait for boot-up to time out */
      sgc_power_state.timer++;       /* periodically count timer up */
      if (sgc_power_state.timer == 100)   /* about 2 sec for display controller to boot up */
       {
        sgc_power_state.ist = 3;  /* proceed to next state */
       }
      break;
     
     case 3:              /* STATE 3: Auto-baud display */
      if (sgc_power_state.baudcounter < 10)     /* try autobauding for up to 10 times until successful */
       {
       pwr_command[0] = 0x55;     /* put autobaud command into sending queue */
       sgc_uart_buffer.rxenable = 1; /* enable the RX line */
       if (sgc_sendcommand(1, pwr_command, 1) == 0) /* send command */
      {
        sgc_power_state.ist = 4;  /* if send command successful, proceed to next state */
      break;
      }
       }
      sgc_power_state.ist = DISP_RESET; /* Send command or 10 tries of autobauding were unsuccessful, restart reset sequence */
      break;
     
     case 4:              /* STATE 4: wait for answer to autobaud command */
      if ((sgc_uart_buffer.ack == NONE) || (sgc_uart_buffer.ack == SENDING))  /* no answer or timeout yet, keep waiting */
       break;
      if (sgc_uart_buffer.ack == ACK)
       {
       sgc_power_state.ist = BEGIN_SHUTDOWN;  /* ACK received, proceed to next state - Shutdown after Init */
       break;
       }
      if (sgc_uart_buffer.ack == TIMEOUT)  /* Timeout, try again */
       {
       sgc_power_state.baudcounter++;         /* increment autobaud counter */
       sgc_power_state.ist = 3;   /* go back to State 3 - Send Autobaud */
       break;
       }
      sgc_power_state.ist = DISP_RESET; /* if NACK or any unexpected response, restart reset sequence */
      break;

     case BEGIN_SHUTDOWN:       /* STATE 5: Start shutdown sequence with display off command */
      sgc_uart_buffer.busy = 1;    /* block UART for command sequence */
      if ((sgc_uart_buffer.ack == NONE) || (sgc_uart_buffer.ack == SENDING)) /* wait for last command to finish if necessary */
       break;
      if (sgc_uart_buffer.ack <= NACK) 
       {               /* continue if ACK or NACK was received; previous unknown command was terminated in defined manner */
       pwr_command[0] = 0x59;    /* "Control" Command */
      pwr_command[1] = 0x01;    /* "Display OnOff" Command */
      pwr_command[2] = 0x00;    /* "Off" Command */
      if (sgc_sendcommand(3, pwr_command, 1) == 0) /* send command */
        {
        sgc_power_state.ist = 6;  /* if send command successful, proceed to next state */
        break;
       }
      }
      sgc_power_state.ist = DISP_RESET; /* if unexpected result or timeout of previous unknown command --> should never happen, go into reset */ 
      break;
     
     case 6:             /* STATE 6: Set contrast to zero */  
       if ((sgc_uart_buffer.ack == NONE) || (sgc_uart_buffer.ack == SENDING)) /* no answer yet received, wait */
       break; 
      if (sgc_uart_buffer.ack == ACK) /* continue only if ACK was received */
       {
       pwr_command[0] = 0x59;   /* "Control" Command */
      pwr_command[1] = 0x02;   /* "Contrast" Command */
      pwr_command[2] = 0x00;   /* "Zero" value */
      if (sgc_sendcommand(3, pwr_command, 1) == 0) /* send command */
        {
        sgc_power_state.ist = 7; /* if send command successful, proceed to next state */
        break;
        }
       }
      sgc_power_state.ist = DISP_RESET; /* if NACK, unexpected result or timeout --> should never happen, go into reset */ 
      break;
     
     case 7:             /* STATE 7: Go into shutdown */  
       if ((sgc_uart_buffer.ack == NONE) || (sgc_uart_buffer.ack == SENDING)) /* no answer yet received, wait */
       break; 
      if (sgc_uart_buffer.ack == ACK) /* continue only if ACK was received */
       {
        pwr_command[0] = 0x59;   /* "Control" Command */
      pwr_command[1] = 0x03;   /* "Display OnOff" Command */
      pwr_command[2] = 0x00;   /* "Shutdown" Command */ 
      if (sgc_sendcommand(3, pwr_command, 1) == 0) /* send command */
        {
        sgc_power_state.ist = 8;  /* if send command successful, proceed to next state */
        break;
        }
       }
      sgc_power_state.ist = DISP_RESET; /* if NACK, unexpected result or timeout --> should never happen, go into reset */ 
      break;

    case 8:              /* STATE 8: wait for shutdown ACK */  
       if ((sgc_uart_buffer.ack == NONE) || (sgc_uart_buffer.ack == SENDING))  /* no answer yet received, wait */
       break; 
      if (sgc_uart_buffer.ack == ACK)  /* continue only if ACK was received */
       {
       sgc_power_state.ist = SHUTDOWN;
       sgc_uart_buffer.busy = 0;   /* release UART */
       break;
       }
      sgc_power_state.ist = DISP_RESET; /* if NACK, unexpected result or timeout --> should never happen, go into reset */ 
      break;

     case SHUTDOWN:          /* Display is shut down, power can safely be turned off. This state is only changed by command. */
     	if (sgc_uart_buffer.ack == TIMEOUT)	/* if any command should time out, display is out of sync */
     		sgc_power_state.ist = DISP_RESET;
      break;
      
     case BEGIN_POWERUP:        /* STATE 10: Start power-up sequence with display power-up command */
      sgc_uart_buffer.busy = 1;    /* block UART for command sequence */
       if ((sgc_uart_buffer.ack == NONE) || (sgc_uart_buffer.ack == SENDING))  /* wait for UART to finish sending if necessary */
       break;
      if (sgc_uart_buffer.ack <= NACK) 
       {               /* continue if ACK or NACK was received; previous unknown command was terminated in defined manner */
       pwr_command[0] = 0x59;    /* "Control" Command */
      pwr_command[1] = 0x03;    /* "Display OnOff" Command */
      pwr_command[2] = 0x01;    /* "Power Up" Command */ 
      if (sgc_sendcommand(3, pwr_command, 1) == 0) /* send command */
        {
        sgc_power_state.ist = 11;  /* if send command successful, proceed to next state */
        break;
       }
      }
      sgc_power_state.ist = DISP_RESET; /* if unexpected result or timeout of previous unknown command --> should never happen, go into reset */ 
      break;
     
     case 11:             /* STATE 11: Restore contrast value */  
       if ((sgc_uart_buffer.ack == NONE) || (sgc_uart_buffer.ack == SENDING))  /* no answer yet received, wait */
       break; 
      if (sgc_uart_buffer.ack == ACK)  /* continue only if ACK was received */
       {
       pwr_command[0] = 0x59;    /* "Control" Command */
      pwr_command[1] = 0x02;    /* "Contrast" Command */
      pwr_command[2] = sgc_power_state.contrast;  /* Restore Contrast */
      if (sgc_sendcommand(3, pwr_command, 1) == 0) /* send command */
        {
        sgc_power_state.ist = 12;  /* if send command successful, proceed to next state */
        break;
        }
       }
      sgc_power_state.ist = DISP_RESET; /* if NACK, unexpected result or timeout --> should never happen, go into reset */ 
      break;
     
     case 12:             /* STATE 12: Turn display on */  
       if ((sgc_uart_buffer.ack == NONE) || (sgc_uart_buffer.ack == SENDING))  /* no answer yet received, wait */
       break; 
      if (sgc_uart_buffer.ack == ACK)  /* continue only if ACK was received */
       {
        pwr_command[0] = 0x59;    /* "Control" Command */
      pwr_command[1] = 0x01;    /* "Display OnOff" Command */
      pwr_command[2] = 0x01;    /* "On" Command */
      if (sgc_sendcommand(3, pwr_command, 1) == 0) /* send command */
        {
        sgc_power_state.ist = 13;  /* if send command successful, proceed to next state */
        break;
        }
       }
      sgc_power_state.ist = DISP_RESET; /* if NACK, unexpected result or timeout --> should never happen, go into reset */ 
      break;

    case 13:              /* STATE 13: wait for power on ACK */  
       if ((sgc_uart_buffer.ack == NONE) || (sgc_uart_buffer.ack == SENDING))  /* no answer yet received, wait */
       break; 
      if (sgc_uart_buffer.ack == ACK)  /* continue only if ACK was received */
       {
       sgc_power_state.ist = POWERUP;
       sgc_uart_buffer.busy = 0;   /* release UART */
       break;
       }
      sgc_power_state.ist = DISP_RESET; /* if NACK, unexpected result or timeout --> should never happen, go into reset */ 
      break;

     case POWERUP:           /* Display is powered up, do not turn off power now. This state is only changed by command. */
     	if (sgc_uart_buffer.ack == TIMEOUT)	/* if any command should time out, display is out of sync */
     	sgc_power_state.ist = DISP_RESET;
      break;
      
     default: sgc_power_state.ist = DISP_RESET; /* Undefinded state, should be impossible, go into reset for recovery */
     }
}

ISR(usart(USART,_TX_vect))
{
 if (sgc_uart_buffer.pos < sgc_uart_buffer.len)  /* still bytes in buffer? */
 {
  usart(UDR) = sgc_uart_buffer.txdata[sgc_uart_buffer.pos]; /* send next byte */
  sgc_uart_buffer.pos++;           /* increment buffer pointer */
 } 
 else 	/* command finished */
 {
      usart(UCSR,B) &= ~(_BV(usart(TXCIE)));  /* Disable this interrupt */
      sgc_uart_buffer.pos=0;          /* clear for next transmission */
      sgc_uart_buffer.len=0;
      sgc_uart_buffer.ack = NONE;          /* reset ACK status and enable timeout counter */
      sgc_uart_buffer.acktimer = 0;         /* reset ACK timeout */
  }
}

ISR(usart(USART,_RX_vect))
{
uint8_t flags;
flags = usart(UCSR,A);
sgc_uart_buffer.rxdata = usart(UDR);   /* read input buffer */
if (sgc_uart_buffer.rxenable == 1)    /* only if reception is enabled */
 {
 	if ((flags & _BV(usart(DOR))) || (flags & _BV(usart(FE))) || ((sgc_uart_buffer.rxdata != 0x06) && (sgc_uart_buffer.rxdata != 0x15)) || (sgc_uart_buffer.ack != NONE)) /* Error or unexpected */
 		{
 		sgc_power_state.ist = DISP_RESET;	/* something must have gone very wrong, reset to resync */
 		return;
 		}
 if (sgc_uart_buffer.rxdata == 0x06)   /* ACK was received*/
 	{
  sgc_uart_buffer.ack = ACK;
  return;
	}
  sgc_uart_buffer.ack = NACK;		/* the only left possibility */
 }
}

/*
 -- Ethersex META --

 header(protocols/sgc/sgc.h)
 init(sgc_init)
 timer(1, sgc_pwr_periodic())
*/
