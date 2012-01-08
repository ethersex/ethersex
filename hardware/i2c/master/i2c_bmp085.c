/*
 * Bosch Sensortec BMP085 barometric pressure sensor support
 * 
 * Copyright (c) 2012 by Gerd v. Egidy <gerd@egidy.de>
 *
 * Based on code by J.C. Wippler <jcw@equi4.com> 
 * see https://github.com/jcw/
 * http://opensource.org/licenses/mit-license.php
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
 * 
 */

#include <avr/io.h>
#include <util/twi.h>
#include <stdint.h>
#include <util/delay.h>
        
#include "config.h"
#include "core/debug.h"
#include "i2c_master.h"
#include "i2c_bmp085.h"

#ifdef I2C_BMP085_SUPPORT

#define BMP085_ADDRESS 0x77    // (0xEE >> 1)

static struct bmp085_cal cal;

int16_t bmp085_readRegister(uint8_t addr)
{
    uint16_t ret = 0xffff;
    
    if (!i2c_master_select(BMP085_ADDRESS, TW_WRITE))
    {
#ifdef DEBUG_I2C
        debug_printf("I2C: i2c_bmp085: error selecting for writing\n");
#endif
        goto end;
    }

    TWDR = addr;
    
    if (i2c_master_transmit() != TW_MT_DATA_ACK )
    {
#ifdef DEBUG_I2C
        debug_printf("I2C: i2c_bmp085: error sending register address\n");
#endif
        goto end;
    }

    if (!i2c_master_select(BMP085_ADDRESS, TW_READ))
    {
#ifdef DEBUG_I2C
        debug_printf("I2C: i2c_bmp085: error selecting for reading\n");
#endif
        goto end;
    }

    if (i2c_master_transmit_with_ack() != TW_MR_DATA_ACK)
    {
#ifdef DEBUG_I2C
        debug_printf("I2C: i2c_bmp085: error reading MSB\n");
#endif
        goto end;
    }

    // MSB first
    *((uint8_t*)(&ret)+1)=TWDR;

    if (i2c_master_transmit() != TW_MR_DATA_NACK)
    {
#ifdef DEBUG_I2C
        debug_printf("I2C: i2c_bmp085: error reading LSB\n");
#endif
        ret = 0xffff;
        goto end;
    }
    
    // then LSB
    *((uint8_t*)(&ret))=TWDR;
    
#ifdef DEBUG_I2C
    debug_printf("I2C: i2c_bmp085: read 0x%.4X from 0x%.2X\n",ret,addr);
#endif
   
end:
    i2c_master_stop();
    return ret;
}


uint8_t bmp085_readCal(uint8_t oss)
{
    uint8_t ret = 0xff;

    cal.initialized=0;
    
    if ((cal.ac1=bmp085_readRegister(0xAA))==0xFFFF)
        return 0xff;
    if ((cal.ac2=bmp085_readRegister(0xAC))==0xFFFF)
        return 0xff;
    if ((cal.ac3=bmp085_readRegister(0xAE))==0xFFFF)
        return 0xff;
    if ((cal.ac4=bmp085_readRegister(0xB0))==0xFFFF)
        return 0xff;
    if ((cal.ac5=bmp085_readRegister(0xB2))==0xFFFF)
        return 0xff;
    if ((cal.ac6=bmp085_readRegister(0xB4))==0xFFFF)
        return 0xff;
    if ((cal.b1=bmp085_readRegister(0xB6))==0xFFFF)
        return 0xff;
    if ((cal.b2=bmp085_readRegister(0xB8))==0xFFFF)
        return 0xff;
    if ((cal.mb=bmp085_readRegister(0xBA))==0xFFFF)
        return 0xff;
    if ((cal.mc=bmp085_readRegister(0xBC))==0xFFFF)
        return 0xff;
    if ((cal.md=bmp085_readRegister(0xBE))==0xFFFF)
        return 0xff;
    
    cal.oss=oss;
    cal.initialized=1;
    
    return ret;
}

uint8_t bmp085_startMeas(bmp085_meas_t type)
{
    uint8_t ret = 0xff;
    uint8_t cmd;
    
    if (!i2c_master_select(BMP085_ADDRESS, TW_WRITE))
    {
#ifdef DEBUG_I2C
        debug_printf("I2C: i2c_bmp085: error selecting for writing\n");
#endif
        goto end;
    }

    // control register
    TWDR = 0xF4;
    
    if (i2c_master_transmit() != TW_MT_DATA_ACK )
    {
#ifdef DEBUG_I2C
        debug_printf("I2C: i2c_bmp085: error sending register address\n");
#endif
        goto end;
    }

    // command
    cmd = (type == BMP085_TEMP ? 0x2E : 0x34 | (cal.oss << 6));
    TWDR = cmd;

    if (i2c_master_transmit() != TW_MT_DATA_ACK )
    {
#ifdef DEBUG_I2C
        debug_printf("I2C: i2c_bmp085: error sending command\n");
#endif
        goto end;
    }

#ifdef DEBUG_I2C
    debug_printf("I2C: i2c_bmp085: written 0x%.2X to control register 0xF4\n",cmd);
#endif

    ret=0;

end:
    i2c_master_stop();
    return ret;
}

int32_t bmp085_getPressure(void)
{
    return 1;
}

void bmp085_calc(int16_t ut, int32_t up, int16_t *tval, int32_t *pval)
{
    int32_t x1, x2, x3, b3, b5, b6, p;
    uint32_t b4, b7;

    x1 = ((int32_t)ut - cal.ac6) * cal.ac5 >> 15;
    x2 = ((int32_t) cal.mc << 11) / (x1 + cal.md);
    b5 = x1 + x2;
    *tval = (b5 + 8) >> 4;
    
    return;
}

void bmp085_init(void)
{
    int16_t ut, tval;
    int32_t up, pval;
    
    cal.initialized=0;
    
    _delay_ms(10);

    bmp085_readCal(3);

    bmp085_startMeas(BMP085_TEMP);

    _delay_us(get_bmp085_measure_us_delay(BMP085_TEMP,3));
    
    ut=bmp085_getTemp();
    
    bmp085_calc(ut,up,&tval,&pval);
    
    debug_printf("bmp085 temp: %d\n",tval);
}

/*
 -- Ethersex META --
 header(hardware/i2c/master/i2c_bmp085.h)
 init(bmp085_init)
 */

#endif /* I2C_BMP085_SUPPORT */
