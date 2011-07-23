/* 
  Copyright (c) 2001 Bob Trower, Trantor Standard Systems Inc.
  Copyright (c) 2008 Christian Dietrich <stettberger@dokucode.de>

  Permission is hereby granted, free of charge, to any person
  obtaining a copy of this software and associated
  documentation files (the "Software"), to deal in the
  Software without restriction, including without limitation
  the rights to use, copy, modify, merge, publish, distribute,
  sublicense, and/or sell copies of the Software, and to
  permit persons to whom the Software is furnished to do so,
  subject to the following conditions:

  The above copyright notice and this permission notice shall
  be included in all copies or substantial portions of the
  Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
  KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
  WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
  PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS
  OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
  OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
  OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <string.h>
#include <stdint.h>
#include <avr/pgmspace.h>

static const char PROGMEM cd64[]="|$$$}rstuvwxyz{$$$$$$$>?@ABCDEFGHIJKLMNOPQRSTUVW"
			         "$$$$$$XYZ[\\]^_`abcdefghijklmnopq";

void base64_decodeblock(char in[4],char out[4] )
{   
    uint8_t i;
    for (i = 0; i < 4; i++) {
	    in[i] = ((in[i] < 43 || in[i] > 122) ? 0 
			    : pgm_read_byte(&cd64[ in[i] - 43 ]));
	    if( in[i] ) {
		    in[i] = ((in[i] == '$') ? 0 : in[i] - 61);
		    if(in[i])
			    in[i]--;
	    }
	
    }

    out[ 0 ] = (in[0] << 2 | in[1] >> 4);
    out[ 1 ] = (in[1] << 4 | in[2] >> 2);
    out[ 2 ] = (((in[2] << 6) & 0xc0) | in[3]);
    out[ 3 ] = 0;
}

void base64_str_decode(char *str) 
{
  uint16_t len = strlen(str);
  char *in = str, *out = str;
   while (in < (str + len)) {
	   base64_decodeblock(in, out);
	   in += 4; out += 3;
   }
}

