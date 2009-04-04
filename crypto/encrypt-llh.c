/*
 * Copyright (c) 2007,2008 by Stefan Siegl <stesie@brokenpipe.de>
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

#include <stdlib.h>

#include "../config.h"
#include "../uip/uip.h"
#include "../spi.h"
#include "../rfm12/rfm12.h"

#if defined(SKIPJACK_SUPPORT) \
    && (defined(RFM12_IP_SUPPORT) || defined(ZBUS_SUPPORT))

#ifdef RFM12_IP_SUPPORT
uint8_t *rfm12_key = CONF_RFM12_KEY;
#endif 

#ifdef ZBUS_SUPPORT
uint8_t *zbus_key = CONF_ZBUS_KEY;
#endif 

void
llh_encrypt (uint8_t *key, uint8_t *data, uint16_t *len)
{
  /* Make room for IV. */
  memmove (data + 8, data, *len);
  *len += 8;

  /* Generate IV */
  for (uint8_t i = 0; i < 8; i ++)
    data[i] = rand() & 0xFF;

  /* Do padding. */
  uint8_t pad_char = 8 - (*len % 8);
  do
    {
      data[*len] = pad_char;
      (*len) ++;
    }
  while (*len & 7);

  /* Do encryption. */
  uint8_t lastblock = *len >> 3;
  for (uint8_t i = 1; i < lastblock; i ++)
    {
      /* Carry CBC forward. */
      for (uint8_t j = 0; j < 8; j ++)
	data[(i << 3) + j] ^= data[(i << 3) + j - 8];

      /* Encrypt data. */
      skipjack_enc (data + (i << 3), key);
    }
}

void
llh_decrypt (uint8_t *key, uint8_t *data, uint16_t *len)
{
  /* discard packet if it's not at least one (padded) data block
     or not a multiple of the block size (8 bytes). */
  if (*len < 16 || *len & 7)
    {
      *len = 0;
      return;
    }

  uint8_t lastblock = *len >> 3;
  for (uint8_t i = 1; i < lastblock; i ++)
    {
      uint8_t buf[8];
      memmove (buf, data + (i << 3), 8);
      skipjack_dec (buf, key);

      for (uint8_t j = 0; j < 8; j ++)
	data[(i << 3) + j - 8] ^= buf[j];
    }

  *len -= 8;

  uint8_t padchar = data[*len - 1];

  if (padchar < 1 || padchar > 8)
    {
      /* invalid pad char */
      *len = 0;
      return;
    }

  for (uint8_t i = *len - padchar; i < *len; i ++)
    if (data[i] != padchar) 
      {
	/* Padding error, destroy packet */
	*len = 0;
	return;
      }

  *len -= padchar;		/* Remove padding. */
}


#endif /* SKIPJACK_SUPPORT and (ZBUS_SUPPORT || RFM12_IP_SUPPORT) */
