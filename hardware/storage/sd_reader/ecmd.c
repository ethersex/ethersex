/*
 * Copyright (c) 2009 Stefan Siegl <stesie@brokenpipe.de>
 * Copyright (c) 2013 Erik Kunze <ethersex@erik-kunze.de>
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

#include <stdio.h>
#include <avr/pgmspace.h>

#include "config.h"
#include "protocols/ecmd/ecmd-base.h"

#include "hardware/storage/sd_reader/sd_raw.h"
#include "hardware/storage/sd_reader/fat.h"
#include "core/vfs/vfs.h"
#include "hardware/storage/sd_reader/vfs_sd.h"
#include "hardware/storage/sd_reader/ecmd.h"

#ifdef SD_INFO_ECMD_SUPPORT
int16_t
parse_cmd_sd_info(char *cmd, char *output, uint16_t len)
{
  struct sd_raw_info info;

  if (sd_raw_get_info(&info) == 0)
    return ECMD_FINAL(snprintf_P(output, len, PSTR("read error")));

  uint16_t cap = info.capacity / 1000000UL;
  return ECMD_FINAL(snprintf_P(output, len,
                               PSTR("%3s %6s %d/20%02d %uMB %d %d %d"),
                               &info.oem, &info.product,
                               info.manufacturing_month,
                               info.manufacturing_year,
                               cap,
                               info.flag_copy,
                               info.flag_write_protect,
                               info.format));
}
#endif /* SD_INFO_ECMD_SUPPORT */


#ifdef SD_DIR_ECMD_SUPPORT
int16_t
parse_cmd_sd_dir(char *cmd, char *output, uint16_t len)
{
  if (vfs_sd_rootnode == 0)
    return ECMD_FINAL(snprintf_P(output, len,
                                 PSTR("SD/MMC backend not available.")));

  if (cmd[0] != ECMD_STATE_MAGIC)
  {
    fat_reset_dir(vfs_sd_rootnode);
    cmd[0] = ECMD_STATE_MAGIC;
  }

  struct fat_dir_entry_struct dir_entry;
  if (!fat_read_dir(vfs_sd_rootnode, &dir_entry))
    return ECMD_FINAL_OK;

  return ECMD_AGAIN(snprintf_P(output, len, PSTR("%32s%c %ld"),
                               dir_entry.long_name,
                               dir_entry.attributes & FAT_ATTRIB_DIR ? '/' : ' ',
                               dir_entry.file_size));
}
#endif /* SD_DIR_ECMD_SUPPORT */


#ifdef SD_WRITE_SUPPORT
#ifdef SD_MKDIR_ECMD_SUPPORT
int16_t
parse_cmd_sd_mkdir(char *cmd, char *output, uint16_t len)
{
  while (*cmd == ' ')
    cmd++;

  return (vfs_sd_mkdir_recursive(cmd) == 0 ?
          ECMD_FINAL_OK : ECMD_FINAL(snprintf_P(output, len, PSTR("write error"))));
}
#endif

#ifdef SD_RM_ECMD_SUPPORT
int16_t
parse_cmd_sd_rm(char *cmd, char *output, uint16_t len)
{
  while (*cmd == ' ')
    cmd++;

  return (vfs_sd_unlink(cmd) == 0 ?
          ECMD_FINAL_OK : ECMD_FINAL(snprintf_P(output, len, PSTR("write error"))));
}
#endif
#endif /* SD_WRITE_SUPPORT */

/*
  -- Ethersex META --
  block([[SD-Karte]])
  ecmd_ifdef(SD_INFO_ECMD_SUPPORT)
    ecmd_feature(sd_info, "sd info",, List information about SD card.)
  ecmd_endif
  ecmd_ifdef(SD_DIR_ECMD_SUPPORT)
  ecmd_feature(sd_dir, "sd dir",, List contents of current SD directory.)
  ecmd_endif
  ecmd_ifdef(SD_WRITE_SUPPORT)
    ecmd_ifdef(SD_MKDIR_ECMD_SUPPORT)
      ecmd_feature(sd_mkdir, "sd mkdir",PATH, Create directory hierarchy PATH.)
    ecmd_endif
    ecmd_ifdef(SD_RM_ECMD_SUPPORT)
      ecmd_feature(sd_rm, "sd rm",PATH, Remove file PATH.)
    ecmd_endif
  ecmd_endif
*/
