/*
 * Copyright (c) 2007,2008 by Jochen Roessner <jochen@lugrot.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
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
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "../net/i2c_state.h"
#include "../uip/uip.h"
#include "../config.h"
#include "i2c.h"
#include "../crypto/skipjack.h"

#ifdef I2C_SUPPORT

/* constants */
#define MAXDATAPAKETLEN (uint8_t)(UIP_APPDATA_SIZE - 3)

#define STATS (uip_udp_conn->appstate.i2c)

/*
 * direkter zugriff zum packet buffer
 */
#define BUF ((struct uip_udpip_hdr *)&uip_appdata[-UIP_IPUDPH_LEN])

enum datalen {
  GETMAXDATA = 1,
  READFROMI2C
};

static void 
i2c_wait_int()
{
  while( (TWCR & _BV(TWINT)) == 0);
}

static uint8_t
    i2c_send ( uint8_t sendbyte )
{
  TWDR = sendbyte;
  TWCR |= _BV(TWINT);
  i2c_wait_int();
  return (TWSR & 0xF8);
}

// static void
//     i2c_send_buffer_immediate(void)
// {
//   //uip_send(&i2ctx, I2C_DATAOFFSET);
//   uip_process(UIP_UDP_SEND_CONN);
//   fill_llh_and_transmit();
//   uip_slen = 0;
// }

static void
    reset_connection(uip_udp_conn_t *i2c_conn)
{
  uip_ipaddr_copy(i2c_conn->ripaddr, all_ones_addr);
  i2c_conn->rport = 0;
  i2c_conn->appstate.i2c.timeout = 0;
  i2c_conn->appstate.i2c.last_seqnum = 0;
}

static void
    i2c_port_init(void)
{
  TWCR = 0;
  /* max speed 400khz (problematisch)  ~(_BV(TWPS0) | _BV(TWPS1)) BR = 16
  speed 100khz (normal) _BV(TWPS0) BR = 92 */
  //TWSR &= ~(_BV(TWPS0) | _BV(TWPS1));
#if F_CPU > 10000000
  TWSR |= _BV(TWPS0);
  TWBR = 92;
#else
  TWBR = 52; //max speed for twi bei 8mhz, ca 100khz by 12Mhz Crystal
  PORTC |= _BV(PC4) | _BV(PC5); //enable pullup vor i2c
#endif
  //TWCR |= _BV(TWEN);
}

void 
    i2c_core_init(uip_udp_conn_t *i2c_conn)
{
  i2c_port_init();
  reset_connection(i2c_conn);
}

void
    i2c_core_periodic(void)
{
  if(STATS.timeout == 1){
    TWCR = 0;
    reset_connection(uip_udp_conn);
    /* FIXME:   PORTC &= ~_BV(PC2); */
  }
  if(STATS.timeout > 0)
    STATS.timeout--;
    
  /* error detection on i2c bus */
  if((TWSR & 0xF8) == 0x00)
    TWCR = 0;
    //i2c_port_init();
}

void i2c_core_newdata(void)
{
	
  struct i2c_t *REQ = uip_appdata;
		/*
  * ueberschreiben der connection info. 
  * port und adresse auf den remotehost begrenzen
  * und antwort paket senden mit der maximalen pufferlaenge (i2c open)
                */
  uint8_t resetconnection = 0;
  if(STATS.last_seqnum == 0){
    uip_ipaddr_copy(uip_udp_conn->ripaddr, BUF->srcipaddr);
    uip_udp_conn->rport = BUF->srcport;
  }

  if(uip_datalen() == GETMAXDATA && REQ->seqnum == 0)
  {
    REQ->maxdatalen = MAXDATAPAKETLEN;
    uip_slen = 2;
    resetconnection = 1;
  }
  else if(uip_datalen() >= READFROMI2C) //  && 
  {
    if(STATS.last_seqnum == 0)
    {
      /* sende startcondition und adresse wenn kein paket vorher da war (last_seqnum = 0) */
      TWCR = _BV(TWINT) | _BV(TWSTA) | _BV(TWEN);
      i2c_wait_int();
      uint8_t TWSRtmp = (TWSR & 0xF8);
      if(TWSRtmp == 0x08 || TWSRtmp == 0x10)
      {
        TWCR = _BV(TWEN);
        TWSRtmp = i2c_send ( REQ->i2c_addr_rw );
        //TWCR = _BV(TWINT) | _BV(TWEN);
        //i2c_wait_int();
        //            revc code          send code ack      send code nack
        if(TWSRtmp != 0x40 && TWSRtmp != 0x18 && TWSRtmp != 0x20)
        {
          TWCR = 0;
          uip_slen = 1;
          resetconnection = 1;
        }
      }
      else
      {
        TWCR = 0;
        uip_slen = 1;
        resetconnection = 1;
      }
    }
    if(!resetconnection){
      uint8_t TWSRtmp;
      if((REQ->i2c_addr_rw & 0x01) == 1)
      {
        uint8_t tmp_datalen = REQ->datalen;
        uint8_t tmp_datapos = 0;
        if (tmp_datalen > MAXDATAPAKETLEN)
          tmp_datalen = MAXDATAPAKETLEN;
        
        while (tmp_datapos < tmp_datalen)
        {
          if(tmp_datapos == (tmp_datalen - 1) && REQ->seqnum == 0)
            TWCR = _BV(TWINT) | _BV(TWEN);
          else
          {
            TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWEA);
          }
          i2c_wait_int();
          TWSRtmp = (TWSR & 0xF0);
          if(TWSRtmp == 0x50 ){
            REQ->readdata[tmp_datapos++] = TWDR;
          }
          else{
            uip_slen = 1;
            resetconnection = 1;
            break;
          }
        }
        if(REQ->seqnum == 0){
          TWCR = _BV(TWINT) | _BV(TWSTO) | _BV(TWEN);
          resetconnection = 1;
        }
        uip_slen = tmp_datalen + 1;
      }
      else
      {
        if(STATS.last_seqnum == 0 || REQ->seqnum != STATS.last_seqnum)
        {
          uint8_t tmp_datapos = 0;
  
          while (tmp_datapos < uip_datalen() - 2){
            TWSRtmp = i2c_send ( REQ->writedata[tmp_datapos++] );
            /* fehler protokollieren */
            if(TWSRtmp != 0x28){
              break;
            }
          }
          if(REQ->seqnum == 0){
            TWCR = _BV(TWINT) | _BV(TWSTO) | _BV(TWEN);
            resetconnection = 1;
          }
          REQ->write_datalen_ack = tmp_datapos;
        }
        else
          REQ->write_datalen_ack = uip_datalen() - 2;
        uip_slen = 2;
      }
    }
  }
  if(!resetconnection){
    STATS.last_seqnum = REQ->seqnum;
    STATS.timeout = 25;
  }
  uip_process(UIP_UDP_SEND_CONN);
  fill_llh_and_transmit();
  uip_slen = 0;
  if(resetconnection)
    reset_connection(uip_udp_conn);
}

#endif
