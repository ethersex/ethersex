/*
 *
 * Copyright (c) 2011-2012 by Maximilian Güntner <maximilian.guentner@gmail.com>
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
#include "core/bool.h"
#include "hardware/i2c/master/i2c_pca9685.h"
#include "services/dmx-storage/dmx_storage.h"
enum starburst_update update;
#ifdef STARBURST_PCA9685
int8_t pca9685_dmx_conn_id = (-1);
uint8_t pca9685_dmx_connected = FALSE;
const prog_uint16_t stevens_power_12bit[256] PROGMEM = {
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
  3956, 3991, 4026, 4061, 4096
};
#endif

#ifdef STARBURST_PCA9685
struct starburst_channel pca9685_channels[STARBURST_PCA9685_CHANNELS] =
  { {0, 0, STARBURST_MODE_NORMAL, STARBURST_NOUPDATE} };
#endif

void
starburst_init()
{
  /*Init all i2c chips */
#ifdef STARBURST_PCA9685
  i2c_pca9685_set_mode(STARBURST_PCA9685_ADDRESS, STARBURST_PCA9685_EXTDRV,
                       STARBURST_PCA9685_IVRT, STARBURST_PCA9685_PRESCALER);
  //Connect to dmx-storage
  pca9685_dmx_conn_id = dmx_storage_connect(STARBURST_PCA9685_UNIVERSE);
  if (pca9685_dmx_conn_id != -1)
    pca9685_dmx_connected = TRUE;
  else
    pca9685_dmx_connected = FALSE;
#endif
}

void
starburst_process()
{
  if (pca9685_dmx_connected == FALSE)
    return;
  starburst_update();
#ifdef STARBURST_PCA9685
  for (uint8_t i = 0; i < STARBURST_PCA9685_CHANNELS; i++)
  {
    /*TODO: Implement a different mode setting scheme:
     * 0: Set without fade
     * 1-9: Fade speed (slow -> fast)
     */
    switch (pca9685_channels[i].mode)
    {
      case (STARBURST_MODE_NORMAL):
      {
        if (pca9685_channels[i].value != pca9685_channels[i].target)
        {
          pca9685_channels[i].value = pca9685_channels[i].target;
          pca9685_channels[i].update = STARBURST_UPDATE;
          update = STARBURST_UPDATE;
        }
        else
          pca9685_channels[i].update = STARBURST_NOUPDATE;

        break;
      }
      case (STARBURST_MODE_FADE):
      {
        if (pca9685_channels[i].value > pca9685_channels[i].target)
        {
          pca9685_channels[i].value--;
          pca9685_channels[i].update = STARBURST_UPDATE;
          update = STARBURST_UPDATE;

        }
        else if (pca9685_channels[i].value < pca9685_channels[i].target)
        {
          pca9685_channels[i].value++;
          pca9685_channels[i].update = STARBURST_UPDATE;
          update = STARBURST_UPDATE;
        }
        else
          pca9685_channels[i].update = STARBURST_NOUPDATE;

        break;
      }
    }
  }
#ifdef STARBURST_PCA9685_STROBO
  /*Hardware stroboscope support:
   * Control all channels of a PCA9685 using the output enable. 
   * Value range: 1-25 (results in 1hz - 25hz)
   */
  static uint8_t pca9685_strobo_counter = 0;
  uint8_t pca9685_strobo =
    2 * get_dmx_channel_slot(STARBURST_PCA9685_UNIVERSE,
                             STARBURST_PCA9685_CHANNELS * 2 +
                             STARBURST_PCA9685_OFFSET, pca9685_dmx_conn_id);
  if (pca9685_strobo > 0 && pca9685_strobo <= 50)
  {
    if (pca9685_strobo_counter >= 50 / pca9685_strobo)
    {
      i2c_pca9685_output_enable(TOGGLE);
      pca9685_strobo_counter = 0;
    }
    if (pca9685_strobo_counter < 50)
      pca9685_strobo_counter++;
    else
      pca9685_strobo_counter = 0;
  }
  else
    i2c_pca9685_output_enable(ON);
#endif
#endif
}

void
starburst_update()
{
#ifdef STARBURST_PCA9685

  if (get_dmx_universe_state(STARBURST_PCA9685_UNIVERSE, pca9685_dmx_conn_id)
      == DMX_NEWVALUES)
  {
    /*Update values if they are really newer */
    /*Layout for starburst is CCCCMMMMS, where C is Channel, M is Mode and S is Strobe (optional) */
    uint8_t tmp = 0;
    for (uint8_t i = 0; i < STARBURST_PCA9685_CHANNELS; i++)
    {
      tmp =
        get_dmx_channel_slot(STARBURST_PCA9685_UNIVERSE,
                             i + STARBURST_PCA9685_OFFSET +
                             STARBURST_PCA9685_CHANNELS, pca9685_dmx_conn_id);
      pca9685_channels[i].mode = tmp;
      tmp =
        get_dmx_channel_slot(STARBURST_PCA9685_UNIVERSE,
                             i + STARBURST_PCA9685_OFFSET,
                             pca9685_dmx_conn_id);
      if (pca9685_channels[i].target != tmp)
      {
        /*Update the new target */
        pca9685_channels[i].target = tmp;
      }
    }
  }
#endif
}

void
starburst_main()
{
#ifdef STARBURST_PCA9685
  if (update == STARBURST_UPDATE)       /*Only transmit if at least one channels has been updated */
  {
    update = STARBURST_NOUPDATE;
    /*Prepare Array */
    uint16_t pca9685_values[STARBURST_PCA9685_CHANNELS];
    for (uint8_t i = 0; i < STARBURST_PCA9685_CHANNELS; i++)
    {
      pca9685_channels[i].update = STARBURST_NOUPDATE;
      pca9685_values[i] =
        pgm_read_word_near(stevens_power_12bit + pca9685_channels[i].value);
    }
    i2c_pca9685_set_leds_fast(STARBURST_PCA9685_ADDRESS, 0,
                              STARBURST_PCA9685_CHANNELS, pca9685_values);
  }
#endif
}

/*
   -- Ethersex META --
   header(services/starburst/starburst.h)
   mainloop(starburst_main)
   timer(1,starburst_process())
   init(starburst_init)
 */
