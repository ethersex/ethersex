#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "config.h"
#include "core/global.h"
#include "protocols/ecmd/parser.h"
#include "protocols/ecmd/ecmd-base.h"

#include "i2c_slave.h"

static char test_buffer[ECMD_TWI_BUFFER_LEN];
static char rx_buffer[ECMD_TWI_BUFFER_LEN];
static char tx_buffer[ECMD_TWI_BUFFER_LEN];
static char i2c_debug_buffer[20];

static int16_t rx_len, tx_len, ecmd_len, twi_parse;

static int16_t debug_len;
static int8_t i2c_debug;



void
twi_init(void){
  /*fuer das Initialisieren bei einem status fehler*/
  TWCR = 0;

  /* INIT fuer den TWI i2c
   * hier wird die Addresse des ï¿½C festgelegt
   * (in den oberen 7 Bit, das LSB(niederwertigstes Bit)
   * steht dafï¿½r ob der ï¿½C auf einen general callreagiert
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
twi_periodic(void)
{
    if((TWSR & 0xF8) == 0x00)
		twi_init();


	if (twi_parse == 1)
	{
		TWIDEBUG("parse data\n");
		twi_parse=0;
		TWIDEBUG("1rx:%s,tx:%s,\n",rx_buffer,tx_buffer);
		//if(rx_len <= 1) return;
		
		ecmd_len = ecmd_parse_command(rx_buffer, tx_buffer, sizeof(tx_buffer));
		TWIDEBUG("2rx:%s,tx:%s,\n",rx_buffer,tx_buffer);
	   if (is_ECMD_AGAIN(ecmd_len)) {
		  /* convert ECMD_AGAIN back to ECMD_FINAL */
		  ecmd_len = ECMD_AGAIN(ecmd_len);
		}
		else if (is_ECMD_ERR(ecmd_len))
		   return;

		//twi_parse = 2;
		/*dont know how to return multiple lines in tx_buffer*/
		
		//twi_parse = 0;
		//if (is_ECMD_AGAIN(ecmd_len)) {
		  /* convert ECMD_AGAIN back to ECMD_FINAL */
		  //ecmd_len = ECMD_AGAIN(ecmd_len);
		  //parse = 1;
		//}
		//else if (is_ECMD_ERR(ecmd_len))
		//{
		//   twi_parse = 0;
		//   return;		
		//}
		//twi_parse = 2;
	}	
	
	
	if (i2c_debug)
	{
		i2c_debug=0;
		int i;
		TWIDEBUG("i2c_debug_buffer:\n");
		for (i = 0; i < debug_len; i++)
		{			
			TWIDEBUG("%d %02X\n",i, i2c_debug_buffer[i]);
		}	
		 debug_len=0;
	}
	
}

/* twi interrupt */
ISR (TWI_vect)
{
	//if (TWSR)
		//i2c_debug_buffer[debug_len++]=TWSR; 
	
	switch (TWSR & 0xF8)
	{	
	//slave receiver		
		case TWI_SRX_ADR_ACK : 				//0x60 //Own SLA+W has been received;ACK has been returned	      
	      rx_len=0;		  
		  //ecmd_len=0;
		  
		  TWCR = (1<<TWEN)|                                 // TWI Interface enabled
			     (1<<TWIE)|(1<<TWINT)|                      // Enable TWI Interupt and clear the flag to send byte
			     (1<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|           // Send ACK after next reception
			     (0<<TWWC);   
		break;

		case TWI_SRX_ADR_DATA_ACK :			//0x80 //Previously addressed with own SLA+W; data has been received; ACK has been returned
          if (rx_len < (sizeof(rx_buffer) - 1))
			rx_buffer[rx_len++] = TWDR;
		  
		  if ( rx_buffer[rx_len-1] == '\0') 
			twi_parse=1;

		  TWCR = (1<<TWEN)|                                 // TWI Interface enabled
			     (1<<TWIE)|(1<<TWINT)|                      // Enable TWI Interupt and clear the flag to send byte
			     (1<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|           // Send ACK after next reception
			     (0<<TWWC);   			
		break;

		case TWI_SRX_STOP_RESTART:       //0xA0   A STOP condition or repeated START condition has been received while still addressed as Slave    
		  TWCR = (1<<TWEN)|                                 // TWI Interface enabled
			     (1<<TWIE)|(1<<TWINT)|                      // Enable TWI Interupt and clear the flag to send byte
			     (1<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|           // Send ACK after next reception
			     (0<<TWWC);   		
        break; 			

		//slave transmitter
		case TWI_STX_ADR_ACK:				//0xA8 //Own SLA+R has been received;ACK has been returned
			tx_len=0;
		    //if (twi_parse == 2)
			TWDR=tx_buffer[tx_len++];

		  TWCR = (1<<TWEN)|                                 // TWI Interface enabled
			     (1<<TWIE)|(1<<TWINT)|                      // Enable TWI Interupt and clear the flag to send byte
			     (1<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|           // Send ACK after next reception
			     (0<<TWWC);   
			break;
		case TWI_STX_DATA_ACK :				//0xB8 //Data byte in TWDR has been transmitted; ACK has been received
			if (tx_len < ecmd_len) 
				TWDR=tx_buffer[tx_len++];
			else
				TWDR= twi_parse ? '\n': 0;
			
 		  TWCR = (1<<TWEN)|                                 // TWI Interface enabled
			     (1<<TWIE)|(1<<TWINT)|                      // Enable TWI Interupt and clear the flag to send byte
			     (1<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|           // Send ACK after next reception
			     (0<<TWWC);   						
		break;		
    	
		case TWI_STX_DATA_NACK:          //0xC0   Data byte in TWDR has been transmitted; NACK has been received. 		
		  //i2c_debug=1;	
 		  TWCR = (1<<TWEN)|                                 // TWI Interface enabled
			     (1<<TWIE)|(1<<TWINT)|                      // Enable TWI Interupt and clear the flag to send byte
			     (1<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|           // Send ACK after next reception
			     (0<<TWWC);   
		break;
		
 
		//default :
		/*		  TWCR = (1<<TWEN)|                                 // TWI Interface enabled
			     (1<<TWIE)|(1<<TWINT)|                      // Enable TWI Interupt and clear the flag to send byte
			     (1<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|           // Send ACK after next reception
			     (0<<TWWC); 
		*/		 
	}
}

/*
  -- Ethersex META --
  header(hardware/i2c/slave/i2c_slave.h)
  init(twi_init)
  timer(1, twi_periodic())
*/