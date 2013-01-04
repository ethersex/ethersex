/*
 * Copyright (c) 2012 by Maximilian Güntner <maximilian.guentner@gmail.com>
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

#ifndef _MBR_H
#define _MBR_H

#include "config.h"
#include <avr/eeprom.h>

#undef MBR_DEBUG

/* Reserve last 4 bytes for flash count
 * http://www.nongnu.org/avrdude/user-manual/avrdude_4.html
 */
#define EEPROM_MBR_OFFSET (uint8_t*) (E2END-sizeof(mbr_t)-3)

typedef union {
  struct {
  /* 
    0 = default
    1 = last firmware boot was successful */
    unsigned success: 1;
  /*
    0 = default
    1 = new firmware has just been flashed */
    unsigned flashed: 1;
  /* 0: bootloader will boot directly to the app
     1: bootloader will wait for an image */ 
    unsigned bootloader: 1;
    unsigned padding: 5;
  };
  uint8_t raw;

} mbr_config_t;

typedef union {
  struct {
      mbr_config_t mbr_config;
      char identifier[3];
  };
  uint32_t raw;
  uint8_t bytes[4];
} mbr_t;

extern mbr_config_t mbr_config;

void write_mbr(void );

int restore_mbr(void );

void mbr_init(void );

void mbr_startup(void );

#endif /* _MBR_H */
