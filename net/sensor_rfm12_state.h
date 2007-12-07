/*
 * Copyright (c) 2007 by Jochen Roessner <jochen@lugrot.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
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

#ifndef _SENSOR_RFM12_STATE_H
#define _SENSOR_RFM12_STATE_H

#define SENSOR_RFM12_ADCMAX 4

struct sensor_rfm12_data_t {
  uint16_t value;
  char valuetext[6];
};

struct sensors_rfm12_datas_t {
  struct sensor_rfm12_data_t sensor[SENSOR_RFM12_ADCMAX];
};


struct sensor_rfm12_connection_state_t {
  union {
    uint8_t buffer[0];
    struct sensors_rfm12_datas_t sensors;
  };
};

#endif
