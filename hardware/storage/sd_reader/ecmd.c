/*
 * Copyright (c) 2009 by Stefan Siegl <stesie@brokenpipe.de>
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * For more information on the GPL, please go to:
 * http://www.gnu.org/copyleft/gpl.html
 */

#include <avr/pgmspace.h>

#include "config.h"
#include "hardware/storage/sd_reader/sd_raw.h"
#include "hardware/storage/sd_reader/fat.h"
#include "core/vfs/vfs.h"
#include "hardware/storage/sd_reader/vfs_sd.h"
#include "core/debug.h"

#include "protocols/ecmd/ecmd-base.h"


#ifdef SD_READER_SUPPORT

int16_t
parse_cmd_sd_dir (char *cmd, char *output, uint16_t len)
{
  if (vfs_sd_rootnode == 0)
    return ECMD_FINAL(snprintf_P(output, len, PSTR("SD/MMC backend not available.")));

  if (cmd[0] != 0x05) {
    fat_reset_dir(vfs_sd_rootnode);
    cmd[0] = 0x05;
  }

  struct fat_dir_entry_struct dir_entry;
  if (! fat_read_dir(vfs_sd_rootnode, &dir_entry))
    return ECMD_FINAL_OK;

  return ECMD_AGAIN(snprintf_P(output, len, PSTR("%32s%c %ld"),
			       dir_entry.long_name,
			       dir_entry.attributes & FAT_ATTRIB_DIR ? '/' : ' ',
			       dir_entry.file_size));
}

#endif  /* SD_READER_SUPPORT */
