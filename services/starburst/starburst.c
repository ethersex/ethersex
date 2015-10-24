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
int8_t pca9685_dmx_conn_id = -1;
uint8_t pca9685_dmx_connected = FALSE;
const uint16_t cie_luminance_12bit[256] PROGMEM =
  { 0, 1, 3, 13, 14, 15, 16, 17, 18, 19, 20,
  21, 23, 24, 26, 27, 29, 30, 32, 33, 35,
  37, 39, 41, 43, 45, 47, 49, 51, 53, 56,
  58, 61, 63, 66, 68, 71, 74, 77, 80, 83,
  86, 89, 93, 96, 99, 103, 107, 110, 114, 118,
  122, 126, 130, 134, 139, 143, 148, 152, 157, 162,
  166, 171, 176, 182, 187, 192, 198, 203, 209, 215,
  221, 227, 233, 239, 245, 252, 258, 265, 272, 278,
  285, 293, 300, 307, 315, 322, 330, 338, 346, 354,
  362, 370, 379, 387, 396, 405, 414, 423, 432, 441,
  451, 460, 470, 480, 490, 500, 510, 521, 531, 542,
  553, 564, 575, 587, 598, 610, 621, 633, 645, 657,
  670, 682, 695, 708, 721, 734, 747, 761, 774, 788,
  802, 816, 830, 845, 859, 874, 889, 904, 919, 935,
  950, 966, 982, 998, 1015, 1031, 1048, 1065, 1082, 1099,
  1116, 1134, 1151, 1169, 1187, 1206, 1224, 1243, 1262, 1281,
  1300, 1319, 1339, 1359, 1379, 1399, 1420, 1440, 1461, 1482,
  1503, 1525, 1546, 1568, 1590, 1612, 1635, 1657, 1680, 1703,
  1726, 1750, 1773, 1797, 1821, 1846, 1870, 1895, 1920, 1945,
  1971, 1996, 2022, 2048, 2074, 2101, 2128, 2155, 2182, 2209,
  2237, 2265, 2293, 2321, 2350, 2379, 2408, 2437, 2466, 2496,
  2526, 2556, 2587, 2617, 2648, 2680, 2711, 2743, 2775, 2807,
  2839, 2872, 2905, 2938, 2971, 3005, 3039, 3073, 3108, 3142,
  3177, 3212, 3248, 3284, 3320, 3356, 3392, 3429, 3466, 3503,
  3541, 3579, 3617, 3655, 3694, 3733, 3772, 3812, 3851, 3891,
  3932, 3972, 4013, 4054, 4096
};
#endif

#ifdef STARBURST_PCA9685
struct starburst_channel pca9685_channels[STARBURST_PCA9685_CHANNELS] =
  { {0, 0, STARBURST_MODE_NORMAL, STARBURST_NOUPDATE} };
#endif

void
starburst_init(void)
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
starburst_process(void)
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
    2 * get_dmx_channel_slot_raw(STARBURST_PCA9685_UNIVERSE,
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
starburst_update(void)
{
#ifdef STARBURST_PCA9685

  if (get_dmx_slot_state(STARBURST_PCA9685_UNIVERSE, pca9685_dmx_conn_id)
      == DMX_NEWVALUES)
  {
    /*Update values if they are really newer */
    /*Layout for starburst is CCCCMMMMS, where C is Channel, M is Mode and S is Strobe (optional) */
    uint8_t tmp = 0;
    for (uint8_t i = 0; i < STARBURST_PCA9685_CHANNELS; i++)
    {
      tmp =
        get_dmx_channel_slot_raw(STARBURST_PCA9685_UNIVERSE,
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
starburst_main(void)
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
        pgm_read_word_near(cie_luminance_12bit + pca9685_channels[i].value);
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
