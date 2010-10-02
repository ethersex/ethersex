/*
 * Infrared-Multiprotokoll-Decoder 
 *
 * for additional information please
 * see http://www.mikrocontroller.net/articles/IRMP
 *
 * Copyright (c) 2010 by Erik Kunze <ethersex@erik-kunze.de>
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

#ifndef IRMP_H
#define IRMP_H

#include <stdint.h>
#include <stdio.h>
#include <avr/pgmspace.h>

#ifdef IRMP_SUPPORT

typedef enum
{
  IR_PROTO_NONE,		/* None */
  IR_PROTO_SIRCS,		/* Sony */
  IR_PROTO_NEC,			/* NEC, Pioneer, JVC, Toshiba, NoName etc. */
  IR_PROTO_SAMSUNG,		/* Samsung */
  IR_PROTO_MATSUSHITA,		/* Matsushita */
  IR_PROTO_KASEIKYO,		/* Kaseikyo (Panasonic etc) */
  IR_PROTO_RECS80,		/* Thomson, Nordmende, Telefunken, Saba */
  IR_PROTO_RC5,			/* Philips RC5 */
  IR_PROTO_DENON,		/* Denon */
  IR_PROTO_RC6,			/* Philips RC6 */
  IR_PROTO_SAMSUNG32,		/* Samsung 32 */
  IR_PROTO_APPLE,		/* Apple */
  IR_PROTO_RECS80EXT,		/* Philips, Technisat, Thomson, Nordmende,
				   Telefunken, Saba */
  IR_PROTO_NUBERT,		/* Nubert */
  IR_PROTO_BANGOLUFSEN,		/* Bang & Olufsen */
  IR_PROTO_GRUNDIG,		/* Grundig */
  IR_PROTO_NOKIA,		/* Nokia */
  IR_PROTO_SIEMENS,		/* Siemens */
  IR_PROTO_FDC,			/* FDC Keyboard */
  IR_PROTO_RCCAR		/* RC Car */
} ir_prot_e;

typedef struct
{
  ir_prot_e protocol;		/* protocol */
  uint16_t address;		/* address */
  uint16_t command;		/* command */
  uint8_t flags;		/* repeated key */
} ir_data_t;

///////////////
#define IRMP_DATA ir_data_t
#include "irmp_lib.h"
#define IRMP_FLAG_REPEAT IRMP_FLAG_REPETITION
#undef F_INTERRUPTS
///////////////

extern const PGM_P proto_names[] PROGMEM;
extern const char cmd_ir_text[] PROGMEM;

/* prototypes */
void irmp_init (void);
uint8_t irmp_read (ir_data_t *);
void irmp_process (void);

#endif /* IRMP_SUPPORT */
#endif /* IRMP_H */
