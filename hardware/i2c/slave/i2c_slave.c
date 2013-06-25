/*
 * Copyright (c) 2008 by Jochen Roessner <jochen@lugrot.de>
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
#include <avr/interrupt.h>
#include <util/delay.h>

#include "config.h"
#include "core/global.h"
#include "protocols/uip/uip.h"
#include "i2c_slave.h"
#include "i2c_slave_state.h"

static uip_udp_conn_t *i2c_slave_conn;

#define STATS (i2c_slave_conn->appstate.i2c_slave)
#define SLAVE (i2c_slave_conn->appstate.i2c_slave.slavedata)

/*
 * direkter zugriff zum packet buffer
 */
#define BUF ((struct uip_udpip_hdr *) (uip_appdata - UIP_IPUDPH_LEN))

void
init_twi(void)
{

  TWCR = 0;                     //fuer das Initialisieren bei einem status fehler

  /* INIT fuer den TWI i2c
   * hier wird die Addresse des µC festgelegt
   * (in den oberen 7 Bit, das LSB(niederwertigstes Bit)
   * steht dafür ob der µC auf einen general callreagiert
   */
  TWAR = TWIADDR << 1;

  /* TWI Control Register, hier wird der TWI aktiviert, 
   * der Interrupt aktiviert und solche Sachen
   */
  TWCR = (1 << TWIE) | (1 << TWEN) | (1 << TWEA);

  /* TWI Status Register init */
  TWSR &= 0xFC;
}




void
i2c_slave_core_init(uip_udp_conn_t * conn)
{
  init_twi();
  i2c_slave_conn = conn;
}

void
i2c_slave_core_periodic(void)
{
  /* error detection on i2c bus */
  if ((TWSR & 0xF8) == 0x00)
    init_twi();
}

void
i2c_slave_core_newdata(void)
{
  struct i2c_slave_request_t *REQ = uip_appdata;

  uip_udp_conn_t return_conn;
  //if ( uip_datalen() == 1 && REQ->type == 0)

  uip_ipaddr_copy(return_conn.ripaddr, BUF->srcipaddr);
  return_conn.rport = BUF->srcport;
  return_conn.lport = HTONS(I2C_SLAVE_PORT);

  uip_send(&STATS, sizeof(struct i2c_slave_connection_state_t));

  uip_udp_conn = &return_conn;
  /* Send immediately */
  uip_process(UIP_UDP_SEND_CONN);
  router_output();
  uip_slen = 0;
  SLAVE.kommando = 0;
}

#define BLINK_PORT PORTB
#define BLINK_PIN _BV(PB6)


/* Interruptroutine des TWI
 */
ISR(TWI_vect)
{

  switch (TWSR & 0xF8)
  {
    case 0x80:
      /* Datenbyte wurde empfangen
       * TWDR auslesen
       */
      if (SLAVE.byteanzahl == 0)
      {
        SLAVE.smbuscommand = TWDR;
      }
      else if (SLAVE.byteanzahl == 1)
      {
        SLAVE.smbuscount = TWDR;
      }

      if (SLAVE.smbuscommand == 0xF0 && SLAVE.smbuscount == 1 && 0xF1 == TWDR)
      {
#ifdef BOOTLOADER_JUMP
        status.request_bootloader = 1;
#endif
      }
      else if (SLAVE.smbuscommand == 0x40)
      {
        if (SLAVE.byteanzahl > 1)
        {
          SLAVE.buf[SLAVE.byteanzahl - 2] = TWDR;
          if (--SLAVE.smbuscount == 0)
            SLAVE.kommando = SLAVE.buf[0];
        }
      }
      SLAVE.byteanzahl++;
      break;
    case 0x60:
      /* Der Avr wurde mit seiner Adresse angesprochen  */
      SLAVE.byteanzahl = 0;
#ifdef BLINK_PORT
      BLINK_PORT |= BLINK_PIN;
#endif
      break;

      /* hier wird an den Master gesendet */
    case 0xA8:
      switch (SLAVE.smbuscommand)
      {
        case 0x44:
          TWDR = 0x20;
          SLAVE.smbuscount = 0x00;
          break;
        default:
          TWDR = 0x10;          //zur demo den zaehler txbyte senden
          break;
      }
      break;
    case 0xB8:
      switch (SLAVE.smbuscommand)
      {
        case 0x44:
          if (SLAVE.smbuscount < 0x20)
          {
            TWDR = SLAVE.buf[SLAVE.smbuscount++];
          }
          break;
        default:
          TWDR++;
          break;
      }
      SLAVE.byteanzahl++;
      break;
    default:
#ifdef BLINK_PORT
      BLINK_PORT &= ~BLINK_PIN;
#endif
      break;
      /* fuer einen zukuenftigen general call */
      /* 
       * if((TWSR & 0xF8) == 0x70){
       * //general call
       * motorschalter = 2;
       * }
       */
  }
  TWCR |= (1 << TWINT);         //TWI wieder aktivieren
}
