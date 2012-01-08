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

#ifndef _I2C_BMP085_H
#define _I2C_BMP085_H

struct bmp085_cal
{
    // calibration data
    int16_t ac1, ac2, ac3, b1, b2, mb, mc, md;
    uint16_t ac4, ac5, ac6;

    // oversampling, accuracy level
    uint8_t oss;
    
    uint8_t initialized;
};

enum bmp085_meas_t { BMP085_TEMP, BMP085_PRES };
typedef enum bmp085_meas_t bmp085_meas_t;

uint8_t bmp085_readCal(uint8_t oss);
int16_t bmp085_readRegister(uint8_t addr);
uint8_t bmp085_startMeas(bmp085_meas_t type);

int32_t bmp085_getPressure(void);
#define bmp085_getTemp() bmp085_readRegister(0xF6)

void bmp085_calc(int16_t ut, int32_t up, int16_t *tval, int32_t *pval);
void bmp085_init(void);

inline uint16_t get_bmp085_measure_us_delay(bmp085_meas_t type, uint8_t oss)
{
    if (type==BMP085_TEMP)
        return 4500;
    else // (type==BMP085_PRES)
    {
        if (oss == 0)
            return 4500;
        else if (oss == 1)
            return 7500;
        else if (oss == 2)
            return 13500;
        else // oss == 3
            return 25500;
    }
}

#endif /* _I2C_BMP085_H */
