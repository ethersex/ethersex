/*
 * Copyright (c) 2009 by Stefan Siegl <stesie@brokenpipe.de>
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

#include "config.h"
#include "protocols/ecmd/parser.h"
#include "protocols/ecmd/ecmd-base.h"
#include "httpd.h"


#ifdef DEBUG_HTTPD
#include "core/debug.h"
#define printf(a...)  debug_printf(a)
#else
#define printf(...)   ((void)0)
#endif

void
httpd_handle_ecmd_setup(char *encoded_cmd)
{
  char *ptr = STATE->u.ecmd.input;
  uint8_t maxlen = ECMD_INPUTBUF_LENGTH;

  for (; *encoded_cmd && maxlen; maxlen--, ptr++, encoded_cmd++)
  {
    uint8_t carry;

    switch (*encoded_cmd)
    {
      case '+':
        *ptr = ' ';
        break;

      case '%':
        carry = encoded_cmd[3];
        encoded_cmd[3] = 0;
        *ptr = strtoul(encoded_cmd + 1, NULL, 16);
        encoded_cmd[3] = carry;
        encoded_cmd += 2;       /* Skip two extra bytes. */
        break;

      default:
        *ptr = *encoded_cmd;
    }
  }

  if (maxlen == 0)
  {
    printf("httpd_ecmd: received ecmd too long.\n");
    STATE->handler = httpd_handle_400;
  } else {
    *ptr = 0;
    STATE->handler = httpd_handle_ecmd;
  }
}


static void
httpd_handle_ecmd_send_header(void)
{
  PASTE_RESET();
  PASTE_P(httpd_header_200);
  PASTE_P(httpd_header_ecmd);
  PASTE_SEND();
}


void
httpd_handle_ecmd(void)
{
  if (uip_acked())
    STATE->header_acked = 1;

  if (!STATE->header_acked)
  {
    httpd_handle_ecmd_send_header();
    return;
  }

  if (!uip_rexmit())
  {
    if (STATE->eof)
      uip_close();
    else
    {
      int16_t len = ecmd_parse_command(STATE->u.ecmd.input,
                                       STATE->u.ecmd.output,
                                       ECMD_OUTPUTBUF_LENGTH - 2);
      if (is_ECMD_AGAIN(len))
      {
        /* convert ECMD_AGAIN back to ECMD_FINAL */
        len = ECMD_AGAIN(len);
      }
      else if (is_ECMD_ERR(len))
      {                         /* Error */
        uip_close();
        return;
      }
      else
        STATE->eof = 1;

      STATE->u.ecmd.output[len++] = 10;
      STATE->u.ecmd.output[len] = 0;
    }
  }

  uip_send(STATE->u.ecmd.output, strlen(STATE->u.ecmd.output));
}
