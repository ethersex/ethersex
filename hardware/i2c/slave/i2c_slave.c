#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "config.h"
#include "core/global.h"

#include "i2c_slave.h"

static char rx_buffer[TWI_BUFFER_LEN];
static char tx_buffer[TWI_BUFFER_LEN];
static int16_t rx_len, tx_len,tx_total_len, twi_parse;

void
twi_init(void){
  /*fuer das Initialisieren bei einem status fehler*/
  TWCR = 0;

  /* INIT fuer den TWI i2c
   * hier wird die Addresse des ï¿½C festgelegt
   * (in den oberen 7 Bit, das LSB(niederwertigstes Bit)
   * steht dafï¿½r ob der ï¿½C auf einen general callreagiert
   */
   
  TWAR = TWI_ADDR << 1;

  /* TWI Control Register, hier wird der TWI aktiviert,
   * der Interrupt aktiviert und solche Sachen
   */
  TWCR = (1 << TWIE) | (1 << TWEN) | (1 << TWEA);

  /* TWI Status Register init */
  TWSR &= 0xFC;
}

unsigned char 
twi_busy( void )
{
	return ( TWCR & (1<<TWIE) );  
}

#ifdef ECMD_TWI_SLAVE_SUPPORT
int16_t 
twi_get_rx_data(char *cmd, int16_t len)
{
	int16_t i;
	len=rx_len;
	TWIDEBUG("len %d\n",len);
	for (i=0; i<rx_len; i++)
	{
		cmd[i] = rx_buffer[i];
	}
	TWIDEBUG("rx_buffer:%s cmd:%s\n",rx_buffer,cmd);
	return len;	
}

void 
twi_set_tx_data(char *cmd, int16_t len)
{
	int16_t i;
	tx_total_len = len;
	
	for (i=0; i<tx_total_len; i++)
	{
		tx_buffer[i] = cmd[i];
		
	}
	TWIDEBUG("tx_buffer:%s cmd:%s\n",tx_buffer,cmd);
}
#endif

void
parse_rawdata_twi_slave(void)
{
		if (rx_len)
		{
			TWIDEBUG("parse_rawdata_twi_slave\n");
			//i2cset -y 1 0x04 0x01 0x02
			if (rx_buffer[0] == 0x01)
				if (rx_buffer[1] == 0x02)
					status.request_wdreset =1;
				
			if (rx_buffer[0] == 0x06)
				status.request_reset = 1;	

			if (rx_buffer[0] == 0x07)
			{
				tx_buffer[0] = 0x67;
				tx_total_len=1;
			}
		}
}

void
twi_periodic(void)
{
    if((TWSR & 0xF8) == 0x00)
		twi_init();

	#ifndef ECMD_TWI_SLAVE_SUPPORT
	if (!twi_busy()) //wait until TWI interrupt is disabled
	{
		parse_rawdata_twi_slave();
	
		TWCR = 	(1<<TWEN)|                                 // TWI Interface enabled
				(1<<TWIE)|(1<<TWINT)|                      // Enable TWI Interupt and clear the flag to send byte
				(1<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|           // Send ACK after next reception
				(0<<TWWC);   
	}
	#endif
}

/* twi interrupt */
ISR (TWI_vect)
{
	switch (TWSR & 0xF8)
	{	
	//slave receiver		
		case TWI_SRX_ADR_ACK : 								//0x60 //Own SLA+W has been received;ACK has been returned	      
	      rx_len=0;		  
		  
		  TWCR = (1<<TWEN)|                                 // TWI Interface enabled
			     (1<<TWIE)|(1<<TWINT)|                      // Enable TWI Interupt and clear the flag to send byte
			     (1<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|           // Send ACK after next reception
			     (0<<TWWC);   
		break;
		case TWI_SRX_ADR_DATA_ACK :							//0x80 //Previously addressed with own SLA+W; data has been received; ACK has been returned
          if (rx_len < (sizeof(rx_buffer) - 1))
			rx_buffer[rx_len++] = TWDR;

		  TWCR = (1<<TWEN)|                                 // TWI Interface enabled
			     (1<<TWIE)|(1<<TWINT)|                      // Enable TWI Interupt and clear the flag to send byte
			     (1<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|           // Send ACK after next reception
			     (0<<TWWC);   			
		break;
		case TWI_SRX_STOP_RESTART:							//0xA0   A STOP condition or repeated START condition has been received while still addressed as Slave    
			TWCR = (1<<TWEN)|                               // Enable TWI-interface and release TWI pins
					(0<<TWIE)|(0<<TWINT)|                      // Disable Interupt
					(0<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|           // Do not acknowledge on any new requests.
					(0<<TWWC);                                 //			
        break; 			

	//slave transmitter
		case TWI_STX_ADR_ACK:								//0xA8 //Own SLA+R has been received;ACK has been returned
			tx_len=0;
			TWDR=tx_buffer[tx_len++];

			TWCR = (1<<TWEN)|                                 // TWI Interface enabled
					(1<<TWIE)|(1<<TWINT)|                      // Enable TWI Interupt and clear the flag to send byte
					(1<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|           // Send ACK after next reception
					(0<<TWWC);   
			break;
		case TWI_STX_DATA_ACK :								//0xB8 //Data byte in TWDR has been transmitted; ACK has been received
			if (tx_len < tx_total_len) 
				TWDR=tx_buffer[tx_len++];
			else
				TWDR= twi_parse ? '\n': 0;
			
			TWCR = (1<<TWEN)|                                 // TWI Interface enabled
					(1<<TWIE)|(1<<TWINT)|                      // Enable TWI Interupt and clear the flag to send byte
					(1<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|           // Send ACK after next reception
					(0<<TWWC);   						
		break;		   	
		case TWI_STX_DATA_NACK:          					//0xC0   Data byte in TWDR has been transmitted; NACK has been received. 		
			TWCR = (1<<TWEN)|                                 // TWI Interface enabled
					(1<<TWIE)|(1<<TWINT)|                      // Enable TWI Interupt and clear the flag to send byte
					(1<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|           // Send ACK after next reception
					(0<<TWWC);   
		break;	 
		
		//case TWI_BUS_ERROR:
		//need to fix 		
	}
}

/*
  -- Ethersex META --
  header(hardware/i2c/slave/i2c_slave.h)
  init(twi_init)
  timer(1, twi_periodic())
*/
