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
#include <string.h>

#include "config.h"
#include "core/debug.h"
#include "core/global.h"
#include "core/mbr.h"

mbr_config_t mbr_config;

void write_mbr(void )
{
  /* Restore volatile parts */
  mbr_t mbr;
  eeprom_read_block(&mbr, EEPROM_MBR_OFFSET, sizeof(mbr));
  memcpy(&mbr.mbr_config, &mbr_config, sizeof(mbr_config_t));
#ifndef BOOTLOADER_SUPPORT
  mbr.identifier[0] = 'e';
  mbr.identifier[1] = '6';
  mbr.identifier[2] = '\0';
#endif
  eeprom_write_block(&mbr, EEPROM_MBR_OFFSET, sizeof(mbr));
}

int restore_mbr(void )
{
  mbr_t mbr;
  eeprom_read_block(&mbr, EEPROM_MBR_OFFSET, sizeof(mbr));
  memcpy(&mbr_config, &mbr.mbr_config, sizeof(mbr_config_t));
#ifdef MBR_DEBUG
  debug_printf("mbr.identifier %c%c (0x%x 0x%x 0x%x)\n",
      mbr.identifier[0],
      mbr.identifier[1],
      mbr.identifier[0],
      mbr.identifier[1],
      mbr.identifier[2]
  );
#endif
#ifdef BOOTLOADER_SUPPORT
  if (mbr.identifier[0] != 'e' || mbr.identifier[1] != '6' || mbr.identifier[2] != '\0')
    return 1;
#endif
  return 0;
}

void mbr_init(void )
{
  mbr_config.raw = 0x00;
#if defined(BOOTLOADER_SUPPORT) && !defined(TFTP_CRC_SUPPORT)
  uint8_t is_e6 = restore_mbr();
#else
  restore_mbr();
#endif
#ifdef MBR_DEBUG
  debug_printf("Master Boot Record: success %d, \
flashed %d, bootloader %d\n",
      mbr_config.success,
      mbr_config.flashed,
      mbr_config.bootloader);
#endif
  if (mbr_config.flashed == 1)
  {
#ifdef MBR_DEBUG
    debug_printf("Flash successful!\n");
#endif
    mbr_config.flashed = 0;
  }
#ifdef BOOTLOADER_SUPPORT
#ifndef TFTP_CRC_SUPPORT
  if (mbr_config.bootloader == 1 || mbr_config.success == 0 || is_e6 == 1)
    bootload_delay = CONF_BOOTLOAD_DELAY;
  else
    bootload_delay = 1;
#else
  bootload_delay = CONF_BOOTLOAD_DELAY;
#endif
#endif
}

void mbr_startup(void )
{
#ifndef BOOTLOADER_SUPPORT
  mbr_config.success = 1;
#else
  mbr_config.success = 0;
#endif
  mbr_config.bootloader = 0;
  write_mbr();
}

/*
   -- Ethersex META --
  header(core/mbr.h)
  init(mbr_init)
  startup(mbr_startup)
*/
