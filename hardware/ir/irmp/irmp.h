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
  IRMP_PROTO_NONE,		/* None */
  IRMP_PROTO_SIRCS,		/* Sony */
  IRMP_PROTO_NEC,		/* NEC, Pioneer, JVC, Toshiba, NoName etc. */
  IRMP_PROTO_SAMSUNG,		/* Samsung */
  IRMP_PROTO_MATSUSHITA,	/* Matsushita */
  IRMP_PROTO_KASEIKYO,		/* Kaseikyo (Panasonic etc) */
  IRMP_PROTO_RECS80,		/* Thomson, Nordmende, Telefunken, Saba */
  IRMP_PROTO_RC5,		/* Philips RC5 */
  IRMP_PROTO_DENON,		/* Denon */
  IRMP_PROTO_RC6,		/* Philips RC6 */
  IRMP_PROTO_SAMSUNG32,		/* Samsung 32 */
  IRMP_PROTO_APPLE,		/* Apple */
  IRMP_PROTO_RECS80EXT,		/* Philips, Technisat, Thomson, Nordmende,
				   Telefunken, Saba */
  IRMP_PROTO_NUBERT,		/* Nubert */
  IRMP_PROTO_BANGOLUFSEN,	/* Bang & Olufsen */
  IRMP_PROTO_GRUNDIG,		/* Grundig */
  IRMP_PROTO_NOKIA,		/* Nokia */
  IRMP_PROTO_SIEMENS,		/* Siemens */
  IRMP_PROTO_FDC,		/* FDC Keyboard */
  IRMP_PROTO_RCCAR		/* RC Car */
} irmp_prot_e;

typedef struct
{
  irmp_prot_e protocol;		/* protocol */
  uint16_t address;		/* address */
  uint16_t command;		/* command */
  uint8_t flags;		/* repeated key */
} irmp_data_t;

///////////////
#define IRMP_DATA irmp_data_t
#include "irmp_lib.h"
#ifdef IRSND_SUPPORT
#include "irsnd_lib.h"
#endif
#define IRMP_FLAG_REPEAT IRMP_FLAG_REPETITION
#undef F_INTERRUPTS
///////////////

extern const PGM_P irmp_proto_names[] PROGMEM;

/* prototypes */
void irmp_init (void);
uint8_t irmp_read (irmp_data_t *);
void irmp_write (irmp_data_t *);
void irmp_process (void);

#endif /* IRMP_SUPPORT */
#endif /* IRMP_H */
