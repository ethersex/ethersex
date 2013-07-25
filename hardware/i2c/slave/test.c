#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "config.h"
#include "core/global.h"
#include "protocols/ecmd/parser.h"
#include "protocols/ecmd/ecmd-base.h"

#include "i2c_slave.h"

static char rx_buffer[ECMD_TWI_BUFFER_LEN];
static char tx_buffer[ECMD_TWI_BUFFER_LEN];
static char i2c_debug_buffer[20];
static int8_t i2c_end=0;


static int16_t rx_len, tx_len, ecmd_len, debug_len, twi_parse;
static unsigned char TWI_state = 0x00;

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

//Status Codes for Slave Receiver Mode
#define TWI_SRX_ADR_ACK            0x60 //Own SLA+W has been received;ACK has been returned
#define TWI_SRX_ADR_DATA_ACK       0x80 //Previously addressed with own SLA+W; data has been received; ACK has been returned
#define TWI_SRX_STOP_RESTART       0xA0 //A STOP condition or repeated START condition has been received while still addressed as Slave

//Status Codes for Slave Transmitter Mode
#define TWI_STX_ADR_ACK            0xA8 //Own SLA+R has been received;ACK has been returned
#define TWI_STX_DATA_ACK   	   0xB8 //Data byte in TWDR has been transmitted; ACK has been received
#define TWI_STX_DATA_NACK 		   0xC0 //Data byte in TWDR has been transmitted; NOT ACK has been received


#define TWI_ACK (TWCR=(1<<TWINT)|(1<<TWEN)|(1<<TWEA))
#define TWI_NACK (TWCR=(1<<TWINT)|(1<<TWEN))

void
twi_periodic(void)
{

    if((TWSR & 0xF8) == 0x00)
		twi_init();


	if (!(TWCR & (1<<TWIE))) //wait until TWI interrupt is disabled
	{

		TWIDEBUG("restart\n");
			  TWCR = (1<<TWEN)|                                 // TWI Interface enabled
					 (1<<TWIE)|(1<<TWINT)|                      // Enable TWI Interupt and clear the flag to send byte
					 (1<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|           // Send ACK after next reception
					 (0<<TWWC);   
		
	}

	if (twi_parse) 
	{
		twi_parse=0;
		
		ecmd_len = ecmd_parse_command(rx_buffer, tx_buffer, sizeof(tx_buffer));
		tx_len=0;

		if (is_ECMD_AGAIN(ecmd_len)) {
		/* convert ECMD_AGAIN back to ECMD_FINAL */
			ecmd_len = ECMD_AGAIN(ecmd_len);
			twi_parse=1;
		}
		else if (is_ECMD_ERR(ecmd_len))
			return;
		
		TWIDEBUG("1 rx:%s, tx:%s, ecmd_len:%d,\n",rx_buffer,tx_buffer,ecmd_len);

	}
	
	if (i2c_end)
	{
		i2c_end=0;
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
	
	if (TWSR)
		i2c_debug_buffer[debug_len++]=TWSR; 
	
	switch (TWSR & 0xF8)
	{
	
	//slave receiver		
		case TWI_SRX_ADR_ACK : 				//0x60 //Own SLA+W has been received;ACK has been returned
		  rx_len=0;
		  twi_parse=1;
		  TWCR = (1<<TWEN)|                                 // TWI Interface enabled
			     (1<<TWIE)|(1<<TWINT)|                      // Enable TWI Interupt and clear the flag to send byte
			     (1<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|           // Send ACK after next reception
			     (0<<TWWC);   
		break;

		case TWI_SRX_ADR_DATA_ACK :			//0x80 //Previously addressed with own SLA+W; data has been received; ACK has been returned
          rx_buffer[rx_len++] = TWDR;
		  TWCR = (1<<TWEN)|                                 // TWI Interface enabled
			     (1<<TWIE)|(1<<TWINT)|                      // Enable TWI Interupt and clear the flag to send byte
			     (1<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|           // Send ACK after next reception
			     (0<<TWWC);   			
		break;
		
	//slave transmitter
		case TWI_STX_ADR_ACK:				//0xA8 //Own SLA+R has been received;ACK has been returned
			tx_len=0;
		    TWDR=tx_buffer[tx_len++];
		  TWCR = (1<<TWEN)|                                 // TWI Interface enabled
			     (1<<TWIE)|(1<<TWINT)|                      // Enable TWI Interupt and clear the flag to send byte
			     (1<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|           // Send ACK after next reception
			     (0<<TWWC);   
			break;
		case TWI_STX_DATA_ACK :				//0xB8 //Data byte in TWDR has been transmitted; ACK has been received
			TWDR=tx_buffer[tx_len++];

			if (tx_buffer[tx_len+1] == '\0') 
			{
				twi_parse=1;
			}
			
			TWCR = (1<<TWEN)|                                 // TWI Interface enabled
			     (1<<TWIE)|(1<<TWINT)|                      // Enable TWI Interupt and clear the flag to send byte
			     (1<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|           // Send ACK after next reception
			     (0<<TWWC);   			
		
		break;		
    	
		case TWI_STX_DATA_NACK:          //0xC0   Data byte in TWDR has been transmitted; NACK has been received. 		
			i2c_end=1;
 		  TWCR = (1<<TWEN)|                                 // TWI Interface enabled
			     (1<<TWIE)|(1<<TWINT)|                      // Enable TWI Interupt and clear the flag to send byte
			     (1<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|           // Send ACK after next reception
			     (0<<TWWC);   
		break;
		
		case TWI_SRX_STOP_RESTART:       //0xA0   A STOP condition or repeated START condition has been received while still addressed as Slave    
		  /*TWCR = (1<<TWEN)|                                 // TWI Interface enabled
			     (1<<TWIE)|(1<<TWINT)|                      // Enable TWI Interupt and clear the flag to send byte
			     (1<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|           // Send ACK after next reception
			     (0<<TWWC);   		
			*/
		  TWCR = (1<<TWEN)|                                 // Enable TWI-interface and release TWI pins
				 (0<<TWIE)|(0<<TWINT)|                      // Disable Interupt
				 (0<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|           // Do not acknowledge on any new requests.
				 (0<<TWWC);                                 //
				
			
        break; 
	 
	}

	
	
}


/*
  -- Ethersex META --
  header(hardware/i2c/slave/i2c_slave.h)
  init(twi_init)
  timer(1, twi_periodic())
*/




