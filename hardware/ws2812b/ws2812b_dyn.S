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


.global ws2812b_dyn_write_byte
.global ws2812b_dyn_write_rgb
.global ws2812b_dyn_write_rgb_n


; Params:
;   r24     value
;   r22,23  *port    (unchanged)
;   r20     pinmask  (unchanged)
; Variables:
;   r25     bit counter
;   r26     tmp for bit manipulation
;   r21     inverted pinmask
;   r30,31  *port
ws2812b_dyn_write_byte:
  movw r30, r22
  mov r21, r20
  com r21

; Params:
;   r24     value
;   r30,31  *port            (unchanged)
;   r20     pinmask          (unchanged)
;   r21     inverted pinmask (unchanged)
; Variables:
;   r25     bit counter
;   r26     tmp for bit manipulation
ws2812b_dyn_write_byte_internal:
  ldi r25, 0x08

#if F_CPU == 20000000  // 20 MHz
ws2812b_dyn_write_byte_loop:
  ld r26, Z
  or r26, r20
  st Z, r26
  rjmp .+0
  sbrs r24, 7

  ld r26, Z
  and r26, r21
  st Z, r26
  rjmp .+0
  sbrc r24, 7

  ; this block is 0.05us (one clock cycle) longer than the other two
  ld r26, Z
  and r26, r21
  st Z, r26
  lsl r24
  subi r25, 1
  brne ws2812b_dyn_write_byte_loop

  ret

#elseif F_CPU == 16000000  // 16 MHz
ws2812b_dyn_write_byte_loop:
  ld r26, Z
  or r26, r20
  st Z, r26
  sbrs r24, 7

  ld r26, Z
  and r26, r21
  st Z, r26
  nop
  sbrc r24, 7

  ld r26, Z
  and r26, r21
  st Z, r26
  lsl r24
  subi r25, 1
  brne ws2812b_dyn_write_byte_loop

  ret

#else
#error ws2812_dyn not implemented for this clock frequency
#endif


; Params:
;   r24     r
;   r22     g        (unchanged)
;   r20     b
;   r18,19  *port
;   r16     pinmask  (unchanged)
; Variables:
;   r0      tmp for saving SREG
;   r23     tmp for saving r
;   r27     tmp for saving b
;   r18     repetition counter (==1)
;   r25,26  clobbered by write_byte
;   r24     param byte
;   r20,21  param pinmask
;   r30,31  param *port
ws2812b_dyn_write_rgb:
  movw r30, r18
  mov r27, r20
  ldi r18, 1
  mov r20, r16
  rjmp ws2812b_dyn_write_rgb_n_internal

; Params:
;   r24     r
;   r22     g        (unchanged)
;   r20     b
;   r18     n
;   r16,17  *port    (unchanged)
;   r14     pinmask  (unchanged)
; Variables:
;   r0      tmp for saving SREG
;   r23     tmp for saving r
;   r27     tmp for saving b
;   r18     repetition counter
;   r25,26  clobbered by write_byte
;   r24     param byte
;   r20,21  param pinmask
;   r30,31  param *port
ws2812b_dyn_write_rgb_n:
  movw r30, r16
  mov r27, r20
  mov r20, r14

ws2812b_dyn_write_rgb_n_internal:
  mov r21, r20
  com r21
  mov r23, r24

  in r0, SREG
  cli

ws2812b_dyn_write_rgb_n_loop:
  mov r24, r22
  call ws2812b_write_byte
  mov r24, r23
  call ws2812b_write_byte
  mov r24, r27
  call ws2812b_write_byte

  subi r18, 1
  brne ws2812b_dyn_write_rgb_n_loop
  out SREG, r0
  ret
