/* vim:fdm=marker ts=4 et ai
 * {{{
 *
 *          enc28j60 api
 *
 * (c) by Alexander Neumann <alexander@bumpern.de>
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
 }}} */


#ifndef _HC165_H
#define _HC165_H

#include <avr/io.h>
#include <util/delay.h>
#include "config.h"

#define HC165_LOAD PA0
#define HC165_CLOCK PA1
#define HC165_DATA PA2

#define HC165_DDR DDRA
#define HC165_PORT PORTA
#define HC165_PIN PINA

/* when using long cables, this might help: */
#if 0
#define HC165_DELAY() _delay_loop_2(2000)
#else
#define HC165_DELAY()
#endif


#ifdef USE_74HC165

void hc165_init(void);
uint8_t hc165_read_byte(void);

#endif

#endif
