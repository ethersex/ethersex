/* vim:fdm=marker et ai
 * {{{
 *
 * Copyright (c) 2008 by Stefan Siegl <stesie@brokenpipe.de>
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
 }}} */

#ifndef ENCRYPT_LLH_H
#define ENCRYPT_LLH_H

#include "../rfm12/rfm12.h"

void llh_encrypt (uint8_t *key, uint8_t *data, uint16_t *len);
void llh_decrypt (uint8_t *key, uint8_t *data, uint16_t *len);

/*
 * rfm12 encryption wrapper
 */
extern uint8_t *rfm12_key;

static inline void
rfm12_encrypt (uint8_t *data, uint8_t *len)
{
  uint8_t pad_char = 8 - (*len % 8);

  if (pad_char + *len + 8 > RFM12_DataLength)
    {
      *len = 0; 		/* destroy packet */
      return;
    }

  uint16_t i = *len;
  llh_encrypt(rfm12_key, data, &i);
  *len = i;
}

static inline void
rfm12_decrypt (uint8_t *data, uint8_t *len)
{
  uint16_t i = *len;
  llh_decrypt(rfm12_key, data, &i);
  *len = i;
}


/*
 * zbus encryption wrapper
 */
extern uint8_t *zbus_key;
#define zbus_encrypt(d,l) llh_encrypt(zbus_key, d, l)
#define zbus_decrypt(d,l) llh_decrypt(zbus_key, d, l)

#endif /* not ENCRYPT_LLH_H */
