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

#include <avr/eeprom.h>

#include "config.h"
#include "core/debug.h"
#include "core/global.h"
#include "core/mbr.h"

mbr_t mbr;

void write_mbr(void )
{
  eeprom_write_byte(EEPROM_MBR_OFFSET, mbr.raw);
}

void restore_mbr(void )
{
  mbr.raw = eeprom_read_byte(EEPROM_MBR_OFFSET);
}

void mbr_init(void )
{
  mbr.raw = 0x00;
  restore_mbr();
  debug_printf("Master Boot Record: success %d, flashed %d, bootloader %d\n", mbr.success, mbr.flashed, mbr.bootloader);
  if (mbr.flashed == 1)
  {
    debug_printf("Flash successful!\n");
    mbr.flashed = 0;
  }
#ifdef BOOTLOADER_SUPPORT
  if (mbr.bootloader == 1 || mbr.success == 0 || mbr.padding != MBR_MAGIC) 
    bootload_delay = CONF_BOOTLOAD_DELAY;
  else
    bootload_delay = 1;
#endif
}

void mbr_startup(void )
{
#ifndef BOOTLOADER_SUPPORT
  mbr.success = 1;
  mbr.padding = MBR_MAGIC;
#else
  mbr.success = 0;
#endif
  mbr.bootloader = 0;
  write_mbr();
}

/*
   -- Ethersex META --
  header(core/mbr.h)
  init(mbr_init)
  startup(mbr_startup)
*/
