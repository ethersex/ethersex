/*
 * Copyright (c) 2013 by Nico Dziubek <hundertvolt@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
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
#include "config.h"
#include "sgc.h"

#ifdef SGC_ECMD_SEND_SUPPORT
#include "protocols/uip/uip.h"
#include "protocols/uip/parse.h"
#endif /* SGC_ECMD_SEND_SUPPORT */

#define USE_USART SGC_USE_USART
#define BAUD SGC_BAUDRATE
#include "core/usart.h"

generate_usart_init()
     struct sgc_buffer sgc_uart_buffer;
     struct sgc_state sgc_power_state;

#ifdef SGC_ECMD_SEND_SUPPORT
     uip_ipaddr_t ip;
#endif /* SGC_ECMD_SEND_SUPPORT */

     void sgc_init(void)
{
  PIN_SET(SGC_RESET);           /* hold display in reset */
  usart_init();                 /* Initialize the usart module */
  sgc_uart_buffer.rxenable = 0; /* ignore anything on the RX line */

#ifdef SGC_TIMEOUT_COUNTER_SUPPORT
  sgc_power_state.mincount = 0; /* any command resets the counter */
  sgc_power_state.timeout = 0;
  sgc_power_state.timer_max = SGC_TIMEOUT;
#endif /* SGC_TIMEOUT_COUNTER_SUPPORT */

#ifdef SGC_ECMD_SEND_SUPPORT
  set_SGC_ECMD_IP(&ip);
#endif /* SGC_ECMD_SEND_SUPPORT */
}

uint8_t
sgc_setpowerstate(uint8_t soll)
{
#ifdef SGC_TIMEOUT_COUNTER_SUPPORT
  sgc_power_state.mincount = 0; /* any command resets the counter */
  sgc_power_state.timeout = 0;
#endif /* SGC_TIMEOUT_COUNTER_SUPPORT */

  if ((sgc_power_state.ist != SHUTDOWN) && (sgc_power_state.ist != POWERUP))
    return 1;                   /* state machine busy */
  if (soll == 0)
  {
    sgc_power_state.ist = BEGIN_SHUTDOWN;
    return 0;
  }
  sgc_power_state.ist = BEGIN_POWERUP;
  return 0;
}

uint8_t
sgc_getpowerstate(void)
{
#ifdef SGC_TIMEOUT_COUNTER_SUPPORT
  sgc_power_state.mincount = 0; /* any command resets the counter */
  sgc_power_state.timeout = 0;
#endif /* SGC_TIMEOUT_COUNTER_SUPPORT */
  return sgc_power_state.ist;
}

#ifdef SGC_ECMD_SEND_SUPPORT
int8_t
sgc_setip(char *data)
{
#ifdef SGC_TIMEOUT_COUNTER_SUPPORT
  sgc_power_state.mincount = 0; /* any command resets the counter */
  sgc_power_state.timeout = 0;
#endif /* SGC_TIMEOUT_COUNTER_SUPPORT */
  return parse_ip(data, &ip);
}
#endif /* SGC_ECMD_SEND_SUPPORT */

#ifdef SGC_TIMEOUT_COUNTER_SUPPORT
void
sgc_settimeout(uint8_t time)
{
  sgc_power_state.mincount = 0; /* any command resets the counter */
  sgc_power_state.timeout = 0;
  sgc_power_state.timer_max = time;
}
#endif /* SGC_TIMEOUT_COUNTER_SUPPORT */

uint8_t
sgc_sendcommand(uint8_t cmdlen, char *data, char *option)
{
  uint8_t stringlen = cmdlen;
  if ((sgc_uart_buffer.pos != 0) || (((sgc_power_state.ist <= SHUTDOWN) ||
                                      (sgc_uart_buffer.busy == 1) ||
                                      (sgc_power_state.ack > NACK)) &&
                                     (option[0] != OPT_INTERNAL)))
    return 1;                   /* Waiting, TX busy, Shutdown, not yet init, reserved */

  sgc_power_state.ack_timeout = 25;     /* 500ms standard timeout for ACK */
  switch (option[0])
  {
    case OPT_NO_ACK:           /* 0 */
      sgc_power_state.ack_timeout = 0;  /* no timeout for ACK */
      goto NORMAL;

    case OPT_LONG_ACK:         /* 1 */
      sgc_power_state.ack_timeout = 200;        /* 4 sec long timeout for ACK */
      goto NORMAL;

    case OPT_STRING:           /* 2 */
      do
        stringlen++;            /* search for first non-font char or max length */
      while ((option[stringlen - cmdlen] >= 0x20) &&
             ((stringlen) < SGC_BUFFER_LENGTH) &&
             ((stringlen - cmdlen + data[cmdlen] + 7) <
              ECMD_INPUTBUF_LENGTH));
      option[stringlen - cmdlen] = 0x00;        /* add terminator character */
      memcpy(&sgc_uart_buffer.txdata[cmdlen], &option[1], stringlen - cmdlen);

    case OPT_NORMAL:           /* 3 */
    NORMAL:
      sgc_power_state.from_reset = 0;   /* no "break" intended, 400ms ACK */

    default:                   /* OPT_INTERNAL = 3 */
      break;
  }

#ifdef SGC_TIMEOUT_COUNTER_SUPPORT
  sgc_power_state.mincount = 0; /* any command resets the counter */
  sgc_power_state.timeout = 0;
#endif /* SGC_TIMEOUT_COUNTER_SUPPORT */
  memcpy(sgc_uart_buffer.txdata, data, cmdlen); /* copy send buffer */
  sgc_uart_buffer.len = stringlen;      /* copy command length */
  sgc_uart_buffer.pos = 1;      /* set TX buffer pointer */
  sgc_power_state.ack = SENDING;
  usart(UCSR, B) |= _BV(usart(TXCIE));  /* activate TX interrupt */
  usart(UDR) = sgc_uart_buffer.txdata[0];       /* transmit first byte */
  return 0;                     /* success */
}

uint8_t
sgc_getcommandresult(void)
{
#ifdef SGC_TIMEOUT_COUNTER_SUPPORT
  sgc_power_state.mincount = 0; /* any command resets the counter */
  sgc_power_state.timeout = 0;
#endif /* SGC_TIMEOUT_COUNTER_SUPPORT */
  if ((sgc_uart_buffer.busy == 1) || (sgc_power_state.ack > NONE))
    return BUSY;                /* not valid while in a command sequence or timeout */
  if (sgc_power_state.from_reset == 1)
    return FROM_RESET;
  return sgc_power_state.ack;   /* send last result */
}

uint8_t
sgc_setcontrast(char contrast)
{
  char option[1], command[3];
  option[0] = OPT_NORMAL;
#ifdef SGC_TIMEOUT_COUNTER_SUPPORT
  sgc_power_state.mincount = 0; /* any command resets the counter */
  sgc_power_state.timeout = 0;
#endif /* SGC_TIMEOUT_COUNTER_SUPPORT */
  if (sgc_power_state.ist == POWERUP)
  {
    command[0] = 0x59;          /* "Control" Command */
    command[1] = 0x02;          /* "Contrast" Command */
    command[2] = contrast;      /* Value */
    if (sgc_sendcommand(3, command, option) != 0)       /* busy with other command */
      return 1;
  }
  sgc_power_state.contrast = contrast;  /* save contrast for next powerup */
  return 0;                     /* success */
}

uint8_t
sgc_setpensize(char pensize)
{
  char option[1], command[2];
  option[0] = OPT_NORMAL;
#ifdef SGC_TIMEOUT_COUNTER_SUPPORT
  sgc_power_state.mincount = 0; /* any command resets the counter */
  sgc_power_state.timeout = 0;
#endif /* SGC_TIMEOUT_COUNTER_SUPPORT */
  if (sgc_power_state.ist == POWERUP)
  {
    command[0] = 0x70;          /* "Pensize" Command */
    command[1] = pensize;       /* Value */
    if (sgc_sendcommand(2, command, option) != 0)       /* busy with other command */
      return 1;
  }
  sgc_power_state.pensize = pensize;    /* save pensize for next powerup */
  return 0;                     /* success */
}

uint8_t
sgc_setfont(char *font)
{
  char option[1], command[2];
  option[0] = OPT_NORMAL;
#ifdef SGC_TIMEOUT_COUNTER_SUPPORT
  sgc_power_state.mincount = 0; /* any command resets the counter */
  sgc_power_state.timeout = 0;
#endif /* SGC_TIMEOUT_COUNTER_SUPPORT */
  if ((sgc_power_state.ist == POWERUP) &&
      (font[0] != sgc_power_state.font[0]))
  {
    command[0] = 0x46;          /* "Set Font" Command */
    command[1] = font[0] & 0x0F;        /* standard without proportional setting */
    if (sgc_sendcommand(2, command, option) != 0)       /* busy with other command */
      return 1;
  }
  memcpy(sgc_power_state.font, font, 3);        /* save font setting */
  return 0;                     /* success */
}

void
sgc_getfont(char *font)         /* internal command - no timeout counter reset! */
{
  memcpy(font, sgc_power_state.font, 3);
}

uint8_t
rgb2sgc(char *col, int8_t stop) /* 0: red; 1:green; 2:blue */
{
  uint8_t green6;
  if ((col[0] > 0x1F) || (col[1] > 0x1F) || (col[2] > 0x1F))
    return 1;                   /* out of range error or no conversion necessary */
  if (stop == 0)
  {
    green6 = (((col[1] & 0x1F) << 1) | (col[1] & 0x01));        /* 5bit to 6bit RGB */
    col[0] = (((green6 & 0x38) >> 3) | ((col[0] & 0x1F) << 3)); /* High Byte */
    col[1] = ((col[2] & 0x1F) | ((green6 & 0x07) << 5));        /* Low Byte */
  }
  return 0;                     /* success */
}

void
sgc_pwr_periodic(void)          /* runs with 20ms */
{
  char option[1], pwr_command[3];
  option[0] = OPT_INTERNAL;
  if (sgc_power_state.ack == NONE)
    sgc_power_state.acktimer++;
  if (sgc_power_state.acktimer > sgc_power_state.ack_timeout)   /* ACK Timer */
  {
    sgc_power_state.ack = TIMEOUT;
    sgc_power_state.acktimer = 0;
    sgc_uart_buffer.rxenable = 0;       /* disable reception */
  }

#ifdef SGC_TIMEOUT_COUNTER_SUPPORT
  if (sgc_power_state.ist == POWERUP)
  {
    if (++sgc_power_state.mincount == 3000)     /* 1 minute */
    {
      sgc_power_state.mincount = 0;
      if (++sgc_power_state.timeout >= sgc_power_state.timer_max)
      {
        sgc_power_state.ist = BEGIN_SHUTDOWN;   /* shutdown if timed out */
      }
    }
  }
#endif /* SGC_TIMEOUT_COUNTER_SUPPORT */

  switch (sgc_power_state.ist)
  {
    case DISP_RESET:           /* STATE 0: Begin Reset Sequence */
      sgc_uart_buffer.busy = 1; /* block UART for command sequence */
      sgc_uart_buffer.rxenable = 0;     /* ignore anything on the RX line */
      if (sgc_uart_buffer.pos != 0)     /* let UART finish if necessary */
        break;
      PIN_SET(SGC_RESET);       /* put display in reset */
      sgc_power_state.baudcounter = 0;  /* reset autobaud command counter */
      sgc_power_state.ack = ACK;        /* for the first time to enable sending */
      sgc_power_state.contrast = 0x0F;  /* default value */
      sgc_power_state.pensize = 0x00;   /* default value */
      sgc_power_state.font[0] = 0x00;   /* font default value */
      sgc_power_state.font[1] = 0xFF;   /* col0 default value */
      sgc_power_state.font[2] = 0xFF;   /* col1 default value */
      sgc_power_state.timer = 0;        /* start reset timer */
      sgc_power_state.ist = 1;  /* proceed to next state */
      sgc_power_state.from_reset = 1;
#ifdef SGC_ECMD_SEND_SUPPORT
      ecmd_sender_send_command(&ip, PSTR("RESET\n"), NULL);
#endif /* SGC_ECMD_SEND_SUPPORT */
      break;

    case 1:                    /* STATE 1: Wait for Reset to time out */
      sgc_power_state.timer++;  /* periodically count timer up */
      if (sgc_power_state.timer == 5)   /* hold reset for 100 ms */
      {
        PIN_CLEAR(SGC_RESET);   /* release reset */
        sgc_power_state.timer = 0;      /* clear for boot timer */
        sgc_power_state.ist = 2;        /* proceed to next state */
      }
      break;

    case 2:                    /* STATE 2: Wait for boot-up to time out */
      sgc_power_state.timer++;  /* periodically count timer up */
      if (sgc_power_state.timer == 100) /* 2sec for display bootup */
      {
        sgc_power_state.ist = 3;        /* proceed to next state */
      }
      break;

    case 3:                    /* STATE 3: Auto-baud display */
      if (sgc_power_state.baudcounter < 10)     /* try autobauding 10 times */
      {
        pwr_command[0] = 0x55;  /* put autobaud command into sending queue */
        sgc_uart_buffer.rxenable = 1;   /* enable the RX line */
        if (sgc_sendcommand(1, pwr_command, option) == 0)       /* send command */
        {
          sgc_power_state.ist = 4;      /* if send command successful, proceed */
          break;
        }
      }
      sgc_power_state.ist = DISP_RESET; /* autobaud not successful, reset */
      break;

    case 4:                    /* STATE 4: wait for answer to autobaud command */
      if ((sgc_power_state.ack == NONE) || (sgc_power_state.ack == SENDING))
        break;                  /* no answer or timeout yet, keep waiting */
      if (sgc_power_state.ack == ACK)
      {
        sgc_power_state.ist = BEGIN_SHUTDOWN;
        break;                  /* ACK received, proceed to next state - Shutdown after Init */
      }
      if (sgc_power_state.ack == TIMEOUT)       /* Timeout, try again */
      {
        sgc_power_state.baudcounter++;  /* increment autobaud counter */
        sgc_power_state.ist = 3;        /* go back to State 3 - Send Autobaud */
        break;
      }
      sgc_power_state.ist = DISP_RESET;
      break;                    /* if NACK or any unexpected response, restart reset sequence */

    case BEGIN_SHUTDOWN:       /* STATE 5: Start shutdown sequence */
      sgc_uart_buffer.busy = 1; /* block UART for command sequence */
      if ((sgc_power_state.ack == NONE) || (sgc_power_state.ack == SENDING))
        break;                  /* wait for last command to finish if necessary */
      if (sgc_power_state.ack <= NACK)
      {                         /* continue if ACK or NACK was received (defined display state) */
        pwr_command[0] = 0x59;  /* "Control" Command */
        pwr_command[1] = 0x01;  /* "Display OnOff" Command */
        pwr_command[2] = 0x00;  /* "Off" Command */
        if (sgc_sendcommand(3, pwr_command, option) == 0)       /* send command */
        {
          sgc_power_state.ist = 6;
          break;                /* if send command successful, proceed to next state */
        }
      }
      sgc_power_state.ist = DISP_RESET;
      break;                    /* previous unknown command failed */

    case 6:                    /* STATE 6: Set contrast to zero */
      if ((sgc_power_state.ack == NONE) || (sgc_power_state.ack == SENDING))
        break;                  /* no answer yet received, wait */
      if (sgc_power_state.ack == ACK)   /* continue only if ACK was received */
      {
        pwr_command[0] = 0x59;  /* "Control" Command */
        pwr_command[1] = 0x02;  /* "Contrast" Command */
        pwr_command[2] = 0x00;  /* "Zero" value */
        if (sgc_sendcommand(3, pwr_command, option) == 0)       /* send command */
        {
          sgc_power_state.ist = 7;
          break;                /* if send command successful, proceed to next state */
        }
      }
      sgc_power_state.ist = DISP_RESET;
      break;                    /* if NACK, unexpected result or timeout --> reset */

    case 7:                    /* STATE 7: Go into shutdown */
      if ((sgc_power_state.ack == NONE) || (sgc_power_state.ack == SENDING))
        break;                  /* no answer yet received, wait */
      if (sgc_power_state.ack == ACK)   /* continue only if ACK was received */
      {
        pwr_command[0] = 0x59;  /* "Control" Command */
        pwr_command[1] = 0x03;  /* "Display OnOff" Command */
        pwr_command[2] = 0x00;  /* "Shutdown" Command */
        if (sgc_sendcommand(3, pwr_command, option) == 0)       /* send command */
        {
          sgc_power_state.ist = 8;
          break;                /* if send command successful, proceed to next state */
        }
      }
      sgc_power_state.ist = DISP_RESET;
      break;                    /* if NACK, unexpected result or timeout --> reset */

    case 8:                    /* STATE 8: wait for shutdown ACK */
      if ((sgc_power_state.ack == NONE) || (sgc_power_state.ack == SENDING))
        break;                  /* no answer yet received, wait */
      if (sgc_power_state.ack == ACK)   /* continue only if ACK was received */
      {
        sgc_power_state.ist = SHUTDOWN;
        sgc_uart_buffer.busy = 0;       /* release UART */
#ifdef SGC_ECMD_SEND_SUPPORT
        ecmd_sender_send_command(&ip, PSTR("SHUTDOWN\n"), NULL);
#endif /* SGC_ECMD_SEND_SUPPORT */
        break;
      }
      sgc_power_state.ist = DISP_RESET;
      break;                    /* if NACK, unexpected result or timeout --> reset */

    case SHUTDOWN:             /* Display is shut down, power can be turned off. */
      if (sgc_power_state.ack == TIMEOUT)       /* any out of sync --> reset */
        sgc_power_state.ist = DISP_RESET;
      break;

    case BEGIN_POWERUP:        /* STATE 10: Start power-up sequence */
      sgc_uart_buffer.busy = 1; /* block UART for command sequence */
      if ((sgc_power_state.ack == NONE) || (sgc_power_state.ack == SENDING))
        break;                  /* wait for UART to finish sending if necessary */
      if (sgc_power_state.ack <= NACK)
      {                         /* continue if ACK or NACK was received */
        pwr_command[0] = 0x59;  /* "Control" Command */
        pwr_command[1] = 0x03;  /* "Display OnOff" Command */
        pwr_command[2] = 0x01;  /* "Power Up" Command */
        if (sgc_sendcommand(3, pwr_command, option) == 0)       /* send command */
        {
          sgc_power_state.ist = 11;     /* if send command successful, proceed */
          break;
        }
      }
      sgc_power_state.ist = DISP_RESET;
      break;                    /* if unexpected result or timeout --> reset */

    case 11:                   /* STATE 11: Restore contrast value */
      if ((sgc_power_state.ack == NONE) || (sgc_power_state.ack == SENDING))
        break;                  /* no answer yet received, wait */
      if (sgc_power_state.ack == ACK)   /* continue only if ACK was received */
      {
        pwr_command[0] = 0x59;  /* "Control" Command */
        pwr_command[1] = 0x02;  /* "Contrast" Command */
        pwr_command[2] = sgc_power_state.contrast;      /* Restore Contrast */
        if (sgc_sendcommand(3, pwr_command, option) == 0)       /* send command */
        {
          sgc_power_state.ist = 12;
          break;                /* if send command successful, proceed to next state */
        }
      }
      sgc_power_state.ist = DISP_RESET;
      break;                    /* if NACK, unexpected result or timeout --> reset */

    case 12:                   /* STATE 12: Turn display on */
      if ((sgc_power_state.ack == NONE) || (sgc_power_state.ack == SENDING))
        break;                  /* no answer yet received, wait */
      if (sgc_power_state.ack == ACK)   /* continue only if ACK was received */
      {
        pwr_command[0] = 0x59;  /* "Control" Command */
        pwr_command[1] = 0x01;  /* "Display OnOff" Command */
        pwr_command[2] = 0x01;  /* "On" Command */
        if (sgc_sendcommand(3, pwr_command, option) == 0)       /* send command */
        {
          sgc_power_state.ist = 13;
          break;                /* if send command successful, proceed to next state */
        }
      }
      sgc_power_state.ist = DISP_RESET;
      break;                    /* if NACK, unexpected result or timeout --> reset */

    case 13:                   /* STATE 13: Restore Font setting */
      if ((sgc_power_state.ack == NONE) || (sgc_power_state.ack == SENDING))
        break;                  /* no answer yet received, wait */
      if (sgc_power_state.ack == ACK)   /* continue only if ACK was received */
      {
        pwr_command[0] = 0x46;  /* "Set Font" Command */
        pwr_command[1] = sgc_power_state.font[0] & 0x0F;        /* standard without proportional setting */
        if (sgc_sendcommand(2, pwr_command, option) == 0)       /* send command */
        {
          sgc_power_state.ist = 14;
          break;                /* if send command successful, proceed to next state */
        }
      }
      sgc_power_state.ist = DISP_RESET;
      break;                    /* if NACK, unexpected result or timeout --> reset */

    case 14:                   /* STATE 14: Restore Pensize setting */
      if ((sgc_power_state.ack == NONE) || (sgc_power_state.ack == SENDING))
        break;                  /* no answer yet received, wait */
      if (sgc_power_state.ack == ACK)   /* continue only if ACK was received */
      {
        pwr_command[0] = 0x70;  /* "Pensize" Command */
        pwr_command[1] = sgc_power_state.pensize;       /* standard without proportional setting */
        if (sgc_sendcommand(2, pwr_command, option) == 0)       /* send command */
        {
          sgc_power_state.ist = 15;
          break;                /* if send command successful, proceed to next state */
        }
      }
      sgc_power_state.ist = DISP_RESET;
      break;                    /* if NACK, unexpected result or timeout --> reset */

    case 15:                   /* STATE 15: wait for last ACK */
      if ((sgc_power_state.ack == NONE) || (sgc_power_state.ack == SENDING))
        break;                  /* no answer yet received, wait */
      if (sgc_power_state.ack == ACK)   /* continue only if ACK was received */
      {
        sgc_power_state.ist = POWERUP;
        sgc_uart_buffer.busy = 0;       /* release UART */
#ifdef SGC_ECMD_SEND_SUPPORT
        ecmd_sender_send_command(&ip, PSTR("POWERUP\n"), NULL);
#endif /* SGC_ECMD_SEND_SUPPORT */
        break;
      }
      sgc_power_state.ist = DISP_RESET;
      break;                    /* if NACK, unexpected result or timeout --> reset */

    case POWERUP:              /* Display is powered up, do not turn off power now. */
      if (sgc_power_state.ack == TIMEOUT)       /* out of sync --> reset */
        sgc_power_state.ist = DISP_RESET;
      break;

    default:
      sgc_power_state.ist = DISP_RESET;
  }                             /* Undefinded state, go into reset for recovery */
}

ISR(usart(USART, _TX_vect))
{
  if (sgc_uart_buffer.pos < sgc_uart_buffer.len)        /* still bytes in buffer? */
  {
    usart(UDR) = sgc_uart_buffer.txdata[sgc_uart_buffer.pos];   /* send next */
    sgc_uart_buffer.pos++;      /* increment buffer pointer */
  }
  else                          /* command finished */
  {
    usart(UCSR, B) &= ~(_BV(usart(TXCIE)));     /* Disable this interrupt */
    sgc_uart_buffer.pos = 0;    /* clear for next transmission */
    sgc_uart_buffer.len = 0;
    sgc_power_state.acktimer = 0;       /* reset ACK timeout */
    if (sgc_power_state.ack_timeout == 0)
      sgc_power_state.ack = ACK;        /* if nominally no ack expected */
    else
      sgc_power_state.ack = NONE;       /* set status, enable timeout counter */
  }
}

ISR(usart(USART, _RX_vect))
{
  uint8_t flags;
  char rxdata;
  flags = usart(UCSR, A);
  rxdata = usart(UDR);          /* read input buffer */
  if (sgc_uart_buffer.rxenable == 1)    /* only if reception is enabled */
  {
    if ((flags & _BV(usart(DOR))) || (flags & _BV(usart(FE))) || ((rxdata != 0x06) && (rxdata != 0x15)) || (sgc_power_state.ack != NONE))       /* Error or unexpected */
    {
      sgc_power_state.ist = DISP_RESET;
      return;                   /* something must have gone very wrong, reset to resync */
    }
    if (rxdata == 0x06)         /* ACK was received */
    {
#ifdef SGC_ECMD_SEND_SUPPORT
      if ((sgc_power_state.ist == SHUTDOWN) ||
          (sgc_power_state.ist == POWERUP))
        ecmd_sender_send_command(&ip, PSTR("ACK\n"), NULL);
#endif /* SGC_ECMD_SEND_SUPPORT */
      sgc_power_state.ack = ACK;
      return;
    }
#ifdef SGC_ECMD_SEND_SUPPORT
    if ((sgc_power_state.ist == SHUTDOWN) || (sgc_power_state.ist == POWERUP))
      ecmd_sender_send_command(&ip, PSTR("NACK\n"), NULL);
#endif /* SGC_ECMD_SEND_SUPPORT */
    sgc_power_state.ack = NACK; /* the only left possibility */
  }
}

/*
 -- Ethersex META --
 header(protocols/sgc/sgc.h)
 init(sgc_init)
 timer(1, sgc_pwr_periodic())
*/
