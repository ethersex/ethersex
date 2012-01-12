/* {{{
 *
 * Copyright (c) 2008 by Christian Dietrich <stettberger@dokucode.de>
 *
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
}}} */

#include <avr/io.h>
#include <util/twi.h>

#include "config.h"
#include "core/debug.h"
#include "i2c_master.h"

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
// R�ckgabewert = TWI-Statusbits
uint8_t
i2c_master_do(uint8_t mode)
{
    TWCR = mode;                    // �bertragung starten
    loop_until_bit_is_set(TWCR,TWINT);
    return TW_STATUS;           // Returncode = Statusbits
}

/* Send an i2c stop condition */
void
i2c_master_stop(void)
{
    TWCR=((1<<TWEN)|(1<<TWINT)|(1<<TWSTO));     // Stopbedingung senden
    loop_until_bit_is_clear(TWCR, TWSTO);
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

/*
  -- Ethersex META --
  header(hardware/i2c/master/i2c_master.h)
  initearly(i2c_master_init)
*/
