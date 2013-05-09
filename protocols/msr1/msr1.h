/*
 *
 * Copyright (c) 2009 by Christian Dietrich <stettberger@dokucode.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
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

#ifndef _MSR1_H
#define _MSR1_H

enum msr1_state {
  MSR1_REQUEST_E8,
  MSR1_REQUEST_C0,
  MSR1_REQUEST_48,
  MSR1_REQUEST_50,
};

/* Default baudrate */
#define MSR1_BAUDRATE 9600

void msr1_init(void);
void msr1_periodic(void);

struct msr1_e8_info {
  uint8_t data[22];
  uint8_t len;
  uint8_t chksum;
};

struct msr1_generic_info {
  uint8_t data[76];
  uint8_t len;
  uint8_t chksum;
};

#endif /* _MSR1_H */
