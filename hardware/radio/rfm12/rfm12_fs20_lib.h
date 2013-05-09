/*
 * Copyright (c) 2012-13 Erik Kunze <ethersex@erik-kunze.de>
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

#ifndef __RFM12_FS20_LIB_H
#define __RFM12_FS20_LIB_H

#include <stdint.h>

#ifdef RFM12_ASK_ESA_SUPPORT
#define FS20_MAXMSG         20  /* ESA messages */
#else
#define FS20_MAXMSG         12  /* EMEM messages */
#endif

typedef struct
{
  uint8_t datatype;
  uint8_t count;
  uint8_t nibble;
  uint8_t data[FS20_MAXMSG];
} fs20_data_t;

extern uint8_t rx_report;

static void rfm12_fs20_lib_init(void);
static void rfm12_fs20_lib_rx_timeout(void);
static void rfm12_fs20_lib_rx_level_changed(uint8_t, uint8_t);
static int rfm12_fs20_lib_process(fs20_data_t *);

#endif /* __RFM12_FS20_LIB_H */
