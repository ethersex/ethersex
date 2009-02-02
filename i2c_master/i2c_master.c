
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/twi.h>
        
#include "../config.h"
#include "../debug.h"
#include "i2c_master.h"

#ifdef I2C_MASTER_SUPPORT

void 
i2c_master_init(void)
{
    TWSR = 0;       // Vorteiler  Bitrate = 0
    TWBR = ((F_CPU / 1000) / CONF_I2C_BAUD - 16) / 2;  // Frequenz setzen
    /* Enable the i2c pullups */
    PIN_SET(SDA);
    PIN_SET(SCL);
}

uint8_t
i2c_master_detect(uint8_t range_start, uint8_t range_end)
{
  uint8_t i;
  uint8_t i2c_address = 0xff;

#ifdef DEBUG_I2C
  debug_printf("I2C: test for base address %x until %x\n", range_start, range_end);
#endif
  for (i = range_start; i < range_end; i++) {
    if (i2c_master_select(i, TW_WRITE)) {
      i2c_address = i;
#ifdef DEBUG_I2C
      debug_printf("I2C: detected at: %X\n", i2c_address);
#endif
      i2c_master_stop();
      break;
    }
    i2c_master_stop();
  }
  return i2c_address;
}

// Wartet bis TWI-Operstion beendet ist
// Rückgabewert = TWI-Statusbits
uint8_t
i2c_master_do(uint8_t mode)
{
    TWCR = mode;                    // Übertragung starten 
    while(!( TWCR & (1<<TWINT)));   // warten bis Byte übertragen ist
    return TW_STATUS;           // Returncode = Statusbits 
}

/* Send an i2c stop condition */
void 
i2c_master_stop(void)
{
    TWCR=((1<<TWEN)|(1<<TWINT)|(1<<TWSTO));     // Stopbedingung senden
    while (!(TWCR & (1<<TWSTO)));               // warten bis TWI fertig
    i2c_master_disable(); 
}

/* failure, if return == 0 */
uint8_t
i2c_master_select(uint8_t address, uint8_t mode)
{
  i2c_master_enable();
  #ifdef DEBUG_I2C
    debug_printf("i2c master select adr+mode 0x%X\n", (address << 1) | mode);
  #endif

    /* Start Condition, test on start or repeated start */
    uint8_t tmp = i2c_master_start();
    if (tmp != TW_START && tmp != TW_REP_START) return 0; 
    /* address chip */
    TWDR = (address << 1) | mode;
    tmp = i2c_master_do ((1<<TWINT)|(1<<TWEN));
    if ((mode == TW_WRITE && tmp == TW_MT_SLA_ACK) 
       || (mode == TW_READ &&  tmp == TW_MR_SLA_ACK))
      return 1; 
    return 0; 
}

#endif /* I2C_MASTER_SUPPORT */


