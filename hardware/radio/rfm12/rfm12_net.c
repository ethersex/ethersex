/*
 * Copyright (c) 2007, 2008 by Stefan Siegl <stesie@brokenpipe.de>
 * Copyright (c) 2007 Jochen Roessner <jochen@lugrot.de>
 * Copyright (c) 2007 Ulrich Radig <mail@ulrichradig.de>
 * Copyright (c) 2012 Erik Kunze <ethersex@erik-kunze.de>
 * Copyright (c) Benedikt K.
 * Copyright (c) Juergen Eckert
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

#include <stdint.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "config.h"
#include "core/bit-macros.h"
#include "core/heartbeat.h"
#include "protocols/uip/uip.h"
#include "protocols/uip/uip_router.h"

#include "rfm12.h"
#include "rfm12_raw_net.h"


rfm12_status_t rfm12_status;

static volatile rfm12_index_t rfm12_index;
static volatile rfm12_index_t rfm12_txlen;

static void rfm12_txstart_hard(void);
//static uint8_t rfm12_rxstop(void);


#ifdef RFM12_INT_VECTOR
ISR(RFM12_INT_VECTOR)
#elif defined(RFM12_USE_POLL)
void
rfm12_int_process(void)
#else
ISR(RFM12_VECTOR)               /* PCINT */
#endif
{
#ifdef HAVE_RFM12_PCINT
  if (PIN_HIGH(RFM12_PCINT))
    return;                     /* pin change to high level -> ignore */
#endif

  rfm12_modul_set_active(RFM12_MODULE_IP);
  uint16_t status = rfm12_trans(RFM12_CMD_STATUS);

  if (status & RFM12_STATUS_POR)
  {
    RFM12_DEBUG("rfm12_net/por -> init");
    rfm12_net_init();
    return;
  }

  if (status & RFM12_STATUS_FFOV)
  {
    RFM12_DEBUG("rfm12_net/overflow -> init");
    rfm12_net_init();
    return;
  }

  if ((status & RFM12_STATUS_FFIT) == 0)
  {
    RFM12_DEBUG("rfm12_net/spurious int: %x", status);
    return;
  }

  switch (rfm12_status)
  {
    case RFM12_RX:
    {
      uint8_t byte = LO8(rfm12_trans(RFM12_CMD_READ));

#ifndef TEENSY_SUPPORT
      if (rfm12_index ? (rfm12_index < RFM12_BUFFER_LEN) : !_uip_buf_lock)
#else
      /* ignore packet if higher len byte set (except source route) */
      if (rfm12_index ? (rfm12_index < RFM12_BUFFER_LEN)
          : (!_uip_buf_lock && (byte & 0x7f) == 0))
#endif
      {
        _uip_buf_lock = 8;
        rfm12_buf[rfm12_index++] = byte;
#ifdef STATUSLED_RFM12_RX_SUPPORT
        PIN_SET(STATUSLED_RFM12_RX);
#endif
        ACTIVITY_LED_RFM12_RX;
      }
      else
      {
        if (rfm12_index)
          uip_buf_unlock();     /* we already locked, therefore unlock */

        rfm12_trans(RFM12_CMD_PWRMGT | RFM12_PWRMGT_EX);
        rfm12_status = RFM12_OFF;
        rfm12_rxstart();
#ifdef STATUSLED_RFM12_RX_SUPPORT
        PIN_CLEAR(STATUSLED_RFM12_RX);
#endif
        return;
      }
    }

#ifdef TEENSY_SUPPORT
      if (rfm12_index > 2 && rfm12_index > (rfm12_buf[1] + 1))
#else
      if (rfm12_index > 2 &&
          rfm12_index > (rfm12_buf[1] + 1 + ((rfm12_buf[0] & 0x7f) << 8)))
#endif
      {
        rfm12_trans(RFM12_CMD_PWRMGT | RFM12_PWRMGT_EX);
        rfm12_status = RFM12_NEW;

        /* We're not ready to receive data yet, but otherwise the RFM12
         * module freaks out and will keep the interrupt line low. */
        rfm12_trans(RFM12_CMD_PWRMGT | RFM12_PWRMGT_ER | RFM12_PWRMGT_EBB |
                    RFM12_PWRMGT_EX);

      }
      break;

    case RFM12_TX:
    case RFM12_TX_PREAMBLE_1:
    case RFM12_TX_PREAMBLE_2:
    case RFM12_TX_DATAEND:
    case RFM12_TX_SUFFIX_1:
    case RFM12_TX_SUFFIX_2:
      rfm12_trans(RFM12_CMD_TX | 0xAA);
      rfm12_status++;
      break;

    case RFM12_TX_PREFIX_1:
      rfm12_trans(RFM12_CMD_TX | 0x2D);
      rfm12_status++;
      break;

    case RFM12_TX_PREFIX_2:
      rfm12_trans(RFM12_CMD_TX | 0xD4);
      rfm12_status++;
      break;

#ifdef RFM12_SOURCE_ROUTE_ALL
    case RFM12_TX_SRCRT_SZHI:
#ifdef TEENSY_SUPPORT
      rfm12_trans(RFM12_CMD_TX | 0x80);
#else
      rfm12_trans(RFM12_CMD_TX | 0x80 | (((3 + rfm12_txlen) & 0x7F00) >> 8));
#endif
      rfm12_status++;
      break;

    case RFM12_TX_SRCRT_SZLO:
      rfm12_trans(RFM12_CMD_TX | ((3 + rfm12_txlen) & 0xFF));
      rfm12_status++;
      break;

    case RFM12_TX_SRCRT_DEST:
      rfm12_trans(RFM12_CMD_TX | CONF_RFM12_SOURCE_ROUTE_ALL_RTRID);
      rfm12_status++;
      break;
#endif /* RFM12_SOURCE_ROUTE_ALL */

    case RFM12_TX_SIZE_HI:
      rfm12_trans(RFM12_CMD_TX | rfm12_buf[0]);
      rfm12_status++;
      break;

    case RFM12_TX_SIZE_LO:
      rfm12_trans(RFM12_CMD_TX | rfm12_buf[1]);
      rfm12_status++;
      break;

    case RFM12_TX_DATA:
      rfm12_trans(RFM12_CMD_TX | rfm12_data[rfm12_index++]);

      if (rfm12_index >= rfm12_txlen)
        rfm12_status = RFM12_TX_DATAEND;

      break;

    case RFM12_TX_END:
      rfm12_status = RFM12_OFF;
#ifdef STATUSLED_RFM12_TX_SUPPORT
      PIN_CLEAR(STATUSLED_RFM12_TX);
#endif
      rfm12_trans(RFM12_CMD_TX | 0x08); /* TX off */
      uip_buf_unlock();
      rfm12_rxstart();
      //break;

    case RFM12_OFF:
    case RFM12_NEW:
      rfm12_trans(RFM12_CMD_STATUS);    /* clear interrupt flags in RFM12 */
  }
  if (rfm12_status >= RFM12_TX)
    _uip_buf_lock = 8;
}

void
rfm12_net_init(void)
{
  /* wait until POR done */
  for (uint8_t i = 15; i; i--)
    _delay_ms(10);

  rfm12_prologue(RFM12_MODULE_IP);

  rfm12_trans(RFM12_CMD_LBDMCD | 0xE0);
  rfm12_trans(RFM12BAND(CONF_RFM12_FREQ));
  rfm12_trans(RFM12_CMD_DATAFILTER | RFM12_DATAFILTER_AL | 0x03);
  rfm12_trans(RFM12_CMD_FIFORESET | 0x80 | RFM12_FIFORESET_DR);
  rfm12_trans(RFM12_CMD_WAKEUP);
  rfm12_trans(RFM12_CMD_DUTYCYCLE);
  rfm12_trans(RFM12_CMD_AFC | 0xF7);

#ifdef CONF_RFM12B_SUPPORT
  rfm12_trans(0xCED4);          /* Set Sync=2DD4 */
  rfm12_trans(0xCC16);          /* pll bandwitdh 0: max bitrate 86.2kHz */
#endif

#ifdef DEBUG
  uint16_t result = rfm12_trans(RFM12_CMD_STATUS);
  RFM12_DEBUG("rfm12_net/init: %x", result);
#endif

#ifdef RFM12_DISABLE
  rfm12_trans(RFM12_CMD_PWRMGT);
#else
#ifdef TEENSY_SUPPORT
  rfm12_trans(RFM12_CMD_FREQUENCY | RFM12FREQ(CONF_RFM12_FREQ));
  rfm12_trans(RFM12_CMD_RXCTRL | 0x04ac);
  rfm12_trans(RFM12_CMD_DATARATE | 0x10);
  rfm12_trans(RFM12_CMD_TXCONF | 0x20);
#else
  rfm12_setfreq(RFM12FREQ(CONF_RFM12_FREQ));
  rfm12_setbandwidth(5, 1, 4);
  rfm12_setbaud(CONF_RFM12_BAUD / 100);
  rfm12_setpower(0, 2);
#endif /* not TEENSY_SUPPORT */

#ifdef STATUSLED_RFM12_RX_SUPPORT
  PIN_CLEAR(STATUSLED_RFM12_RX);
#endif
#ifdef STATUSLED_RFM12_TX_SUPPORT
  PIN_CLEAR(STATUSLED_RFM12_TX);
#endif

  rfm12_status = RFM12_OFF;
  rfm12_int_enable();
  rfm12_rxstart();

#ifdef DEBUG
  result = rfm12_trans(RFM12_CMD_STATUS);
  RFM12_DEBUG("rfm12_net/init'd: %x", result);
#endif
#endif /* !RFM12_DISABLE */

  rfm12_epilogue();
}

uint8_t
rfm12_rxstart(void)
{
  if (rfm12_status != RFM12_OFF)
  {
    return 1;                   /* rfm12 is not free for RX or now in RX */
  }

  rfm12_prologue(RFM12_MODULE_IP);

  rfm12_trans(RFM12_CMD_PWRMGT | RFM12_PWRMGT_ER | RFM12_PWRMGT_EBB |
              RFM12_PWRMGT_EX);
  rfm12_trans(RFM12_CMD_FIFORESET | 0x80 | RFM12_FIFORESET_DR);
  rfm12_trans(RFM12_CMD_FIFORESET | 0x80 | RFM12_FIFORESET_FF |
              RFM12_FIFORESET_DR);

  rfm12_epilogue();

  rfm12_index = 0;
  rfm12_status = RFM12_RX;

  rfm12_int_enable();
  return 0;
}

#if 0
static uint8_t
rfm12_rxstop(void)
{
  uint8_t result = 1;

  rfm12_prologue(RFM12_MODULE_IP);

  if (rfm12_status > RFM12_RX
      || (rfm12_status == RFM12_RX && rfm12_index > 0))
    goto out;                   /* module not idle */

  rfm12_trans(RFM12_CMD_PWRMGT);        /* now turn off the oscillator */
  result = 0;

  rfm12_status = RFM12_OFF;
  rfm12_int_disable();

out:;
  rfm12_epilogue();
  return result;
}
#endif


rfm12_index_t
rfm12_rxfinish(void)
{
  if (rfm12_status != RFM12_NEW)
    return (0);                 /* no new Packet */

#ifdef HAVE_STATUSLED_RFM12_RX
  PIN_CLEAR(STATUSLED_RFM12_RX);
#endif

  rfm12_index_t len = rfm12_buf[1];
#ifndef TEENSY_SUPPORT
  len += (rfm12_buf[0] & 0x7F) << 8;
#endif

  if (rfm12_buf[0] & 0x80)
  {
    /* We've received a source routed packet. */
#ifdef RFM12_PCKT_FWD
    if (rfm12_buf[2] == CONF_RFM12_STATID)
    {
      /* Strip source route header. */
      memmove(rfm12_buf, rfm12_buf + 3, len - 1);

      for (uint8_t j = 0; j < 15; j++)
        _delay_ms(10);          /* Wait 150ms for slower receivers to get
                                 * ready again. */

      rfm12_txlen = len - 3;    /* Num of bytes excluding LLH. */
      rfm12_txstart_hard();
      return 0;                 /* We mustn't parse the packet,
                                 * since this might cause a reply. */
    }
#else
    /* We're dumb, let's ignore that packet. */
    len = 0;
#endif
  }

  rfm12_status = RFM12_OFF;

  if (!len)
  {
    uip_buf_unlock();
    rfm12_rxstart();            /* we destroyed the packet ... */
  }

  return (len);                 /* receive size */
}

void
rfm12_txstart(rfm12_index_t size)
{
  if (rfm12_status > RFM12_RX
      || (rfm12_status == RFM12_RX && rfm12_index > 0))
  {
    return;                     /* rx or tx in action or
                                 * new packet left in buffer */
  }

  rfm12_txlen = size;

#ifdef TEENSY_SUPPORT
  rfm12_buf[0] = 0;
#else
  rfm12_buf[0] = HI8(rfm12_txlen);
#endif
  rfm12_buf[1] = LO8(rfm12_txlen);

  rfm12_txstart_hard();
}


static void
rfm12_txstart_hard(void)
{
  rfm12_status = RFM12_TX;

#ifdef STATUSLED_RFM12_TX_SUPPORT
  PIN_SET(STATUSLED_RFM12_TX);
#endif

  ACTIVITY_LED_RFM12_TX;

  rfm12_index = 0;

  rfm12_prologue(RFM12_MODULE_IP);
  rfm12_trans(RFM12_CMD_PWRMGT | RFM12_PWRMGT_ET | RFM12_PWRMGT_ES |
              RFM12_PWRMGT_EX);
  rfm12_epilogue();

  /* Force interrupts active no matter what.
   * 
   * If we're forwarding a packet from say Ethernet, uip_buf_unlock won't
   * unlock since there's an active RFM12 transfer, but it'd leave
   * the RFM12 interrupt disabled as well. */
  _uip_buf_lock = 8;
  rfm12_int_enable();
}


void
rfm12_process(void)
{
  uip_len = rfm12_rxfinish();
  if (!uip_len)
    return;

#ifdef ROUTER_SUPPORT
#ifdef RFM12_RAW_SUPPORT
  if (rfm12_raw_conn->rport)
  {
    /* rfm12 raw capturing active, forward in udp/ip encapsulated form,
     * thusly don't push to the stack. */
    /* FIXME This way we cannot accept rfm12_raw requests from anything
     * but ethernet.  This shalt be improved somewhen. */
    uip_stack_set_active(STACK_ENC);
    memmove(uip_buf + UIP_IPUDPH_LEN + UIP_LLH_LEN, rfm12_data, uip_len);
    uip_slen = uip_len;
    uip_udp_conn = rfm12_raw_conn;
    uip_process(UIP_UDP_SEND_CONN);
    router_output();

    uip_buf_unlock();
    rfm12_rxstart();
    return;
  }
#endif /* RFM12_RAW_SUPPORT */

  /* uip_input expects the number of bytes including the LLH. */
  uip_len = uip_len + RFM12_BRIDGE_OFFSET + RFM12_LLH_LEN;
#endif /* not ROUTER_SUPPORT */

  rfm12_rxstart();

  router_input(STACK_RFM12);

  if (uip_len == 0)
  {
    uip_buf_unlock();
    return;                     /* The stack didn't generate any data
                                 * that has to be sent back. */
  }

  /* Application has generated output, send it out. */
  router_output();
}

/*
  -- Ethersex META --
  header(hardware/radio/rfm12/rfm12_net.h)
  mainloop(rfm12_process)
  ifdef(`conf_RFM12_USE_POLL',`mainloop(rfm12_int_process)')
  init(rfm12_net_init)
*/
