/*
 * Copyright (c) 2010 by Stefan Riepenhausen <rhn@gmx.net>
 * Copyright (c) 2010 by Dirk Pannenbecker <dp@sd-gp.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
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

#include <stdio.h>
#include "config.h"
#include <avr/pgmspace.h>

#include "soft_uart.h"
#include "../dmx/dmx.h"

#include "remote-proto2.h"

#include "protocols/ecmd/ecmd-base.h"

/**
 * Sent initial sync sequenz
 */
void
fnordlicht_init(void)
{
 // clear buffer and sync soft uart
  for (uint8_t i=0;i<14;i++)
    soft_uart_putchar(0xff);
    
  soft_uart_putchar(0x00);
  // send sync
  for (uint8_t i=0;i<15;i++)
    soft_uart_putchar(0x1b);

  soft_uart_putchar(0x00);

  soft_uart_putchar(0xff);
  soft_uart_putchar(0x08);
  for (uint8_t i=0;i<13;i++)
    soft_uart_putchar(0x00);
    
}


/**
 * Sent some bytes as Fnordlicht-channels
 */
void
fnordlicht_send_chan_x(uint8_t sizeofchan, struct remote_msg_fade_rgb_t msg)
{
  soft_uart_putchar(msg.address);
  soft_uart_putchar(msg.cmd);
  soft_uart_putchar(msg.step);
  soft_uart_putchar(msg.delay);
  soft_uart_putchar(msg.color.red);
  soft_uart_putchar(msg.color.green);
  soft_uart_putchar(msg.color.blue);
 
  for (uint8_t i=sizeof(struct remote_msg_fade_rgb_t); i < sizeofchan; i++)
    soft_uart_putchar(0x00);

}

/**
 * Send Fnordlicht-packet
 */
void
fnordlicht_periodic(void)
{
  wdt_kick();
/*
  //if(dmx_index == 0) {
    fnordlicht_send_chan_x(15, (uint8_t []){0x00, 0x01, 0xff, 0x01, dmx_data[6], dmx_data[7], dmx_data[8], 0, 0, 0, 0, 0, 0, 0, 0});
    fnordlicht_send_chan_x(15, (uint8_t []){0x01, 0x01, 255, 1, dmx_data[9], dmx_data[10], dmx_data[11], 0, 0, 0, 0, 0, 0, 0, 0});
    //fnordlicht_sent_chan_x(15, (uint8_t []){0xff, 0x01, 255, 0, dmx_data[3], dmx_data[4], dmx_data[5], 0, 0, 0, 0, 0, 0, 0, 0});
    wdt_kick();
 // }
*/
}

int16_t 
parse_cmd_fnordlicht_reinit (char *cmd, char *output, uint16_t len)
{
  fnordlicht_init();
  return ECMD_FINAL_OK;
}

int16_t 
parse_cmd_fnordlicht (char *cmd, char *output, uint16_t len)
{
  struct remote_msg_fade_rgb_t msg;

  sscanf_P(cmd, PSTR("%u %u,%u,%u"), &msg.address, &msg.color.red, &msg.color.green, &msg.color.blue);
  msg.cmd=REMOTE_CMD_FADE_RGB;
  msg.step=0xff;
  msg.delay=0x01;
//  uint8_t channel[] = {adr, 0x01, 0xff, 0x01, rgb[0], rgb[1], rgb[2], 0, 0, 0, 0, 0, 0, 0, 0};
  fnordlicht_send_chan_x(15, msg);
  return ECMD_FINAL_OK;
}


/*
  -- Ethersex META --
  header(protocols/fnordlicht/fnordlicht.h)
  init(fnordlicht_init)
  timer(1, fnordlicht_periodic())
  block(Fnordlicht)
  ecmd_feature(fnordlicht_reinit, "fnordlicht_init",,fnordlicht init)
  ecmd_feature(fnordlicht, "fnordlicht ","ADDRESS RED,GREEN,BLUE",fnordlicht command to set RGB color)
*/
