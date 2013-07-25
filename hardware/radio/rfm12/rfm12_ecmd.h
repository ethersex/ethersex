/*
 * Copyright (c) 2008 Stefan Siegl <stesie@brokenpipe.de>
 * Copyright (c) 2013 Erik Kunze <ethersex@erik-kunze.de>
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

#ifndef __RFM12_ECMD_H_
#define __RFM12_ECMD_H_

int16_t parse_cmd_rfm12_status(char *, char *, uint16_t);
int16_t parse_cmd_rfm12_setbandwidth(char *, char *, uint16_t);
int16_t parse_cmd_rfm12_setgain(char *, char *, uint16_t);
int16_t parse_cmd_rfm12_setdrssi(char *, char *, uint16_t);
int16_t parse_cmd_rfm12_reinit(char *, char *, uint16_t);
int16_t parse_cmd_rfm12_setbaud(char *, char *, uint16_t);
int16_t parse_cmd_rfm12_setmod(char *, char *, uint16_t);

#endif /* __RFM12_ECMD_H_ */
