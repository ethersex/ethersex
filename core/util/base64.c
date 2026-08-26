/*
 * Copyright (c) 2001 Bob Trower, Trantor Standard Systems Inc.
 * Copyright (c) 2008 Christian Dietrich <stettberger@dokucode.de>
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <string.h>
#include <stdint.h>
#include <avr/pgmspace.h>

#include "base64.h"

static const char PROGMEM cd64[] =
  "|$$$}rstuvwxyz{$$$$$$$>?@ABCDEFGHIJKLMNOPQRSTUVW"
  "$$$$$$$XYZ[\\]^_`abcdefghijklmnopq";

void base64_decodeblock(char in[4], char out[3])
{
	uint8_t i;
	for (i = 0; i < 4; i++) {
		in[i] = ((in[i] < 43 || in[i] > 122) ? 0
			: pgm_read_byte(&cd64[in[i] - 43]));
		if (in[i]) {
			in[i] = ((in[i] == '$') ? 0 : in[i] - 61);
			if (in[i])
				in[i]--;
		}
	}

	out[0] = (in[0] << 2 | in[1] >> 4);
	out[1] = (in[1] << 4 | in[2] >> 2);
	out[2] = (((in[2] << 6) & 0xc0) | in[3]);
}

void base64_decode(char *str, uint8_t *output, uint16_t max_len)
{
	uint16_t len = strlen(str);
	char *in = str;
	uint8_t *out = output;
	uint16_t out_len = 0;

	while (in < (str + len) && out_len < max_len) {
		base64_decodeblock(in, (char *)out);
		in += 4;
		out += 3;
		out_len += 3;
	}
	*out = 0;
}

void base64_encode(const uint8_t *input, uint16_t input_len,
			  char *output, uint16_t max_len)
{
	static const char PROGMEM base64_table[] =
		"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

	uint16_t i = 0, j = 0;
	while (i < input_len && j + 4 <= max_len) {
		uint32_t chunk = 0;
		uint8_t bits = 0;
		uint8_t k;

		for (k = 0; k < 3 && i < input_len; k++, i++) {
			chunk = (chunk << 8) | input[i];
			bits += 8;
		}

		while (bits >= 6 && j + 1 <= max_len) {
			bits -= 6;
			output[j++] = pgm_read_byte(&base64_table[(chunk >> bits) & 0x3F]);
		}
	}

	/* Padding */
	while (j < max_len && (j - 1) % 4 != 3) {
		output[j++] = '=';
	}
	if (j < max_len) {
		output[j] = 0;
	}
}
