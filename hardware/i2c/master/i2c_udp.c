/*
 * Copyright (c) 2007,2008 by Jochen Roessner <jochen@lugrot.de>
 * Copyright (c) 2009 by Christian Dietrich <stettberger@dokucode.de>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License (either version 2 or
 * version 3) as published by the Free Software Foundation.
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
#include <util/delay.h>
#include <util/twi.h>

#include "protocols/uip/uip.h"
#include "protocols/uip/uip_router.h"
#include "config.h"
#include "i2c_master.h"
#include "i2c_udp.h"
#include "i2c_state.h"

#ifdef I2C_UDP_SUPPORT

/* constants */
#if defined(ENC28J60_SUPPORT)
  #define MAXDATAPAKETLEN ((UIP_BUFSIZE) - (UIP_IPUDPH_LEN) - 3)
#elif defined(RFM12_IP_SUPPORT)
  #define MAXDATAPAKETLEN (196 - 76)
#elif defined(ZBUS_SUPPORT)
  #define MAXDATAPAKETLEN (128 - 76)
#else
  #define MAXDATAPAKETLEN (40)
#endif

#define STATSI2C (uip_udp_conn->appstate.i2c)

/*
 * direkter zugriff zum packet buffer
 */
#define BUF ((struct uip_udpip_hdr *)&((char *)uip_appdata)[-UIP_IPUDPH_LEN])

enum datalen {
  GETMAXDATA = 1,
  READFROMI2C
};

static void
reset_connection(uip_udp_conn_t *i2c_conn)
{
  uip_ipaddr_copy(i2c_conn->ripaddr, all_ones_addr);
  i2c_conn->rport = 0;
  i2c_conn->appstate.i2c.timeout = 0;
  i2c_conn->appstate.i2c.last_seqnum = 0;
}

void 
i2c_udp_init(uip_udp_conn_t *i2c_conn)
{
  reset_connection(i2c_conn);
}

void
i2c_udp_periodic(void)
{
  if(STATSI2C.timeout == 1){
    i2c_master_disable();
    reset_connection(uip_udp_conn);
    /* FIXME:   PORTC &= ~_BV(PC2); */
  }
  if(STATSI2C.timeout > 0)
    STATSI2C.timeout--;
    
  /* error detection on i2c bus */
  if((TWSR & 0xF8) == 0x00)
    i2c_master_disable();
}

void i2c_udp_newdata(void)
{
	
  struct i2c_t *REQ = uip_appdata;
		/*
  * ueberschreiben der connection info. 
  * port und adresse auf den remotehost begrenzen
  * und antwort paket senden mit der maximalen pufferlaenge (i2c open)
                */
  uint8_t resetconnection = 0;
  uint8_t error = 0;

  if(STATSI2C.last_seqnum == 0){
    uip_ipaddr_copy(uip_udp_conn->ripaddr, BUF->srcipaddr);
    uip_udp_conn->rport = BUF->srcport;
  }

  if(uip_datalen() == GETMAXDATA && REQ->seqnum == 0)
  {
    uint16_t mdpl = MAXDATAPAKETLEN;
    REQ->maxdatalen = mdpl;
    uip_slen = 2;
    resetconnection = 1;
  }
  else if(uip_datalen() >= READFROMI2C) //  && 
  {
    if(STATSI2C.last_seqnum == 0)
    {
      /* sende startcondition und adresse wenn kein paket vorher da war (last_seqnum = 0) */
      uint8_t mode = REQ->i2c_addr_rw & 0x01;
      uint8_t addr = (REQ->i2c_addr_rw & 0xfe) >> 1;
      if (! i2c_master_select(addr, mode)) 
        error = 1;
    }
    if(!resetconnection){
      if((REQ->i2c_addr_rw & 0x01) == TW_READ)
      {
        uint8_t tmp_datalen = REQ->datalen;
        uint8_t tmp_datapos = 0;
        uint16_t mdpl = MAXDATAPAKETLEN;
        if (tmp_datalen > mdpl)
          tmp_datalen = mdpl;
        
        while (tmp_datapos < tmp_datalen)
        {
          if (tmp_datapos == (tmp_datalen - 1) && REQ->seqnum == 0) {
            /* letztest Byte, wir erwarten ein NACK */
            if (i2c_master_transmit() != TW_MR_DATA_NACK)  {
              error = 1;
              break;
            }
          } else {
            if (i2c_master_transmit_with_ack() != TW_MR_DATA_ACK) {
              error = 1;
              break;
            }
          }
          REQ->readdata[tmp_datapos++] = TWDR;
        }
        if(REQ->seqnum == 0) {
          i2c_master_stop();
          resetconnection = 1;
        }
        uip_slen = tmp_datalen + 1;
      } else { /* TW_WRITE */
        if(STATSI2C.last_seqnum == 0 || REQ->seqnum != STATSI2C.last_seqnum)
        {
          uint8_t tmp_datapos = 0;
  
          while (tmp_datapos < uip_datalen() - 2) {
            TWDR = REQ->writedata[tmp_datapos++];
            /* fehler protokollieren */
            if (i2c_master_transmit() != TW_MT_DATA_ACK) {
              break;
            }
          }
          if(REQ->seqnum == 0){
            i2c_master_stop();
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
  if (error) {
    i2c_master_disable();
    uip_slen = 1;
    resetconnection = 1;
  }
  if(!resetconnection){
    STATSI2C.last_seqnum = REQ->seqnum;
    STATSI2C.timeout = 25;
  }
  uip_process(UIP_UDP_SEND_CONN);
  router_output();
  uip_slen = 0;
  if(resetconnection) 
    reset_connection(uip_udp_conn);

}

#endif /* I2C_UDP_SUPPORT */
