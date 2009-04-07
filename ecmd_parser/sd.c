/* 
 * Copyright (c) 2008 by Stefan Siegl <stesie@brokenpipe.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by 
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
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

#include <avr/pgmspace.h>

#include "config.h"
#include "../sd_reader/sd_raw.h"
#include "../sd_reader/fat.h"
#include "core/debug.h"

#ifdef SD_READER_SUPPORT

int16_t
parse_cmd_sd_dir (char *cmd, char *output, uint16_t len)
{
  if (sd_cwd == 0)
    return snprintf_P (output, len, PSTR ("SD/MMC backend not available."));

  if (cmd[0] != 0x05) {
    fat_reset_dir(sd_cwd);
    cmd[0] = 0x05;
  }

  struct fat_dir_entry_struct dir_entry;
  if (! fat_read_dir(sd_cwd, &dir_entry))
    return 0;

  return -10 - snprintf_P (output, len, PSTR ("%32s%c %ld"), 
                           dir_entry.long_name,
                           dir_entry.attributes & FAT_ATTRIB_DIR ? '/' : ' ',
			   dir_entry.file_size);
}

#endif  /* SD_READER_SUPPORT */
