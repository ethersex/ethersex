/*
 * Bosch Sensortec BMP085 and BMP180 barometric pressure sensor support
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

    // not required by datasheet but fixes transmission problems seen in practice
    i2c_master_stop();

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
        {
            // more bytes to read
            res=(i2c_master_transmit_with_ack() != TW_MR_DATA_ACK);
        }
        else
        {
            // last byte
            res=(i2c_master_transmit() != TW_MR_DATA_NACK);
        }
        
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
    
    // check chip id
    uint8_t id;
    if (bmp085_read(BMP085_CHIP_ID_REG,1,&id)!=0)
        return 0xff;

    if (id != BMP085_CHIP_ID)
        return 0xfe;

    // read chip version (documented in bosch api source)
    if (bmp085_read(BMP085_VERSION_REG,1,&cal.version)!=0)
        return 0xff;
        
    // read individual calibration data
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

    TWDR = BMP085_CTRL_MEAS_REG;
    
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

void bmp085_calc(int32_t ut, int32_t up, int16_t *tval, int32_t *pval)
{
    int32_t x1, x2, x3, b3, b5, b6, p;
    uint32_t b4, b7;

    // see datasheet for formula
    
    x1 = (ut - cal.ac6) * cal.ac5 >> 15;
    x2 = ((int32_t) cal.mc << 11) / (x1 + cal.md);
    b5 = x1 + x2;
    *tval = ((b5 + 8) >> 4);

    if (pval==NULL)
        return;
    
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

int16_t bmp085_get_temp()
{
    int32_t ut = 0; //Needs to be initialized to 0 because bmp085_read only reads 2 bytes
    int16_t tval;     

    if (!cal.initialized)
        bmp085_readCal(I2C_BMP085_OVERSAMPLING);
    
    if (bmp085_startMeas(BMP085_TEMP)!=0)
    {
#ifdef DEBUG_I2C
        debug_printf("bmp085 write error\n");
#endif
        return -1;
    }

    _delay_us(get_bmp085_measure_us_delay(BMP085_TEMP,I2C_BMP085_OVERSAMPLING));

    if(bmp085_read(BMP085_ADC_OUT_START_REG,2,&ut)!=0)
    {
#ifdef DEBUG_I2C
        debug_printf("bmp085 read error\n");
#endif
        return -1;
    }
    
    bmp085_calc(ut,0,&tval,NULL);
    
    return tval;
}

int32_t bmp085_get_abs_press()
{
    int32_t ut = 0; 
    int16_t tval;
    int32_t up = 0, pval;

    if (!cal.initialized)
        bmp085_readCal(I2C_BMP085_OVERSAMPLING);
    
    if (bmp085_startMeas(BMP085_TEMP)!=0)
    {
#ifdef DEBUG_I2C
        debug_printf("bmp085 write error\n");
#endif
        return -1;
    }

    _delay_us(get_bmp085_measure_us_delay(BMP085_TEMP,I2C_BMP085_OVERSAMPLING));

    if(bmp085_read(BMP085_ADC_OUT_START_REG,2,&ut)!=0)
    {
#ifdef DEBUG_I2C
        debug_printf("bmp085 read error\n");
#endif
        return -1;
    }
    
    if (bmp085_startMeas(BMP085_PRES)!=0)
    {
#ifdef DEBUG_I2C
        debug_printf("bmp085 write error\n");
#endif
        return -1;
    }

    _delay_us(get_bmp085_measure_us_delay(BMP085_PRES,I2C_BMP085_OVERSAMPLING));

    up=0;
    if(bmp085_read(BMP085_ADC_OUT_START_REG,3,&up)!=0)
    {
#ifdef DEBUG_I2C
        debug_printf("bmp085 read error\n");
#endif
        return -1;
    }
    
    up >>= 8-cal.oss;

    bmp085_calc(ut,up,&tval,&pval);
    
    return pval;
}

#ifdef I2C_BMP085_BAROCALC_SUPPORT

int32_t bmp085_get_height_cm(int32_t abs_pa_pressure, int32_t pa_pressure_nn)
{
    double p = (double)abs_pa_pressure / 100;
    double pnn = (double)pa_pressure_nn / 100;

    return (int32_t)(4433076.92*(1-pow(p/pnn,0.190295)));
}

int32_t bmp085_get_pa_pressure_nn(int32_t abs_pa_pressure, int32_t height_cm)
{
    return (int32_t)((double)abs_pa_pressure/pow((1-((double)height_cm/4433076.92)),5.255));
}

#endif /* I2C_BMP085_BAROCALC_SUPPORT */

void bmp085_init(void)
{
    cal.initialized=0;
}

/*
 -- Ethersex META --
 header(hardware/i2c/master/i2c_bmp085.h)
 init(bmp085_init)
 */

#endif /* I2C_BMP085_SUPPORT */
