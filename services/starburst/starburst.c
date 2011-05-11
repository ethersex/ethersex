/*
 *
 * Copyright (c) 2011 by Maximilian Güntner <maximilian.guentner@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
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

/* Module description: starburst is a forwarder of ethersex dmx information to various i2c PWM chips*/
#include <avr/pgmspace.h>
#include "starburst.h"
#include "core/debug.h"
#include "hardware/i2c/master/i2c_pca9685.h"
#include "services/dmx-storage/dmx_storage.h"

#ifdef STARBURST_PCA9685
prog_uint16_t stevens_power_12bit[256] PROGMEM = {
0, 0, 0, 0, 0, 1, 1, 2, 2, 3, 3,
4, 5, 6, 7, 8, 9, 11, 12, 14, 15,
17, 19, 21, 23, 25, 27, 29, 32, 34, 37,
40, 43, 46, 49, 52, 55, 59, 62, 66, 70,
73, 77, 82, 86, 90, 95, 99, 104, 109, 114,
119, 124, 129, 135, 140, 146, 152, 158, 164, 170,
176, 182, 189, 196, 202, 209, 216, 224, 231, 238,
246, 254, 261, 269, 277, 286, 294, 302, 311, 320,
329, 338, 347, 356, 365, 375, 385, 394, 404, 414,
424, 435, 445, 456, 467, 477, 489, 500, 511, 522,
534, 546, 557, 569, 582, 594, 606, 619, 631, 644,
657, 670, 684, 697, 710, 724, 738, 752, 766, 780,
795, 809, 824, 838, 853, 869, 884, 899, 915, 930,
946, 962, 978, 994, 1011, 1027, 1044, 1061, 1078, 1095,
1112, 1130, 1147, 1165, 1183, 1201, 1219, 1238, 1256, 1275,
1293, 1312, 1331, 1351, 1370, 1389, 1409, 1429, 1449, 1469,
1489, 1510, 1530, 1551, 1572, 1593, 1614, 1636, 1657, 1679,
1700, 1722, 1745, 1767, 1789, 1812, 1834, 1857, 1880, 1904,
1927, 1950, 1974, 1998, 2022, 2046, 2070, 2095, 2119, 2144,
2169, 2194, 2219, 2245, 2270, 2296, 2322, 2348, 2374, 2400,
2427, 2453, 2480, 2507, 2534, 2561, 2589, 2616, 2644, 2672,
2700, 2728, 2757, 2785, 2814, 2843, 2872, 2901, 2931, 2960,
2990, 3020, 3050, 3080, 3110, 3141, 3171, 3202, 3233, 3264,
3295, 3327, 3359, 3390, 3422, 3454, 3487, 3519, 3552, 3585,
3618, 3651, 3684, 3717, 3751, 3785, 3819, 3853, 3887, 3921,
3956, 3991, 4026, 4061, 4096 };
#endif
void starburst_init()
{
	/*Init all i2c chips*/
#ifdef STARBURST_PCA9685
	i2c_pca9685_set_mode(STARBURST_PCA9685_ADDRESS,STARBURST_PCA9685_EXTDRV,STARBURST_PCA9685_IVRT,STARBURST_PCA9685_PRESCALER);
#endif
}
void starburst_process()
{

#ifdef STARBURST_PCA9685
	if(get_dmx_universe_state(STARBURST_PCA9685_UNIVERSE) == DMX_NEWVALUES)
	{
		/*Prepare Array*/
		uint16_t pca9685_values[2*STARBURST_PCA9685_CHANNELS];
		uint16_t tmp=0;
		for(uint8_t i=0;i<STARBURST_PCA9685_CHANNELS*2;i+=2)
		{
			tmp=pgm_read_word_near(stevens_power_12bit + get_dmx_channel(i/2+STARBURST_PCA9685_OFFSET,STARBURST_PCA9685_UNIVERSE));
			if(tmp == 4096) /*Special case: LED is always on, that means we need to set ON to 4096*/
			{
				pca9685_values[i]=4096;
				pca9685_values[i+1]=0;
			}
			else if(tmp == 0) /*Special case: LED is always off, that means we need to set OFF to 4096*/
			{
				pca9685_values[i]=0;
				pca9685_values[i+1]=4096;
			}
			else /*Default case: LED needs PWM*/
			{
				pca9685_values[i]=0;
				pca9685_values[i+1]=tmp;
			}
		}
		i2c_pca9685_set_leds(STARBURST_PCA9685_ADDRESS,0,STARBURST_PCA9685_CHANNELS*2,pca9685_values);
	}
#endif
#ifdef STARBURST_PCA9685_TEST
	set_dmx_channel(0,0,4);
	static uint8_t counter[3]={1,1,1};
	static uint8_t finished=1;
	if(finished == 1)
	{
	counter[0]++;
	counter[1]++;
	counter[2]++;
	if(counter[0] > 254)
		counter[0] = 1;
	if(counter[1] > 254)
		counter[1] = 1;
	if(counter[2] > 254)
		counter[2] = 1;
	for(uint8_t i=0;i<STARBURST_PCA9685_CHANNELS;i+=3)
	{
		i2c_pca9685_set_led(STARBURST_PCA9685_ADDRESS,i,0,stevens_power[counter[0]]);
		i2c_pca9685_set_led(STARBURST_PCA9685_ADDRESS,i+1,0,stevens_power[counter[1]]);
		i2c_pca9685_set_led(STARBURST_PCA9685_ADDRESS,i+2,0,stevens_power[counter[2]]);
	}
	}
#endif
}
/*
  -- Ethersex META --
  mainloop(starburst_process)
  init(starburst_init)
*/
