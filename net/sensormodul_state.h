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

#ifndef _SENSORMODUL_STATE_H
#define _SENSORMODUL_STATE_H
#include "../uip/uip-conf.h"
#define SENSORMODUL_ADCMAX 4

struct sensormodul_data_t {
  uint16_t value;
  char valuetext[6];
};

struct sensormodul_datas_t {
  struct sensormodul_data_t sensor[SENSORMODUL_ADCMAX];
  uint8_t maxfeuchte_div[5];
  uint8_t countdown;
  uint8_t lcd_blocked    :1;
  uint8_t led_blink      :2;
  uint8_t fill           :1;
  uint8_t ledstate_akt   :4;
  uint8_t tastersend;
};

struct sensormodul_request_t {
  union{
    uint8_t raw[0];
    uint8_t type;
  };
  union{
    uint8_t data[0];
    uint8_t digit[3];
  };
};

struct sensormodul_connection_state_t {
  union {
    uint8_t buffer[0];
    struct sensormodul_datas_t sensors;
  };
  uip_ipaddr_t ripaddr;
  u16_t rport;
};

#endif
