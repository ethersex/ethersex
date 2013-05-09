/*
 * Copyright (c) 2009 by Stefan Siegl <stesie@brokenpipe.de>
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

#ifndef SOAP_H
#define SOAP_H

#include <avr/pgmspace.h>
#include <inttypes.h>

#define SOAP_MAXARGS	5

enum soap_parser_state 
  {
    SOAP_PARSER_WHITE,
    SOAP_PARSER_IGNORE_STRING,
    SOAP_PARSER_WAIT_STRING_END,
    SOAP_PARSER_ELEMENT,
    SOAP_PARSER_ELEMENT_WHITE,
    SOAP_PARSER_DATA,
  };

enum soap_data_types
  {
    SOAP_TYPE_INT,
    SOAP_TYPE_STRING,

    /* further types thar are only serializable */
    SOAP_TYPE_UINT32,
  };

typedef struct soap_data soap_data_t;
struct soap_data {
  uint8_t type;
  union {
    int16_t	d_int;
    char *	d_string;
    uint32_t	d_uint32;
  } u;
};

typedef struct soap_command soap_command_t;
struct soap_command {
  PGM_P name;
  uint8_t (* handler) (uint8_t num, soap_data_t *, soap_data_t *);
};


typedef struct soap_context soap_context_t;
struct soap_context {
  unsigned found_envelope	:1;
  /* unsigned found_header	:1; */
  unsigned found_body		:1;
  unsigned found_funcname	:1;

  unsigned error		:1;
  unsigned parsing		:1;
  unsigned parsing_complete	:1;
  unsigned copy_string		:1;
  unsigned evaluated		:1;

  uint8_t parser_state;

  char buf[69];
  uint8_t buflen;
  uint8_t buf_backtrack_pos;

  soap_data_t args[SOAP_MAXARGS];
  uint8_t argslen;

  soap_command_t rpc;
};

void soap_initialize_context (soap_context_t *ctx);
void soap_parse (soap_context_t *ctx, char *buf, uint16_t len);
void soap_paste_result (soap_context_t *ctx);
void soap_evaluate (soap_context_t *ctx);
void soap_deallocate_context (soap_context_t *ctx);

#include "config.h"
#include "core/debug.h"
#define SOAP_DEBUG(a...) debug_printf("SOAP: " a);

extern const soap_command_t PROGMEM soap_cmds[];

#endif	/* SOAP_H */
