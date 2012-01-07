/*
 * Copyright (c) 2009 by Dirk Pannenbecker <dp@sd-gp.de>
 *
 * Author:         Stefan Krupop <mail@stefankrupop.de>
 *                 Dirk Pannenbecker <dp@sd-gp.de>
 *
 * taken from:
 *   http://www.dmxcontroler.de/wiki/Art-Net-Node_für_25_Euro
 *    Copyright:      Stefan Krupop  mailto: mail@stefankrupop.de
 *    Author:         Stefan Krupop
 *    Remarks:        
 *    known Problems: none
 *    Version:        17.01.2009
 *    Description:    Implementation des ArtNet-Protokolls für DMX-Übertragung über Ethernet
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
  
#include "config.h"
#include <stdint.h>
#ifndef _ARTNET_H
#define _ARTNETCLIENT_H
  
#ifdef ARTNET_SUPPORT
extern uint16_t artnet_port;
  
#include <avr/io.h>
#include <avr/pgmspace.h>
#include "protocols/artnet/artnet_net.h"
  
#ifdef DEBUG_ARTNET
#include "core/debug.h"
#define ARTNET_DEBUG(str...) debug_printf ("artnet: " str)
#else   /*  */
#define ARTNET_DEBUG(...)    ((void) 0)
#endif  /*  */
#define ARTNET_SUBNET_EEPROM_STORE    	50
#define ARTNET_INUNIVERSE_EEPROM_STORE	51
#define ARTNET_OUTUNIVERSE_EEPROM_STORE	52
#define ARTNET_PORT_EEPROM_STORE    	54
#define ARTNET_NETCONFIG_EEPROM_STORE   56
#define ARTNET_SHORTNAME_EEPROM_STORE	60
#define ARTNET_LONGNAME_EEPROM_STORE    78
void artnet_init(void);
void artnet_sendPollReply(void);
void artnet_main(void);
void artnet_get(void);
 
#endif  /* _ARTNET_H */
#endif  /* ARTNET_SUPPORT */
