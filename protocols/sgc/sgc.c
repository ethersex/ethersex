/*
 * Copyright (c) 2013 by Nico Dziubek <hundertvolt@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
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
#include "protocols/ecmd/sender/ecmd_sender_net.h"
#endif /* SGC_ECMD_SEND_SUPPORT */

/* define bitstates */
#define SH_SLEEP 0              /* go to sleep mode */
#define SH_SLEEPING 1           /* in sleep mode */
#define SH_F_RESET 2            /* coming from reset */
#define SH_TXBUSY 3             /* reserve TX for internal command */
#define SH_RXENABLE 4           /* enable RX after reset */
#define SH_SHDN_SLEEP 5         /* enable auto-sleep in shutdown */
#define SH_SHDN_SLEEPMODE (SH_SHDN_SLEEP + 1)   /* auto-sleep mode */
#define SLEEP (0x01 << SH_SLEEP)
#define SLEEPING (0x01 << SH_SLEEPING)
#define F_RESET (0x01 << SH_F_RESET)
#define TXBUSY (0x01 << SH_TXBUSY)
#define RXENABLE (0x01 << SH_RXENABLE)
#define SHDN_SLEEP (0x01 << SH_SHDN_SLEEP)
#define SHDN_SLEEPMODE (0x01 << SH_SHDN_SLEEPMODE)

/* UART defines */
#define USE_USART SGC_USE_USART
#define BAUD SGC_BAUDRATE
#include "core/usart.h"

generate_usart_init()
     struct sgc_buffer sgc_uart_buffer;
     struct sgc_state sgc_power_state;
     struct sgc_vars sgc_disp_vars;

#ifdef SGC_ECMD_SEND_SUPPORT
     uip_ipaddr_t ip;
#endif /* SGC_ECMD_SEND_SUPPORT */

     uint8_t bitstates;

     void sgc_init(void)
{
  PIN_SET(SGC_RESET);           /* hold display in reset */
  usart_init();                 /* Initialize the usart module */

#ifdef SGC_TIMEOUT_COUNTER_SUPPORT
  sgc_power_state.mincount = 0; /* any command resets the counter */
  sgc_power_state.timeout = 0;
  sgc_power_state.timer_max = SGC_CONN_TIMEOUT;
#endif /* SGC_TIMEOUT_COUNTER_SUPPORT */

#ifdef SGC_ECMD_SEND_SUPPORT
  set_SGC_ECMD_IP(&ip);
#endif /* SGC_ECMD_SEND_SUPPORT */

#ifdef SGC_SHDN_SLEEP_ACTIVE
  bitstates |= SHDN_SLEEP;      /* set shutdown sleep bit */
#endif /* SGC_SHDN_SLEEP_ACTIVE */
  bitstates |= (SHDN_SLEEPMODE << SH_SHDN_SLEEPMODE);   /* set sleepmode bit */
}

uint8_t
sgc_setpowerstate(uint8_t soll)
{
#ifdef SGC_TIMEOUT_COUNTER_SUPPORT
  sgc_power_state.mincount = 0; /* any command resets the counter */
  sgc_power_state.timeout = 0;
#endif /* SGC_TIMEOUT_COUNTER_SUPPORT */

  if ((sgc_power_state.ist != SGC_SHUTDOWN) && (sgc_power_state.ist != SGC_POWERUP))
    return 1;                   /* state machine busy */
  bitstates &= ~F_RESET;

  if (soll == 0)
  {
    if (sgc_power_state.ist == SGC_POWERUP) /* not already in shutdown? */
      sgc_power_state.ist = SGC_BEGIN_SHUTDOWN;
    return 0;
  }

  if (sgc_power_state.ist == SGC_SHUTDOWN)  /* not already in powerup? */
    sgc_power_state.ist = SGC_BEGIN_POWERUP;
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

void                            /* change auto shudtown sleep and default sleepmode */
sgc_setshdnsleep(char sleepmode)
{
#ifdef SGC_TIMEOUT_COUNTER_SUPPORT
  sgc_power_state.mincount = 0; /* any command resets the counter */
  sgc_power_state.timeout = 0;
#endif /* SGC_TIMEOUT_COUNTER_SUPPORT */

  bitstates |= (sleepmode << SH_SHDN_SLEEP);
  bitstates &= (~SHDN_SLEEP | (sleepmode << SH_SHDN_SLEEP));
}                               /* set / reset both bits at a time */

#ifdef SGC_ECMD_SEND_SUPPORT
int8_t                          /* change default answer IP */
sgc_setip(char *data)
{
#ifdef SGC_TIMEOUT_COUNTER_SUPPORT
  sgc_power_state.mincount = 0; /* any command resets the counter */
  sgc_power_state.timeout = 0;
#endif /* SGC_TIMEOUT_COUNTER_SUPPORT */

  return parse_ip(data, &ip);   /* convert IP address */
}
#endif /* SGC_ECMD_SEND_SUPPORT */

#ifdef SGC_TIMEOUT_COUNTER_SUPPORT
void                            /* change default inactivity shutdown time */
sgc_settimeout(uint8_t time)
{
  sgc_power_state.mincount = 0; /* any command resets the counter */
  sgc_power_state.timeout = 0;
  sgc_power_state.timer_max = time;
}
#endif /* SGC_TIMEOUT_COUNTER_SUPPORT */

uint8_t                         /* set processor into sleep mode */
sgc_sleep(char mode, uint8_t option)
{
  char command[3];

#ifdef SGC_TIMEOUT_COUNTER_SUPPORT
  sgc_power_state.mincount = 0; /* any command resets the counter */
  sgc_power_state.timeout = 0;
#endif /* SGC_TIMEOUT_COUNTER_SUPPORT */

  command[0] = 0x5A;            /* "Sleep" Command */
  command[1] = mode + 1;        /* set sleep mode (1 Ser. 2 Joyst.) */
  command[2] = 0x00;            /* unused delay */

  if (sgc_sendcommand(3, SGC_INFINITE, option, command, command) != 0)
    return 1;                   /* inf. timeout - sleep ACKs when exiting sleep mode! */

  bitstates |= SLEEP;           /* set status bit */
  return 0;                     /* success */
}

uint8_t                         /* length of command, timeout, options, command data and additional data */
sgc_sendcommand(uint8_t cmdlen, uint16_t timeout, uint8_t option, char *data,
                char *cmdline)
{
  uint8_t stringlen = cmdlen;

#ifdef SGC_TIMEOUT_COUNTER_SUPPORT
  sgc_power_state.mincount = 0; /* any command resets the counter */
  sgc_power_state.timeout = 0;
#endif /* SGC_TIMEOUT_COUNTER_SUPPORT */

  if ((sgc_uart_buffer.pos != 0) || (((sgc_power_state.ist <= SGC_SHUTDOWN) ||
                                      (bitstates & TXBUSY) ||
                                      (sgc_power_state.ack > SGC_NACK)) &&
                                     (~option & SGC_OPT_INTERNAL)))
    return 1;                   /* Waiting, TX busy, Shutdown, no answer to last command, not internal command */

  if (option & SGC_OPT_STRING)      /* send string in text mode */
  {
    do
      stringlen++;              /* search for first non-font char or max length */
    while ((cmdline[stringlen - 1 - cmdlen] >= 0x20) &&
           (stringlen < SGC_BUFFER_LENGTH) &&
           ((stringlen - cmdlen + data[cmdlen] + 7) < ECMD_INPUTBUF_LENGTH));
    /* 7 = length of the ECMD itself */
    cmdline[stringlen - 1 - cmdlen] = 0x00;     /* add terminator character */
    memcpy(&sgc_uart_buffer.txdata[cmdlen], &cmdline[0], stringlen - cmdlen);
  }                             /* copy to send buffer, leave space for the command itself */
  bitstates &= (~F_RESET | (((option & SGC_OPT_INTERNAL) >> SGC_SH_INTERNAL) << SH_F_RESET));   /* SGC_FROM_RESET */
  memcpy(sgc_uart_buffer.txdata, data, cmdlen); /* copy send buffer */
  sgc_uart_buffer.len = stringlen;      /* copy command length */
  sgc_power_state.ack = SGC_SENDING;
  usart(UCSR, B) |= _BV(usart(TXCIE));  /* activate TX interrupt */

  if (bitstates & SLEEPING)     /* first: wake up processor */
  {
    sgc_power_state.acktimer = timeout; /* reset ACK including 5sec wakeup */
    sgc_power_state.ack_timeout = timeout + SGC_FIVE_SEC;   /* wakeup timeout 5sec combined */
    sgc_uart_buffer.pos = 0;    /* set TX buffer pointer */
    usart(UDR) = 0x55;          /* harmless message for wakeup (Autobaud) */
  }
  else
  {
    sgc_power_state.acktimer = 0;       /* reset ACK timeout */
    sgc_power_state.ack_timeout = timeout;      /* set timeout (20ms steps) */
    sgc_uart_buffer.pos = 1;    /* set TX buffer pointer */
    usart(UDR) = sgc_uart_buffer.txdata[0];     /* transmit first byte */
  }
  return 0;                     /* success */
}

uint8_t                         /* poll result of last command */
sgc_getcommandresult(void)
{
#ifdef SGC_TIMEOUT_COUNTER_SUPPORT
  sgc_power_state.mincount = 0; /* any command resets the counter */
  sgc_power_state.timeout = 0;
#endif /* SGC_TIMEOUT_COUNTER_SUPPORT */

  if ((bitstates & TXBUSY) || (sgc_power_state.ack > SGC_NONE))
    return SGC_BUSY;                /* not valid while in a command sequence or timeout */

  if (bitstates & F_RESET)
    return SGC_FROM_RESET;
  return sgc_power_state.ack;   /* send last result */
}

uint8_t                         /* set and save contrast value */
sgc_setcontrast(char contrast)
{
  char command[3];

#ifdef SGC_TIMEOUT_COUNTER_SUPPORT
  sgc_power_state.mincount = 0; /* any command resets the counter */
  sgc_power_state.timeout = 0;
#endif /* SGC_TIMEOUT_COUNTER_SUPPORT */

  if (sgc_power_state.ist == SGC_POWERUP)
  {
    command[0] = 0x59;          /* "Control" Command */
    command[1] = 0x02;          /* "Contrast" Command */
    command[2] = contrast;      /* Value */

    if (sgc_sendcommand(3, SGC_ONE_SEC, SGC_OPT_NORMAL, command, command) != 0)
      return 1;                 /* busy with other command */
  }

#ifdef SGC_ECMD_SEND_SUPPORT
  else                          /* also send ACK if no changes are necessary */
    ecmd_sender_send_command_P(&ip, NULL, PSTR("ACK\n"));
#endif /* SGC_ECMD_SEND_SUPPORT */

  sgc_disp_vars.contrast = contrast;    /* save contrast for next powerup */
  return 0;                     /* success */
}

uint8_t                         /* set and save pensize value */
sgc_setpensize(char pensize)
{
  char command[2];

#ifdef SGC_TIMEOUT_COUNTER_SUPPORT
  sgc_power_state.mincount = 0; /* any command resets the counter */
  sgc_power_state.timeout = 0;
#endif /* SGC_TIMEOUT_COUNTER_SUPPORT */

  if (sgc_power_state.ist == SGC_POWERUP)
  {
    command[0] = 0x70;          /* "Pensize" Command */
    command[1] = pensize;       /* Value */

    if (sgc_sendcommand(2, SGC_ONE_SEC, SGC_OPT_NORMAL, command, command) != 0)
      return 1;                 /* busy with other command */
  }

#ifdef SGC_ECMD_SEND_SUPPORT
  else                          /* also send ACK if no changes are necessary */
    ecmd_sender_send_command_P(&ip, NULL, PSTR("ACK\n"));
#endif /* SGC_ECMD_SEND_SUPPORT */

  sgc_disp_vars.pensize = pensize;      /* save pensize for next powerup */
  return 0;                     /* success */
}

uint8_t                         /* set and save font and prop. settings */
sgc_setfont(char font)
{
  char command[2];

#ifdef SGC_TIMEOUT_COUNTER_SUPPORT
  sgc_power_state.mincount = 0; /* any command resets the counter */
  sgc_power_state.timeout = 0;
#endif /* SGC_TIMEOUT_COUNTER_SUPPORT */

  if ((sgc_power_state.ist == SGC_POWERUP) && (font != sgc_disp_vars.font))
  {
    command[0] = 0x46;          /* "Set Font" Command */
    command[1] = font & 0x0F;   /* standard without proportional setting */

    if (sgc_sendcommand(2, SGC_ONE_SEC, SGC_OPT_NORMAL, command, command) != 0)
      return 1;                 /* busy with other command */
  }

#ifdef SGC_ECMD_SEND_SUPPORT
  else                          /* also send ACK if no changes are necessary */
    ecmd_sender_send_command_P(&ip, NULL, PSTR("ACK\n"));
#endif /* SGC_ECMD_SEND_SUPPORT */

  sgc_disp_vars.font = font;    /* save font setting */
  return 0;                     /* success */
}

char
sgc_getfont(void)               /* internal command - no timeout counter reset! */
{
  return sgc_disp_vars.font;
}

void                            /* save colour settings */
sgc_setcolour(char *colour)
{
#ifdef SGC_TIMEOUT_COUNTER_SUPPORT
  sgc_power_state.mincount = 0; /* any command resets the counter */
  sgc_power_state.timeout = 0;
#endif /* SGC_TIMEOUT_COUNTER_SUPPORT */

#ifdef SGC_ECMD_SEND_SUPPORT
  ecmd_sender_send_command_P(&ip, NULL, PSTR("ACK\n"));
#endif /* SGC_ECMD_SEND_SUPPORT */

  sgc_disp_vars.colour[0] = colour[0];  /* save colour setting */
  sgc_disp_vars.colour[1] = colour[1];  /* save colour setting */
}

uint8_t                         /* set and save opacity settings */
sgc_setopacity(char opacity)
{
  char command[2];

#ifdef SGC_TIMEOUT_COUNTER_SUPPORT
  sgc_power_state.mincount = 0; /* any command resets the counter */
  sgc_power_state.timeout = 0;
#endif /* SGC_TIMEOUT_COUNTER_SUPPORT */

  if (sgc_power_state.ist == SGC_POWERUP)
  {
    command[0] = 0x4F;          /* "Opacity" Command */
    command[1] = opacity;       /* Value */

    if (sgc_sendcommand(2, SGC_ONE_SEC, SGC_OPT_NORMAL, command, command) != 0)
      return 1;                 /* busy with other command */
  }

#ifdef SGC_ECMD_SEND_SUPPORT
  else                          /* also send ACK if no changes are necessary */
    ecmd_sender_send_command_P(&ip, NULL, PSTR("ACK\n"));
#endif /* SGC_ECMD_SEND_SUPPORT */

  sgc_disp_vars.opacity = opacity;      /* save pensize for next powerup */
  return 0;                     /* success */
}

void                            /* internal function */
sgc_getcolour(char *colour)
{
  colour[0] = sgc_disp_vars.colour[0];  /* copy colour setting */
  colour[1] = sgc_disp_vars.colour[1];  /* copy colour setting */
}

uint8_t                         /* convert 3byte RGB 5:5:5 to 2Byte RGB 5:6:5 */
rgb2sgc(char *col, int8_t stop) /* 0: red; 1:green; 2:blue */
{
  uint8_t green6;

  if ((col[0] > 0x1F) || (col[1] > 0x1F) || (col[2] > 0x1F))
    return 1;                   /* out of range error */

  if (stop == 0)                /* leave colour unchanged even if function is called */
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
  char pcmd[3];

  if ((sgc_power_state.ack == SGC_NONE) || (sgc_power_state.ack == SGC_WAKEUP))
  {                             /* timeout counter when waiting for command or wakeup response */
    sgc_power_state.acktimer++; /* ACK timeout counter */
    if (sgc_power_state.acktimer > sgc_power_state.ack_timeout)
    {
      sgc_power_state.acktimer = 0;
      bitstates &= ~RXENABLE;   /* disable reception */

      if (sgc_power_state.ack == SGC_WAKEUP)        /* wakeup timed out */
        sgc_power_state.ist = SGC_DISP_RESET;
      else                      /* command timed out */
        sgc_power_state.ack = SGC_TIMEOUT;
    }
  }
#ifdef SGC_TIMEOUT_COUNTER_SUPPORT
  if (sgc_power_state.ist == SGC_POWERUP)   /* counter for no-connection auto shutdown */
  {
    if (++sgc_power_state.mincount == 3000)     /* 1 minute steps */
    {
      sgc_power_state.mincount = 0;

      if (++sgc_power_state.timeout >= sgc_power_state.timer_max)
        sgc_power_state.ist = SGC_BEGIN_SHUTDOWN;   /* shutdown if timed out */
    }
  }
#endif /* SGC_TIMEOUT_COUNTER_SUPPORT */

  switch (sgc_power_state.ist)
  {
    case SGC_DISP_RESET:           /* STATE 0: Begin Reset Sequence */
      if (sgc_uart_buffer.pos != 0)     /* let UART finish if necessary */
        break;                  /* attention, this is combined with TX pointer settings with sleep mode */
      PIN_SET(SGC_RESET);       /* put display in reset */
      sgc_power_state.counter = 0;      /* set reset timer */
      sgc_power_state.ack = SGC_ACK;    /* for the first time to enable sending */
      sgc_power_state.ist = 1;  /* proceed to next state */
      sgc_disp_vars.contrast = 0x0F;    /* default value */
      sgc_disp_vars.pensize = 0x00;     /* default value */
      sgc_disp_vars.font = 0x00;        /* font default value */
      sgc_disp_vars.opacity = 0x00;     /* opacity default value */
      sgc_disp_vars.colour[0] = 0xFF;   /* col0 default value */
      sgc_disp_vars.colour[1] = 0xFF;   /* col1 default value */
      bitstates &= ~(SLEEP | SLEEPING | RXENABLE);      /* not sleeping and ignore RX line, ACK normal */
      bitstates |= (F_RESET | TXBUSY);  /* Rrom Reset and block UART for sequence */
      break;

    case 1:                    /* STATE 1: Wait for Reset to time out */
      sgc_power_state.counter++;        /* periodically count timer up */
      if (sgc_power_state.counter == 5) /* hold reset for 100 ms */
      {
        PIN_CLEAR(SGC_RESET);   /* release reset */
        sgc_power_state.counter = 0;    /* clear for boot timer */
        sgc_power_state.ist = 2;        /* proceed to next state */
      }
      break;

    case 2:                    /* STATE 2: Wait for boot-up to time out */
      sgc_power_state.counter++;        /* periodically count timer up */
      if (sgc_power_state.counter == 100)       /* 2sec for display bootup */
      {
        sgc_power_state.counter = 0;    /* reset counter */
        sgc_power_state.ist = 3;        /* proceed to next state */
      }
      break;

    case 3:                    /* STATE 3: Auto-baud display */
      if (sgc_power_state.counter < 10) /* try autobauding 10 times */
      {
        pcmd[0] = 0x55;         /* put autobaud command into sending queue */
        bitstates |= RXENABLE;  /* enable the RX line */
        if (sgc_sendcommand(1, SGC_ONE_SEC, SGC_OPT_INTERNAL, pcmd, pcmd) == 0) /* busy with other command */
        {
          sgc_power_state.ist = 4;      /* if send command successful, proceed */
          break;
        }
      }
      sgc_power_state.ist = SGC_DISP_RESET; /* autobaud not successful, reset */
      break;

    case 4:                    /* STATE 4: wait for answer to autobaud command */
      if ((sgc_power_state.ack >= SGC_SENDING) && (sgc_power_state.ack <= SGC_NONE))
        break;                  /* no answer or timeout yet, keep waiting */
      if (sgc_power_state.ack == SGC_ACK)
      {
        sgc_power_state.ist = SGC_BEGIN_SHUTDOWN;
        break;                  /* ACK received, proceed to next state - Shutdown after Init */
      }
      if (sgc_power_state.ack == SGC_TIMEOUT)       /* Timeout, try again */
      {
        sgc_power_state.counter++;      /* increment autobaud counter */
        sgc_power_state.ist = 3;        /* go back to State 3 - Send Autobaud */
        break;
      }
      sgc_power_state.ist = SGC_DISP_RESET;
      break;                    /* if any unexpected response, restart reset sequence */

    case SGC_BEGIN_SHUTDOWN:       /* STATE 5: Start shutdown sequence */
      bitstates |= TXBUSY;      /* block UART for command sequence */
      if ((sgc_power_state.ack >= SGC_SENDING) && (sgc_power_state.ack <= SGC_NONE))
        break;                  /* wait for last command to finish if necessary */
      if (sgc_power_state.ack <= SGC_NACK)
      {                         /* continue if last command was finished "clean" */
        pcmd[0] = 0x59;         /* "Control" Command */
        pcmd[1] = 0x01;         /* "Display OnOff" Command */
        pcmd[2] = 0x00;         /* "Off" Command */
        if (sgc_sendcommand(3, SGC_ONE_SEC, SGC_OPT_INTERNAL, pcmd, pcmd) == 0) /* busy with other command */
        {
          sgc_power_state.ist = 6;
          break;                /* if send command successful, proceed to next state */
        }
      }
      sgc_power_state.ist = SGC_DISP_RESET;
      break;                    /* previous unknown command failed */

    case 6:                    /* STATE 6: Clear screen for later no-flicker turn-on */
      if ((sgc_power_state.ack >= SGC_SENDING) && (sgc_power_state.ack <= SGC_NONE))
        break;                  /* no answer yet received, wait */
      if (sgc_power_state.ack == SGC_ACK)   /* continue only if ACK was received */
      {
        pcmd[0] = 0x45;         /* "Clear Screen" Command */
        if (sgc_sendcommand(1, SGC_ONE_SEC, SGC_OPT_INTERNAL, pcmd, pcmd) == 0) /* busy with other command */
        {
          sgc_power_state.ist = 7;
          break;                /* if send command successful, proceed to next state */
        }
      }
      sgc_power_state.ist = SGC_DISP_RESET;
      break;                    /* if unexpected result or timeout --> reset */

    case 7:                    /* STATE 7: Set contrast to zero */
      if ((sgc_power_state.ack >= SGC_SENDING) && (sgc_power_state.ack <= SGC_NONE))
        break;                  /* no answer yet received, wait */
      if (sgc_power_state.ack == SGC_ACK)   /* continue only if ACK was received */
      {
        pcmd[0] = 0x59;         /* "Control" Command */
        pcmd[1] = 0x02;         /* "Contrast" Command */
        pcmd[2] = 0x00;         /* "Zero" value */
        if (sgc_sendcommand(3, SGC_ONE_SEC, SGC_OPT_INTERNAL, pcmd, pcmd) == 0) /* busy with other command */
        {
          sgc_power_state.ist = 8;
          break;                /* if send command successful, proceed to next state */
        }
      }
      sgc_power_state.ist = SGC_DISP_RESET;
      break;                    /* if unexpected result or timeout --> reset */

    case 8:                    /* STATE 8: Go into shutdown */
      if ((sgc_power_state.ack >= SGC_SENDING) && (sgc_power_state.ack <= SGC_NONE))
        break;                  /* no answer yet received, wait */
      if (sgc_power_state.ack == SGC_ACK)   /* continue only if ACK was received */
      {
        pcmd[0] = 0x59;         /* "Control" Command */
        pcmd[1] = 0x03;         /* "Display OnOff" Command */
        pcmd[2] = 0x00;         /* "Shutdown" Command */
        if (sgc_sendcommand(3, SGC_ONE_SEC, SGC_OPT_INTERNAL, pcmd, pcmd) == 0) /* busy with other command */
        {                       /* proceed to sleep or skip sleep, depending on sleep-in-shutdown setting */
          sgc_power_state.ist =
            (10 - ((bitstates & SHDN_SLEEP) >> SH_SHDN_SLEEP));
          break;                /* if send command successful, proceed to next state */
        }
      }
      sgc_power_state.ist = SGC_DISP_RESET;
      break;                    /* if unexpected result or timeout --> reset */

    case 9:                    /* STATE 9: Sleep in Shutdown */
      if ((sgc_power_state.ack >= SGC_SENDING) && (sgc_power_state.ack <= SGC_NONE))
        break;                  /* no answer yet received, wait */
      if (sgc_power_state.ack == SGC_ACK)   /* continue only if ACK was received */
      {
        if (sgc_sleep
            (((bitstates & SHDN_SLEEPMODE) >> SH_SHDN_SLEEPMODE),
             SGC_OPT_INTERNAL) == 0)
        {
          sgc_power_state.ist = 10;
          break;                /* if send command successful, proceed to next state */
        }
      }
      sgc_power_state.ist = SGC_DISP_RESET;
      break;                    /* if unexpected result or timeout --> reset */

    case 10:                   /* STATE 10: wait for shutdown or sleep ACK */
      if ((sgc_power_state.ack >= SGC_SENDING) && (sgc_power_state.ack <= SGC_NONE))
        break;                  /* no answer yet received, wait */
      if (sgc_power_state.ack == SGC_ACK)   /* continue only if ACK was received */
      {
        sgc_power_state.ist = SGC_SHUTDOWN;
        bitstates &= ~TXBUSY;   /* release UART */

#ifdef SGC_ECMD_SEND_SUPPORT
        if (bitstates & F_RESET)
        {                       /* notify host: display ready from reset */
          ecmd_sender_send_command_P(&ip, NULL, PSTR("RESET\n"));
          break;
        }                       /* notify host: display in shutdown */
        ecmd_sender_send_command_P(&ip, NULL, PSTR("SHUTDOWN\n"));
#endif /* SGC_ECMD_SEND_SUPPORT */

        break;
      }
      sgc_power_state.ist = SGC_DISP_RESET;
      break;                    /* if unexpected result or timeout --> reset */

    case SGC_SHUTDOWN:             /* Display is shut down, power can be turned off. */
      if (sgc_power_state.ack == SGC_TIMEOUT)       /* any out of sync --> reset */
        sgc_power_state.ist = SGC_DISP_RESET;
      break;

    case SGC_BEGIN_POWERUP:        /* STATE 12: Start power-up sequence */
      bitstates |= TXBUSY;      /* block UART for command sequence */
      if ((sgc_power_state.ack >= SGC_SENDING) && (sgc_power_state.ack <= SGC_NONE))
        break;                  /* wait for UART to finish sending if necessary */
      if (sgc_power_state.ack <= SGC_NACK)
      {                         /* continue if last command was finished "clean" */
        pcmd[0] = 0x59;         /* "Control" Command */
        pcmd[1] = 0x03;         /* "Display OnOff" Command */
        pcmd[2] = 0x01;         /* "Power Up" Command */
        if (sgc_sendcommand(3, SGC_ONE_SEC, SGC_OPT_INTERNAL, pcmd, pcmd) == 0) /* busy with other command */
        {
          sgc_power_state.ist = 13;     /* if send command successful, proceed */
          break;
        }
      }
      sgc_power_state.ist = SGC_DISP_RESET;
      break;                    /* if unexpected result or timeout --> reset */

    case 13:                   /* STATE 13: Restore contrast value */
      if ((sgc_power_state.ack >= SGC_SENDING) && (sgc_power_state.ack <= SGC_NONE))
        break;                  /* no answer yet received, wait */
      if (sgc_power_state.ack == SGC_ACK)   /* continue only if ACK was received */
      {
        pcmd[0] = 0x59;         /* "Control" Command */
        pcmd[1] = 0x02;         /* "Contrast" Command */
        pcmd[2] = sgc_disp_vars.contrast;       /* Restore Contrast */
        if (sgc_sendcommand(3, SGC_ONE_SEC, SGC_OPT_INTERNAL, pcmd, pcmd) == 0) /* busy with other command */
        {
          sgc_power_state.ist = 14;
          break;                /* if send command successful, proceed to next state */
        }
      }
      sgc_power_state.ist = SGC_DISP_RESET;
      break;                    /* if unexpected result or timeout --> reset */

    case 14:                   /* STATE 14: Restore Font setting */
      if ((sgc_power_state.ack >= SGC_SENDING) && (sgc_power_state.ack <= SGC_NONE))
        break;                  /* no answer yet received, wait */
      if (sgc_power_state.ack == SGC_ACK)   /* continue only if ACK was received */
      {
        pcmd[0] = 0x46;         /* "Set Font" Command */
        pcmd[1] = sgc_disp_vars.font & 0x0F;    /* standard without proportional setting */
        if (sgc_sendcommand(2, SGC_ONE_SEC, SGC_OPT_INTERNAL, pcmd, pcmd) == 0) /* busy with other command */
        {
          sgc_power_state.ist = 15;
          break;                /* if send command successful, proceed to next state */
        }
      }
      sgc_power_state.ist = SGC_DISP_RESET;
      break;                    /* if unexpected result or timeout --> reset */

    case 15:                   /* STATE 15: Restore Opacity setting */

      if ((sgc_power_state.ack >= SGC_SENDING) && (sgc_power_state.ack <= SGC_NONE))
        break;                  /* no answer yet received, wait */
      if (sgc_power_state.ack == SGC_ACK)   /* continue only if ACK was received */
      {
        pcmd[0] = 0x4F;         /* "Opacity" Command */
        pcmd[1] = sgc_disp_vars.opacity;        /* value */
        if (sgc_sendcommand(2, SGC_ONE_SEC, SGC_OPT_INTERNAL, pcmd, pcmd) == 0) /* busy with other command */
        {
          sgc_power_state.ist = 16;
          break;                /* if send command successful, proceed to next state */
        }
      }
      sgc_power_state.ist = SGC_DISP_RESET;
      break;                    /* if unexpected result or timeout --> reset */

    case 16:                   /* STATE 16: Restore pensize setting */
      if ((sgc_power_state.ack >= SGC_SENDING) && (sgc_power_state.ack <= SGC_NONE))
        break;                  /* no answer yet received, wait */
      if (sgc_power_state.ack == SGC_ACK)   /* continue only if ACK was received */
      {
        pcmd[0] = 0x70;         /* "Pensize" Command */
        pcmd[1] = sgc_disp_vars.pensize;        /* standard without proportional setting */
        if (sgc_sendcommand(2, SGC_ONE_SEC, SGC_OPT_INTERNAL, pcmd, pcmd) == 0) /* busy with other command */
        {
          sgc_power_state.ist = 17;
          break;                /* if send command successful, proceed to next state */
        }
      }
      sgc_power_state.ist = SGC_DISP_RESET;
      break;                    /* if unexpected result or timeout --> reset */

    case 17:                   /* STATE 17: Turn display on */
      if ((sgc_power_state.ack >= SGC_SENDING) && (sgc_power_state.ack <= SGC_NONE))
        break;                  /* no answer yet received, wait */
      if (sgc_power_state.ack == SGC_ACK)   /* continue only if ACK was received */
      {
        pcmd[0] = 0x59;         /* "Control" Command */
        pcmd[1] = 0x01;         /* "Display OnOff" Command */
        pcmd[2] = 0x01;         /* "On" Command */
        if (sgc_sendcommand(3, SGC_ONE_SEC, SGC_OPT_INTERNAL, pcmd, pcmd) == 0) /* busy with other command */
        {
          sgc_power_state.ist = 18;
          break;                /* if send command successful, proceed to next state */
        }
      }
      sgc_power_state.ist = SGC_DISP_RESET;
      break;                    /* if unexpected result or timeout --> reset */

    case 18:                   /* STATE 18: wait for last SGC_ACK */
      if ((sgc_power_state.ack >= SGC_SENDING) && (sgc_power_state.ack <= SGC_NONE))
        break;                  /* no answer yet received, wait */
      if (sgc_power_state.ack == SGC_ACK)   /* continue only if SGC_ACK was received */
      {
        sgc_power_state.ist = SGC_POWERUP;
        bitstates &= ~TXBUSY;   /* release UART */

#ifdef SGC_ECMD_SEND_SUPPORT    /* notify host: display powered up */
        ecmd_sender_send_command_P(&ip, NULL, PSTR("POWERUP\n"));
#endif /* SGC_ECMD_SEND_SUPPORT */

        break;
      }
      sgc_power_state.ist = SGC_DISP_RESET;
      break;                    /* if unexpected result or timeout --> reset */

    case SGC_POWERUP:              /* Display is powered up, do not turn off power now. */
      if (sgc_power_state.ack == SGC_TIMEOUT)       /* out of sync --> reset */
        sgc_power_state.ist = SGC_DISP_RESET;
      break;

    default:
      sgc_power_state.ist = SGC_DISP_RESET;
  }                             /* Undefinded state, go into reset for recovery */
}

ISR(usart(USART, _TX_vect))
{
  if ((sgc_uart_buffer.pos < sgc_uart_buffer.len) && (~bitstates & SLEEPING))   /* still bytes in buffer? */
  {
    usart(UDR) = sgc_uart_buffer.txdata[sgc_uart_buffer.pos];   /* send next */
    sgc_uart_buffer.pos++;      /* increment buffer pointer */
  }
  else if (bitstates & SLEEPING)        /* wakeup command sent */
  {
    sgc_power_state.ack = SGC_WAKEUP;       /* set state and start timeout counter */
  }
  else                          /* command finished */
  {
    usart(UCSR, B) &= ~(_BV(usart(TXCIE)));     /* Disable this interrupt */
    sgc_uart_buffer.pos = 0;    /* clear for next transmission */
    sgc_uart_buffer.len = 0;

    if (sgc_power_state.ack_timeout == SGC_INFINITE)        /* infinite timeout */
    {
      sgc_power_state.ack = SGC_ACK;        /* set status: infinite timeout is always ACK */
#ifdef SGC_ECMD_SEND_SUPPORT
      ecmd_sender_send_command_P(&ip, NULL, PSTR("ACK\n"));
#endif /* SGC_ECMD_SEND_SUPPORT */
    }
    else
      sgc_power_state.ack = SGC_NONE;       /* set status, enable timeout counter */

    if (bitstates & SLEEP)      /* sleep command was sent? */
    {
      bitstates &= ~SLEEP;
      bitstates |= SLEEPING;
    }
  }
}

ISR(usart(USART, _RX_vect))
{
  uint8_t flags;
  char rxdata;
  flags = usart(UCSR, A);
  rxdata = usart(UDR);          /* read input buffer */

  if (bitstates & RXENABLE)     /* only if reception is enabled */
  {
    if ((flags & _BV(usart(DOR))) || (flags & _BV(usart(FE))) ||
        ((rxdata != 0x06) && (rxdata != 0x15)) ||
        ((sgc_power_state.ack != SGC_NONE) && (sgc_power_state.ack != SGC_WAKEUP)))
    {                           /* UART RX Error, wrong or unexpected character received */
      sgc_power_state.ist = SGC_DISP_RESET;
      return;                   /* something must have gone very wrong, reset to resync */
    }

    if (bitstates & SLEEPING)   /* display has woken up with any answer */
    {
      sgc_power_state.ack = SGC_SENDING;
      sgc_power_state.acktimer = 0;     /* reset ACK timeout */
      sgc_power_state.ack_timeout -= SGC_FIVE_SEC;  /* restore original command timeout */
      sgc_uart_buffer.pos = 1;  /* set TX buffer pointer */
      usart(UDR) = sgc_uart_buffer.txdata[0];   /* transmit first byte */
    }
    else
    {
      if (rxdata == 0x06)       /* ACK received */
      {
        sgc_power_state.ack = SGC_ACK;      /* ACK was received */
#ifdef SGC_ECMD_SEND_SUPPORT
        if ((sgc_power_state.ist == SGC_SHUTDOWN) ||
            (sgc_power_state.ist == SGC_POWERUP))
          ecmd_sender_send_command_P(&ip, NULL, PSTR("ACK\n"));
#endif /* SGC_ECMD_SEND_SUPPORT */
      }
      else                      /* NACK was received */
      {
        sgc_power_state.ack = SGC_NACK;     /* ACK was received */
#ifdef SGC_ECMD_SEND_SUPPORT
        if ((sgc_power_state.ist == SGC_SHUTDOWN) ||
            (sgc_power_state.ist == SGC_POWERUP))
          ecmd_sender_send_command_P(&ip, NULL, PSTR("NACK\n"));
#endif /* SGC_ECMD_SEND_SUPPORT */
      }
    }
    bitstates &= ~SLEEPING;     /* sleep mode ends with first response from display */
    return;
  }
}

/*
 -- Ethersex META --
 header(protocols/sgc/sgc.h)
 init(sgc_init)
 timer(1, sgc_pwr_periodic())
*/
