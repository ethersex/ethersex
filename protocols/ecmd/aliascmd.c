/*
 *
 * Copyright (c) 2009 by Christian Dietrich <stettberger@dokucode.de>
 * Copyright (c) 2009 by Stefan Riepenhausen <rhn@gmx.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
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

#include <string.h>
#include <avr/pgmspace.h>

#include "config.h"
#include "core/debug.h"

#include "aliascmd.h"

#include "protocols/ecmd/ecmd-base.h"

#include "alias_defs.c"

#define ALIASCMD_MAX sizeof(aliascmdlist) / sizeof(aliascmd_t)

char *
aliascmd_decode(char *cmd)
{
  aliascmd_t alias;
  uint8_t alias_cmp_len = 0;
  int8_t alias_cmp_idx = -1;
  uint8_t i;
  for (i = 0; i < ALIASCMD_MAX; i++)
  {
    memcpy_P(&alias, &aliascmdlist[i], sizeof(aliascmd_t));
#ifdef DEBUG_ECMD
    debug_printf("test cmd %s vs. alias %S\n", cmd + 1, alias.name);
#endif
    size_t len = strlen_P(alias.name);
    if (strncmp_P(cmd + 1, alias.name, len) == 0 && alias_cmp_len < len)
    {
      alias_cmp_len = len;
      alias_cmp_idx = i;
    }
  }

  if (alias_cmp_idx != -1)
  {                             /* copy alias in cmd buffer */
    memcpy_P(&alias, &aliascmdlist[alias_cmp_idx], sizeof(aliascmd_t));
    uint8_t newlen = strlen_P(alias.cmd);

    memmove(cmd + newlen, cmd + alias_cmp_len + 1,
            strlen(cmd + alias_cmp_len + 1) + 1);
    memcpy_P(cmd, alias.cmd, newlen);

#ifdef DEBUG_ECMD
    debug_printf("alias found at pos %i: %S -> %S\n", i, alias.name,
                 alias.cmd);
#endif
    return cmd;
  }
#ifdef DEBUG_ECMD
  debug_printf("no alias found\n");
#endif
  return NULL;
}

int16_t
parse_cmd_alias_list(char *cmd, char *output, uint16_t len)
{

  if (cmd[0] != ECMD_STATE_MAGIC)
  {
    cmd[0] = ECMD_STATE_MAGIC;  //magic byte
    cmd[1] = 0x00;
    return ECMD_AGAIN(snprintf_P(output, len, PSTR("aliases:")));
  }
  else
  {
    uint8_t i = cmd[1]++;
    if (i < ALIASCMD_MAX)
    {
      const aliascmd_t *aliasp = &aliascmdlist[i];
      return
        ECMD_AGAIN(snprintf_P
                   (output, len, PSTR("%S -> %S"),
                    pgm_read_word(aliasp->name), pgm_read_word(aliasp->cmd)));
    }
    else
      return ECMD_FINAL_OK;
  }
}

/*
  -- Ethersex META --
  block([[AliasCmd]])
  ecmd_feature(alias_list, "alias list",, List all available aliases)
*/
