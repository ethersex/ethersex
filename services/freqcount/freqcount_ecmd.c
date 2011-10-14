/*
* ECMD-commands for frequency counter
*
* Copyright (c) 2011 by Gerd v. Egidy <gerd@egidy.de>
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

#include <string.h>
#include <stdlib.h>
#include <util/delay.h>
#include <avr/pgmspace.h>

#include "config.h"
#include "core/debug.h"
#include "protocols/ecmd/ecmd-base.h"

#include "services/freqcount/freqcount.h"

int16_t parse_cmd_fc_ticks(char *cmd, char *output, uint16_t len)
{
    // the longest message is 10 bytes + \0
    if (len < 11)
        return ECMD_FINAL(ECMD_ERR_PARSE_ERROR);

    return ECMD_FINAL(snprintf_P(output, len, PSTR("%lu"),freqcount_ticks_result));
}

int16_t parse_cmd_fc_freq(char *cmd, char *output, uint16_t len)
{
    // the longest message is 10 bytes + \0
    if (len < 11)
        return ECMD_FINAL(ECMD_ERR_PARSE_ERROR);

    return ECMD_FINAL(snprintf_P(output, len, PSTR("%lu"),freqcount_get_freq_hz()));
}

int16_t parse_cmd_fc_duty(char *cmd, char *output, uint16_t len)
{
    // the longest message is 3 bytes + \0
    if (len < 4)
        return ECMD_FINAL(ECMD_ERR_PARSE_ERROR);

    itoa(freqcount_duty_result,output,10);
    return ECMD_FINAL(strlen(output));
}

int16_t parse_cmd_fc_percent_duty(char *cmd, char *output, uint16_t len)
{
    // the longest message is 4 bytes + \0
    if (len < 5)
        return ECMD_FINAL(ECMD_ERR_PARSE_ERROR);

    uint8_t percent=(((uint16_t)(freqcount_duty_result))*100)/256;

    itoa(percent,output,10);
    return ECMD_FINAL(strlen(output));
}

/*
  -- Ethersex META --
  block([[Frequency Counter]])
  ecmd_ifdef(FREQCOUNT_SUPPORT)
    ecmd_feature(fc_freq, "fc freq", [CHANNEL], "returns last frequency in Hz, channel is always 0 (for now)")
    ecmd_feature(fc_ticks, "fc ticks", [CHANNEL], "returns last frequency in CPU ticks, channel is always 0 (for now)")
    ecmd_ifdef(FREQCOUNT_DUTY_SUPPORT)
        ecmd_feature(fc_duty, "fc duty", [CHANNEL], "returns last on duty cycle (0-255), channel is always 0 (for now)")
        ecmd_feature(fc_percent_duty, "fc %duty", [CHANNEL], "returns last on duty cycle in percent, channel is always 0 (for now)")
    ecmd_endif()
  ecmd_endif()
*/
