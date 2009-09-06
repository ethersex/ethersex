/* 
 * Copyright(C) 2009 Stefan Siegl <stesie@brokenpipe.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
 */

#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>

#include "core/host/avr/eeprom.h"

uint8_t eeprom_data[EEPROM_SIZE];

void
eeprom_host_init (void)
{
  int fd = open ("eeprom.bin", O_RDONLY);
  if (fd < 0) return;

  read (fd, eeprom_data, EEPROM_SIZE);
  close (fd);
}

void
eeprom_host_exit (void)
{
  int fd = open ("eeprom.bin", O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if (fd < 0) return;

  write (fd, eeprom_data, EEPROM_SIZE);
  close (fd);
}

/*
  -- Ethersex META --
  header(core/host/avr/eeprom.h)
  initearly(eeprom_host_init)
  atexit(eeprom_host_exit)
*/
