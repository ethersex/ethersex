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
#include <stdlib.h>

#include "config.h"
#include "hardware/storage/dataflash/df.h"
#include "hardware/storage/dataflash/fs.h"

#include "protocols/ecmd/ecmd-base.h"


int16_t 
parse_cmd_df_status (char *cmd, char *output, uint16_t len)
{
  (void) cmd;

  return ECMD_FINAL(snprintf_P(output, len, PSTR("df status: 0x%02x"),
			       df_status(NULL)));
}


int16_t
parse_cmd_fs_format (char *cmd, char *output, uint16_t len)
{
  (void) cmd;
  (void) output;
  (void) len;

  if (fs_format (&fs) != FS_OK) 
    return ECMD_FINAL(snprintf_P(output, len, PSTR("fs: error while formating")));
  if (fs_init () != FS_OK) 
    return ECMD_FINAL(snprintf_P(output, len, PSTR("fs: error while initializing")));

  return ECMD_FINAL_OK;
}


int16_t
parse_cmd_fs_list (char *cmd, char *output, uint16_t len)
{
  char name[FS_FILENAME + 1];

  if (cmd[0] != ECMD_STATE_MAGIC) 
    {
      /* first function entry */
      cmd[0] = ECMD_STATE_MAGIC;	/* set magic byte ... */
      cmd[1] = 0x00;

      return ECMD_AGAIN(snprintf_P(output, len, PSTR("name  :inode :size\n"
						     "----------------------")));
    }
  else
    {
      if (fs_list (&fs, NULL, name, cmd[1] ++) != FS_OK)
	return ECMD_FINAL_OK;		/* no repare, out. */
      
      name[FS_FILENAME] = 0;

      fs_inode_t inode = fs_get_inode (&fs, name);
      fs_size_t size = fs_size (&fs, inode);

      return ECMD_AGAIN(snprintf_P(output, len, PSTR("%-6s:0x%04x:0x%04x"),
			            name, inode, size));
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
    return ECMD_FINAL(snprintf_P(output, len, PSTR("fs_create: returned 0x%02x"), ret));

  fs_inode_t i = fs_get_inode (&fs, cmd);
  return ECMD_FINAL(snprintf_P(output, len, PSTR("fs_create: inode 0x%04x"), i));
}


int16_t
parse_cmd_fs_remove (char *cmd, char *output, uint16_t len)
{
  /* ignore leading spaces */
  while (*cmd == ' ')
    cmd ++;

  fs_status_t ret = fs_remove (&fs, cmd);

  return (ret == FS_OK) ? ECMD_FINAL_OK : ECMD_ERR_PARSE_ERROR;
}


int16_t
parse_cmd_fs_truncate (char *cmd, char *output, uint16_t len)
{
  /* Ignore leading spaces. */
  while (* cmd == ' ') cmd ++;

  char *ptr = strchr (cmd, ' ');
  if (ptr == NULL) return ECMD_ERR_PARSE_ERROR;	/* invalid args. */

  *(ptr ++) = 0;		/* Zero terminate filename. */

  fs_inode_t i = fs_get_inode (&fs, cmd);

  if (i == 0xffff)
    return ECMD_FINAL(snprintf_P(output, len, PSTR("no such file.")));

  fs_status_t ret = fs_truncate (&fs, i, strtoul (ptr, NULL, 10));
  return (ret == FS_OK) ? ECMD_FINAL_OK : ECMD_ERR_PARSE_ERROR;
}


#ifdef DEBUG_FS
int16_t
parse_cmd_fs_inspect_node (char *cmd, char *output, uint16_t len)
{
  (void) output;
  (void) len;

  /* Ignore leading spaces. */
  while (* cmd == ' ') cmd ++;

  fs_inspect_node (&fs, strtoul (cmd, NULL, 0));
  return ECMD_FINAL_OK;
}


int16_t
parse_cmd_fs_inspect_inode (char *cmd, char *output, uint16_t len)
{
  (void) output;
  (void) len;

  /* Ignore leading spaces. */
  while (* cmd == ' ') cmd ++;

  fs_inspect_inode (&fs, strtoul (cmd, NULL, 0));
  return ECMD_FINAL_OK;
}

#endif	/* DEBUG_FS */

/*
  -- Ethersex META --
  block([[DataFlash]])
  ecmd_feature(df_status, "df status",, Display internal status.)

  ecmd_feature(fs_format, "fs format",, Format the filesystem.)
  ecmd_feature(fs_list, "fs list",, List the directory.)
  ecmd_feature(fs_mkfile, "fs mkfile ", NAME, Create a new file NAME.)
  ecmd_feature(fs_remove, "fs remove ", NAME, Delete the file NAME.)
  ecmd_feature(fs_truncate, "fs truncate ", NAME LEN, Truncate the file NAME to LEN bytes.)

  ecmd_ifdef(DEBUG_FS)
    ecmd_feature(fs_inspect_node, "fs inspect node ", NODE, Inspect NODE and dump to serial.)
    ecmd_feature(fs_inspect_inode, "fs inspect inode ", INODE, Inspect INODE (and associated page).)
  ecmd_endif()
*/
