/*
 *
 * Copyright (c) 2008,2009 Christian Dietrich <stettberger@dokucode.de>
 * Copyright (c) 2013      Erik Kunze <ethersex@erik-kunze.de>
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
#include <util/twi.h>

#include "config.h"
#include "core/debug.h"
#include "core/bit-macros.h"
#include "i2c_master.h"
#include "i2c_24CXX.h"

static uint8_t i2c_24cxx_address;

void
i2c_24CXX_init(void)
{
  i2c_24cxx_address = i2c_master_detect(I2C_SLA_24CXX, I2C_SLA_24CXX + 8);
#ifdef DEBUG_I2C
  debug_printf("i2c eeprom address: %i\n", i2c_24cxx_address);
#endif
}

uint8_t
i2c_24CXX_set_addr(uint16_t addr)
{
  uint8_t ret;

  if (!i2c_master_select(i2c_24cxx_address, TW_WRITE))
  {
    ret = 0;
    goto end;
  }

  TWDR = HI8(addr);
  if (i2c_master_transmit() != TW_MT_DATA_ACK)
  {
    ret = 0;
    goto end;
  }
  TWDR = LO8(addr);
  if (i2c_master_transmit() != TW_MT_DATA_ACK)
  {
    ret = 0;
    goto end;
  }

  ret = 1;
end:
  return ret;
}

uint8_t
i2c_24CXX_read_block(uint16_t addr, uint8_t * ptr, uint8_t len)
{
  uint8_t ret;
#ifdef DEBUG_I2C
  debug_printf("read %i bytes at address: %i\n", len, addr);
#endif

  if (!i2c_24CXX_set_addr(addr))
  {
    ret = 0;
    goto end;
  }

  /* Do an repeated start condition */
  if (i2c_master_start() != TW_REP_START)
  {
    ret = 0;
    goto end;
  }

  /* Send the address again */
  TWDR = (i2c_24cxx_address << 1) | TW_READ;
  if (i2c_master_transmit() != TW_MR_SLA_ACK)
  {
    ret = 0;
    goto end;
  }

  for (ret = 0; ret < (len - 1); ret++)
  {
    /* Recv one byte and ack */
    if (i2c_master_transmit_with_ack() != TW_MR_DATA_ACK)
    {
      goto end;
    }
    ptr[ret] = TWDR;
#ifdef DEBUG_I2C
    debug_printf("0x%02x, ", ptr[ret]);
#endif
  }
  /* recv one byte and do not ack */
  if (i2c_master_transmit() != TW_MR_DATA_NACK)
  {
    goto end;
  }
  ptr[ret++] = TWDR;
#ifdef DEBUG_I2C
  debug_printf("0x%02x\n", ptr[ret - 1]);
#endif
end:
  i2c_master_stop();
  return ret;

}

uint8_t
i2c_24CXX_write_block_int(uint16_t addr, uint8_t * ptr, uint8_t len)
{
  uint8_t ret;

  if (!i2c_24CXX_set_addr(addr))
  {
    ret = 0;
    goto end;
  }

  for (ret = 0; ret < len; ret++)
  {
#ifdef DEBUG_I2C
    debug_printf("%i, ", ptr[ret]);
#endif
    TWDR = ptr[ret];
    if (i2c_master_transmit() != TW_MT_DATA_ACK)
    {
      ret = 0;
      goto end;
    }
  }
#ifdef DEBUG_I2C
  debug_printf("\n");
#endif
end:
  TWCR = _BV(TWEN) | _BV(TWINT) | _BV(TWSTO);   // Stopbedingung senden
  loop_until_bit_is_set(TWCR, TWSTO);           // warten bis TWI fertig

  /* Here we start to do the polling of the write cycle */
  uint16_t polls = 500;
  while (polls--)
  {
    if (i2c_master_select(i2c_24cxx_address, TW_WRITE))
    {
      break;
    }
  }

#ifdef DEBUG_I2C
  if (!polls)
  {
    debug_printf("NOT WRITTEN!!\n");
  }
#endif

  i2c_master_stop();
  return ret;
}

uint8_t
i2c_24CXX_write_block(uint16_t addr, uint8_t * ptr, uint8_t len)
{
  uint8_t ret;
  uint8_t templen;
  uint8_t writelen;
#ifdef DEBUG_I2C
  debug_printf("write %i bytes at address: %i\n", len, addr);
#endif

  ret = 0;
  writelen = 0;
  do
  {
    if (CONF_I2C_24CXX_PAGESIZE -
        (addr + writelen) % CONF_I2C_24CXX_PAGESIZE < (len - writelen))
      templen =
        (CONF_I2C_24CXX_PAGESIZE -
         (addr + writelen) % CONF_I2C_24CXX_PAGESIZE);
    else
      templen = len - writelen;
    ret += i2c_24CXX_write_block_int(addr + writelen, ptr + writelen,
                                     templen);
    writelen += templen;
  }
  while ((writelen < len) && (ret == writelen));
  return ret;
}

uint8_t
i2c_24CXX_write_byte(uint16_t addr, uint8_t data)
{
  return i2c_24CXX_write_block(addr, &data, 1);
}

uint8_t
i2c_24CXX_compare_block(uint16_t addr, uint8_t * ptr, uint8_t len)
{
  uint8_t ret;

  if (!i2c_24CXX_set_addr(addr))
  {
    ret = 0;
    goto end;
  }

  /* Do an repeated start condition */
  if (i2c_master_start() != TW_REP_START)
  {
    ret = 0;
    goto end;
  }

  /* Send the address again */
  TWDR = (i2c_24cxx_address << 1) | TW_READ;
  if (i2c_master_transmit() != TW_MR_SLA_ACK)
  {
    ret = 0;
    goto end;
  }

  for (ret = 0; ret < (len - 1); ret++)
  {
    /* Recv one byte and ack */
    if (i2c_master_transmit_with_ack() != TW_MR_DATA_ACK)
    {
      ret = 0;
      goto end;
    }
    if (ptr[ret] != TWDR)
    {
      ret = 0;
      goto end;
    }
  }

  /* recv one byte and do not ack */
  if (i2c_master_transmit() != TW_MR_DATA_NACK)
  {
    goto end;
  }
  if (ptr[ret] != TWDR)
  {
    ret = 0;
    goto end;
  }

  ret = 1;

end:
  i2c_master_stop();
  return ret;
}

/*
  -- Ethersex META --
  header(hardware/i2c/master/i2c_24CXX.h)
  initearly(i2c_24CXX_init)
*/
