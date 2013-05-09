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

#include "core/tty/tty.h"
#include "protocols/uip/uip.h"

#define STATE (&uip_conn->appstate.tty_vt100)

static inline void
tty_vt100_send_all (void)
{
  char *ptr = memcpy_P (uip_sappdata, vt100_clr_str, 7) + 7;

  for (uint8_t y = 0; y < LINES; y ++)
    {
      memcpy (ptr, &tty_image[COLS * y], COLS);
      ptr += COLS;

      *(ptr ++) = '\r';
      *(ptr ++) = '\n';
    }

  uip_send (uip_sappdata, ptr - ((char *)uip_sappdata));
  STATE->sent = vt100_head;	/* after send_all we're up to head. */
}


static void
tty_vt100_main (void)
{
  if (uip_connected())
    STATE->send_all = 1;

  if (uip_acked())
    {
      if (STATE->send_all)
	STATE->send_all = 0;
      STATE->acked = STATE->sent;
    }

  if (uip_newdata ())
    {
      uint8_t len = uip_len;
      for (uint8_t i = 0; i < len; i ++)
	{
	  int8_t ch = ((char *) uip_appdata)[i];

	  if (ch == 12)		/* C-l, retransmit everything. */
	    {
	      STATE->send_all = 1;
	      continue;
	    }

	  _getch_queue (ch);
	}
    }

  if (uip_rexmit() || uip_newdata() || uip_acked() || uip_connected()
      || uip_poll())
    {
      /* Send new data, if any. */
      if (STATE->send_all)
	tty_vt100_send_all ();

      else
	{
	  /* we're just sending an update, ... */
	  if (vt100_head > STATE->acked)
	    {
	      uint8_t len = vt100_head - STATE->acked;
	      memcpy (uip_sappdata, STATE->acked, len);
	      uip_send (uip_sappdata, len);
	    }
	  else if (vt100_head < STATE->acked)
	    {
	      /* vt100_head wrapped around, let's be careful. */
	      uint8_t len = vt100_end - STATE->acked;
	      memcpy (uip_sappdata, STATE->acked, len);
	      uint8_t len2 = vt100_head - vt100_buf;
	      memcpy (uip_sappdata + len, vt100_buf, len2);
	      uip_send (uip_sappdata, len + len2);
	    }

	  STATE->sent = vt100_head;
	}
    }
}

void
tty_vt100_telnet_init (void)
{
  uip_listen(HTONS(TELNET_TCP_PORT), tty_vt100_main);
}

/*
  -- Ethersex META --
  header(core/tty/tty-vt100-telnet.h)
  net_init(tty_vt100_telnet_init)

  state_header(core/tty/tty-vt100-telnet.h)
  state_tcp(struct tty_vt100_state_t tty_vt100)
*/
