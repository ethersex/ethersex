/*
 *
 * Copyright (c) by Alexander Neumann <alexander@bumpern.de>
 * Copyright (c) 2007 by Stefan Siegl <stesie@brokenpipe.de>
 * Copyright (c) 2007 by Christian Dietrich <stettberger@dokucode.de>
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

#include <string.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>

#include "config.h"
#include "core/debug.h"
#include "hardware/radio/fs20/fs20.h"
#include "hardware/lcd/hd44780.h"
#include "core/portio/named_pin.h"
#include "hardware/onewire/onewire.h"
#include "hardware/ir/rc5/rc5.h"
#include "hardware/radio/rfm12/rfm12.h"
#include "protocols/dns/resolv.h"
#include "services/clock/clock.h"
#include "protocols/yport/yport.h"


#ifdef RC5_SUPPORT
int16_t parse_cmd_ir_send(char *cmd, char *output, uint16_t len)
{
    int16_t ret;

    uint16_t addr, command;

    ret = sscanf_P(cmd, PSTR("%d %d"), &addr, &command);

    debug_printf("sending ir: device %d, command %d\n", addr, command);

    /* check if two values have been given */
    if (ret != 2)
        return -1;

    rc5_send(LO8(addr), LO8(command));
    return 0;

}

int16_t parse_cmd_ir_receive(char *cmd, char *output, uint16_t len)
{
    char *s = output;
    uint8_t l = 0;
    uint8_t outlen = 0;

#ifdef DEBUG_ECMD_RC5
    debug_printf("%u positions in queue\n", rc5_global.len);
#endif

    while (l < rc5_global.len && (uint8_t)(outlen+5) < len) {
#ifdef DEBUG_ECMD_RC5
        debug_printf("generating for pos %u: %02u/%02u", l,
                rc5_global.queue[l].address,
                rc5_global.queue[l].code);
#endif

        sprintf_P(s, PSTR("%02u%02u\n"),
                rc5_global.queue[l].address,
                rc5_global.queue[l].code);

        s += 5;
        outlen += 5;
        l++;

#ifdef DEBUG_ECMD_RC5
        *s = '\0';
        debug_printf("output is \"%s\"\n", output);
#endif
    }

    /* clear queue */
    rc5_global.len = 0;

    return outlen;
}
#endif /* RC5_SUPPORT */

