;
; Copyright (c) 2017 by Philip Matura <ike@tura-home.de>
;
; This program is free software; you can redistribute it and/or
; modify it under the terms of the GNU General Public License
; as published by the Free Software Foundation; either version 3
; of the License, or (at your option) any later version.
;
; This program is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License
; along with this program; if not, write to the Free Software
; Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
;
; For more information on the GPL, please go to:
; http://www.gnu.org/copyleft/gpl.html
;

#define __SFR_OFFSET 0

#include <avr/io.h>

#define WS2812B_PORT   PORTA
#define WS2812B_DDR    DDRA
#define WS2812B_PINNUM PA7


.global ws2812b_init
.global ws2812b_write_byte
.global ws2812b_write_rgb
.global ws2812b_write_rgb_n


ws2812b_init:
  sbi WS2812B_DDR, WS2812B_PINNUM
  cbi WS2812B_PORT, WS2812B_PINNUM
  ret


; Params:
;   r24     value
; Variables:
;   r25     bit counter
ws2812b_write_byte:
  ldi r25, 0x08

#if F_CPU == 20000000  // 20 MHz
ws2812b_write_byte_loop:
  sbi WS2812B_PORT, WS2812B_PINNUM
  rjmp .+0
  rjmp .+0
  rjmp .+0
  sbrs r24, 7

  cbi WS2812B_PORT, WS2812B_PINNUM
  rjmp .+0
  rjmp .+0
  rjmp .+0
  sbrc r24, 7

  ; this block is 0.05us (one clock cycle) longer than the other two
  cbi WS2812B_PORT, WS2812B_PINNUM
  rjmp .+0
  rjmp .+0
  lsl r24
  subi r25, 1
  brne ws2812b_write_byte_loop

  ret

#elif F_CPU == 16000000  // 16 MHz
ws2812b_write_byte_loop:
  sbi WS2812B_PORT, WS2812B_PINNUM
  rjmp .+0
  rjmp .+0
  sbrs r24, 7

  cbi WS2812B_PORT, WS2812B_PINNUM
  rjmp .+0
  rjmp .+0
  nop
  sbrc r24, 7

  cbi WS2812B_PORT, WS2812B_PINNUM
  rjmp .+0
  lsl r24
  subi r25, 1
  brne ws2812b_write_byte_loop

  ret

#else
#error ws2812 not implemented for this clock frequency
#endif


; Params:
;   r24     r
;   r22     g  (unchanged)
;   r20     b  (unchanged)
; Variables:
;   r18     repetition counter (==1)
ws2812b_write_rgb:
  ldi r18, 1

; Params:
;   r24     r
;   r22     g  (unchanged)
;   r20     b  (unchanged)
;   r18     n
; Variables:
;   r0      tmp for saving SREG
;   r23     tmp for saving r
ws2812b_write_rgb_n:
  in r0, SREG
  cli
  mov r23, r24

ws2812b_write_rgb_n_loop:
  mov r24, r22
  call ws2812b_write_byte
  mov r24, r23
  call ws2812b_write_byte
  mov r24, r20
  call ws2812b_write_byte

  subi r18, 1
  brne ws2812b_write_rgb_n_loop
  out SREG, r0
  ret
