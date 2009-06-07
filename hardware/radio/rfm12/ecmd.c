/*
 * Copyright (c) 2008 by Stefan Siegl <stesie@brokenpipe.de>
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

#include "config.h"
#include "core/debug.h"
#include "hardware/radio/rfm12/rfm12.h"

#ifndef TEENSY_SUPPORT
#ifdef RFM12_SUPPORT

int16_t 
parse_cmd_rfm12_status(char *cmd, char *output, uint16_t len)
{
    return snprintf_P (output, len, PSTR ("rfm12 status: %04x"),
                       rfm12_get_status ());
}

#endif /* RFM12_SUPPORT */

#ifdef RFM12_IP_SUPPORT
int16_t
parse_cmd_rfm12_setbaud(char *cmd, char *output, uint16_t len)
{
    (void) output;
    (void) len;

    uint16_t baud;
    cmd[strlen(cmd) - 2] = 0;
    uint8_t ret = sscanf_P (cmd, PSTR ("%u"), &baud);

    if (ret != 1)
        return -1;

    rfm12_setbaud (baud);
    return 0;
}

int16_t
parse_cmd_rfm12_setbandwidth(char *cmd, char *output, uint16_t len)
{
  (void) output;
  (void) len;
  
  uint16_t bandwidth;
  uint8_t ret = sscanf_P (cmd, PSTR ("%u"), &bandwidth);
  
  if (ret != 1)
    return -1;
  
  rfm12_setbandwidth(bandwidth, rfm12_gain, rfm12_drssi);
  return 0;
}

int16_t
parse_cmd_rfm12_setgain(char *cmd, char *output, uint16_t len)
{
  (void) output;
  (void) len;
  
  uint16_t gain;
  uint8_t ret = sscanf_P (cmd, PSTR ("%u"), &gain);
  
  if (ret != 1)
    return -1;
  
  rfm12_setbandwidth(rfm12_bandwidth, gain, rfm12_drssi);
  return 0;
}

int16_t
parse_cmd_rfm12_setdrssi(char *cmd, char *output, uint16_t len)
{
  (void) output;
  (void) len;
  
  uint16_t drssi;
  uint8_t ret = sscanf_P (cmd, PSTR ("%u"), &drssi);
  
  if (ret != 1)
    return -1;
  
  rfm12_setbandwidth(rfm12_bandwidth, rfm12_gain, drssi);
  return 0;
}


int16_t
parse_cmd_rfm12_setmod(char *cmd, char *output, uint16_t len)
{
  (void) output;
  (void) len;
  
  uint16_t mod;
  uint8_t ret = sscanf_P (cmd, PSTR ("%u"), &mod);
  
  if (ret != 1)
    return -1;
  
  rfm12_setpower(0, mod);
  return 0;
}

#endif /* RFM12_IP_SUPPORT */
#endif /* not TEENSY_SUPPORT */
