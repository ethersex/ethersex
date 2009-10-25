/*
 * Copyright (c) 2009 by Michael Stapelberg <michael+es@stapelberg.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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
#include <avr/pgmspace.h>
#include "config.h"
#include "sram.h"

#include "core/debug.h"

#include "protocols/ecmd/ecmd-base.h"

static uint8_t sram_memtest(void);

void
sram_init(void)
{
	SRAM_DEBUG("Enabling SRAM\n");

	MCUCR |= _BV(SRE);

#ifdef SRAM_MEMTEST_ON_BOOT
	sram_memtest();
#endif
}

/*
 * Writes a simple pattern (increasing values, going through one byte) to the
 * external SRAM to make sure that it is connected correctly. This is not a
 * thorough test and it will not necessarily find defect parts of your SRAM.
 *
 */
static uint8_t
sram_memtest(void)
{
	uint8_t *sram = SRAM_START_ADDRESS;
	uint8_t *cnt;
	uint8_t c = 0;
    uint8_t ok = 1;
	uint32_t wrote = 0;

	SRAM_DEBUG("verify: Writing to SRAM...\n");
	cnt = sram;
	for (cnt = sram; cnt < SRAM_END_ADDRESS; cnt++) {
		*cnt = c++;
		wrote++;
	}
	SRAM_DEBUG("verify: wrote %lu values\n", wrote);
	c = 0;
	for (cnt = sram; cnt < SRAM_END_ADDRESS; cnt++) {
		if (*cnt != c++) {
			debug_printf("RAM error at address %p: %d != %d\n", cnt, *cnt, c-1);
            ok = 0;
		}
	}
	SRAM_DEBUG("verify: finished\n");
    return ok;
}

#ifdef ECMD_PARSER_SUPPORT
int16_t
parse_cmd_sram_memtest(char *cmd, char *output, uint16_t len)
{
	if (sram_memtest())
        return ECMD_FINAL_OK;
    else
        return ECMD_FINAL( snprintf_P(output, len,
               PSTR("memtest error: see debugging output for more information")));
}
#endif

/*
  -- Ethersex META --
  header(hardware/sram/sram.h)
  init(sram_init)
  block(External SRAM support)
  ecmd_feature(sram_memtest, "sram memtest",, Perform a memory test)
*/
