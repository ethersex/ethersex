/* Droids-corp, Eirbot, Microb Technology 2005 - Zer0
 * Inspired by the ARC4 implementation by Christophe Devine
 * The original licence is below
 * Implementation for RC4
 */

/*
 *  An implementation of the ARC4 algorithm
 *
 *  Copyright (C) 2001-2003  Christophe Devine
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "rc4.h"
#include "md5.h"
#include "../uip/uip.h"

#ifdef RC4_SUPPORT


static void 
_rc4_init(rc4_state_t *s, const uint8_t *key,  uint8_t length )
{
  uint8_t i, j, k, a;
  uint8_t * m;

  s->x = 0;
  s->y = 0;
  m = s->m;
  
  i=0;
  m[i] = i;
  for( i = 1; i !=0 ; i++ )
    {
      m[i] = i;
    }
  
  j = k = 0;


  i=0;
  a = m[i];
  j = (uint8_t) ( j + a + key[k] );
  m[i] = m[j]; m[j] = a;
  if( ++k >= length ) k = 0;

  for( i = 1; i != 0 ; i++ )
    {
      a = m[i];
      j = (uint8_t) ( j + a + key[k] );
      m[i] = m[j]; m[j] = a;
      if( ++k >= length ) k = 0;
    }
}


void
rc4_init(rc4_state_t *s, const unsigned char *iv)
{
  md5_ctx_t ctx;
  char key[24] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                   0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
                   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  memcpy(key + 16, iv, 8);

  md5_init(&ctx);
  md5_lastBlock(&ctx, key, 24 * 8);

  _rc4_init(s, (void *) &ctx.a[0], 16);
}


uint8_t 
rc4_crypt_char(rc4_state_t *s, uint8_t data)
{ 
    uint8_t a, b;

    s->x = (uint8_t) ( s->x + 1 ); 
    a = s->m[s->x];
    s->y = (uint8_t) ( s->y + a );
    s->m[s->x] = b = s->m[s->y];
    s->m[s->y] = a;
    data ^= s->m[(uint8_t) ( a + b )];

    return data;
}


#endif /* RC4_SUPPORT */
