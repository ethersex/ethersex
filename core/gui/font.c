/*
 * Copyright (c) 2003, Adam Dunkels.
 * All rights reserved. 
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met: 
 * 1. Redistributions of source code must retain the above copyright 
 *    notice, this list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution. 
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.  
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  
 *
 * This file is part of the Contiki VNC server
 *
 * $Id: ctk-vncfont.c,v 1.1 2006/06/17 22:41:16 adamdunkels Exp $
 *
 */

#include "font.h"
#include <avr/io.h>
#include <avr/pgmspace.h>
#include "config.h"

void 
gui_putchar(struct gui_block *dest,
            char data, 
            uint8_t color, 
            uint8_t char_line, 
            uint8_t char_column) 
{
    uint8_t x, y;
    /* We have to select the right line */
    if (char_line / 2 != dest->y) return;

/*
  ______
  ______
  _XX___
  ___X__
  _XXX__
  X__X__
  _XXX__
  ______ 
   ^  ^
   |  + char_x_offset + char_x_len;
   +- char_x_offset
 */
    /* Now we select the right row */
    uint16_t tmp = char_column * GUI_FONT_WIDTH;
    if (! (tmp / GUI_BLOCK_WIDTH == dest->x)  
        && ! ((tmp / GUI_BLOCK_WIDTH == (dest->x - 1) 
        && tmp > (dest->x * GUI_BLOCK_WIDTH - GUI_FONT_WIDTH)))) return;

    uint8_t char_x_offset, char_x_len;

    /* Offset within the block */
    uint8_t x_offset;

    /* map characters (a-z) */
    if (data >= 'a' && data <= 'z')
      data = data - 'a' + 1;

    /* Start Pixel of character in block */
    if (tmp >= dest->x * GUI_BLOCK_WIDTH) {
        char_x_offset = 0;
        x_offset = tmp % GUI_BLOCK_WIDTH;
        if (tmp + GUI_FONT_WIDTH > (dest->x + 1) * GUI_BLOCK_WIDTH)
            char_x_len =  (dest->x + 1) * GUI_BLOCK_WIDTH - tmp;
        else
            char_x_len = GUI_FONT_WIDTH;
    } else {
        /* Start Pixel is in the block before */
        char_x_offset = dest->x * GUI_BLOCK_WIDTH - tmp;
        char_x_len = GUI_FONT_WIDTH - char_x_offset;
        x_offset = 0;
    } 

    for (x = 0; x < char_x_len; x++) {
        char font_data = pgm_read_byte(&gui_font[(uint8_t) data][char_x_offset + x]);

        for (y = 0; y < 8; y++) {
            if (font_data & _BV(y)) {
                dest->data[((char_line % 2) * 8 + y) * GUI_BLOCK_WIDTH + x + x_offset ] = color;
            }
        }
    }
}

/* Here comes the font data */

char gui_font[128][GUI_FONT_WIDTH] PROGMEM = {
{0x01, 0x02, 0x00, 0x00, 0x00, 0x00},
/* char 0 0x00
  X_____
  _X____
  ______
  ______
  ______
  ______
  ______
  ______ */
{0x20, 0x54, 0x54, 0x78, 0x00, 0x00},
/* char 1 0x01
  ______
  ______
  _XX___
  ___X__
  _XXX__
  X__X__
  _XXX__
  ______ */
{0x7f, 0x44, 0x44, 0x38, 0x00, 0x00},
/* char 2 0x02
  X_____
  X_____
  XXX___
  X__X__
  X__X__
  X__X__
  XXX___
  ______ */
{0x38, 0x44, 0x44, 0x28, 0x00, 0x00},
/* char 3 0x03
  ______
  ______
  _XX___
  X__X__
  X_____
  X__X__
  _XX___
  ______ */
{0x38, 0x44, 0x44, 0x7f, 0x00, 0x00},
/* char 4 0x04
  ___X__
  ___X__
  _XXX__
  X__X__
  X__X__
  X__X__
  _XXX__
  ______ */
{0x38, 0x54, 0x54, 0x18, 0x00, 0x00},
/* char 5 0x05
  ______
  ______
  _XX___
  X__X__
  XXXX__
  X_____
  _XX___
  ______ */
{0x04, 0x7e, 0x05, 0x01, 0x00, 0x00},
/* char 6 0x06
  __XX__
  _X____
  XXX___
  _X____
  _X____
  _X____
  _X____
  ______ */
{0x38, 0x44, 0x44, 0xfc, 0x00, 0x00},
/* char 7 0x07
  ______
  ______
  _XXX__
  X__X__
  X__X__
  X__X__
  _XXX__
  ___X__ */
{0x7f, 0x04, 0x04, 0x78, 0x00, 0x00},
/* char 8 0x08
  X_____
  X_____
  XXX___
  X__X__
  X__X__
  X__X__
  X__X__
  ______ */
{0x04, 0x7d, 0x00, 0x00, 0x00, 0x00},
/* char 9 0x09
  _X____
  ______
  XX____
  _X____
  _X____
  _X____
  _X____
  ______ */
{0x00, 0x04, 0xfd, 0x00, 0x00, 0x00},
/* char 10 0x0a
  __X___
  ______
  _XX___
  __X___
  __X___
  __X___
  __X___
  __X___ */
{0x7f, 0x10, 0x28, 0x44, 0x00, 0x00},
/* char 11 0x0b
  X_____
  X_____
  X__X__
  X_X___
  XX____
  X_X___
  X__X__
  ______ */
{0x00, 0x01, 0x7f, 0x00, 0x00, 0x00},
/* char 12 0x0c
  _XX___
  __X___
  __X___
  __X___
  __X___
  __X___
  __X___
  ______ */
{0x7c, 0x04, 0x04, 0x78, 0x04, 0x78},
/* char 13 0x0d
  ______
  ______
  XXX_X_
  X__X_X
  X__X_X
  X__X_X
  X__X_X
  ______ */
{0x7c, 0x04, 0x04, 0x78, 0x00, 0x00},
/* char 14 0x0e
  ______
  ______
  XXX___
  X__X__
  X__X__
  X__X__
  X__X__
  ______ */
{0x38, 0x44, 0x44, 0x38, 0x00, 0x00},
/* char 15 0x0f
  ______
  ______
  _XX___
  X__X__
  X__X__
  X__X__
  _XX___
  ______ */
{0xfc, 0x44, 0x44, 0x38, 0x00, 0x00},
/* char 16 0x10
  ______
  ______
  XXX___
  X__X__
  X__X__
  X__X__
  XXX___
  X_____ */
{0x38, 0x44, 0x44, 0xfc, 0x00, 0x00},
/* char 17 0x11
  ______
  ______
  _XXX__
  X__X__
  X__X__
  X__X__
  _XXX__
  ___X__ */
{0x7c, 0x08, 0x04, 0x04, 0x00, 0x00},
/* char 18 0x12
  ______
  ______
  X_XX__
  XX____
  X_____
  X_____
  X_____
  ______ */
{0x48, 0x54, 0x54, 0x24, 0x00, 0x00},
/* char 19 0x13
  ______
  ______
  _XXX__
  X_____
  _XX___
  ___X__
  XXX___
  ______ */
{0x00, 0x04, 0x3f, 0x44, 0x00, 0x00},
/* char 20 0x14
  __X___
  __X___
  _XXX__
  __X___
  __X___
  __X___
  ___X__
  ______ */
{0x3c, 0x40, 0x40, 0x7c, 0x00, 0x00},
/* char 21 0x15
  ______
  ______
  X__X__
  X__X__
  X__X__
  X__X__
  _XXX__
  ______ */
{0x04, 0x18, 0x60, 0x18, 0x04, 0x00},
/* char 22 0x16
  ______
  ______
  X___X_
  _X_X__
  _X_X__
  __X___
  __X___
  ______ */
{0x04, 0x18, 0x60, 0x18, 0x60, 0x1c},
/* char 23 0x17
  ______
  ______
  X____X
  _X_X_X
  _X_X_X
  __X_X_
  __X_X_
  ______ */
{0x44, 0x28, 0x10, 0x28, 0x44, 0x00},
/* char 24 0x18
  ______
  ______
  X___X_
  _X_X__
  __X___
  _X_X__
  X___X_
  ______ */
{0x3c, 0x40, 0x40, 0xfc, 0x00, 0x00},
/* char 25 0x19
  ______
  ______
  X__X__
  X__X__
  X__X__
  X__X__
  _XXX__
  ___X__ */
{0x64, 0x54, 0x4c, 0x44, 0x00, 0x00},
/* char 26 0x1a
  ______
  ______
  XXXX__
  __X___
  _X____
  X_____
  XXXX__
  ______ */
{0x08, 0x77, 0x80, 0x00, 0x00, 0x00},
/* char 27 0x1b
  _X____
  _X____
  _X____
  X_____
  _X____
  _X____
  _X____
  __X___ */
{0xff, 0x00, 0x00, 0x00, 0x00, 0x00},
/* char 28 0x1c
  X_____
  X_____
  X_____
  X_____
  X_____
  X_____
  X_____
  X_____ */
{0x80, 0x77, 0x08, 0x00, 0x00, 0x00},
/* char 29 0x1d
  _X____
  _X____
  _X____
  __X___
  _X____
  _X____
  _X____
  X_____ */
{0x02, 0x01, 0x03, 0x02, 0x01, 0x00},
/* char 30 0x1e
  _XX_X_
  X_XX__
  ______
  ______
  ______
  ______
  ______
  ______ */
{0x00, 0xff, 0xff, 0x00, 0x00, 0x00},
/* char 31 0x1f
  _XX___
  _XX___
  _XX___
  _XX___
  _XX___
  _XX___
  _XX___
  _XX___ */
{0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
/* char 32 0x20
  ______
  ______
  ______
  ______
  ______
  ______
  ______
  ______ */
{0x00, 0x5f, 0x00, 0x00, 0x00, 0x00},
/* char 33 0x21
  _X____
  _X____
  _X____
  _X____
  _X____
  ______
  _X____
  ______ */
{0x03, 0x00, 0x03, 0x00, 0x00, 0x00},
/* char 34 0x22
  X_X___
  X_X___
  ______
  ______
  ______
  ______
  ______
  ______ */
{0x05, 0x0f, 0x05, 0x0f, 0x05, 0x00},
/* char 35 0x23
  XXXXX_
  _X_X__
  XXXXX_
  _X_X__
  ______
  ______
  ______
  ______ */
{0x26, 0x49, 0xff, 0x49, 0x32, 0x00},
/* char 36 0x24
  _XXX__
  X_X_X_
  X_X___
  _XXX__
  __X_X_
  X_X_X_
  _XXX__
  __X___ */
{0x06, 0x49, 0x29, 0x17, 0x39, 0x4d},
/* char 37 0x25
  _XXXXX
  X__X__
  X__X_X
  _XX_XX
  ___XX_
  __X_X_
  _X___X
  ______ */
{0x30, 0x4b, 0x44, 0x2a, 0x11, 0x28},
/* char 38 0x26
  _X__X_
  _X_X__
  __X___
  _X_X_X
  X___X_
  X__X_X
  _XX___
  ______ */
{0x03, 0x00, 0x00, 0x00, 0x00, 0x00},
/* char 39 0x27
  X_____
  X_____
  ______
  ______
  ______
  ______
  ______
  ______ */
{0x3e, 0x41, 0x80, 0x00, 0x00, 0x00},
/* char 40 0x28
  _X____
  X_____
  X_____
  X_____
  X_____
  X_____
  _X____
  __X___ */
{0x80, 0x41, 0x3e, 0x00, 0x00, 0x00},
/* char 41 0x29
  _X____
  __X___
  __X___
  __X___
  __X___
  __X___
  _X____
  X_____ */
{0x04, 0x15, 0x0e, 0x15, 0x04, 0x00},
/* char 42 0x2a
  _X_X__
  __X___
  XXXXX_
  __X___
  _X_X__
  ______
  ______
  ______ */
{0x04, 0x04, 0x1f, 0x04, 0x04, 0x00},
/* char 43 0x2b
  __X___
  __X___
  XXXXX_
  __X___
  __X___
  ______
  ______
  ______ */
{0x00, 0xc0, 0x00, 0x00, 0x00, 0x00},
/* char 44 0x2c
  ______
  ______
  ______
  ______
  ______
  ______
  _X____
  _X____ */
{0x04, 0x04, 0x04, 0x04, 0x00, 0x00},
/* char 45 0x2d
  ______
  ______
  XXXX__
  ______
  ______
  ______
  ______
  ______ */
{0x00, 0x00, 0x40, 0x00, 0x00, 0x00},
/* char 46 0x2e
  ______
  ______
  ______
  ______
  ______
  ______
  __X___
  ______ */
{0x60, 0x18, 0x06, 0x01, 0x00, 0x00},
/* char 47 0x2f
  ___X__
  __X___
  __X___
  _X____
  _X____
  X_____
  X_____
  ______ */
{0x3e, 0x41, 0x41, 0x41, 0x3e, 0x00},
/* char 48 0x30
  _XXX__
  X___X_
  X___X_
  X___X_
  X___X_
  X___X_
  _XXX__
  ______ */
{0x00, 0x02, 0x7f, 0x00, 0x00, 0x00},
/* char 49 0x31
  __X___
  _XX___
  __X___
  __X___
  __X___
  __X___
  __X___
  ______ */
{0x42, 0x61, 0x51, 0x49, 0x46, 0x00},
/* char 50 0x32
  _XXX__
  X___X_
  ____X_
  ___X__
  __X___
  _X____
  XXXXX_
  ______ */
{0x21, 0x49, 0x4d, 0x4b, 0x31, 0x00},
/* char 51 0x33
  XXXXX_
  ___X__
  __X___
  _XXX__
  ____X_
  X___X_
  _XXX__
  ______ */
{0x18, 0x14, 0x12, 0x7f, 0x10, 0x00},
/* char 52 0x34
  ___X__
  __XX__
  _X_X__
  X__X__
  XXXXX_
  ___X__
  ___X__
  ______ */
{0x27, 0x45, 0x45, 0x45, 0x39, 0x00},
/* char 53 0x35
  XXXXX_
  X_____
  XXXX__
  ____X_
  ____X_
  X___X_
  _XXX__
  ______ */
{0x3c, 0x4a, 0x49, 0x49, 0x30, 0x00},
/* char 54 0x36
  __XX__
  _X____
  X_____
  XXXX__
  X___X_
  X___X_
  _XXX__
  ______ */
{0x01, 0x01, 0x71, 0x0d, 0x03, 0x00},
/* char 55 0x37
  XXXXX_
  ____X_
  ___X__
  ___X__
  __X___
  __X___
  __X___
  ______ */
{0x36, 0x49, 0x49, 0x49, 0x36, 0x00},
/* char 56 0x38
  _XXX__
  X___X_
  X___X_
  _XXX__
  X___X_
  X___X_
  _XXX__
  ______ */
{0x06, 0x49, 0x49, 0x29, 0x1e, 0x00},
/* char 57 0x39
  _XXX__
  X___X_
  X___X_
  _XXXX_
  ____X_
  ___X__
  _XX___
  ______ */
{0x44, 0x00, 0x00, 0x00, 0x00, 0x00},
/* char 58 0x3a
  ______
  ______
  X_____
  ______
  ______
  ______
  X_____
  ______ */
{0x00, 0xc4, 0x00, 0x00, 0x00, 0x00},
/* char 59 0x3b
  ______
  ______
  _X____
  ______
  ______
  ______
  _X____
  _X____ */
{0x08, 0x14, 0x22, 0x00, 0x00, 0x00},
/* char 60 0x3c
  ______
  __X___
  _X____
  X_____
  _X____
  __X___
  ______
  ______ */
{0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x00},
/* char 61 0x3d
  ______
  XXXXX_
  ______
  XXXXX_
  ______
  ______
  ______
  ______ */
{0x22, 0x14, 0x08, 0x00, 0x00, 0x00},
/* char 62 0x3e
  ______
  X_____
  _X____
  __X___
  _X____
  X_____
  ______
  ______ */
{0x02, 0x51, 0x09, 0x06, 0x00, 0x00},
/* char 63 0x3f
  _XX___
  X__X__
  ___X__
  __X___
  _X____
  ______
  _X____
  ______ */
{0x3c, 0x42, 0x99, 0xa5, 0xbd, 0x22},
/* char 64 0x40
  __XXX_
  _X___X
  X__XX_
  X_X_X_
  X_X_X_
  X__XXX
  _X____
  __XXX_ */
{0x70, 0x1c, 0x13, 0x1c, 0x70, 0x00},
/* char 65 0x41
  __X___
  __X___
  _X_X__
  _X_X__
  XXXXX_
  X___X_
  X___X_
  ______ */
{0x7f, 0x49, 0x49, 0x49, 0x36, 0x00},
/* char 66 0x42
  XXXX__
  X___X_
  X___X_
  XXXX__
  X___X_
  X___X_
  XXXX__
  ______ */
{0x3e, 0x41, 0x41, 0x41, 0x22, 0x00},
/* char 67 0x43
  _XXX__
  X___X_
  X_____
  X_____
  X_____
  X___X_
  _XXX__
  ______ */
{0x7f, 0x41, 0x41, 0x22, 0x1c, 0x00},
/* char 68 0x44
  XXX___
  X__X__
  X___X_
  X___X_
  X___X_
  X__X__
  XXX___
  ______ */
{0x7f, 0x49, 0x49, 0x41, 0x00, 0x00},
/* char 69 0x45
  XXXX__
  X_____
  X_____
  XXX___
  X_____
  X_____
  XXXX__
  ______ */
{0x7f, 0x09, 0x09, 0x01, 0x00, 0x00},
/* char 70 0x46
  XXXX__
  X_____
  X_____
  XXX___
  X_____
  X_____
  X_____
  ______ */
{0x3e, 0x41, 0x41, 0x49, 0x3a, 0x00},
/* char 71 0x47
  _XXX__
  X___X_
  X_____
  X__XX_
  X___X_
  X___X_
  _XXX__
  ______ */
{0x7f, 0x08, 0x08, 0x08, 0x7f, 0x00},
/* char 72 0x48
  X___X_
  X___X_
  X___X_
  XXXXX_
  X___X_
  X___X_
  X___X_
  ______ */
{0x00, 0x00, 0x7f, 0x00, 0x00, 0x00},
/* char 73 0x49
  __X___
  __X___
  __X___
  __X___
  __X___
  __X___
  __X___
  ______ */
{0x30, 0x40, 0x40, 0x40, 0x3f, 0x00},
/* char 74 0x4a
  ____X_
  ____X_
  ____X_
  ____X_
  X___X_
  X___X_
  _XXX__
  ______ */
{0x7f, 0x08, 0x14, 0x22, 0x41, 0x00},
/* char 75 0x4b
  X___X_
  X__X__
  X_X___
  XX____
  X_X___
  X__X__
  X___X_
  ______ */
{0x7f, 0x40, 0x40, 0x40, 0x00, 0x00},
/* char 76 0x4c
  X_____
  X_____
  X_____
  X_____
  X_____
  X_____
  XXXX__
  ______ */
{0x7f, 0x02, 0x04, 0x08, 0x04, 0x7f},
/* char 77 0x4d
  X____X
  XX___X
  X_X_XX
  X__X_X
  X____X
  X____X
  X____X
  ______ */
{0x7f, 0x03, 0x0c, 0x30, 0x7f, 0x00},
/* char 78 0x4e
  XX__X_
  XX__X_
  X_X_X_
  X_X_X_
  X__XX_
  X__XX_
  X___X_
  ______ */
{0x3e, 0x41, 0x41, 0x41, 0x3e, 0x00},
/* char 79 0x4f
  _XXX__
  X___X_
  X___X_
  X___X_
  X___X_
  X___X_
  _XXX__
  ______ */
{0x7f, 0x09, 0x09, 0x09, 0x06, 0x00},
/* char 80 0x50
  XXXX__
  X___X_
  X___X_
  XXXX__
  X_____
  X_____
  X_____
  ______ */
{0x3e, 0x41, 0x61, 0xc1, 0x3e, 0x00},
/* char 81 0x51
  _XXX__
  X___X_
  X___X_
  X___X_
  X___X_
  X_X_X_
  _XXX__
  ___X__ */
{0x7f, 0x09, 0x19, 0x29, 0x46, 0x00},
/* char 82 0x52
  XXXX__
  X___X_
  X___X_
  XXXX__
  X_X___
  X__X__
  X___X_
  ______ */
{0x26, 0x49, 0x49, 0x49, 0x32, 0x00},
/* char 83 0x53
  _XXX__
  X___X_
  X_____
  _XXX__
  ____X_
  X___X_
  _XXX__
  ______ */
{0x01, 0x01, 0x7f, 0x01, 0x01, 0x00},
/* char 84 0x54
  XXXXX_
  __X___
  __X___
  __X___
  __X___
  __X___
  __X___
  ______ */
{0x3f, 0x40, 0x40, 0x40, 0x3f, 0x00},
/* char 85 0x55
  X___X_
  X___X_
  X___X_
  X___X_
  X___X_
  X___X_
  _XXX__
  ______ */
{0x07, 0x18, 0x60, 0x18, 0x07, 0x00},
/* char 86 0x56
  X___X_
  X___X_
  X___X_
  _X_X__
  _X_X__
  __X___
  __X___
  ______ */
{0x03, 0x0c, 0x70, 0x0c, 0x70, 0x0f},
/* char 87 0x57
  X____X
  X____X
  _X_X_X
  _X_X_X
  __X_X_
  __X_X_
  __X_X_
  ______ */
{0x63, 0x14, 0x08, 0x14, 0x63, 0x00},
/* char 88 0x58
  X___X_
  X___X_
  _X_X__
  __X___
  _X_X__
  X___X_
  X___X_
  ______ */
{0x03, 0x04, 0x78, 0x04, 0x03, 0x00},
/* char 89 0x59
  X___X_
  X___X_
  _X_X__
  __X___
  __X___
  __X___
  __X___
  ______ */
{0x71, 0x49, 0x45, 0x43, 0x00, 0x00},
/* char 90 0x5a
  XXXX__
  ___X__
  __X___
  _X____
  X_____
  X_____
  XXXX__
  ______ */
{0xff, 0x80, 0x00, 0x00, 0x00, 0x00},
/* char 91 0x5b
  X_____
  X_____
  X_____
  X_____
  X_____
  X_____
  X_____
  XX____ */
{0x01, 0x06, 0x18, 0x60, 0x00, 0x00},
/* char 92 0x5c
  X_____
  _X____
  _X____
  __X___
  __X___
  ___X__
  ___X__
  ______ */
{0x80, 0xff, 0x00, 0x00, 0x00, 0x00},
/* char 93 0x5d
  _X____
  _X____
  _X____
  _X____
  _X____
  _X____
  _X____
  XX____ */
{0x02, 0x01, 0x02, 0x00, 0x00, 0x00},
/* char 94 0x5e
  _X____
  X_X___
  ______
  ______
  ______
  ______
  ______
  ______ */
{0x40, 0x40, 0x40, 0x40, 0x40, 0x40},
/* char 95 0x5f
  ______
  ______
  ______
  ______
  ______
  ______
  XXXXXX
  ______ */
{0xff, 0x00, 0x00, 0x00, 0x00, 0x00},
/* char 96 0x60
  X_____
  X_____
  X_____
  X_____
  X_____
  X_____
  X_____
  X_____ */
{0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
/* char 97 0x61
  ______
  ______
  ______
  ______
  ______
  ______
  ______
  ______ */
{0x00, 0x00, 0x00, 0x00, 0x00, 0xff},
/* char 98 0x62
  _____X
  _____X
  _____X
  _____X
  _____X
  _____X
  _____X
  _____X */
{0x00, 0x00, 0x00, 0x00, 0xff, 0xff},
/* char 99 0x63
  ____XX
  ____XX
  ____XX
  ____XX
  ____XX
  ____XX
  ____XX
  ____XX */
{0xc0, 0xc0, 0xc0, 0xc0, 0xff, 0xff},
/* char 100 0x64
  ____XX
  ____XX
  ____XX
  ____XX
  ____XX
  ____XX
  XXXXXX
  XXXXXX */
{0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0},
/* char 101 0x65
  ______
  ______
  ______
  ______
  ______
  ______
  XXXXXX
  XXXXXX */
{0xff, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0},
/* char 102 0x66
  X_____
  X_____
  X_____
  X_____
  X_____
  X_____
  XXXXXX
  XXXXXX */
{0xff, 0x00, 0x00, 0x00, 0x00, 0x00},
/* char 103 0x67
  X_____
  X_____
  X_____
  X_____
  X_____
  X_____
  X_____
  X_____ */
{0x00, 0x00, 0xff, 0x00, 0x00, 0x00},
/* char 104 0x68
  __X___
  __X___
  __X___
  __X___
  __X___
  __X___
  __X___
  __X___ */
{0x05, 0x05, 0x05, 0x05, 0x05, 0x05},
/* char 105 0x69
  XXXXXX
  ______
  XXXXXX
  ______
  ______
  ______
  ______
  ______ */
{0x05, 0x05, 0x05, 0xfd, 0x01, 0xff},
/* char 106 0x6a
  XXXXXX
  _____X
  XXXX_X
  ___X_X
  ___X_X
  ___X_X
  ___X_X
  ___X_X */
{0x00, 0x00, 0x00, 0xff, 0x00, 0xff},
/* char 107 0x6b
  ___X_X
  ___X_X
  ___X_X
  ___X_X
  ___X_X
  ___X_X
  ___X_X
  ___X_X */
{0xa0, 0xa0, 0xa0, 0xbf, 0x80, 0xff},
/* char 108 0x6c
  ___X_X
  ___X_X
  ___X_X
  ___X_X
  ___X_X
  XXXX_X
  _____X
  XXXXXX */
{0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0},
/* char 109 0x6d
  ______
  ______
  ______
  ______
  ______
  XXXXXX
  ______
  XXXXXX */
{0xff, 0x80, 0xbf, 0xa0, 0xa0, 0xa0},
/* char 110 0x6e
  X_X___
  X_X___
  X_X___
  X_X___
  X_X___
  X_XXXX
  X_____
  XXXXXX */
{0xff, 0x00, 0xff, 0x00, 0x00, 0x00},
/* char 111 0x6f
  X_X___
  X_X___
  X_X___
  X_X___
  X_X___
  X_X___
  X_X___
  X_X___ */
{0xff, 0x7f, 0x03, 0x03, 0x03, 0x01},
/* char 112 0x70
  XXXXXX
  XXXXX_
  XX____
  XX____
  XX____
  XX____
  XX____
  X_____ */
{0x80, 0xc0, 0xc0, 0xc0, 0xfe, 0xff},
/* char 113 0x71
  _____X
  ____XX
  ____XX
  ____XX
  ____XX
  ____XX
  _XXXXX
  XXXXXX */
{0xff, 0x01, 0xfd, 0x05, 0x05, 0x05},
/* char 114 0x72
  XXXXXX
  X_____
  X_XXXX
  X_X___
  X_X___
  X_X___
  X_X___
  X_X___ */
{0x18, 0x18, 0x18, 0x18, 0x18, 0x18},
/* char 115 0x73
  ______
  ______
  ______
  XXXXXX
  XXXXXX
  ______
  ______
  ______ */
{0xff, 0x00, 0x00, 0x00, 0x00, 0x00},
/* char 116 0x74
  X_____
  X_____
  X_____
  X_____
  X_____
  X_____
  X_____
  X_____ */
{0xff, 0xff, 0x00, 0x00, 0x00, 0x00},
/* char 117 0x75
  XX____
  XX____
  XX____
  XX____
  XX____
  XX____
  XX____
  XX____ */
{0x00, 0x00, 0x00, 0x00, 0xff, 0xff},
/* char 118 0x76
  ____XX
  ____XX
  ____XX
  ____XX
  ____XX
  ____XX
  ____XX
  ____XX */
{0x03, 0x03, 0x03, 0x03, 0x03, 0x03},
/* char 119 0x77
  XXXXXX
  XXXXXX
  ______
  ______
  ______
  ______
  ______
  ______ */
{0x07, 0x07, 0x07, 0x07, 0x07, 0x07},
/* char 120 0x78
  XXXXXX
  XXXXXX
  XXXXXX
  ______
  ______
  ______
  ______
  ______ */
{0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0},
/* char 121 0x79
  ______
  ______
  ______
  ______
  ______
  XXXXXX
  XXXXXX
  XXXXXX */
{0x78, 0x78, 0x30, 0x18, 0x0c, 0x06},
/* char 122 0x7a
  ______
  _____X
  ____XX
  XX_XX_
  XXXX__
  XXX___
  XX____
  ______ */
{0xf0, 0xf0, 0xf0, 0x00, 0x00, 0x00},
/* char 123 0x7b
  ______
  ______
  ______
  ______
  XXX___
  XXX___
  XXX___
  XXX___ */
{0x00, 0x00, 0x00, 0x0f, 0x0f, 0x0f},
/* char 124 0x7c
  ___XXX
  ___XXX
  ___XXX
  ___XXX
  ______
  ______
  ______
  ______ */
{0x18, 0x18, 0x1f, 0x1f, 0x00, 0x00},
/* char 125 0x7d
  __XX__
  __XX__
  __XX__
  XXXX__
  XXXX__
  ______
  ______
  ______ */
{0x0f, 0x0f, 0x0f, 0x00, 0x00, 0x00},
/* char 126 0x7e
  XXX___
  XXX___
  XXX___
  XXX___
  ______
  ______
  ______
  ______ */
{0x0f, 0x0f, 0x0f, 0xf0, 0xf0, 0xf0},
/* char 127 0x7f
  XXX___
  XXX___
  XXX___
  XXX___
  ___XXX
  ___XXX
  ___XXX
  ___XXX */
};
