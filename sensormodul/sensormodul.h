/*
 * Copyright (c) 2007 by Jochen Roessner <jochen@lugrot.de>
 * Copyright (c) 2008 by Jochen Roessner <jochen@lugrot.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
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

#ifndef _SENSORMODUL_H
#define _SENSORMODUL_H

#include <stdint.h>
#include "../uip/uip.h"

//#define SENSORMODUL_LCDTEXTLEN 9

/* prototypes */
void sensormodul_core_init (uip_udp_conn_t *sensormodul_conn);
void sensormodul_core_periodic (void);
void sensormodul_core_newdata (void);
//void sensormodul_setlcdtext (char *text, uint8_t len);

#endif
