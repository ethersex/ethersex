/*
 * Copyright (c) 2013 by Erik Kunze <ethersex@erik-kunze.de>
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

#include "core/vfs/vfs.h"
#include "hardware/i2c/master/vfs_eeprom.h"
#include "hardware/i2c/master/i2c_24CXX.h"
#include "hardware/i2c/master/i2c_24CXX_ecmd.h"


int16_t
parse_cmd_i2c_24CXX_dir(char *cmd, char *output, uint16_t len)
{
  unsigned char buf[SFS_PAGE_SIZE];
  vfs_eeprom_inode_t inode;

  if (cmd[0] != ECMD_STATE_MAGIC)
  {
    cmd[0] = ECMD_STATE_MAGIC;

    if (!i2c_24CXX_read_block(0, buf,
                              sizeof(struct vfs_eeprom_page_superblock)))
      goto read_error;

    inode = ((struct vfs_eeprom_page_superblock *) buf)->next_file;
    if (!inode)
      return ECMD_FINAL_OK;
  }
  else
  {
    inode = *((vfs_eeprom_inode_t *) & cmd[1]);
  }

  struct vfs_eeprom_page_file *file = (struct vfs_eeprom_page_file *) buf;
  if (!i2c_24CXX_read_block(inode * SFS_PAGE_SIZE, buf, SFS_PAGE_SIZE) ||
      file->magic != SFS_MAGIC_FILE)
  {
  read_error:
    return ECMD_FINAL(snprintf_P(output, len, PSTR("read error")));
  }

  int16_t l = snprintf_P(output, len, PSTR("%s"), file->filename);
  *((vfs_eeprom_inode_t *) & cmd[1]) = file->next_file;
  return (file->next_file == 0 ? ECMD_FINAL(l) : ECMD_AGAIN(l));
}

#ifdef VFS_EEPROM_SUPPORT
int16_t
parse_cmd_i2c_24CXX_rm(char *cmd, char *output, uint16_t len)
{
  while (*cmd == ' ')
    cmd++;

  return (vfs_eeprom_unlink(cmd) == 0 ?
          ECMD_FINAL_OK : ECMD_FINAL(snprintf_P(output, len, PSTR("write error"))));

}
#endif /* VFS_EEPROM_SUPPORT */

/*
  -- Ethersex META --

  block([[I2C]] (TWI))
  ecmd_feature(i2c_24CXX_dir, "ee dir",, List files in I2C EEPROM.)
  ecmd_ifdef(VFS_EEPROM_SUPPORT)
    ecmd_feature(i2c_24CXX_rm, "ee rm",PATH, Remove file PATH.)
  ecmd_endif()
*/
