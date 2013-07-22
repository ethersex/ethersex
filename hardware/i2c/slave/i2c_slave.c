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
//case 0x68:  /* Arbitration lost in SLA+R/W as Master; own SLA+W has been received; ACK has been returned */
//case 0xB0:  /* Arbitration lost in SLA+R/W as Master; own SLA+R has been received; ACK has been returned */
//case 0xC0: /* Data byte in TWDR has been transmitted; NOT ACK has been received */
//case 0xC8: /* Last data byte in TWDR has been transmitted (TWEA = .0.); ACK has been received */
 
#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "config.h"
#include "core/global.h"
#include "protocols/ecmd/parser.h"
#include "protocols/ecmd/ecmd-base.h"

#include "i2c_slave.h"

static char recv_buffer[ECMD_TWI_BUFFER_LEN];
static char write_buffer[ECMD_TWI_BUFFER_LEN + 2]; //for returning \r\n 
static int16_t recv_len,sent_len, ecmd_ret_len;
static int8_t twi_parse;

void 
twi_slave_init(void)
{
  /*fuer das Initialisieren bei einem status fehler*/
  TWCR = 0;                     

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
twi_slave_periodic(void)
{


	/* error detection on i2c bus */
	if((TWSR & 0xF8) == 0x00)
		twi_slave_init();
	
//	if (twi_parse == 1 && !ecmd_ret_len) {
	if (twi_parse == 1) {
		twi_parse=0;
		twi_parse_ecmd();
	}

	
}

void
twi_parse_ecmd (void)
{
//int i=0;
//	for (i = 0; i < sizeof write_buffer; i++)   
//		write_buffer[i]=0;
//        TWIDEBUG("qwe\n");

	/* twi master can send 2 raw bytes (protocol specified), 
	 * dont know how to determ if 2 bytes are 2 raw bytes 
	 * or ecmd command like 'ip'
	 */

	ecmd_ret_len = ecmd_parse_command(recv_buffer, write_buffer, 
					sizeof(write_buffer));	
					
	if (is_ECMD_AGAIN(ecmd_ret_len)) {
	      	/* convert ECMD_AGAIN back to ECMD_FINAL */
      		ecmd_ret_len = ECMD_AGAIN(ecmd_ret_len);
     	}
	else if (is_ECMD_ERR(ecmd_ret_len)) {
	       	return;
	}

        TWIDEBUG("write_buffer:%s ecmd_ret_len:%d\n",write_buffer,ecmd_ret_len);

//	write_buffer[ecmd_ret_len++] = '\r';
//        write_buffer[ecmd_ret_len++] = '\n';
}

/* Interruptroutine des TWI
 */

ISR(TWI_vect)
{
//int i = 0;
  switch (TWSR & 0xF8)
  {
    case 0x60:	/* Own SLA+W has been received; ACK has been returned */
//	for (i = 0; i < sizeof recv_buffer; i++)   
//		recv_buffer[i]=0;

		sent_len=0;
		recv_len=0;
		twi_parse=0;
      break;
    case 0x80:	/* Previously addressed with own SLA+W; data has been received; ACK has been returned */
		if (recv_len < (sizeof(recv_buffer) - 1))
			recv_buffer[recv_len++] = TWDR;	

		if (TWDR == '\0') {
			//TWIDEBUG("2write_buffer:%s ecmd_ret_len:%d\n",write_buffer,ecmd_ret_len);
			//twi_parse_ecmd();
			twi_parse=1;
		 }

      break;
    case 0xA8:	/* Own SLA+R has been received; ACK has been returned */
		TWDR=write_buffer[sent_len++];
	break;
    case 0xB8:	/* Data byte in TWDR has been transmitted; ACK has been received */
		if (sent_len < ecmd_ret_len )
			TWDR=write_buffer[sent_len++];
		else {
			TWDR = twi_parse ? '\n': 0;
		//	ecmd_ret_len=0;
		}
      break;

    default:
      break;
  }
  TWCR |= (1 << TWINT);         //TWI wieder aktivieren
}

/*
  -- Ethersex META --
  header(hardware/i2c/slave/i2c_slave.h)
  init(twi_slave_init)
  timer(1,twi_slave_periodic())
*/

