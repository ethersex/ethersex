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

#include<utils/utils.h>
#include "rc4.h"

struct rc4_state s;



void rc4_init(u08 *key,  u08 length )
{
  u08 i, j, k, a;
  u08 * m;

  s.x = 0;
  s.y = 0;
  m = s.m;
  
  i=0;
  m[i] = i;
  for( i = 1; i !=0 ; i++ )
    {
      m[i] = i;
    }
  
  j = k = 0;


  i=0;
  a = m[i];
  j = (u08) ( j + a + key[k] );
  m[i] = m[j]; m[j] = a;
  if( ++k >= length ) k = 0;

  for( i = 1; i != 0 ; i++ )
    {
      a = m[i];
      j = (u08) ( j + a + key[k] );
      m[i] = m[j]; m[j] = a;
      if( ++k >= length ) k = 0;
    }
}



u08 rc4_crypt_char(u08 data)
{ 
    u08 a, b;

    s.x = (u08) ( s.x + 1 ); 
    a = s.m[s.x];
    s.y = (u08) ( s.y + a );
    s.m[s.x] = b = s.m[s.y];
    s.m[s.y] = a;
    data ^= s.m[(u08) ( a + b )];

    return data;
}

