/*
 * Copyright (c) 2008 by Christian Dietrich <stettberger@dokucode.de>
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
#include "ecmd_i2c.h"
#include "protocols/ecmd/parser.h"
#include "protocols/ecmd/ecmd-base.h"

static char recv_buffer[ECMD_I2C_BUFFER_LEN];
static char write_buffer[ECMD_I2C_BUFFER_LEN];
static int16_t recv_len, write_len, sent, parse;

static void
init_twi(void){

  TWCR = 0; //fuer das Initialisieren bei einem status fehler

  /* INIT fuer den TWI i2c
  * hier wird die Addresse des µC festgelegt
  * (in den oberen 7 Bit, das LSB(niederwertigstes Bit)
  * steht dafür ob der µC auf einen general callreagiert
  */
  TWAR = ECMD_I2C_ADDR << 1;

  /* TWI Control Register, hier wird der TWI aktiviert,
   * der Interrupt aktiviert und solche Sachen
   */
  TWCR = (1<<TWIE) | (1<<TWEN) | (1<<TWEA);

  /* TWI Status Register init */
  TWSR &= 0xFC;
}

void ecmd_serial_i2c_init(void) {
  recv_len = 0;
  write_len = 0;
  sent = 0;
  parse = 0;
  init_twi();
}

void
ecmd_serial_i2c_periodic(void)
{
  /* error detection on i2c bus */
  if((TWSR & 0xF8) == 0x00)
    init_twi();
  if (parse && !write_len ) {

     if(recv_len <= 1) return;

    write_len = ecmd_parse_command(recv_buffer, write_buffer, sizeof(write_buffer));
    parse = 0;
    if (is_ECMD_AGAIN(write_len)) {
      /* convert ECMD_AGAIN back to ECMD_FINAL */
      write_len = ECMD_AGAIN(write_len);
      parse = 1;
    }
    else if (is_ECMD_ERR(write_len))
       return;
    else
       recv_len = 0;

    sent=1;
  }
}

/* twi interrupt */
ISR (TWI_vect)
{
  switch (TWSR & 0xF8){
  case 0x80: /* databyte was received */
    if (recv_len < (sizeof(recv_buffer) - 1))
        recv_buffer[recv_len++] = TWDR;
    if (recv_buffer[recv_len-1] == 0) {
      /* EOF message */
      parse = 1;
    }
    break;
  case 0x60: /* Start condition + write was received */
    /* Der Avr wurde mit seiner Adresse angesprochen  */
    recv_len = 0;
  break;
  case 0xA8: /* Start condition + read was received */
    sent = 0; /* fall through */
  case 0xB8: /* the answer is sent */
    if (sent < write_len)
      TWDR = write_buffer[sent++];
    else {
      TWDR = parse ? '\n': 0;
      write_len = 0;
    }
  break;
  }
  TWCR |= (1<<TWINT); //TWI wieder aktivieren
}

/*
  -- Ethersex META --
  header(protocols/ecmd/via_i2c/ecmd_i2c.h)
  init(ecmd_serial_i2c_init)
  timer(1, ecmd_serial_i2c_periodic())
*/
