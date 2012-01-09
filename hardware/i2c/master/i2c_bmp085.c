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
#include <math.h>
        
#include "config.h"
#include "core/debug.h"
#include "i2c_master.h"
#include "i2c_bmp085.h"

#ifdef I2C_BMP085_SUPPORT

#define BMP085_ADDRESS 0x77    // (0xEE >> 1)

static struct bmp085_cal cal;

// attention: reads reverse into the buffer:
// MSB is read first, on the atmega with gcc the MSB is last
uint8_t bmp085_read(uint8_t regaddr, uint8_t bytes, void* buffer)
{
    uint8_t ret = 0xff;
    
    if (!i2c_master_select(BMP085_ADDRESS, TW_WRITE))
    {
#ifdef DEBUG_I2C
        debug_printf("I2C: i2c_bmp085: error selecting for writing\n");
#endif
        goto end;
    }

    TWDR = regaddr;
    
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

    while (bytes-- > 0)
    {
        uint8_t res;

        if (bytes > 0)
            res=(i2c_master_transmit_with_ack() != TW_MR_DATA_ACK);
        else
            res=(i2c_master_transmit() != TW_MR_DATA_NACK);
        
        if (res)
        {
    #ifdef DEBUG_I2C
            debug_printf("I2C: i2c_bmp085: error reading byte %d\n",bytes);
    #endif
            goto end;
        }

        *((unsigned char*)buffer+bytes)=TWDR;

    #ifdef DEBUG_I2C
        debug_printf("I2C: i2c_bmp085: read 0x%.2X at addr 0x%.2X\n",TWDR,regaddr++);
    #endif
    }

    // success
    ret = 0;
    
end:
    i2c_master_stop();
    return ret;
}


uint8_t bmp085_readCal(uint8_t oss)
{
    cal.initialized=0;
    
    if (bmp085_read(0xAA,2,&cal.ac1)!=0)
        return 0xff;
    if (bmp085_read(0xAC,2,&cal.ac2)!=0)
        return 0xff;
    if (bmp085_read(0xAE,2,&cal.ac3)!=0)
        return 0xff;
    if (bmp085_read(0xB0,2,&cal.ac4)!=0)
        return 0xff;
    if (bmp085_read(0xB2,2,&cal.ac5)!=0)
        return 0xff;
    if (bmp085_read(0xB4,2,&cal.ac6)!=0)
        return 0xff;
    if (bmp085_read(0xB6,2,&cal.b1)!=0)
        return 0xff;
    if (bmp085_read(0xB8,2,&cal.b2)!=0)
        return 0xff;
    if (bmp085_read(0xBA,2,&cal.mb)!=0)
        return 0xff;
    if (bmp085_read(0xBC,2,&cal.mc)!=0)
        return 0xff;
    if (bmp085_read(0xBE,2,&cal.md)!=0)
        return 0xff;

/*
    // example values from the datasheet
    cal.ac1 = 408;
    cal.ac2 = -72;
    cal.ac3 = -14383;
    cal.ac4 = 32741;
    cal.ac5 = 32757;
    cal.ac6 = 23153;
    cal.b1 = 6190;
    cal.b2 = 4;
    cal.mb = -32767;
    cal.mc = -8711;
    cal.md = 2868;
*/

    cal.oss=oss;
    cal.initialized=1;
    
    return 0;
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
    cmd = (type == BMP085_TEMP ? 0x2E : (0x34 | (cal.oss << 6)));
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

void bmp085_calc(int16_t ut, int32_t up, int16_t *tval, int32_t *pval)
{
    int32_t x1, x2, x3, b3, b5, b6, p;
    uint32_t b4, b7;

    x1 = ((int32_t)ut - cal.ac6) * cal.ac5 >> 15;
    x2 = ((int32_t) cal.mc << 11) / (x1 + cal.md);
    b5 = x1 + x2;
    *tval = (b5 + 8) >> 4;

    b6 = b5 - 4000;
    x1 = (b6*b6) >> 12;
    x1 *= cal.b2;
    x1 >>=11;

    x2 = cal.ac2*b6;
    x2 >>=11;

    x3 = x1 + x2;

    b3 = (((((int32_t)cal.ac1 )*4 + x3) << cal.oss) + 2) >> 2;

    x1 = (cal.ac3* b6) >> 13;
    x2 = (cal.b1 * ((b6*b6) >> 12) ) >> 16;
    x3 = ((x1 + x2) + 2) >> 2;

    b4 = (cal.ac4 * (uint32_t) (x3 + 32768)) >> 15;

    b7 = ((uint32_t)(up - b3) * (50000>>cal.oss));
    if (b7 < 0x80000000)
        p = (b7 << 1) / b4;
    else
        p = (b7 / b4) << 1;

    x1 = p >> 8;
    x1 *= x1;
    x1 = (x1 * 3038) >> 16;
    x2 = (p * -7357) >> 16;
    *pval = p + ((x1 + x2 + 3791) >> 4);    // pressure in Pa
    
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
    
    if(bmp085_read(0xF6,2,&ut)!=0)
        debug_printf("bmp085 read error\n");
    
    bmp085_startMeas(BMP085_PRES);

    _delay_us(get_bmp085_measure_us_delay(BMP085_PRES,3));
    
    up=0;
    if(bmp085_read(0xF6,3,&up)!=0)
        debug_printf("bmp085 read error\n");
    up >>= 8-cal.oss;

    bmp085_calc(ut,up,&tval,&pval);

    debug_printf("bmp085 temp: %d\n",tval);
    debug_printf("bmp085 press Pa: %ld\n",pval);
}

/*
 -- Ethersex META --
 header(hardware/i2c/master/i2c_bmp085.h)
 init(bmp085_init)
 */

#endif /* I2C_BMP085_SUPPORT */
