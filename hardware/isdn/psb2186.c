/*
 * Copyright (c) 2009 by Stefan Siegl <stesie@brokenpipe.de>
 * Copyright (c) 2009 by Christian Dietrich <stettberger@dokucode.de>
 * Copyright (c) 2009 by Jochen Roessner <jochen@zerties.org>
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

#include <string.h>

#include <avr/pgmspace.h>

#include "config.h"
#include "core/debug.h"
#include "hardware/isdn/psb2186.h"

#define PSB2186_MASK 0x20
#define PSB2186_ADF2 0x39
#define PSB2186_ADF1 0x38
#define PSB2186_SPCR 0x30
#define PSB2186_MODE 0x22
#define PSB2186_STCR 0x37
#define PSB2186_CMDR 0x21

#define nop()	asm("nop"); asm("nop"); asm("nop");asm("nop"); asm("nop");

static struct {
    unsigned initialized    : 1;
    unsigned read_caller    : 1;
    unsigned read_callee    : 1;
    unsigned read_len	    : 1;
    unsigned found_call	    : 1;
    unsigned prefixlen	    : 3;
    uint8_t leftlen;

    char caller[CALLER_BUF_LENGTH];
    char callee[CALLEE_BUF_LENGTH];
} psb2186_state;

psb2186_callback_t psb2186_callback = NULL;

static uint8_t
psb2186_read(uint8_t addr)
{
    PIN_CLEAR(PSB2186_CS);

    DDRA = 0xFF;
    PORTA = addr;
    PIN_SET(PSB2186_ALE);
    nop();
    PIN_CLEAR(PSB2186_ALE);

    DDRA = 0;
    PORTA = 0;

    PIN_CLEAR(PSB2186_RD);
    nop();
    uint8_t byte = PINA;
    PIN_SET(PSB2186_RD);

    PIN_SET(PSB2186_CS);
    return byte;
}

static void
psb2186_write(uint8_t addr, uint8_t value)
{
    PIN_CLEAR(PSB2186_CS);

    DDRA = 0xFF;
    PORTA = addr;
    PIN_SET(PSB2186_ALE);
    nop();
    PIN_CLEAR(PSB2186_ALE);

    PORTA = value;

    PIN_CLEAR(PSB2186_WR);
    nop();
    PIN_SET(PSB2186_WR);

    PIN_SET(PSB2186_CS);
}

void
psb2186_init(void)
{
    PIN_SET(PSB2186_CS);
    PIN_SET(PSB2186_RD);
    PIN_SET(PSB2186_WR);
    PIN_CLEAR(PSB2186_ALE);
}

void
psb2186_readdata(uint8_t *buf, uint8_t pos, uint8_t valid)
{

    for (; pos < valid; pos ++)
    {
	if (psb2186_state.read_len)
	{
	    psb2186_state.leftlen = buf[pos];
	    psb2186_state.read_len = 0;
	}
	else if (psb2186_state.leftlen)
	{
	    if (psb2186_state.prefixlen)
		psb2186_state.prefixlen --;
	    else if (psb2186_state.read_caller)
	    {
		int l = strlen(psb2186_state.caller);
		psb2186_state.caller[l] = buf[pos];
	    }
	    else if (psb2186_state.read_callee)
	    {
		int l = strlen(psb2186_state.callee);
		psb2186_state.callee[l] = buf[pos];
	    }

	    psb2186_state.leftlen --;
	}
	else // read field type byte
	{
	    uint8_t field_type = buf[pos];
	    if (field_type == 0x6c)
	    {
		psb2186_state.read_caller = 1;
		psb2186_state.prefixlen = 2;
	    }
	    else
		psb2186_state.read_caller = 0;

	    if (field_type == 0x70)
	    {
		psb2186_state.read_callee = 1;
		psb2186_state.prefixlen = 1;
	    }
	    else
		psb2186_state.read_callee = 0;

	    psb2186_state.read_len = 1;
	}
    }
}

void
psb2186_incoming_call(void)
{
    debug_printf ("incoming call: %s -> %s\n",
	psb2186_state.caller[0] ? psb2186_state.caller : "Rufnummer unbekannt",
	psb2186_state.callee);
	if (psb2186_callback != NULL)
	  psb2186_callback(&psb2186_state.callee[0], &psb2186_state.caller[0]);
}

void
psb2186_process(void)
{
    if (!psb2186_state.initialized && psb2186_read(0x21)) {
	psb2186_state.initialized = 1;

	psb2186_write(PSB2186_MASK, 0xFF);
	psb2186_write(PSB2186_ADF2, 0x00);
	psb2186_write(PSB2186_ADF1, 0x08);
	psb2186_write(PSB2186_SPCR, 0x00);
	psb2186_write(PSB2186_MODE, 0xc9);
	psb2186_write(PSB2186_STCR, 0x70);
	psb2186_write(PSB2186_CMDR, 0x40);
	psb2186_write(PSB2186_MASK, 0x00);
    }

    uint8_t isr = psb2186_read(0x20);
    if (isr & 0xC0) {
	uint8_t len = psb2186_read(0x25);
	uint8_t buf[len];

	for (uint8_t i = 0; i < (len > 0x20 ? (len & 0x1F) : len); i ++)
	    buf[i] = psb2186_read(i);

	if (psb2186_state.found_call && len > 0x20) {
	    // second message part
	    psb2186_readdata(buf, 0, len & 0x1F);
	    psb2186_incoming_call();
	}
	else if (len > 0x10 && buf[6] == 0x05 && buf[7] == 0xa1) {
	    // first part of setup message
	    memset (&psb2186_state, 0, sizeof(psb2186_state));
	    psb2186_state.found_call = 1;
	    psb2186_state.initialized = 1;
	    psb2186_readdata(buf, 8, len);

	    if (isr & 0x80)
		psb2186_incoming_call();
	}
	else
	    psb2186_state.found_call = 0;

	psb2186_write(PSB2186_CMDR, 0x80);
    }

}

/*
  -- Ethersex META --
  header(hardware/isdn/psb2186.h)
  init(psb2186_init)
  timer(1, psb2186_process())
*/
