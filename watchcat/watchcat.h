/*
 *
 * Copyright (c) 2007 by Christian Dietrich <stettberger@dokucode.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
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

#ifndef _WATCHCAT_H
#define _WATCHCAT_H

#include "../config.h"

struct VirtualPin {
  uint8_t old_state;
  uint8_t state;
  uint8_t last_input;
  /* Call function on edge */
  void (*func)(uint8_t);
};

struct EcmdSenderReaction {
  /* On which port */
  uint8_t port;
  /* On wich pin */
  uint8_t pin;
  /* on rising edge? */
  uint8_t rising;
  /* To which host should we connect? */
  uip_ipaddr_t address;
  /* What should we send him? */
  const char *message;
};

void watchcat_init();

/* Call this e.g. every 20 ms */
void watchcat_periodic();

#endif
