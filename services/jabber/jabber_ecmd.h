/*
 * Copyright (c) 2009 Stefan Müller <mueller@cos-gbr.de>
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

#ifndef JABBER_ECMD_H
#define JABBER_ECMD_H

#include <stdint.h>

int16_t parse_cmd_user(char *, char *, uint16_t);
int16_t parse_cmd_pass(char *, char *, uint16_t);
int16_t parse_cmd_resrc(char *, char *, uint16_t);
int16_t parse_cmd_host(char *, char *, uint16_t);

#endif /* JABBER_ECMD_H */
