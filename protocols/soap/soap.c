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

#include <ctype.h>
#include <string.h>

#include "protocols/soap/soap.h"

#define SOAP_STREAM_ERROR()			\
  do {						\
    SOAP_DEBUG("XML stream error.\n");		\
    ctx->error = 1;				\
    return;					\
  } while(0)

void
soap_initialize_context (soap_context_t *ctx)
{
  memset (ctx, 0, sizeof (soap_context_t));
}

static void
soap_parse_element (soap_context_t *ctx)
{
  ctx->buf[ctx->buflen] = 0;
  SOAP_DEBUG ("parse_element: %s\n", ctx->buf);
}

static void
soap_parse_data (soap_context_t *ctx)
{
  ctx->buf[ctx->buflen] = 0;
  SOAP_DEBUG ("parse_data: %s\n", ctx->buf);
}

static inline void
soap_buf_putchar (soap_context_t *ctx, char ch)
{
  if (ctx->buflen >= sizeof(ctx->buf) - 1)
    {
      SOAP_DEBUG ("putchar: out of memory.\n");
      return;			/* Not enough memory. */
    }

  /* SOAP_DEBUG ("putchar '%c', state=%d\n", ch, ctx->parser_state); */
  ctx->buf[ctx->buflen ++] = ch;
}

static inline void
soap_buf_backtrack (soap_context_t *ctx)
{
  ctx->buflen = ctx->buf_backtrack_pos;
}

static inline void
soap_buf_backtrack_savepos (soap_context_t *ctx)
{
  ctx->buf_backtrack_pos = ctx->buflen;
}

static inline void
soap_buf_clear (soap_context_t *ctx)
{
  ctx->buflen = 0;
  ctx->buf_backtrack_pos = 0;
}

void
soap_parse (soap_context_t *ctx, char *buf, uint16_t len)
{
  SOAP_DEBUG ("soap_parse: %s (error=%d, len=%d).\n", buf, ctx->error, len);

  if (ctx->error)
    return;

  for (; len; buf ++, len --)
    {
      switch (ctx->parser_state)
	{
	case SOAP_PARSER_WHITE:
	  if (isblank (*buf))
	    break;		/* Ignore whitespace. */

	  if (*buf == '<')
	    {			/* Found start of element. */
	      soap_buf_putchar (ctx, *buf);
	      soap_buf_backtrack_savepos (ctx);
	      ctx->parser_state = SOAP_PARSER_ELEMENT;
	      break;
	    }

	  SOAP_STREAM_ERROR();

	  /*******************************************************************/
	case SOAP_PARSER_IGNORE_STRING:
	  if (*buf == '"')
	    {
	      ctx->copy_string = 0;
	      ctx->parser_state = SOAP_PARSER_WAIT_STRING_END;
	    }
	  break;

	case SOAP_PARSER_WAIT_STRING_END:
	  if (ctx->copy_string)
	    soap_buf_putchar (ctx, *buf);
	  if (*buf == '"')
	    ctx->parser_state = SOAP_PARSER_ELEMENT_WHITE;
	  break;

	case SOAP_PARSER_ELEMENT_WHITE:
	  if (isblank (*buf))
	    break;		/* Ignore extra whitespace. */

	  /* Switch back to normal element parsing, fall through. */
	  ctx->parser_state = SOAP_PARSER_ELEMENT;

	case SOAP_PARSER_ELEMENT:
	  if (*buf == ' ')
	    {
	      soap_buf_putchar (ctx, *buf);
	      soap_buf_backtrack_savepos (ctx);
	      ctx->parser_state = SOAP_PARSER_ELEMENT_WHITE;
	      break;
	    }

	  else if (*buf == '=' || *buf == ':')
	    {
	      if (ctx->buflen >= 5
		  && strncmp_P (&ctx->buf[ctx->buflen - 5],
				PSTR("xmlns"), 5) == 0)
		{
		  /* Found xmlns="..." structure, ignore it. */
		  soap_buf_backtrack (ctx);
		  ctx->parser_state = SOAP_PARSER_IGNORE_STRING;
		  break;
		}

	      if (*buf == ':')
		{			/* Ooops, namespace, kill it. */
		  soap_buf_backtrack (ctx);
		  break;		/* Ignore the colon. */
		}
	    }

	  else if (*buf == '>')
	    {			/* Found end of element. */
	      if (ctx->buflen && isblank(ctx->buf[ctx->buflen - 1]))
		ctx->buflen --;	/* Don't make <element >, but remove space */

	      soap_buf_putchar (ctx, *buf);
	      soap_parse_element (ctx);

	      if (ctx->buflen > 2 && ctx->buf[1] == '/')
		ctx->parser_state = SOAP_PARSER_WHITE; /* End Tag */
	      else
		ctx->parser_state = SOAP_PARSER_DATA;

	      soap_buf_clear (ctx);
	      break;
	    }

	  else if (*buf == '"')
	    {
	      ctx->copy_string = 1;
	      ctx->parser_state = SOAP_PARSER_WAIT_STRING_END;
	    }

	  soap_buf_putchar (ctx, *buf);

	  if (*buf == '/')
	    soap_buf_backtrack_savepos (ctx);
	  break;

	case SOAP_PARSER_DATA:
	  if (*buf == '<')
	    {
	      if (ctx->buflen)
		soap_parse_data (ctx);
	      soap_buf_clear (ctx);
	      ctx->parser_state = SOAP_PARSER_ELEMENT;
	    }
	  soap_buf_putchar (ctx, *buf);
	  soap_buf_backtrack_savepos (ctx);
	  break;
	}
    }
}
