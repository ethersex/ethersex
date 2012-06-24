/*
 *
 * Copyright(c) 2012 by Sascha Ittner <sascha.ittner@modusoft.de>
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

#ifndef _TANKLEVEL_H
#define _TANKLEVEL_H

#include "services/clock/clock.h"

/* sensor sensitivity in mV/kPa */
#define TANKLEVEL_SENSOR_SENS   90.0F

/* gravity in mm/(sec*sec) */
#define TANKLEVEL_GRAVITY       9.81F

typedef struct {
  int16_t sensor_offset;
  uint16_t med_density;
  uint16_t ltr_per_m;
  uint16_t ltr_full;
  uint16_t raise_time;
  uint16_t hold_time;
} tanklevel_params_t;

extern tanklevel_params_t tanklevel_params_ram;

void tanklevel_init(void);
void tanklevel_update_factor(void);
void tanklevel_periodic(void);
void tanklevel_start(void);
int8_t tanklevel_check_busy(void);
uint16_t tanklevel_get(void);
timestamp_t tanklevel_get_ts(void);
void tanklevel_set_lock(uint8_t val);

int16_t parse_cmd_tanklevel_show_params(char *cmd, char *output, uint16_t len);
int16_t parse_cmd_tanklevel_set_param(char *cmd, char *output, uint16_t len);
int16_t parse_cmd_tanklevel_save_param(char *cmd, char *output, uint16_t len);
int16_t parse_cmd_tanklevel_zero_sensor(char *cmd, char *output, uint16_t len);
int16_t parse_cmd_tanklevel_start(char *cmd, char *output, uint16_t len);
int16_t parse_cmd_tanklevel_get(char *cmd, char *output, uint16_t len);

#endif
