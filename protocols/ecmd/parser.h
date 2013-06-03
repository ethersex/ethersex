/*
 * (c) by Alexander Neumann <alexander@bumpern.de>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License (either version 2 or
 * version 3) as published by the Free Software Foundation.
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

#ifndef _ECMD_PARSER_H
#define _ECMD_PARSER_H

#include "config.h"
#include <avr/pgmspace.h>
#include <inttypes.h>

/* returns >= 0 for output, -1 for parse error,
 * < _ECMD_AGAIN_MAGIC  for "generated output, but needs to be called again,
 *        output bytes: ECMD_AGAIN(ret) */
int16_t ecmd_parse_command(char *cmd, char *output, uint16_t len);

/* struct for storing commands */
struct ecmd_command_t {
    PGM_P name;
    int16_t (*func)(char*, char*, uint16_t);
};

/* automatically generated via meta system */
extern const struct ecmd_command_t ecmd_cmds[];

#endif /* _ECMD_PARSER_H */
