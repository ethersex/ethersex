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
 *  GNU General Public License for more detailuip_conn->rc4.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "rc4.h"
#include "../uip/uip.h"

/* struct rc4_state s; */

void rc4_init(uint8_t *key,  uint8_t length )
{
  uint8_t i, j, k, a;
  uint8_t * m;

  uip_conn->rc4.x = 0;
  uip_conn->rc4.y = 0;
  m = uip_conn->rc4.m;
  
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



uint8_t rc4_crypt_char(uint8_t data)
{ 
    uint8_t a, b;

    uip_conn->rc4.x = (uint8_t) ( uip_conn->rc4.x + 1 ); 
    a = uip_conn->rc4.m[uip_conn->rc4.x];
    uip_conn->rc4.y = (uint8_t) ( uip_conn->rc4.y + a );
    uip_conn->rc4.m[uip_conn->rc4.x] = b = uip_conn->rc4.m[uip_conn->rc4.y];
    uip_conn->rc4.m[uip_conn->rc4.y] = a;
    data ^= uip_conn->rc4.m[(uint8_t) ( a + b )];

    return data;
}

