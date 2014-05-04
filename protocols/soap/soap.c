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
#include "services/httpd/httpd.h"

static const char PROGMEM soap_xml_start[] =
  "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";

static const char PROGMEM soap_xml_envelope[] =
  "<soap:Envelope xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" "
  //"xmlns:soapenc=\"http://schemas.xmlsoap.org/soap/encoding/\" "
  "xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" "
  "soap:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" "
  "xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\">"
  "<soap:Body>";

static const char PROGMEM soap_xml_fault[] =
  "<soap:Fault><faultcode>soap:Client</faultcode>"
  "<faultstring>Unable to handle request.</faultstring>"
  "</soap:Fault>";

static const char PROGMEM soap_xml_result_start[] =
  "<%SResponse xmlns=\"http://ethersex.de/SOAP\">"
  "<s-sex xsi:type=\"xsd:%S\">";

static const char PROGMEM soap_xml_result_end[] =
  "</s-sex></%SResponse>";

static const char PROGMEM soap_xml_end[] =
  "</soap:Body></soap:Envelope>";

static const char PROGMEM soap_type_int[] = "int";
static const char PROGMEM soap_type_string[] = "string";
static const char* const soap_type_table[] PROGMEM =
{
  soap_type_int,
  soap_type_string,
  soap_type_int,
};


#define SOAP_STREAM_ERROR()					\
  do {								\
    SOAP_DEBUG("XML stream error in line %d.\n", __LINE__);	\
    ctx->error = 1;						\
    ctx->parsing_complete = 1;					\
    return;							\
  } while(0)

void
soap_initialize_context (soap_context_t *ctx)
{
  memset (ctx, 0, sizeof (soap_context_t));
}

static void
soap_lookup_funcname (soap_context_t *ctx)
{
  ctx->buf[ctx->buflen - 1] = 0; /* Strip away '>' */
  char *funcname = ctx->buf + 1;

  for (char *ptr = funcname; *ptr; ptr ++)
    if (isblank (*ptr))
      {
	*ptr = 0;
	break;
      }

  for (uint8_t i = 0;; i++)
    {
      PGM_P text = (PGM_P)pgm_read_word (&soap_cmds[i].name);
      if (text == NULL)
	/* End of list, i.e. not found */
	SOAP_STREAM_ERROR ();

      if (strcmp_P (funcname, text))
	continue;		/* Mis-match, try next. */

      ctx->rpc.name = text;
      ctx->rpc.handler = (void *) pgm_read_word (&soap_cmds[i].handler);
      return;
    }
}

static void
soap_parse_element (soap_context_t *ctx)
{
  if (*ctx->buf != '<')
    SOAP_STREAM_ERROR();

  if (ctx->buf[1] == '?')
    return;			/* ignore parser instruction. */

  if (ctx->parsing_complete)
    return;

  ctx->buf[ctx->buflen] = 0;
  SOAP_DEBUG ("parse_element %s\n", ctx->buf);
  if (!ctx->found_envelope)
    {
      if (strncmp_P (ctx->buf + 1, PSTR("Envelope"), 8))
	SOAP_STREAM_ERROR();
      ctx->found_envelope = 1;
    }

  else if (!ctx->found_body)
    {
      if (strncmp_P (ctx->buf + 1, PSTR("Body"), 4))
	return;			/* ignore anything until <Body> */
      ctx->found_body = 1;
    }
  else if (!ctx->found_funcname)
    {
      soap_lookup_funcname (ctx);
      ctx->found_funcname = 1;
    }
  else if (strncmp_P (ctx->buf + 1, PSTR("/Body"), 5) == 0)
    {
      ctx->parsing_complete = 1;
    }
  else if (ctx->buf[1] != '/' && ctx->argslen < SOAP_MAXARGS)
    {
      char *ptr = strstr_P (ctx->buf + 1, PSTR("type="));
      if (!ptr)
	SOAP_STREAM_ERROR ();

      ptr += 6;			/* Skip type=" */
      char *end = strchr (ptr, '"');
      if (!end)
	SOAP_STREAM_ERROR ();
      *end = 0;			/* chop off rest beyond type specifier */

      end = strchr (ptr, ':');
      if (end) ptr = end + 1;	/* Ignore namespace specifier */

      SOAP_DEBUG ("found arg type: '%s'\n", ptr);
      if (strcmp_P (ptr, PSTR("int")) == 0)
	ctx->args[ctx->argslen].type = SOAP_TYPE_INT;
      else if (strcmp_P (ptr, PSTR("string")) == 0)
	ctx->args[ctx->argslen].type = SOAP_TYPE_STRING;
      else
	SOAP_STREAM_ERROR ();
    }
}

static void
soap_parse_data (soap_context_t *ctx)
{
  if (ctx->argslen >= SOAP_MAXARGS) return;

  /* Zero-terminate data */
  ctx->buf[ctx->buflen] = 0;
  SOAP_DEBUG ("parse-data: '%s'\n", ctx->buf);

  switch (ctx->args[ctx->argslen].type)
    {
    case SOAP_TYPE_INT:
      ctx->args[ctx->argslen].u.d_int = atoi (ctx->buf);
      break;

    case SOAP_TYPE_STRING:
      ctx->args[ctx->argslen].u.d_string = malloc(strlen(ctx->buf));
      if (!ctx->args[ctx->argslen].u.d_string) return;
      strcpy(ctx->args[ctx->argslen].u.d_string, ctx->buf);
      SOAP_DEBUG ("args[%d].u.d_string = %s\n", ctx->argslen,
		  ctx->args[ctx->argslen].u.d_string);
      break;
    }

  ctx->argslen ++;
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


void
soap_evaluate (soap_context_t *ctx)
{
  soap_data_t result;
  if (ctx->rpc.handler (ctx->argslen, ctx->args, &result))
    ctx->error = 1;

  /* Free arg[0], before we store the result into it. */
  if (ctx->args[0].type == SOAP_TYPE_STRING)
    {
      free (ctx->args[0].u.d_string);
      ctx->args[0].u.d_string = NULL;
    }

  ctx->evaluated = 1;
  ctx->args[0] = result;
}


void
soap_deallocate_context (soap_context_t *ctx)
{
  for (uint8_t i = 0; i < SOAP_MAXARGS; i ++)
    if (ctx->args[i].type == SOAP_TYPE_STRING)
      {
	free (ctx->args[i].u.d_string);
	ctx->args[i].u.d_string = NULL;
      }
}

void
soap_paste_result (soap_context_t *ctx)
{
  PASTE_P (soap_xml_start);
  PASTE_P (soap_xml_envelope);

  if (ctx->error)
    PASTE_P (soap_xml_fault);
  else
    {
      uint8_t type = ctx->args[0].type;
      SOAP_DEBUG ("type = %d\n", type);

      PASTE_PF (soap_xml_result_start, ctx->rpc.name,
		pgm_read_word(&soap_type_table[type]));

      switch (type)
	{
	case SOAP_TYPE_INT:
	  PASTE_PF (PSTR("%d"), ctx->args[0].u.d_int);
	  break;

	case SOAP_TYPE_STRING:
	  strcat (uip_appdata, ctx->args[0].u.d_string);
	  break;

	case SOAP_TYPE_UINT32:
	  PASTE_PF (PSTR("%lu"), ctx->args[0].u.d_uint32);
	  break;

	default:
	  SOAP_DEBUG ("invalid soap-type assigned to result.\n");
	}

      PASTE_PF (soap_xml_result_end, ctx->rpc.name);
    }

  PASTE_P (soap_xml_end);
}
