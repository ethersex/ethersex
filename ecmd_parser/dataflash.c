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

#include <stdio.h>
#include <string.h>

#include "../config.h"
#include "../dataflash/df.h"
#include "../dataflash/fs.h"

#ifdef DATAFLASH_SUPPORT

int16_t 
parse_cmd_df_status (char *cmd, char *output, uint16_t len)
{
  (void) cmd;

  return snprintf_P (output, len, PSTR ("df status: 0x%02x"),
		     df_status (NULL));
}


int16_t
parse_cmd_fs_format (char *cmd, char *output, uint16_t len)
{
  (void) cmd;
  (void) output;
  (void) len;

  if (fs_format (&fs) != FS_OK) 
    return snprintf_P (output, len, PSTR ("fs: error while formating"));
  if (fs_init (&fs, NULL) != FS_OK) 
    return snprintf_P (output, len, PSTR ("fs: error while initializing"));

  return 0;
}


int16_t
parse_cmd_fs_list (char *cmd, char *output, uint16_t len)
{
  char name[FS_FILENAME + 1];

  if (cmd[0] != 0x05) 
    {
      /* first function entry */
      cmd[0] = 0x05;		/* set magic byte ... */
      cmd[1] = 0x00;

      return -10 - snprintf_P (output, len, PSTR ("name  :inode :size\n"
						  "----------------------"));
    }
  else
    {
      if (fs_list (&fs, NULL, name, cmd[1] ++) != FS_OK)
	return 0;		/* no repare, out. */
      
      name[FS_FILENAME] = 0;

      fs_inode_t inode = fs_get_inode (&fs, name);
      fs_size_t size = fs_size (&fs, inode);

      return -10 - snprintf_P (output, len, PSTR ("%-6s:0x%04x:0x%04x"),
			       name, inode, size);
    }
}


int16_t
parse_cmd_fs_mkfile (char *cmd, char *output, uint16_t len)
{
  /* ignore leading spaces */
  while (*cmd == ' ')
    cmd ++;

  fs_status_t ret = fs_create (&fs, cmd);
  
  if (ret != FS_OK)
    return snprintf_P (output, len, PSTR ("fs_create: returned 0x%02x"), ret);

  fs_inode_t i = fs_get_inode (&fs, cmd);
  return snprintf_P (output, len, PSTR ("fs_create: inode 0x%04x"), i);
}


int16_t
parse_cmd_fs_remove (char *cmd, char *output, uint16_t len)
{
  /* ignore leading spaces */
  while (*cmd == ' ')
    cmd ++;

  fs_status_t ret = fs_remove (&fs, cmd);

  return (ret == FS_OK) ? 0 : -1;
}


#endif /* DATAFLASH_SUPPORT */
