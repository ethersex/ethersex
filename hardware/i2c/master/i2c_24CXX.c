/*
 *
 * Copyright (c) 2008,2009 by Christian Dietrich <stettberger@dokucode.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
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

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/twi.h>
        
#include "config.h"
#include "core/debug.h"
#include "i2c_master.h"
#include "i2c_24CXX.h"

static uint8_t i2c_24cxx_address;

void 
i2c_24CXX_init(void)
{
  i2c_24cxx_address = i2c_master_detect(I2C_SLA_24CXX, I2C_SLA_24CXX + 8);
}

uint8_t 
i2c_24CXX_set_addr(uint16_t addr)
{
  uint8_t ret;

  if (! i2c_master_select(i2c_24cxx_address, TW_WRITE)) { ret = 0; goto end; }

  TWDR = (addr >> 8) & 0xff;
  if (i2c_master_transmit() != TW_MT_DATA_ACK) { ret = 0; goto end; }
  TWDR = addr & 0xff;
  if (i2c_master_transmit() != TW_MT_DATA_ACK) { ret = 0; goto end; } 

  ret = 1;
end:
  return ret;
}

uint8_t 
i2c_24CXX_read_block(uint16_t addr, uint8_t *ptr, uint8_t len) 
{
  uint8_t ret;

  if (!i2c_24CXX_set_addr(addr)) {ret = 0; goto end; }

  /* Do an repeated start condition */
  if (i2c_master_start() != TW_REP_START) {ret = 0; goto end; }

  /* Send the address again */
  TWDR = (i2c_24cxx_address << 1) | TW_READ;
  if(i2c_master_transmit() != TW_MR_SLA_ACK) {ret = 0; goto end; }

  for (ret = 0; ret < ( len - 1) ; ret ++) {
    /* Recv one byte and ack */
    if (i2c_master_transmit_with_ack() != TW_MR_DATA_ACK) {goto end; }
    ptr[ret] = TWDR;
  }
  /* recv one byte and do not ack */
  if (i2c_master_transmit() != TW_MR_DATA_NACK) {goto end; }
  ptr[ret++] = TWDR;

end:
  i2c_master_stop();
  return ret;

}

uint8_t 
i2c_24CXX_write_block(uint16_t addr, uint8_t *ptr, uint8_t len)
{
  uint8_t ret;
  if (!i2c_24CXX_set_addr(addr)) { ret = 0; goto end; }

  for (ret = 0; ret < len; ret++) {
    TWDR = ptr[ret];
    if (i2c_master_transmit() != TW_MT_DATA_ACK) {ret = 0; goto end; }
  }

end:
  TWCR=((1<<TWEN)|(1<<TWINT)|(1<<TWSTO));     // Stopbedingung senden
  while (!(TWCR & (1<<TWSTO)));               // warten bis TWI fertig

  /* Here we start to do the polling of the write cycle */
  uint16_t polls = 500;
  while (polls--) {
    if (i2c_master_select(i2c_24cxx_address, TW_WRITE)) {
      break;
    }
  }

  i2c_master_stop();
  return ret;
}

uint8_t 
i2c_24CXX_write_byte(uint16_t addr, uint8_t data)
{
  return i2c_24CXX_write_block(addr, &data, 1);
}

uint8_t 
i2c_24CXX_compare_block(uint16_t addr, uint8_t *ptr, uint8_t len) 
{
  uint8_t ret;

  if (!i2c_24CXX_set_addr(addr)) {ret = 0; goto end; }

  /* Do an repeated start condition */
  if (i2c_master_start() != TW_REP_START) {ret = 0; goto end; }

  /* Send the address again */
  TWDR = (i2c_24cxx_address << 1) | TW_READ;
  if(i2c_master_transmit() != TW_MR_SLA_ACK) {ret = 0; goto end; }

  for (ret = 0; ret < ( len - 1) ; ret ++) {
    /* Recv one byte and ack */
    if (i2c_master_transmit_with_ack() != TW_MR_DATA_ACK) { ret = 0; goto end; }
    if (ptr[ret] != TWDR) {ret = 0; goto end;}
  }

  /* recv one byte and do not ack */
  if (i2c_master_transmit() != TW_MR_DATA_NACK) {goto end; }
  if (ptr[ret] != TWDR) {ret = 0; goto end;}

  ret = 1;

end:
  i2c_master_stop();
  return ret;
}

/*
  -- Ethersex META --
  init(i2c_24CXX_init)
*/
