#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "config.h"
#include "core/global.h"

#include "protocols/ecmd/parser.h"
#include "protocols/ecmd/ecmd-base.h"

#include "i2c_slave.h"

static char cmd_rx_buffer[TWI_BUFFER_LEN];
static char cmd_tx_buffer[TWI_BUFFER_LEN];
static int16_t cmd_rx_len,ecmd_len;

void
ecmd_twi_periodic(void)
{
	if (!twi_busy()) //wait until TWI interrupt is disabled
	{
		TWIDEBUG("ecmd_twi_periodic\n");		
		
		cmd_rx_len=twi_rx_len();
		if (cmd_rx_len<=2) //assume its raw
			parse_rawdata_twi_slave();
		else 
		{
			parse_ecmd_twi_slave();	
		}
		TWCR = 	(1<<TWEN)|                                 // TWI Interface enabled
				(1<<TWIE)|(1<<TWINT)|                      // Enable TWI Interupt and clear the flag to send byte
				(1<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|           // Send ACK after next reception
				(0<<TWWC);  		
	}	   
}

int16_t parse_ecmd_twi_slave (void)
{
	TWIDEBUG("parse_ecmd_twi_slave\n");
	twi_get_rx_data(cmd_rx_buffer);
	ecmd_len = ecmd_parse_command(cmd_rx_buffer, cmd_tx_buffer, sizeof(cmd_tx_buffer));
	twi_set_tx_data(cmd_tx_buffer);		

	/*dont know how to return multiple lines in tx_buffer*/
	//if (is_ECMD_AGAIN(ecmd_len)) {
	  /* convert ECMD_AGAIN back to ECMD_FINAL */
	//ecmd_len = ECMD_AGAIN(ecmd_len);
	//}
	//else if (is_ECMD_ERR(ecmd_len))
	  // return;
	return ECMD_FINAL_OK;
}

/*
  -- Ethersex META --
  header(hardware/i2c/slave/i2c_slave.h)  
  timer(1, ecmd_twi_periodic())
*/
