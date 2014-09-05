/*
 * Infrared-Multiprotokoll-Decoder 
 *
 * for additional information please
 * see http://www.mikrocontroller.net/articles/IRMP
 *
 * Copyright (c) 2010-14 by Erik Kunze <ethersex@erik-kunze.de>
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

#ifndef __IRMP_H
#define __IRMP_H

#include <stdint.h>
#include <stdio.h>
#include <avr/pgmspace.h>

#ifdef IRMP_SUPPORT

typedef enum
{
  IRMP_PROTO_NONE,              /* None */
  IRMP_PROTO_SIRCS,             /* Sony */
  IRMP_PROTO_NEC,               /* NEC, Pioneer, JVC, Toshiba, NoName etc. */
  IRMP_PROTO_SAMSUNG,           /* Samsung */
  IRMP_PROTO_MATSUSHITA,        /* Matsushita */
  IRMP_PROTO_KASEIKYO,          /* Kaseikyo (Panasonic etc) */
  IRMP_PROTO_RECS80,            /* Thomson, Nordmende, Telefunken, Saba */
  IRMP_PROTO_RC5,               /* Philips RC5 */
  IRMP_PROTO_DENON,             /* Denon */
  IRMP_PROTO_RC6,               /* Philips RC6 */
  IRMP_PROTO_SAMSUNG32,         /* Samsung 32 */
  IRMP_PROTO_APPLE,             /* Apple */
  IRMP_PROTO_RECS80EXT,         /* Philips, Technisat, Thomson, Nordmende,
                                 * Telefunken, Saba */
  IRMP_PROTO_NUBERT,            /* Nubert */
  IRMP_PROTO_BANGOLUFSEN,       /* Bang & Olufsen */
  IRMP_PROTO_GRUNDIG,           /* Grundig */
  IRMP_PROTO_NOKIA,             /* Nokia */
  IRMP_PROTO_SIEMENS,           /* Siemens */
  IRMP_PROTO_FDC,               /* FDC Keyboard */
  IRMP_PROTO_JVC,               /* JVC */
  IRMP_PROTO_RC6A,              /* RC6A, e.g. Kathrein, XBOX */
  IRMP_PROTO_NIKON,             /* Nikon */
  IRMP_PROTO_RUWIDO,            /* Ruwido, e.g. T-Home Mediareceiver */
  IRMP_PROTO_IR60,              /* IR60 (SAB2008) */
  IRMP_PROTO_KATHREIN,          /* Kathrein */
  IRMP_PROTO_NETBOX,            /* Netbox keyboard (bitserial) */
  IRMP_PROTO_NEC16,             /* NEC with 16 bits */
  IRMP_PROTO_NEC42,             /* NEC with 42 bits */
  IRMP_PROTO_LEGO,              /* LEGO Power Functions RC */
  IRMP_PROTO_THOMSON,           /* Thomson */
  IRMP_PROTO_BOSE,              /* Bose */
  IRMP_PROTO_A1TVBOX,           /* A1 TV-Box */
  IRMP_PROTO_ORTEK,             /* ORTEK - Hama */
  IRMP_PROTO_TELEFUNKEN,        /* Telefunken (1560) */
  IRMP_PROTO_ROOMBA,            /* iRobot Roomba vacuum cleaner */
  IRMP_PROTO_RCMM32,            /* Fujitsu-Siemens (Activy remote control) */
  IRMP_PROTO_RCMM24,            /* Fujitsu-Siemens (Activy remote control) */
  IRMP_PROTO_RCMM12,            /* Fujitsu-Siemens (Activy remote control) */
  IRMP_PROTO_SPEAKER,           /* Another loudspeaker protocol, similar to Nubert */
  IRMP_PROTO_LGAIR,             /* LG air conditioner */
  IRMP_PROTO_SAMSUNG48,         /* air conditioner with SAMSUNG protocol (48 bits) */
  IRMP_PROTO_RADIO1             /* Radio protocol (experimental stat */
} irmp_prot_e;

typedef struct
{
  irmp_prot_e protocol;         /* protocol */
  uint16_t address;             /* address */
  uint16_t command;             /* command */
  uint8_t flags;                /* repeated key */
} irmp_data_t;

#ifdef DEBUG_IRMP
extern const PGM_P const irmp_proto_names[] PROGMEM;
#endif

/* prototypes */
void irmp_init(void);
irmp_data_t * irmp_read(void);
void irmp_write(irmp_data_t *);
void irmp_process(void);

#endif /* IRMP_SUPPORT */
#endif /* __IRMP_H */
