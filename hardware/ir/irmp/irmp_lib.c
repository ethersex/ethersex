/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * irmp.c - infrared multi-protocol decoder, supports several remote control protocols
 *
 * Copyright (c) 2009-2010 Frank Meyer - frank(at)fli4l.de
 *
 * $Id: irmp.c,v 1.83 2010/09/04 18:11:52 fm Exp $
 *
 * ATMEGA88 @ 8 MHz
 *
 * Typical manufacturers:
 *
 * SIRCS      - Sony
 * NEC        - NEC, Yamaha, Canon, Tevion, Harman/Kardon, Hitachi, JVC, Pioneer, Toshiba, Xoro, Orion, and many other Japanese manufacturers
 * SAMSUNG    - Samsung
 * SAMSUNG32  - Samsung
 * MATSUSHITA - Matsushita
 * KASEIKYO   - Panasonic, Denon & other Japanese manufacturers (members of "Japan's Association for Electric Home Application")
 * RECS80     - Philips, Nokia, Thomson, Nordmende, Telefunken, Saba
 * RC5        - Philips and other European manufacturers
 * DENON      - Denon
 * RC6        - Philips and other European manufacturers
 * APPLE      - Apple
 * NUBERT     - Nubert Subwoofer System
 * B&O        - Bang & Olufsen
 * PANASONIC  - Panasonic (older, yet not implemented)
 * GRUNDIG    - Grundig
 * NOKIA      - Nokia
 * SIEMENS    - Siemens, e.g. Gigaset M740AV
 *
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 *
 *   SIRCS
 *   -----
 *
 *   frame: 1 start bit + 12-20 data bits + no stop bit
 *   data:  7 command bits + 5 address bits + 0 to 8 additional bits
 *
 *   start bit:                           data "0":                 data "1":                 stop bit:
 *   -----------------_________           ------_____               ------------______
 *       2400us         600us             600us 600us               1200us      600 us        no stop bit
 *
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 *
 *   NEC + extended NEC
 *   -------------------------
 *
 *   frame: 1 start bit + 32 data bits + 1 stop bit
 *   data NEC:          8 address bits + 8 inverted address bits + 8 command bits + 8 inverted command bits
 *   data extended NEC: 16 address bits + 8 command bits + 8 inverted command bits
 *
 *   start bit:                           data "0":                 data "1":                 stop bit:
 *   -----------------_________           ------______              ------________________    ------______....
 *       9000us        4500us             560us  560us              560us    1690 us          560us
 *
 *
 *   Repetition frame:
 *
 *   -----------------_________------______  .... ~100ms Pause, then repeat
 *       9000us        2250us   560us
 *
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 *
 *   SAMSUNG
 *   -------
 *
 *   frame: 1 start bit + 16 data(1) bits + 1 sync bit + additional 20 data(2) bits + 1 stop bit
 *   data(1): 16 address bits
 *   data(2): 4 ID bits + 8 command bits + 8 inverted command bits
 *
 *   start bit:                           data "0":                 data "1":                 sync bit:               stop bit:
 *   ----------______________             ------______              ------________________    ------______________    ------______....
 *    4500us       4500us                 550us  450us              550us    1450us           550us    4500us         550us
 *
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 *
 *   SAMSUNG32
 *   ----------
 *
 *   frame: 1 start bit + 32 data bits + 1 stop bit
 *   data: 16 address bits + 16 command bits
 *
 *   start bit:                           data "0":                 data "1":                 stop bit:
 *   ----------______________             ------______              ------________________    ------______....
 *    4500us       4500us                 550us  450us              550us    1450us           550us
 *
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 *
 *   MATSUSHITA
 *   ----------
 *
 *   frame: 1 start bit + 24 data bits + 1 stop bit
 *   data:  6 custom bits + 6 command bits + 12 address bits
 *
 *   start bit:                           data "0":                 data "1":                 stop bit:
 *   ----------_________                  ------______              ------________________    ------______....
 *    3488us     3488us                   872us  872us              872us    2616us           872us
 *
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 *
 *   KASEIKYO
 *   --------
 *
 *   frame: 1 start bit + 48 data bits + 1 stop bit
 *   data:  16 manufacturer bits + 4 parity bits + 4 genre1 bits + 4 genre2 bits + 10 command bits + 2 id bits + 8 parity bits
 *
 *   start bit:                           data "0":                 data "1":                 stop bit:
 *   ----------______                     ------______              ------________________    ------______....
 *    3380us   1690us                     423us  423us              423us    1269us           423us
 *
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 *
 *   RECS80
 *   ------
 *
 *   frame: 2 start bits + 10 data bits + 1 stop bit
 *   data:  1 toggle bit + 3 address bits + 6 command bits
 *
 *   start bit:                           data "0":                 data "1":                 stop bit:
 *   -----_____________________           -----____________         -----______________       ------_______....
 *   158us       7432us                   158us   4902us            158us    7432us           158us
 *
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 *
 *   RECS80EXT
 *   ---------
 *
 *   frame: 2 start bits + 11 data bits + 1 stop bit
 *   data:  1 toggle bit + 4 address bits + 6 command bits
 *
 *   start bit:                           data "0":                 data "1":                 stop bit:
 *   -----_____________________           -----____________         -----______________       ------_______....
 *   158us       3637us                   158us   4902us            158us    7432us           158us
 *
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 *
 *   RC5 + RC5X
 *   ----------
 *
 *   RC5 frame:  2 start bits + 12 data bits + no stop bit
 *   RC5 data:   1 toggle bit + 5 address bits + 6 command bits
 *   RC5X frame: 1 start bit +  13 data bits + no stop bit
 *   RC5X data:  1 inverted command bit + 1 toggle bit + 5 address bits + 6 command bits
 *
 *   start bit:              data "0":                data "1":
 *   ______-----             ------______             ______------
 *   889us 889us             889us 889us              889us 889us
 *
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 *
 *   DENON
 *   -----
 *
 *   frame: 0 start bits + 16 data bits + stop bit + 65ms pause + 16 inverted data bits + stop bit
 *   data:  5 address bits + 10 command bits
 *
 *   data "0":                 data "1":
 *   ------________________    ------______________
 *   275us       775us         275us   1900us
 *
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 *
 *   RC6
 *   ---
 *
 *   RC6 frame:  1 start bit + 1 bit "1" + 3 mode bits + 1 toggle bit + 16 data bits + 2666 µs pause
 *   RC6 data:   8 address bits + 8 command bits
 *
 *   start  bit               toggle bit "0":      toggle bit "1":     data/mode "0":      data/mode "1":
 *   ____________-------      _______-------       -------_______      _______-------      -------_______
 *      2666us    889us        889us  889us         889us  889us        444us  444us        444us  444us
 *
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 *
 *   APPLE
 *   -----
 *
 *   frame: 1 start bit + 32 data bits + 1 stop bit
 *   data:  16 address bits + 11100000 + 8 command bits
 *
 *   start bit:                           data "0":                 data "1":                 stop bit:
 *   -----------------_________           ------______              ------________________    ------______....
 *       9000us        4500us             560us  560us              560us    1690 us          560us
 *
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 *
 *   NUBERT (subwoofer system)
 *   -------------------------
 *
 *   frame: 1 start bit + 10 data bits + 1 stop bit
 *   data:  0 address bits + 10 command bits ?
 *
 *   start bit:                       data "0":                 data "1":                 stop bit:
 *   ----------_____                  ------______              ------________________    ------______....
 *    1340us   340us                  500us 1300us              1340us 340us              500us
 *
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 *
 *   BANG_OLUFSEN
 *   ------------
 *
 *   frame: 4 start bits + 16 data bits + 1 trailer bit + 1 stop bit
 *   data:  0 address bits + 16 command bits
 *
 *   1st start bit:  2nd start bit:      3rd start bit:       4th start bit:
 *   -----________   -----________       -----_____________   -----________
 *   210us 3000us    210us 3000us        210us   15000us      210us 3000us
 *
 *   data "0":       data "1":           data "repeat bit":   trailer bit:         stop bit:
 *   -----________   -----_____________  -----___________     -----_____________   -----____...
 *   210us 3000us    210us   9000us      210us   6000us       210us   12000us      210us
 *
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 *
 *   GRUNDIG
 *   -------
 *
 *   packet:  1 start frame + 19,968ms pause + N info frames + 117,76ms pause + 1 stop frame
 *   frame: 1 pre bit + 1 start bit + 9 data bits + no stop bit
 *   pause between info frames: 117,76ms
 *
 *   data of start frame:   9 x 1
 *   data of info  frame:   9 command bits
 *   data of stop  frame:   9 x 1
 *
 *   pre bit:              start bit           data "0":            data "1":
 *   ------____________    ------______        ______------         ------______             
 *   528us  2639us         528us  528us        528us  528us         528us  528us
 *
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 *
 *   NOKIA:
 *   ------
 *
 *   Timing similar to Grundig, but 16 data bits:
 *   frame: 1 pre bit + 1 start bit + 8 command bits + 8 address bits + no stop bit
 *
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 *
 *   SIEMENS:
 *   --------
 *
 *   SIEMENS frame:  1 start bit + 22 data bits + no stop bit
 *   SIEMENS data:   13 address bits + 1 repeat bit + 7 data bits + 1 unknown bit
 *
 *   start  bit           data "0":            data "1":
 *   -------_______       _______-------       -------_______
 *    250us  250us         250us  250us         250us  250us
 *
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 *
 *   PANASONIC (older protocol, yet not implemented, see also MATSUSHITA, timing very similar)
 *   -----------------------------------------------------------------------------------------
 *
 *   frame: 1 start bit + 22 data bits + 1 stop bit
 *   22 data bits = 5 custom bits + 6 data bits + 5 inverted custom bits + 6 inverted data bits
 *
 *   European version:      T = 456us
 *   USA & Canada version:  T = 422us
 *
 *   start bit:                           data "0":                 data "1":                 stop bit:
 *        8T            8T                 2T   2T                   2T      6T                2T
 *   -------------____________            ------_____               ------_____________       ------_______....
 *      3648us        3648us              912us 912us               912us    2736us           912us                (Europe)
 *      3376us        3376us              844us 844us               844us    2532us           844us                (US)
 *
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */

#if defined(__PCM__) || defined(__PCB__) || defined(__PCH__)                // CCS PIC Compiler instead of AVR
#define PIC_CCS_COMPILER
#endif

#ifdef unix                                                                 // test on linux/unix
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#define ANALYZE
#define PROGMEM
#define memcpy_P        memcpy

#else // not unix:

#ifdef WIN32
#include <stdio.h>
#include <string.h>
typedef unsigned char   uint8_t;
typedef unsigned short  uint16_t;
#define ANALYZE
#define PROGMEM
#define memcpy_P        memcpy

#else

#ifndef CODEVISION

#ifdef PIC_CCS_COMPILER

#include <string.h>
typedef unsigned int8   uint8_t;
typedef unsigned int16  uint16_t;
#define PROGMEM
#define memcpy_P        memcpy

#else // AVR:

#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>

#endif  // PIC_CCS_COMPILER
#endif  // CODEVISION

#endif // windows
#endif // unix

#ifndef IRMP_USE_AS_LIB
//#include "irmpconfig.h"
#endif
#include "irmp.h"

#if IRMP_SUPPORT_GRUNDIG_PROTOCOL == 1 || IRMP_SUPPORT_NOKIA_PROTOCOL == 1
#define IRMP_SUPPORT_GRUNDIG_OR_NOKIA_PROTOCOL  1
#else
#define IRMP_SUPPORT_GRUNDIG_OR_NOKIA_PROTOCOL  0
#endif

#if IRMP_SUPPORT_RC5_PROTOCOL == 1 || IRMP_SUPPORT_RC6_PROTOCOL == 1 || IRMP_SUPPORT_GRUNDIG_OR_NOKIA_PROTOCOL == 1 || IRMP_SUPPORT_SIEMENS_PROTOCOL == 1 
#define IRMP_SUPPORT_MANCHESTER                 1
#else
#define IRMP_SUPPORT_MANCHESTER                 0
#endif

#define IRMP_KEY_REPETITION_LEN                 (uint16_t)(F_INTERRUPTS * 150.0e-3 + 0.5)           // autodetect key repetition within 150 msec

#define MIN_TOLERANCE_00                        1.0                           // -0%
#define MAX_TOLERANCE_00                        1.0                           // +0%

#define MIN_TOLERANCE_05                        0.95                          // -5%
#define MAX_TOLERANCE_05                        1.05                          // +5%

#define MIN_TOLERANCE_10                        0.9                           // -10%
#define MAX_TOLERANCE_10                        1.1                           // +10%

#define MIN_TOLERANCE_15                        0.85                          // -15%
#define MAX_TOLERANCE_15                        1.15                          // +15%

#define MIN_TOLERANCE_20                        0.8                           // -20%
#define MAX_TOLERANCE_20                        1.2                           // +20%

#define MIN_TOLERANCE_30                        0.7                           // -30%
#define MAX_TOLERANCE_30                        1.3                           // +30%

#define MIN_TOLERANCE_40                        0.6                           // -40%
#define MAX_TOLERANCE_40                        1.4                           // +40%

#define MIN_TOLERANCE_50                        0.5                           // -50%
#define MAX_TOLERANCE_50                        1.5                           // +50%

#define MIN_TOLERANCE_60                        0.4                           // -60%
#define MAX_TOLERANCE_60                        1.6                           // +60%

#define SIRCS_START_BIT_PULSE_LEN_MIN           ((uint8_t)(F_INTERRUPTS * SIRCS_START_BIT_PULSE_TIME * MIN_TOLERANCE_10 + 0.5) - 1)
#define SIRCS_START_BIT_PULSE_LEN_MAX           ((uint8_t)(F_INTERRUPTS * SIRCS_START_BIT_PULSE_TIME * MAX_TOLERANCE_10 + 0.5) + 1)
#define SIRCS_START_BIT_PAUSE_LEN_MIN           ((uint8_t)(F_INTERRUPTS * SIRCS_START_BIT_PAUSE_TIME * MIN_TOLERANCE_20 + 0.5) - 1)
#define SIRCS_START_BIT_PAUSE_LEN_MAX           ((uint8_t)(F_INTERRUPTS * SIRCS_START_BIT_PAUSE_TIME * MAX_TOLERANCE_05 + 0.5) + 1) // only 5% to avoid conflict with RC6
#define SIRCS_1_PULSE_LEN_MIN                   ((uint8_t)(F_INTERRUPTS * SIRCS_1_PULSE_TIME * MIN_TOLERANCE_10 + 0.5) - 1)
#define SIRCS_1_PULSE_LEN_MAX                   ((uint8_t)(F_INTERRUPTS * SIRCS_1_PULSE_TIME * MAX_TOLERANCE_10 + 0.5) + 1)
#define SIRCS_0_PULSE_LEN_MIN                   ((uint8_t)(F_INTERRUPTS * SIRCS_0_PULSE_TIME * MIN_TOLERANCE_10 + 0.5) - 1)
#define SIRCS_0_PULSE_LEN_MAX                   ((uint8_t)(F_INTERRUPTS * SIRCS_0_PULSE_TIME * MAX_TOLERANCE_10 + 0.5) + 1)
#define SIRCS_PAUSE_LEN_MIN                     ((uint8_t)(F_INTERRUPTS * SIRCS_PAUSE_TIME * MIN_TOLERANCE_10 + 0.5) - 1)
#define SIRCS_PAUSE_LEN_MAX                     ((uint8_t)(F_INTERRUPTS * SIRCS_PAUSE_TIME * MAX_TOLERANCE_10 + 0.5) + 1)

#define NEC_START_BIT_PULSE_LEN_MIN             ((uint8_t)(F_INTERRUPTS * NEC_START_BIT_PULSE_TIME * MIN_TOLERANCE_40 + 0.5) - 1)
#define NEC_START_BIT_PULSE_LEN_MAX             ((uint8_t)(F_INTERRUPTS * NEC_START_BIT_PULSE_TIME * MAX_TOLERANCE_40 + 0.5) + 1)
#define NEC_START_BIT_PAUSE_LEN_MIN             ((uint8_t)(F_INTERRUPTS * NEC_START_BIT_PAUSE_TIME * MIN_TOLERANCE_40 + 0.5) - 1)
#define NEC_START_BIT_PAUSE_LEN_MAX             ((uint8_t)(F_INTERRUPTS * NEC_START_BIT_PAUSE_TIME * MAX_TOLERANCE_40 + 0.5) + 1)
#define NEC_REPEAT_START_BIT_PAUSE_LEN_MIN      ((uint8_t)(F_INTERRUPTS * NEC_REPEAT_START_BIT_PAUSE_TIME * MIN_TOLERANCE_40 + 0.5) - 1)
#define NEC_REPEAT_START_BIT_PAUSE_LEN_MAX      ((uint8_t)(F_INTERRUPTS * NEC_REPEAT_START_BIT_PAUSE_TIME * MAX_TOLERANCE_40 + 0.5) + 1)
#define NEC_PULSE_LEN_MIN                       ((uint8_t)(F_INTERRUPTS * NEC_PULSE_TIME * MIN_TOLERANCE_40 + 0.5) - 1)
#define NEC_PULSE_LEN_MAX                       ((uint8_t)(F_INTERRUPTS * NEC_PULSE_TIME * MAX_TOLERANCE_40 + 0.5) + 1)
#define NEC_1_PAUSE_LEN_MIN                     ((uint8_t)(F_INTERRUPTS * NEC_1_PAUSE_TIME * MIN_TOLERANCE_40 + 0.5) - 1)
#define NEC_1_PAUSE_LEN_MAX                     ((uint8_t)(F_INTERRUPTS * NEC_1_PAUSE_TIME * MAX_TOLERANCE_40 + 0.5) + 1)
#define NEC_0_PAUSE_LEN_MIN                     ((uint8_t)(F_INTERRUPTS * NEC_0_PAUSE_TIME * MIN_TOLERANCE_40 + 0.5) - 1)
#define NEC_0_PAUSE_LEN_MAX                     ((uint8_t)(F_INTERRUPTS * NEC_0_PAUSE_TIME * MAX_TOLERANCE_40 + 0.5) + 1)
// autodetect nec repetition frame within 50 msec:
// NEC seems to send the first repetition frame after 40ms, further repetition frames after 100 ms
#if 0
#define NEC_FRAME_REPEAT_PAUSE_LEN_MAX          (uint16_t)(F_INTERRUPTS * NEC_FRAME_REPEAT_PAUSE_TIME * MAX_TOLERANCE_20 + 0.5)
#else
#define NEC_FRAME_REPEAT_PAUSE_LEN_MAX          (uint16_t)(F_INTERRUPTS * 100.0e-3 * MAX_TOLERANCE_20 + 0.5)
#endif

#define SAMSUNG_START_BIT_PULSE_LEN_MIN         ((uint8_t)(F_INTERRUPTS * SAMSUNG_START_BIT_PULSE_TIME * MIN_TOLERANCE_10 + 0.5) - 1)
#define SAMSUNG_START_BIT_PULSE_LEN_MAX         ((uint8_t)(F_INTERRUPTS * SAMSUNG_START_BIT_PULSE_TIME * MAX_TOLERANCE_10 + 0.5) + 1)
#define SAMSUNG_START_BIT_PAUSE_LEN_MIN         ((uint8_t)(F_INTERRUPTS * SAMSUNG_START_BIT_PAUSE_TIME * MIN_TOLERANCE_10 + 0.5) - 1)
#define SAMSUNG_START_BIT_PAUSE_LEN_MAX         ((uint8_t)(F_INTERRUPTS * SAMSUNG_START_BIT_PAUSE_TIME * MAX_TOLERANCE_10 + 0.5) + 1)
#define SAMSUNG_PULSE_LEN_MIN                   ((uint8_t)(F_INTERRUPTS * SAMSUNG_PULSE_TIME * MIN_TOLERANCE_30 + 0.5) - 1)
#define SAMSUNG_PULSE_LEN_MAX                   ((uint8_t)(F_INTERRUPTS * SAMSUNG_PULSE_TIME * MAX_TOLERANCE_30 + 0.5) + 1)
#define SAMSUNG_1_PAUSE_LEN_MIN                 ((uint8_t)(F_INTERRUPTS * SAMSUNG_1_PAUSE_TIME * MIN_TOLERANCE_30 + 0.5) - 1)
#define SAMSUNG_1_PAUSE_LEN_MAX                 ((uint8_t)(F_INTERRUPTS * SAMSUNG_1_PAUSE_TIME * MAX_TOLERANCE_30 + 0.5) + 1)
#define SAMSUNG_0_PAUSE_LEN_MIN                 ((uint8_t)(F_INTERRUPTS * SAMSUNG_0_PAUSE_TIME * MIN_TOLERANCE_30 + 0.5) - 1)
#define SAMSUNG_0_PAUSE_LEN_MAX                 ((uint8_t)(F_INTERRUPTS * SAMSUNG_0_PAUSE_TIME * MAX_TOLERANCE_30 + 0.5) + 1)

#define MATSUSHITA_START_BIT_PULSE_LEN_MIN      ((uint8_t)(F_INTERRUPTS * MATSUSHITA_START_BIT_PULSE_TIME * MIN_TOLERANCE_20 + 0.5) - 1)
#define MATSUSHITA_START_BIT_PULSE_LEN_MAX      ((uint8_t)(F_INTERRUPTS * MATSUSHITA_START_BIT_PULSE_TIME * MAX_TOLERANCE_20 + 0.5) + 1)
#define MATSUSHITA_START_BIT_PAUSE_LEN_MIN      ((uint8_t)(F_INTERRUPTS * MATSUSHITA_START_BIT_PAUSE_TIME * MIN_TOLERANCE_20 + 0.5) - 1)
#define MATSUSHITA_START_BIT_PAUSE_LEN_MAX      ((uint8_t)(F_INTERRUPTS * MATSUSHITA_START_BIT_PAUSE_TIME * MAX_TOLERANCE_20 + 0.5) + 1)
#define MATSUSHITA_PULSE_LEN_MIN                ((uint8_t)(F_INTERRUPTS * MATSUSHITA_PULSE_TIME * MIN_TOLERANCE_40 + 0.5) - 1)
#define MATSUSHITA_PULSE_LEN_MAX                ((uint8_t)(F_INTERRUPTS * MATSUSHITA_PULSE_TIME * MAX_TOLERANCE_40 + 0.5) + 1)
#define MATSUSHITA_1_PAUSE_LEN_MIN              ((uint8_t)(F_INTERRUPTS * MATSUSHITA_1_PAUSE_TIME * MIN_TOLERANCE_40 + 0.5) - 1)
#define MATSUSHITA_1_PAUSE_LEN_MAX              ((uint8_t)(F_INTERRUPTS * MATSUSHITA_1_PAUSE_TIME * MAX_TOLERANCE_40 + 0.5) + 1)
#define MATSUSHITA_0_PAUSE_LEN_MIN              ((uint8_t)(F_INTERRUPTS * MATSUSHITA_0_PAUSE_TIME * MIN_TOLERANCE_40 + 0.5) - 1)
#define MATSUSHITA_0_PAUSE_LEN_MAX              ((uint8_t)(F_INTERRUPTS * MATSUSHITA_0_PAUSE_TIME * MAX_TOLERANCE_40 + 0.5) + 1)

#define KASEIKYO_START_BIT_PULSE_LEN_MIN        ((uint8_t)(F_INTERRUPTS * KASEIKYO_START_BIT_PULSE_TIME * MIN_TOLERANCE_10 + 0.5) - 1)
#define KASEIKYO_START_BIT_PULSE_LEN_MAX        ((uint8_t)(F_INTERRUPTS * KASEIKYO_START_BIT_PULSE_TIME * MAX_TOLERANCE_10 + 0.5) + 1)
#define KASEIKYO_START_BIT_PAUSE_LEN_MIN        ((uint8_t)(F_INTERRUPTS * KASEIKYO_START_BIT_PAUSE_TIME * MIN_TOLERANCE_10 + 0.5) - 1)
#define KASEIKYO_START_BIT_PAUSE_LEN_MAX        ((uint8_t)(F_INTERRUPTS * KASEIKYO_START_BIT_PAUSE_TIME * MAX_TOLERANCE_10 + 0.5) + 1)
#define KASEIKYO_PULSE_LEN_MIN                  ((uint8_t)(F_INTERRUPTS * KASEIKYO_PULSE_TIME * MIN_TOLERANCE_50 + 0.5) - 1)
#define KASEIKYO_PULSE_LEN_MAX                  ((uint8_t)(F_INTERRUPTS * KASEIKYO_PULSE_TIME * MAX_TOLERANCE_50 + 0.5) + 1)
#define KASEIKYO_1_PAUSE_LEN_MIN                ((uint8_t)(F_INTERRUPTS * KASEIKYO_1_PAUSE_TIME * MIN_TOLERANCE_30 + 0.5) - 1)
#define KASEIKYO_1_PAUSE_LEN_MAX                ((uint8_t)(F_INTERRUPTS * KASEIKYO_1_PAUSE_TIME * MAX_TOLERANCE_30 + 0.5) + 1)
#define KASEIKYO_0_PAUSE_LEN_MIN                ((uint8_t)(F_INTERRUPTS * KASEIKYO_0_PAUSE_TIME * MIN_TOLERANCE_50 + 0.5) - 1)
#define KASEIKYO_0_PAUSE_LEN_MAX                ((uint8_t)(F_INTERRUPTS * KASEIKYO_0_PAUSE_TIME * MAX_TOLERANCE_50 + 0.5) + 1)

#define RECS80_START_BIT_PULSE_LEN_MIN          ((uint8_t)(F_INTERRUPTS * RECS80_START_BIT_PULSE_TIME * MIN_TOLERANCE_00 + 0.5) - 1)
#define RECS80_START_BIT_PULSE_LEN_MAX          ((uint8_t)(F_INTERRUPTS * RECS80_START_BIT_PULSE_TIME * MAX_TOLERANCE_10 + 0.5) + 1)
#define RECS80_START_BIT_PAUSE_LEN_MIN          ((uint8_t)(F_INTERRUPTS * RECS80_START_BIT_PAUSE_TIME * MIN_TOLERANCE_10 + 0.5) - 1)
#define RECS80_START_BIT_PAUSE_LEN_MAX          ((uint8_t)(F_INTERRUPTS * RECS80_START_BIT_PAUSE_TIME * MAX_TOLERANCE_10 + 0.5) + 1)
#define RECS80_PULSE_LEN_MIN                    ((uint8_t)(F_INTERRUPTS * RECS80_PULSE_TIME * MIN_TOLERANCE_10 + 0.5) - 1)
#define RECS80_PULSE_LEN_MAX                    ((uint8_t)(F_INTERRUPTS * RECS80_PULSE_TIME * MAX_TOLERANCE_10 + 0.5) + 1)
#define RECS80_1_PAUSE_LEN_MIN                  ((uint8_t)(F_INTERRUPTS * RECS80_1_PAUSE_TIME * MIN_TOLERANCE_10 + 0.5) - 1)
#define RECS80_1_PAUSE_LEN_MAX                  ((uint8_t)(F_INTERRUPTS * RECS80_1_PAUSE_TIME * MAX_TOLERANCE_10 + 0.5) + 1)
#define RECS80_0_PAUSE_LEN_MIN                  ((uint8_t)(F_INTERRUPTS * RECS80_0_PAUSE_TIME * MIN_TOLERANCE_10 + 0.5) - 1)
#define RECS80_0_PAUSE_LEN_MAX                  ((uint8_t)(F_INTERRUPTS * RECS80_0_PAUSE_TIME * MAX_TOLERANCE_10 + 0.5) + 1)

#define RC5_START_BIT_LEN_MIN                   ((uint8_t)(F_INTERRUPTS * RC5_BIT_TIME * MIN_TOLERANCE_10 + 0.5) - 1)
#define RC5_START_BIT_LEN_MAX                   ((uint8_t)(F_INTERRUPTS * RC5_BIT_TIME * MAX_TOLERANCE_10 + 0.5) + 1)
#define RC5_BIT_LEN_MIN                         ((uint8_t)(F_INTERRUPTS * RC5_BIT_TIME * MIN_TOLERANCE_10 + 0.5) - 1)
#define RC5_BIT_LEN_MAX                         ((uint8_t)(F_INTERRUPTS * RC5_BIT_TIME * MAX_TOLERANCE_10 + 0.5) + 1)

#define DENON_PULSE_LEN_MIN                     ((uint8_t)(F_INTERRUPTS * DENON_PULSE_TIME * MIN_TOLERANCE_10 + 0.5) - 1)
#define DENON_PULSE_LEN_MAX                     ((uint8_t)(F_INTERRUPTS * DENON_PULSE_TIME * MAX_TOLERANCE_10 + 0.5) + 1)
#define DENON_1_PAUSE_LEN_MIN                   ((uint8_t)(F_INTERRUPTS * DENON_1_PAUSE_TIME * MIN_TOLERANCE_10 + 0.5) - 1)
#define DENON_1_PAUSE_LEN_MAX                   ((uint8_t)(F_INTERRUPTS * DENON_1_PAUSE_TIME * MAX_TOLERANCE_10 + 0.5) + 1)
#define DENON_0_PAUSE_LEN_MIN                   ((uint8_t)(F_INTERRUPTS * DENON_0_PAUSE_TIME * MIN_TOLERANCE_10 + 0.5) - 1)
#define DENON_0_PAUSE_LEN_MAX                   ((uint8_t)(F_INTERRUPTS * DENON_0_PAUSE_TIME * MAX_TOLERANCE_10 + 0.5) + 1)

#define RC6_START_BIT_PULSE_LEN_MIN             ((uint8_t)(F_INTERRUPTS * RC6_START_BIT_PULSE_TIME * MIN_TOLERANCE_10 + 0.5) - 1)
#define RC6_START_BIT_PULSE_LEN_MAX             ((uint8_t)(F_INTERRUPTS * RC6_START_BIT_PULSE_TIME * MAX_TOLERANCE_10 + 0.5) + 1)
#define RC6_START_BIT_PAUSE_LEN_MIN             ((uint8_t)(F_INTERRUPTS * RC6_START_BIT_PAUSE_TIME * MIN_TOLERANCE_10 + 0.5) - 1)
#define RC6_START_BIT_PAUSE_LEN_MAX             ((uint8_t)(F_INTERRUPTS * RC6_START_BIT_PAUSE_TIME * MAX_TOLERANCE_10 + 0.5) + 1)
#define RC6_TOGGLE_BIT_LEN_MIN                  ((uint8_t)(F_INTERRUPTS * RC6_TOGGLE_BIT_TIME * MIN_TOLERANCE_10 + 0.5) - 1)
#define RC6_TOGGLE_BIT_LEN_MAX                  ((uint8_t)(F_INTERRUPTS * RC6_TOGGLE_BIT_TIME * MAX_TOLERANCE_10 + 0.5) + 1)
#define RC6_BIT_PULSE_LEN_MIN                   ((uint8_t)(F_INTERRUPTS * RC6_BIT_TIME * MIN_TOLERANCE_10 + 0.5) - 1)
#define RC6_BIT_PULSE_LEN_MAX                   ((uint8_t)(F_INTERRUPTS * RC6_BIT_TIME * MAX_TOLERANCE_30 + 0.5) + 1)       // pulses: 300 - 700
#define RC6_BIT_PAUSE_LEN_MIN                   ((uint8_t)(F_INTERRUPTS * RC6_BIT_TIME * MIN_TOLERANCE_10 + 0.5) - 1)       // pauses: 300 - 600
#define RC6_BIT_PAUSE_LEN_MAX                   ((uint8_t)(F_INTERRUPTS * RC6_BIT_TIME * MAX_TOLERANCE_10 + 0.5) + 1)

#define RECS80EXT_START_BIT_PULSE_LEN_MIN       ((uint8_t)(F_INTERRUPTS * RECS80EXT_START_BIT_PULSE_TIME * MIN_TOLERANCE_00 + 0.5) - 1)
#define RECS80EXT_START_BIT_PULSE_LEN_MAX       ((uint8_t)(F_INTERRUPTS * RECS80EXT_START_BIT_PULSE_TIME * MAX_TOLERANCE_00 + 0.5) + 1)
#define RECS80EXT_START_BIT_PAUSE_LEN_MIN       ((uint8_t)(F_INTERRUPTS * RECS80EXT_START_BIT_PAUSE_TIME * MIN_TOLERANCE_05 + 0.5) - 1)
#define RECS80EXT_START_BIT_PAUSE_LEN_MAX       ((uint8_t)(F_INTERRUPTS * RECS80EXT_START_BIT_PAUSE_TIME * MAX_TOLERANCE_05 + 0.5) + 1)
#define RECS80EXT_PULSE_LEN_MIN                 ((uint8_t)(F_INTERRUPTS * RECS80EXT_PULSE_TIME * MIN_TOLERANCE_10 + 0.5) - 1)
#define RECS80EXT_PULSE_LEN_MAX                 ((uint8_t)(F_INTERRUPTS * RECS80EXT_PULSE_TIME * MAX_TOLERANCE_10 + 0.5) + 1)
#define RECS80EXT_1_PAUSE_LEN_MIN               ((uint8_t)(F_INTERRUPTS * RECS80EXT_1_PAUSE_TIME * MIN_TOLERANCE_10 + 0.5) - 1)
#define RECS80EXT_1_PAUSE_LEN_MAX               ((uint8_t)(F_INTERRUPTS * RECS80EXT_1_PAUSE_TIME * MAX_TOLERANCE_10 + 0.5) + 1)
#define RECS80EXT_0_PAUSE_LEN_MIN               ((uint8_t)(F_INTERRUPTS * RECS80EXT_0_PAUSE_TIME * MIN_TOLERANCE_10 + 0.5) - 1)
#define RECS80EXT_0_PAUSE_LEN_MAX               ((uint8_t)(F_INTERRUPTS * RECS80EXT_0_PAUSE_TIME * MAX_TOLERANCE_10 + 0.5) + 1)

#define NUBERT_START_BIT_PULSE_LEN_MIN          ((uint8_t)(F_INTERRUPTS * NUBERT_START_BIT_PULSE_TIME * MIN_TOLERANCE_20 + 0.5) - 1)
#define NUBERT_START_BIT_PULSE_LEN_MAX          ((uint8_t)(F_INTERRUPTS * NUBERT_START_BIT_PULSE_TIME * MAX_TOLERANCE_20 + 0.5) + 1)
#define NUBERT_START_BIT_PAUSE_LEN_MIN          ((uint8_t)(F_INTERRUPTS * NUBERT_START_BIT_PAUSE_TIME * MIN_TOLERANCE_20 + 0.5) - 1)
#define NUBERT_START_BIT_PAUSE_LEN_MAX          ((uint8_t)(F_INTERRUPTS * NUBERT_START_BIT_PAUSE_TIME * MAX_TOLERANCE_20 + 0.5) + 1)
#define NUBERT_1_PULSE_LEN_MIN                  ((uint8_t)(F_INTERRUPTS * NUBERT_1_PULSE_TIME * MIN_TOLERANCE_20 + 0.5) - 1)
#define NUBERT_1_PULSE_LEN_MAX                  ((uint8_t)(F_INTERRUPTS * NUBERT_1_PULSE_TIME * MAX_TOLERANCE_20 + 0.5) + 1)
#define NUBERT_1_PAUSE_LEN_MIN                  ((uint8_t)(F_INTERRUPTS * NUBERT_1_PAUSE_TIME * MIN_TOLERANCE_20 + 0.5) - 1)
#define NUBERT_1_PAUSE_LEN_MAX                  ((uint8_t)(F_INTERRUPTS * NUBERT_1_PAUSE_TIME * MAX_TOLERANCE_20 + 0.5) + 1)
#define NUBERT_0_PULSE_LEN_MIN                  ((uint8_t)(F_INTERRUPTS * NUBERT_0_PULSE_TIME * MIN_TOLERANCE_20 + 0.5) - 1)
#define NUBERT_0_PULSE_LEN_MAX                  ((uint8_t)(F_INTERRUPTS * NUBERT_0_PULSE_TIME * MAX_TOLERANCE_20 + 0.5) + 1)
#define NUBERT_0_PAUSE_LEN_MIN                  ((uint8_t)(F_INTERRUPTS * NUBERT_0_PAUSE_TIME * MIN_TOLERANCE_20 + 0.5) - 1)
#define NUBERT_0_PAUSE_LEN_MAX                  ((uint8_t)(F_INTERRUPTS * NUBERT_0_PAUSE_TIME * MAX_TOLERANCE_20 + 0.5) + 1)

#define BANG_OLUFSEN_START_BIT1_PULSE_LEN_MIN   ((uint8_t)(F_INTERRUPTS * BANG_OLUFSEN_START_BIT1_PULSE_TIME * MIN_TOLERANCE_10 + 0.5) - 1)
#define BANG_OLUFSEN_START_BIT1_PULSE_LEN_MAX   ((uint8_t)(F_INTERRUPTS * BANG_OLUFSEN_START_BIT1_PULSE_TIME * MAX_TOLERANCE_10 + 0.5) + 1)
#define BANG_OLUFSEN_START_BIT1_PAUSE_LEN_MIN   ((uint8_t)(F_INTERRUPTS * BANG_OLUFSEN_START_BIT1_PAUSE_TIME * MIN_TOLERANCE_10 + 0.5) - 1)
#define BANG_OLUFSEN_START_BIT1_PAUSE_LEN_MAX   ((uint8_t)(F_INTERRUPTS * BANG_OLUFSEN_START_BIT1_PAUSE_TIME * MAX_TOLERANCE_10 + 0.5) + 1)
#define BANG_OLUFSEN_START_BIT2_PULSE_LEN_MIN   ((uint8_t)(F_INTERRUPTS * BANG_OLUFSEN_START_BIT2_PULSE_TIME * MIN_TOLERANCE_10 + 0.5) - 1)
#define BANG_OLUFSEN_START_BIT2_PULSE_LEN_MAX   ((uint8_t)(F_INTERRUPTS * BANG_OLUFSEN_START_BIT2_PULSE_TIME * MAX_TOLERANCE_10 + 0.5) + 1)
#define BANG_OLUFSEN_START_BIT2_PAUSE_LEN_MIN   ((uint8_t)(F_INTERRUPTS * BANG_OLUFSEN_START_BIT2_PAUSE_TIME * MIN_TOLERANCE_10 + 0.5) - 1)
#define BANG_OLUFSEN_START_BIT2_PAUSE_LEN_MAX   ((uint8_t)(F_INTERRUPTS * BANG_OLUFSEN_START_BIT2_PAUSE_TIME * MAX_TOLERANCE_10 + 0.5) + 1)
#define BANG_OLUFSEN_START_BIT3_PULSE_LEN_MIN   ((uint8_t)(F_INTERRUPTS * BANG_OLUFSEN_START_BIT3_PULSE_TIME * MIN_TOLERANCE_10 + 0.5) - 1)
#define BANG_OLUFSEN_START_BIT3_PULSE_LEN_MAX   ((uint8_t)(F_INTERRUPTS * BANG_OLUFSEN_START_BIT3_PULSE_TIME * MAX_TOLERANCE_10 + 0.5) + 1)
#define BANG_OLUFSEN_START_BIT3_PAUSE_LEN_MIN   ((uint8_t)(F_INTERRUPTS * BANG_OLUFSEN_START_BIT3_PAUSE_TIME * MIN_TOLERANCE_10 + 0.5) - 1)
#define BANG_OLUFSEN_START_BIT3_PAUSE_LEN_MAX   ((PAUSE_LEN)(F_INTERRUPTS * BANG_OLUFSEN_START_BIT3_PAUSE_TIME * MAX_TOLERANCE_05 + 0.5) + 1) // value must be below IRMP_TIMEOUT
#define BANG_OLUFSEN_START_BIT4_PULSE_LEN_MIN   ((uint8_t)(F_INTERRUPTS * BANG_OLUFSEN_START_BIT4_PULSE_TIME * MIN_TOLERANCE_10 + 0.5) - 1)
#define BANG_OLUFSEN_START_BIT4_PULSE_LEN_MAX   ((uint8_t)(F_INTERRUPTS * BANG_OLUFSEN_START_BIT4_PULSE_TIME * MAX_TOLERANCE_10 + 0.5) + 1)
#define BANG_OLUFSEN_START_BIT4_PAUSE_LEN_MIN   ((uint8_t)(F_INTERRUPTS * BANG_OLUFSEN_START_BIT4_PAUSE_TIME * MIN_TOLERANCE_10 + 0.5) - 1)
#define BANG_OLUFSEN_START_BIT4_PAUSE_LEN_MAX   ((uint8_t)(F_INTERRUPTS * BANG_OLUFSEN_START_BIT4_PAUSE_TIME * MAX_TOLERANCE_10 + 0.5) + 1)
#define BANG_OLUFSEN_PULSE_LEN_MIN              ((uint8_t)(F_INTERRUPTS * BANG_OLUFSEN_PULSE_TIME * MIN_TOLERANCE_10 + 0.5) - 1)
#define BANG_OLUFSEN_PULSE_LEN_MAX              ((uint8_t)(F_INTERRUPTS * BANG_OLUFSEN_PULSE_TIME * MAX_TOLERANCE_10 + 0.5) + 1)
#define BANG_OLUFSEN_1_PAUSE_LEN_MIN            ((uint8_t)(F_INTERRUPTS * BANG_OLUFSEN_1_PAUSE_TIME * MIN_TOLERANCE_10 + 0.5) - 1)
#define BANG_OLUFSEN_1_PAUSE_LEN_MAX            ((uint8_t)(F_INTERRUPTS * BANG_OLUFSEN_1_PAUSE_TIME * MAX_TOLERANCE_10 + 0.5) + 1)
#define BANG_OLUFSEN_0_PAUSE_LEN_MIN            ((uint8_t)(F_INTERRUPTS * BANG_OLUFSEN_0_PAUSE_TIME * MIN_TOLERANCE_10 + 0.5) - 1)
#define BANG_OLUFSEN_0_PAUSE_LEN_MAX            ((uint8_t)(F_INTERRUPTS * BANG_OLUFSEN_0_PAUSE_TIME * MAX_TOLERANCE_10 + 0.5) + 1)
#define BANG_OLUFSEN_R_PAUSE_LEN_MIN            ((uint8_t)(F_INTERRUPTS * BANG_OLUFSEN_R_PAUSE_TIME * MIN_TOLERANCE_10 + 0.5) - 1)
#define BANG_OLUFSEN_R_PAUSE_LEN_MAX            ((uint8_t)(F_INTERRUPTS * BANG_OLUFSEN_R_PAUSE_TIME * MAX_TOLERANCE_10 + 0.5) + 1)
#define BANG_OLUFSEN_TRAILER_BIT_PAUSE_LEN_MIN  ((uint8_t)(F_INTERRUPTS * BANG_OLUFSEN_TRAILER_BIT_PAUSE_TIME * MIN_TOLERANCE_10 + 0.5) - 1)
#define BANG_OLUFSEN_TRAILER_BIT_PAUSE_LEN_MAX  ((uint8_t)(F_INTERRUPTS * BANG_OLUFSEN_TRAILER_BIT_PAUSE_TIME * MAX_TOLERANCE_10 + 0.5) + 1)

#define GRUNDIG_OR_NOKIA_START_BIT_LEN_MIN      ((uint8_t)(F_INTERRUPTS * GRUNDIG_OR_NOKIA_BIT_TIME * MIN_TOLERANCE_20 + 0.5) - 1)
#define GRUNDIG_OR_NOKIA_START_BIT_LEN_MAX      ((uint8_t)(F_INTERRUPTS * GRUNDIG_OR_NOKIA_BIT_TIME * MAX_TOLERANCE_20 + 0.5) + 1)
#define GRUNDIG_OR_NOKIA_BIT_LEN_MIN            ((uint8_t)(F_INTERRUPTS * GRUNDIG_OR_NOKIA_BIT_TIME * MIN_TOLERANCE_20 + 0.5) - 1)
#define GRUNDIG_OR_NOKIA_BIT_LEN_MAX            ((uint8_t)(F_INTERRUPTS * GRUNDIG_OR_NOKIA_BIT_TIME * MAX_TOLERANCE_20 + 0.5) + 1)
#define GRUNDIG_OR_NOKIA_PRE_PAUSE_LEN_MIN      ((uint8_t)(F_INTERRUPTS * GRUNDIG_OR_NOKIA_PRE_PAUSE_TIME * MIN_TOLERANCE_20 + 0.5) + 1)
#define GRUNDIG_OR_NOKIA_PRE_PAUSE_LEN_MAX      ((uint8_t)(F_INTERRUPTS * GRUNDIG_OR_NOKIA_PRE_PAUSE_TIME * MAX_TOLERANCE_20 + 0.5) + 1)

#define SIEMENS_START_BIT_LEN_MIN               ((uint8_t)(F_INTERRUPTS * SIEMENS_BIT_TIME * 1 + 0.5) - 1)
#define SIEMENS_START_BIT_LEN_MAX               ((uint8_t)(F_INTERRUPTS * SIEMENS_BIT_TIME * 1 + 0.5) + 1)
#define SIEMENS_BIT_LEN_MIN                     ((uint8_t)(F_INTERRUPTS * SIEMENS_BIT_TIME * 1 + 0.5) - 1)
#define SIEMENS_BIT_LEN_MAX                     ((uint8_t)(F_INTERRUPTS * SIEMENS_BIT_TIME * 1 + 0.5) + 1)

#define FDC_START_BIT_PULSE_LEN_MIN             ((uint8_t)(F_INTERRUPTS * FDC_START_BIT_PULSE_TIME * MIN_TOLERANCE_10 + 0.5) - 1)
#define FDC_START_BIT_PULSE_LEN_MAX             ((uint8_t)(F_INTERRUPTS * FDC_START_BIT_PULSE_TIME * MAX_TOLERANCE_10 + 0.5) + 1)
#define FDC_START_BIT_PAUSE_LEN_MIN             ((uint8_t)(F_INTERRUPTS * FDC_START_BIT_PAUSE_TIME * MIN_TOLERANCE_10 + 0.5) - 1)
#define FDC_START_BIT_PAUSE_LEN_MAX             ((uint8_t)(F_INTERRUPTS * FDC_START_BIT_PAUSE_TIME * MAX_TOLERANCE_10 + 0.5) + 1)
#define FDC_PULSE_LEN_MIN                       ((uint8_t)(F_INTERRUPTS * FDC_PULSE_TIME * MIN_TOLERANCE_40 + 0.5) - 1)
#define FDC_PULSE_LEN_MAX                       ((uint8_t)(F_INTERRUPTS * FDC_PULSE_TIME * MAX_TOLERANCE_50 + 0.5) + 1)
#define FDC_1_PAUSE_LEN_MIN                     ((uint8_t)(F_INTERRUPTS * FDC_1_PAUSE_TIME * MIN_TOLERANCE_20 + 0.5) - 1)
#define FDC_1_PAUSE_LEN_MAX                     ((uint8_t)(F_INTERRUPTS * FDC_1_PAUSE_TIME * MAX_TOLERANCE_20 + 0.5) + 1)
#if 0
#define FDC_0_PAUSE_LEN_MIN                     ((uint8_t)(F_INTERRUPTS * FDC_0_PAUSE_TIME * MIN_TOLERANCE_40 + 0.5) - 1)   // could be negative: 255
#else
#define FDC_0_PAUSE_LEN_MIN                     (1)                                                                         // simply use 1
#endif
#define FDC_0_PAUSE_LEN_MAX                     ((uint8_t)(F_INTERRUPTS * FDC_0_PAUSE_TIME * MAX_TOLERANCE_10 + 0.5) + 1)

#define RCCAR_START_BIT_PULSE_LEN_MIN           ((uint8_t)(F_INTERRUPTS * RCCAR_START_BIT_PULSE_TIME * MIN_TOLERANCE_10 + 0.5) - 1)
#define RCCAR_START_BIT_PULSE_LEN_MAX           ((uint8_t)(F_INTERRUPTS * RCCAR_START_BIT_PULSE_TIME * MAX_TOLERANCE_10 + 0.5) + 1)
#define RCCAR_START_BIT_PAUSE_LEN_MIN           ((uint8_t)(F_INTERRUPTS * RCCAR_START_BIT_PAUSE_TIME * MIN_TOLERANCE_10 + 0.5) - 1)
#define RCCAR_START_BIT_PAUSE_LEN_MAX           ((uint8_t)(F_INTERRUPTS * RCCAR_START_BIT_PAUSE_TIME * MAX_TOLERANCE_10 + 0.5) + 1)
#define RCCAR_PULSE_LEN_MIN                     ((uint8_t)(F_INTERRUPTS * RCCAR_PULSE_TIME * MIN_TOLERANCE_20 + 0.5) - 1)
#define RCCAR_PULSE_LEN_MAX                     ((uint8_t)(F_INTERRUPTS * RCCAR_PULSE_TIME * MAX_TOLERANCE_20 + 0.5) + 1)
#define RCCAR_1_PAUSE_LEN_MIN                   ((uint8_t)(F_INTERRUPTS * RCCAR_1_PAUSE_TIME * MIN_TOLERANCE_30 + 0.5) - 1)
#define RCCAR_1_PAUSE_LEN_MAX                   ((uint8_t)(F_INTERRUPTS * RCCAR_1_PAUSE_TIME * MAX_TOLERANCE_30 + 0.5) + 1)
#define RCCAR_0_PAUSE_LEN_MIN                   ((uint8_t)(F_INTERRUPTS * RCCAR_0_PAUSE_TIME * MIN_TOLERANCE_30 + 0.5) - 1)
#define RCCAR_0_PAUSE_LEN_MAX                   ((uint8_t)(F_INTERRUPTS * RCCAR_0_PAUSE_TIME * MAX_TOLERANCE_30 + 0.5) + 1)

#define JVC_START_BIT_PULSE_LEN_MIN             ((uint8_t)(F_INTERRUPTS * JVC_START_BIT_PULSE_TIME * MIN_TOLERANCE_40 + 0.5) - 1)
#define JVC_START_BIT_PULSE_LEN_MAX             ((uint8_t)(F_INTERRUPTS * JVC_START_BIT_PULSE_TIME * MAX_TOLERANCE_40 + 0.5) + 1)
#define JVC_REPEAT_START_BIT_PAUSE_LEN_MIN      ((uint8_t)(F_INTERRUPTS * (JVC_FRAME_REPEAT_PAUSE_TIME - IRMP_TIMEOUT_TIME) * MIN_TOLERANCE_40 + 0.5) - 1)  // HACK!
#define JVC_REPEAT_START_BIT_PAUSE_LEN_MAX      ((uint8_t)(F_INTERRUPTS * (JVC_FRAME_REPEAT_PAUSE_TIME - IRMP_TIMEOUT_TIME) * MAX_TOLERANCE_40 + 0.5) - 1)  // HACK!
#define JVC_PULSE_LEN_MIN                       ((uint8_t)(F_INTERRUPTS * JVC_PULSE_TIME * MIN_TOLERANCE_40 + 0.5) - 1)
#define JVC_PULSE_LEN_MAX                       ((uint8_t)(F_INTERRUPTS * JVC_PULSE_TIME * MAX_TOLERANCE_40 + 0.5) + 1)
#define JVC_1_PAUSE_LEN_MIN                     ((uint8_t)(F_INTERRUPTS * JVC_1_PAUSE_TIME * MIN_TOLERANCE_40 + 0.5) - 1)
#define JVC_1_PAUSE_LEN_MAX                     ((uint8_t)(F_INTERRUPTS * JVC_1_PAUSE_TIME * MAX_TOLERANCE_40 + 0.5) + 1)
#define JVC_0_PAUSE_LEN_MIN                     ((uint8_t)(F_INTERRUPTS * JVC_0_PAUSE_TIME * MIN_TOLERANCE_40 + 0.5) - 1)
#define JVC_0_PAUSE_LEN_MAX                     ((uint8_t)(F_INTERRUPTS * JVC_0_PAUSE_TIME * MAX_TOLERANCE_40 + 0.5) + 1)
// autodetect JVC repetition frame within 50 msec:
#define JVC_FRAME_REPEAT_PAUSE_LEN_MAX          (uint16_t)(F_INTERRUPTS * JVC_FRAME_REPEAT_PAUSE_TIME * MAX_TOLERANCE_20 + 0.5)

#define AUTO_FRAME_REPETITION_LEN               (uint16_t)(F_INTERRUPTS * AUTO_FRAME_REPETITION_TIME + 0.5)       // use uint16_t!

#ifdef ANALYZE
#define ANALYZE_PUTCHAR(a)                        { if (! silent)             { putchar (a);          } }
#define ANALYZE_ONLY_NORMAL_PUTCHAR(a)            { if (! silent && !verbose) { putchar (a);          } }
#define ANALYZE_PRINTF(...)                       { if (verbose)              { printf (__VA_ARGS__); } }
#define ANALYZE_NEWLINE()                         { if (verbose)              { putchar ('\n');       } }
static int      silent;
static int      time_counter;
static int      verbose;
#else
#define ANALYZE_PUTCHAR(a)
#define ANALYZE_ONLY_NORMAL_PUTCHAR(a)
#define ANALYZE_PRINTF(...)
#define ANALYZE_NEWLINE()
#endif

#if IRMP_LOGGING == 1
#define BAUD                                    9600L
#include <util/setbaud.h>

#ifdef UBRR0H

#define UART0_UBRRH                             UBRR0H
#define UART0_UBRRL                             UBRR0L
#define UART0_UCSRA                             UCSR0A
#define UART0_UCSRB                             UCSR0B
#define UART0_UCSRC                             UCSR0C
#define UART0_UDRE_BIT_VALUE                    (1<<UDRE0)
#define UART0_UCSZ1_BIT_VALUE                   (1<<UCSZ01)
#define UART0_UCSZ0_BIT_VALUE                   (1<<UCSZ00)
#ifdef URSEL0
#define UART0_URSEL_BIT_VALUE                   (1<<URSEL0)
#else
#define UART0_URSEL_BIT_VALUE                   (0)
#endif
#define UART0_TXEN_BIT_VALUE                    (1<<TXEN0)
#define UART0_UDR                               UDR0
#define UART0_U2X                               U2X0

#else

#define UART0_UBRRH                             UBRRH
#define UART0_UBRRL                             UBRRL
#define UART0_UCSRA                             UCSRA
#define UART0_UCSRB                             UCSRB
#define UART0_UCSRC                             UCSRC
#define UART0_UDRE_BIT_VALUE                    (1<<UDRE)
#define UART0_UCSZ1_BIT_VALUE                   (1<<UCSZ1)
#define UART0_UCSZ0_BIT_VALUE                   (1<<UCSZ0)
#ifdef URSEL
#define UART0_URSEL_BIT_VALUE                   (1<<URSEL)
#else
#define UART0_URSEL_BIT_VALUE                   (0)
#endif
#define UART0_TXEN_BIT_VALUE                    (1<<TXEN)
#define UART0_UDR                               UDR
#define UART0_U2X                               U2X

#endif

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 *  Initialize  UART
 *  @details  Initializes UART
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
irmp_uart_init (void)
{
    UART0_UBRRH = UBRRH_VALUE;                                                                      // set baud rate
    UART0_UBRRL = UBRRL_VALUE;

#if USE_2X
    UART0_UCSRA |= (1<<UART0_U2X);
#else
    UART0_UCSRA &= ~(1<<UART0_U2X);
#endif

    UART0_UCSRC = UART0_UCSZ1_BIT_VALUE | UART0_UCSZ0_BIT_VALUE | UART0_URSEL_BIT_VALUE;
    UART0_UCSRB |= UART0_TXEN_BIT_VALUE;                                                            // enable UART TX
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 *  Send character
 *  @details  Sends character
 *  @param    ch character to be transmitted
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
irmp_uart_putc (unsigned char ch)
{
    while (!(UART0_UCSRA & UART0_UDRE_BIT_VALUE))
    {
        ;
    }

    UART0_UDR = ch;
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 *  Log IR signal
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */

#define STARTCYCLES                       2                                 // min count of zeros before start of logging
#define ENDBITS                        1000                                 // number of sequenced highbits to detect end
#define DATALEN                         700                                 // log buffer size

static void
irmp_log (uint8_t val)
{
    static uint8_t  buf[DATALEN];                                           // logging buffer
    static uint16_t buf_idx;                                                // number of written bits
    static uint8_t  startcycles;                                            // current number of start-zeros
    static uint16_t cnt;                                                    // counts sequenced highbits - to detect end

    if (! val && (startcycles < STARTCYCLES) && !buf_idx)                   // prevent that single random zeros init logging
    {
        startcycles++;
    }
    else
    {
        startcycles = 0;

        if (! val || (val && buf_idx != 0))                                 // start or continue logging on "0", "1" cannot init logging
        {
            if (buf_idx < DATALEN * 8)                                      // index in range?
            {                                                               // yes
                if (val)
                {
                    buf[(buf_idx / 8)] |=  (1<<(buf_idx % 8));              // set bit
                }
                else
                {
                    buf[(buf_idx / 8)] &= ~(1<<(buf_idx % 8));              // reset bit
                }

                buf_idx++;
            }

            if (val)
            {                                                               // if high received then look at log-stop condition
                cnt++;

                if (cnt > ENDBITS)
                {                                                           // if stop condition is true, output on uart
                    uint16_t i;

                    for (i = 0; i < STARTCYCLES; i++)
                    {
                        irmp_uart_putc ('0');                               // the ignored starting zeros
                    }

                    for (i = 0; i < (buf_idx - ENDBITS + 20) / 8; i++)      // transform bitset into uart chars
                    {
                        uint8_t d = buf[i];
                        uint8_t j;

                        for (j = 0; j < 8; j++)
                        {
                            irmp_uart_putc ((d & 1) + '0');
                            d >>= 1;
                        }
                    }

                    irmp_uart_putc ('\n');
                    buf_idx = 0;
                }
            }
            else
            {
                cnt = 0;
            }
        }
    }
}

#else
#define irmp_log(val)
#endif

typedef struct
{
    uint8_t    protocol;                                                // ir protocol
    uint8_t    pulse_1_len_min;                                         // minimum length of pulse with bit value 1
    uint8_t    pulse_1_len_max;                                         // maximum length of pulse with bit value 1
    uint8_t    pause_1_len_min;                                         // minimum length of pause with bit value 1
    uint8_t    pause_1_len_max;                                         // maximum length of pause with bit value 1
    uint8_t    pulse_0_len_min;                                         // minimum length of pulse with bit value 0
    uint8_t    pulse_0_len_max;                                         // maximum length of pulse with bit value 0
    uint8_t    pause_0_len_min;                                         // minimum length of pause with bit value 0
    uint8_t    pause_0_len_max;                                         // maximum length of pause with bit value 0
    uint8_t    address_offset;                                          // address offset
    uint8_t    address_end;                                             // end of address
    uint8_t    command_offset;                                          // command offset
    uint8_t    command_end;                                             // end of command
    uint8_t    complete_len;                                            // complete length of frame
    uint8_t    stop_bit;                                                // flag: frame has stop bit
    uint8_t    lsb_first;                                               // flag: LSB first
    uint8_t    flags;                                                   // some flags
} IRMP_PARAMETER;

#if IRMP_SUPPORT_SIRCS_PROTOCOL == 1

static PROGMEM IRMP_PARAMETER sircs_param =
{
    IRMP_SIRCS_PROTOCOL,                                                // protocol:        ir protocol
    SIRCS_1_PULSE_LEN_MIN,                                              // pulse_1_len_min: minimum length of pulse with bit value 1
    SIRCS_1_PULSE_LEN_MAX,                                              // pulse_1_len_max: maximum length of pulse with bit value 1
    SIRCS_PAUSE_LEN_MIN,                                                // pause_1_len_min: minimum length of pause with bit value 1
    SIRCS_PAUSE_LEN_MAX,                                                // pause_1_len_max: maximum length of pause with bit value 1
    SIRCS_0_PULSE_LEN_MIN,                                              // pulse_0_len_min: minimum length of pulse with bit value 0
    SIRCS_0_PULSE_LEN_MAX,                                              // pulse_0_len_max: maximum length of pulse with bit value 0
    SIRCS_PAUSE_LEN_MIN,                                                // pause_0_len_min: minimum length of pause with bit value 0
    SIRCS_PAUSE_LEN_MAX,                                                // pause_0_len_max: maximum length of pause with bit value 0
    SIRCS_ADDRESS_OFFSET,                                               // address_offset:  address offset
    SIRCS_ADDRESS_OFFSET + SIRCS_ADDRESS_LEN,                           // address_end:     end of address
    SIRCS_COMMAND_OFFSET,                                               // command_offset:  command offset
    SIRCS_COMMAND_OFFSET + SIRCS_COMMAND_LEN,                           // command_end:     end of command
    SIRCS_COMPLETE_DATA_LEN,                                            // complete_len:    complete length of frame
    SIRCS_STOP_BIT,                                                     // stop_bit:        flag: frame has stop bit
    SIRCS_LSB,                                                          // lsb_first:       flag: LSB first
    SIRCS_FLAGS                                                         // flags:           some flags
};

#endif

#if IRMP_SUPPORT_NEC_PROTOCOL == 1

static PROGMEM IRMP_PARAMETER nec_param =
{
    IRMP_NEC_PROTOCOL,                                                  // protocol:        ir protocol
    NEC_PULSE_LEN_MIN,                                                  // pulse_1_len_min: minimum length of pulse with bit value 1
    NEC_PULSE_LEN_MAX,                                                  // pulse_1_len_max: maximum length of pulse with bit value 1
    NEC_1_PAUSE_LEN_MIN,                                                // pause_1_len_min: minimum length of pause with bit value 1
    NEC_1_PAUSE_LEN_MAX,                                                // pause_1_len_max: maximum length of pause with bit value 1
    NEC_PULSE_LEN_MIN,                                                  // pulse_0_len_min: minimum length of pulse with bit value 0
    NEC_PULSE_LEN_MAX,                                                  // pulse_0_len_max: maximum length of pulse with bit value 0
    NEC_0_PAUSE_LEN_MIN,                                                // pause_0_len_min: minimum length of pause with bit value 0
    NEC_0_PAUSE_LEN_MAX,                                                // pause_0_len_max: maximum length of pause with bit value 0
    NEC_ADDRESS_OFFSET,                                                 // address_offset:  address offset
    NEC_ADDRESS_OFFSET + NEC_ADDRESS_LEN,                               // address_end:     end of address
    NEC_COMMAND_OFFSET,                                                 // command_offset:  command offset
    NEC_COMMAND_OFFSET + NEC_COMMAND_LEN,                               // command_end:     end of command
    NEC_COMPLETE_DATA_LEN,                                              // complete_len:    complete length of frame
    NEC_STOP_BIT,                                                       // stop_bit:        flag: frame has stop bit
    NEC_LSB,                                                            // lsb_first:       flag: LSB first
    NEC_FLAGS                                                           // flags:           some flags
};

static PROGMEM IRMP_PARAMETER nec_rep_param =
{
    IRMP_NEC_PROTOCOL,                                                  // protocol:        ir protocol
    NEC_PULSE_LEN_MIN,                                                  // pulse_1_len_min: minimum length of pulse with bit value 1
    NEC_PULSE_LEN_MAX,                                                  // pulse_1_len_max: maximum length of pulse with bit value 1
    NEC_1_PAUSE_LEN_MIN,                                                // pause_1_len_min: minimum length of pause with bit value 1
    NEC_1_PAUSE_LEN_MAX,                                                // pause_1_len_max: maximum length of pause with bit value 1
    NEC_PULSE_LEN_MIN,                                                  // pulse_0_len_min: minimum length of pulse with bit value 0
    NEC_PULSE_LEN_MAX,                                                  // pulse_0_len_max: maximum length of pulse with bit value 0
    NEC_0_PAUSE_LEN_MIN,                                                // pause_0_len_min: minimum length of pause with bit value 0
    NEC_0_PAUSE_LEN_MAX,                                                // pause_0_len_max: maximum length of pause with bit value 0
    0,                                                                  // address_offset:  address offset
    0,                                                                  // address_end:     end of address
    0,                                                                  // command_offset:  command offset
    0,                                                                  // command_end:     end of command
    0,                                                                  // complete_len:    complete length of frame
    NEC_STOP_BIT,                                                       // stop_bit:        flag: frame has stop bit
    NEC_LSB,                                                            // lsb_first:       flag: LSB first
    NEC_FLAGS                                                           // flags:           some flags
};

#endif

#if IRMP_SUPPORT_SAMSUNG_PROTOCOL == 1

static PROGMEM IRMP_PARAMETER samsung_param =
{
    IRMP_SAMSUNG_PROTOCOL,                                              // protocol:        ir protocol
    SAMSUNG_PULSE_LEN_MIN,                                              // pulse_1_len_min: minimum length of pulse with bit value 1
    SAMSUNG_PULSE_LEN_MAX,                                              // pulse_1_len_max: maximum length of pulse with bit value 1
    SAMSUNG_1_PAUSE_LEN_MIN,                                            // pause_1_len_min: minimum length of pause with bit value 1
    SAMSUNG_1_PAUSE_LEN_MAX,                                            // pause_1_len_max: maximum length of pause with bit value 1
    SAMSUNG_PULSE_LEN_MIN,                                              // pulse_0_len_min: minimum length of pulse with bit value 0
    SAMSUNG_PULSE_LEN_MAX,                                              // pulse_0_len_max: maximum length of pulse with bit value 0
    SAMSUNG_0_PAUSE_LEN_MIN,                                            // pause_0_len_min: minimum length of pause with bit value 0
    SAMSUNG_0_PAUSE_LEN_MAX,                                            // pause_0_len_max: maximum length of pause with bit value 0
    SAMSUNG_ADDRESS_OFFSET,                                             // address_offset:  address offset
    SAMSUNG_ADDRESS_OFFSET + SAMSUNG_ADDRESS_LEN,                       // address_end:     end of address
    SAMSUNG_COMMAND_OFFSET,                                             // command_offset:  command offset
    SAMSUNG_COMMAND_OFFSET + SAMSUNG_COMMAND_LEN,                       // command_end:     end of command
    SAMSUNG_COMPLETE_DATA_LEN,                                          // complete_len:    complete length of frame
    SAMSUNG_STOP_BIT,                                                   // stop_bit:        flag: frame has stop bit
    SAMSUNG_LSB,                                                        // lsb_first:       flag: LSB first
    SAMSUNG_FLAGS                                                       // flags:           some flags
};

#endif

#if IRMP_SUPPORT_MATSUSHITA_PROTOCOL == 1

static PROGMEM IRMP_PARAMETER matsushita_param =
{
    IRMP_MATSUSHITA_PROTOCOL,                                           // protocol:        ir protocol
    MATSUSHITA_PULSE_LEN_MIN,                                           // pulse_1_len_min: minimum length of pulse with bit value 1
    MATSUSHITA_PULSE_LEN_MAX,                                           // pulse_1_len_max: maximum length of pulse with bit value 1
    MATSUSHITA_1_PAUSE_LEN_MIN,                                         // pause_1_len_min: minimum length of pause with bit value 1
    MATSUSHITA_1_PAUSE_LEN_MAX,                                         // pause_1_len_max: maximum length of pause with bit value 1
    MATSUSHITA_PULSE_LEN_MIN,                                           // pulse_0_len_min: minimum length of pulse with bit value 0
    MATSUSHITA_PULSE_LEN_MAX,                                           // pulse_0_len_max: maximum length of pulse with bit value 0
    MATSUSHITA_0_PAUSE_LEN_MIN,                                         // pause_0_len_min: minimum length of pause with bit value 0
    MATSUSHITA_0_PAUSE_LEN_MAX,                                         // pause_0_len_max: maximum length of pause with bit value 0
    MATSUSHITA_ADDRESS_OFFSET,                                          // address_offset:  address offset
    MATSUSHITA_ADDRESS_OFFSET + MATSUSHITA_ADDRESS_LEN,                 // address_end:     end of address
    MATSUSHITA_COMMAND_OFFSET,                                          // command_offset:  command offset
    MATSUSHITA_COMMAND_OFFSET + MATSUSHITA_COMMAND_LEN,                 // command_end:     end of command
    MATSUSHITA_COMPLETE_DATA_LEN,                                       // complete_len:    complete length of frame
    MATSUSHITA_STOP_BIT,                                                // stop_bit:        flag: frame has stop bit
    MATSUSHITA_LSB,                                                     // lsb_first:       flag: LSB first
    MATSUSHITA_FLAGS                                                    // flags:           some flags
};

#endif

#if IRMP_SUPPORT_KASEIKYO_PROTOCOL == 1

static PROGMEM IRMP_PARAMETER kaseikyo_param =
{
    IRMP_KASEIKYO_PROTOCOL,                                             // protocol:        ir protocol
    KASEIKYO_PULSE_LEN_MIN,                                             // pulse_1_len_min: minimum length of pulse with bit value 1
    KASEIKYO_PULSE_LEN_MAX,                                             // pulse_1_len_max: maximum length of pulse with bit value 1
    KASEIKYO_1_PAUSE_LEN_MIN,                                           // pause_1_len_min: minimum length of pause with bit value 1
    KASEIKYO_1_PAUSE_LEN_MAX,                                           // pause_1_len_max: maximum length of pause with bit value 1
    KASEIKYO_PULSE_LEN_MIN,                                             // pulse_0_len_min: minimum length of pulse with bit value 0
    KASEIKYO_PULSE_LEN_MAX,                                             // pulse_0_len_max: maximum length of pulse with bit value 0
    KASEIKYO_0_PAUSE_LEN_MIN,                                           // pause_0_len_min: minimum length of pause with bit value 0
    KASEIKYO_0_PAUSE_LEN_MAX,                                           // pause_0_len_max: maximum length of pause with bit value 0
    KASEIKYO_ADDRESS_OFFSET,                                            // address_offset:  address offset
    KASEIKYO_ADDRESS_OFFSET + KASEIKYO_ADDRESS_LEN,                     // address_end:     end of address
    KASEIKYO_COMMAND_OFFSET,                                            // command_offset:  command offset
    KASEIKYO_COMMAND_OFFSET + KASEIKYO_COMMAND_LEN,                     // command_end:     end of command
    KASEIKYO_COMPLETE_DATA_LEN,                                         // complete_len:    complete length of frame
    KASEIKYO_STOP_BIT,                                                  // stop_bit:        flag: frame has stop bit
    KASEIKYO_LSB,                                                       // lsb_first:       flag: LSB first
    KASEIKYO_FLAGS                                                      // flags:           some flags
};

#endif

#if IRMP_SUPPORT_RECS80_PROTOCOL == 1

static PROGMEM IRMP_PARAMETER recs80_param =
{
    IRMP_RECS80_PROTOCOL,                                               // protocol:        ir protocol
    RECS80_PULSE_LEN_MIN,                                               // pulse_1_len_min: minimum length of pulse with bit value 1
    RECS80_PULSE_LEN_MAX,                                               // pulse_1_len_max: maximum length of pulse with bit value 1
    RECS80_1_PAUSE_LEN_MIN,                                             // pause_1_len_min: minimum length of pause with bit value 1
    RECS80_1_PAUSE_LEN_MAX,                                             // pause_1_len_max: maximum length of pause with bit value 1
    RECS80_PULSE_LEN_MIN,                                               // pulse_0_len_min: minimum length of pulse with bit value 0
    RECS80_PULSE_LEN_MAX,                                               // pulse_0_len_max: maximum length of pulse with bit value 0
    RECS80_0_PAUSE_LEN_MIN,                                             // pause_0_len_min: minimum length of pause with bit value 0
    RECS80_0_PAUSE_LEN_MAX,                                             // pause_0_len_max: maximum length of pause with bit value 0
    RECS80_ADDRESS_OFFSET,                                              // address_offset:  address offset
    RECS80_ADDRESS_OFFSET + RECS80_ADDRESS_LEN,                         // address_end:     end of address
    RECS80_COMMAND_OFFSET,                                              // command_offset:  command offset
    RECS80_COMMAND_OFFSET + RECS80_COMMAND_LEN,                         // command_end:     end of command
    RECS80_COMPLETE_DATA_LEN,                                           // complete_len:    complete length of frame
    RECS80_STOP_BIT,                                                    // stop_bit:        flag: frame has stop bit
    RECS80_LSB,                                                         // lsb_first:       flag: LSB first
    RECS80_FLAGS                                                        // flags:           some flags
};

#endif

#if IRMP_SUPPORT_RC5_PROTOCOL == 1

static PROGMEM IRMP_PARAMETER rc5_param =
{
    IRMP_RC5_PROTOCOL,                                                  // protocol:        ir protocol
    RC5_BIT_LEN_MIN,                                                    // pulse_1_len_min: minimum length of pulse with bit value 1
    RC5_BIT_LEN_MAX,                                                    // pulse_1_len_max: maximum length of pulse with bit value 1
    RC5_BIT_LEN_MIN,                                                    // pause_1_len_min: minimum length of pause with bit value 1
    RC5_BIT_LEN_MAX,                                                    // pause_1_len_max: maximum length of pause with bit value 1
    1,  // tricky: use this as stop bit length                          // pulse_0_len_min: minimum length of pulse with bit value 0
    1,                                                                  // pulse_0_len_max: maximum length of pulse with bit value 0
    1,                                                                  // pause_0_len_min: minimum length of pause with bit value 0
    1,                                                                  // pause_0_len_max: maximum length of pause with bit value 0
    RC5_ADDRESS_OFFSET,                                                 // address_offset:  address offset
    RC5_ADDRESS_OFFSET + RC5_ADDRESS_LEN,                               // address_end:     end of address
    RC5_COMMAND_OFFSET,                                                 // command_offset:  command offset
    RC5_COMMAND_OFFSET + RC5_COMMAND_LEN,                               // command_end:     end of command
    RC5_COMPLETE_DATA_LEN,                                              // complete_len:    complete length of frame
    RC5_STOP_BIT,                                                       // stop_bit:        flag: frame has stop bit
    RC5_LSB,                                                            // lsb_first:       flag: LSB first
    RC5_FLAGS                                                           // flags:           some flags
};

#endif

#if IRMP_SUPPORT_DENON_PROTOCOL == 1

static PROGMEM IRMP_PARAMETER denon_param =
{
    IRMP_DENON_PROTOCOL,                                                // protocol:        ir protocol
    DENON_PULSE_LEN_MIN,                                                // pulse_1_len_min: minimum length of pulse with bit value 1
    DENON_PULSE_LEN_MAX,                                                // pulse_1_len_max: maximum length of pulse with bit value 1
    DENON_1_PAUSE_LEN_MIN,                                              // pause_1_len_min: minimum length of pause with bit value 1
    DENON_1_PAUSE_LEN_MAX,                                              // pause_1_len_max: maximum length of pause with bit value 1
    DENON_PULSE_LEN_MIN,                                                // pulse_0_len_min: minimum length of pulse with bit value 0
    DENON_PULSE_LEN_MAX,                                                // pulse_0_len_max: maximum length of pulse with bit value 0
    DENON_0_PAUSE_LEN_MIN,                                              // pause_0_len_min: minimum length of pause with bit value 0
    DENON_0_PAUSE_LEN_MAX,                                              // pause_0_len_max: maximum length of pause with bit value 0
    DENON_ADDRESS_OFFSET,                                               // address_offset:  address offset
    DENON_ADDRESS_OFFSET + DENON_ADDRESS_LEN,                           // address_end:     end of address
    DENON_COMMAND_OFFSET,                                               // command_offset:  command offset
    DENON_COMMAND_OFFSET + DENON_COMMAND_LEN,                           // command_end:     end of command
    DENON_COMPLETE_DATA_LEN,                                            // complete_len:    complete length of frame
    DENON_STOP_BIT,                                                     // stop_bit:        flag: frame has stop bit
    DENON_LSB,                                                          // lsb_first:       flag: LSB first
    DENON_FLAGS                                                         // flags:           some flags
};

#endif

#if IRMP_SUPPORT_RC6_PROTOCOL == 1

static PROGMEM IRMP_PARAMETER rc6_param =
{
    IRMP_RC6_PROTOCOL,                                                  // protocol:        ir protocol
    RC6_BIT_PULSE_LEN_MIN,                                              // pulse_1_len_min: minimum length of pulse with bit value 1
    RC6_BIT_PULSE_LEN_MAX,                                              // pulse_1_len_max: maximum length of pulse with bit value 1
    RC6_BIT_PAUSE_LEN_MIN,                                              // pause_1_len_min: minimum length of pause with bit value 1
    RC6_BIT_PAUSE_LEN_MAX,                                              // pause_1_len_max: maximum length of pause with bit value 1
    1,  // tricky: use this as stop bit length                          // pulse_0_len_min: minimum length of pulse with bit value 0
    1,                                                                  // pulse_0_len_max: maximum length of pulse with bit value 0
    1,                                                                  // pause_0_len_min: minimum length of pause with bit value 0
    1,                                                                  // pause_0_len_max: maximum length of pause with bit value 0
    RC6_ADDRESS_OFFSET,                                                 // address_offset:  address offset
    RC6_ADDRESS_OFFSET + RC6_ADDRESS_LEN,                               // address_end:     end of address
    RC6_COMMAND_OFFSET,                                                 // command_offset:  command offset
    RC6_COMMAND_OFFSET + RC6_COMMAND_LEN,                               // command_end:     end of command
    RC6_COMPLETE_DATA_LEN_SHORT,                                        // complete_len:    complete length of frame
    RC6_STOP_BIT,                                                       // stop_bit:        flag: frame has stop bit
    RC6_LSB,                                                            // lsb_first:       flag: LSB first
    RC6_FLAGS                                                           // flags:           some flags
};

#endif

#if IRMP_SUPPORT_RECS80EXT_PROTOCOL == 1

static PROGMEM IRMP_PARAMETER recs80ext_param =
{
    IRMP_RECS80EXT_PROTOCOL,                                            // protocol:        ir protocol
    RECS80EXT_PULSE_LEN_MIN,                                            // pulse_1_len_min: minimum length of pulse with bit value 1
    RECS80EXT_PULSE_LEN_MAX,                                            // pulse_1_len_max: maximum length of pulse with bit value 1
    RECS80EXT_1_PAUSE_LEN_MIN,                                          // pause_1_len_min: minimum length of pause with bit value 1
    RECS80EXT_1_PAUSE_LEN_MAX,                                          // pause_1_len_max: maximum length of pause with bit value 1
    RECS80EXT_PULSE_LEN_MIN,                                            // pulse_0_len_min: minimum length of pulse with bit value 0
    RECS80EXT_PULSE_LEN_MAX,                                            // pulse_0_len_max: maximum length of pulse with bit value 0
    RECS80EXT_0_PAUSE_LEN_MIN,                                          // pause_0_len_min: minimum length of pause with bit value 0
    RECS80EXT_0_PAUSE_LEN_MAX,                                          // pause_0_len_max: maximum length of pause with bit value 0
    RECS80EXT_ADDRESS_OFFSET,                                           // address_offset:  address offset
    RECS80EXT_ADDRESS_OFFSET + RECS80EXT_ADDRESS_LEN,                   // address_end:     end of address
    RECS80EXT_COMMAND_OFFSET,                                           // command_offset:  command offset
    RECS80EXT_COMMAND_OFFSET + RECS80EXT_COMMAND_LEN,                   // command_end:     end of command
    RECS80EXT_COMPLETE_DATA_LEN,                                        // complete_len:    complete length of frame
    RECS80EXT_STOP_BIT,                                                 // stop_bit:        flag: frame has stop bit
    RECS80EXT_LSB,                                                      // lsb_first:       flag: LSB first
    RECS80EXT_FLAGS                                                     // flags:           some flags
};

#endif

#if IRMP_SUPPORT_NUBERT_PROTOCOL == 1

static PROGMEM IRMP_PARAMETER nubert_param =
{
    IRMP_NUBERT_PROTOCOL,                                               // protocol:        ir protocol
    NUBERT_1_PULSE_LEN_MIN,                                             // pulse_1_len_min: minimum length of pulse with bit value 1
    NUBERT_1_PULSE_LEN_MAX,                                             // pulse_1_len_max: maximum length of pulse with bit value 1
    NUBERT_1_PAUSE_LEN_MIN,                                             // pause_1_len_min: minimum length of pause with bit value 1
    NUBERT_1_PAUSE_LEN_MAX,                                             // pause_1_len_max: maximum length of pause with bit value 1
    NUBERT_0_PULSE_LEN_MIN,                                             // pulse_0_len_min: minimum length of pulse with bit value 0
    NUBERT_0_PULSE_LEN_MAX,                                             // pulse_0_len_max: maximum length of pulse with bit value 0
    NUBERT_0_PAUSE_LEN_MIN,                                             // pause_0_len_min: minimum length of pause with bit value 0
    NUBERT_0_PAUSE_LEN_MAX,                                             // pause_0_len_max: maximum length of pause with bit value 0
    NUBERT_ADDRESS_OFFSET,                                              // address_offset:  address offset
    NUBERT_ADDRESS_OFFSET + NUBERT_ADDRESS_LEN,                         // address_end:     end of address
    NUBERT_COMMAND_OFFSET,                                              // command_offset:  command offset
    NUBERT_COMMAND_OFFSET + NUBERT_COMMAND_LEN,                         // command_end:     end of command
    NUBERT_COMPLETE_DATA_LEN,                                           // complete_len:    complete length of frame
    NUBERT_STOP_BIT,                                                    // stop_bit:        flag: frame has stop bit
    NUBERT_LSB,                                                         // lsb_first:       flag: LSB first
    NUBERT_FLAGS                                                        // flags:           some flags
};

#endif

#if IRMP_SUPPORT_BANG_OLUFSEN_PROTOCOL == 1

static PROGMEM IRMP_PARAMETER bang_olufsen_param =
{
    IRMP_BANG_OLUFSEN_PROTOCOL,                                         // protocol:        ir protocol
    BANG_OLUFSEN_PULSE_LEN_MIN,                                         // pulse_1_len_min: minimum length of pulse with bit value 1
    BANG_OLUFSEN_PULSE_LEN_MAX,                                         // pulse_1_len_max: maximum length of pulse with bit value 1
    BANG_OLUFSEN_1_PAUSE_LEN_MIN,                                       // pause_1_len_min: minimum length of pause with bit value 1
    BANG_OLUFSEN_1_PAUSE_LEN_MAX,                                       // pause_1_len_max: maximum length of pause with bit value 1
    BANG_OLUFSEN_PULSE_LEN_MIN,                                         // pulse_0_len_min: minimum length of pulse with bit value 0
    BANG_OLUFSEN_PULSE_LEN_MAX,                                         // pulse_0_len_max: maximum length of pulse with bit value 0
    BANG_OLUFSEN_0_PAUSE_LEN_MIN,                                       // pause_0_len_min: minimum length of pause with bit value 0
    BANG_OLUFSEN_0_PAUSE_LEN_MAX,                                       // pause_0_len_max: maximum length of pause with bit value 0
    BANG_OLUFSEN_ADDRESS_OFFSET,                                        // address_offset:  address offset
    BANG_OLUFSEN_ADDRESS_OFFSET + BANG_OLUFSEN_ADDRESS_LEN,             // address_end:     end of address
    BANG_OLUFSEN_COMMAND_OFFSET,                                        // command_offset:  command offset
    BANG_OLUFSEN_COMMAND_OFFSET + BANG_OLUFSEN_COMMAND_LEN,             // command_end:     end of command
    BANG_OLUFSEN_COMPLETE_DATA_LEN,                                     // complete_len:    complete length of frame
    BANG_OLUFSEN_STOP_BIT,                                              // stop_bit:        flag: frame has stop bit
    BANG_OLUFSEN_LSB,                                                   // lsb_first:       flag: LSB first
    BANG_OLUFSEN_FLAGS                                                  // flags:           some flags
};

#endif

#if IRMP_SUPPORT_GRUNDIG_OR_NOKIA_PROTOCOL == 1

static PROGMEM IRMP_PARAMETER grundig_param =
{
    IRMP_GRUNDIG_PROTOCOL,                                              // protocol:        ir protocol
    GRUNDIG_OR_NOKIA_BIT_LEN_MIN,                                       // pulse_1_len_min: minimum length of pulse with bit value 1
    GRUNDIG_OR_NOKIA_BIT_LEN_MAX,                                       // pulse_1_len_max: maximum length of pulse with bit value 1
    GRUNDIG_OR_NOKIA_BIT_LEN_MIN,                                       // pause_1_len_min: minimum length of pause with bit value 1
    GRUNDIG_OR_NOKIA_BIT_LEN_MAX,                                       // pause_1_len_max: maximum length of pause with bit value 1
    1,  // tricky: use this as stop bit length                          // pulse_0_len_min: minimum length of pulse with bit value 0
    1,                                                                  // pulse_0_len_max: maximum length of pulse with bit value 0
    1,                                                                  // pause_0_len_min: minimum length of pause with bit value 0
    1,                                                                  // pause_0_len_max: maximum length of pause with bit value 0
    GRUNDIG_ADDRESS_OFFSET,                                             // address_offset:  address offset
    GRUNDIG_ADDRESS_OFFSET + GRUNDIG_ADDRESS_LEN,                       // address_end:     end of address
    GRUNDIG_COMMAND_OFFSET,                                             // command_offset:  command offset
    GRUNDIG_COMMAND_OFFSET + GRUNDIG_COMMAND_LEN + 1,                   // command_end:     end of command (USE 1 bit MORE to STORE NOKIA DATA!)
    NOKIA_COMPLETE_DATA_LEN,                                            // complete_len:    complete length of frame, here: NOKIA instead of GRUNDIG!
    GRUNDIG_OR_NOKIA_STOP_BIT,                                          // stop_bit:        flag: frame has stop bit
    GRUNDIG_OR_NOKIA_LSB,                                               // lsb_first:       flag: LSB first
    GRUNDIG_OR_NOKIA_FLAGS                                              // flags:           some flags
};

#endif

#if IRMP_SUPPORT_SIEMENS_PROTOCOL == 1

static PROGMEM IRMP_PARAMETER siemens_param =
{
    IRMP_SIEMENS_PROTOCOL,                                              // protocol:        ir protocol
    SIEMENS_BIT_LEN_MIN,                                                // pulse_1_len_min: minimum length of pulse with bit value 1
    SIEMENS_BIT_LEN_MAX,                                                // pulse_1_len_max: maximum length of pulse with bit value 1
    SIEMENS_BIT_LEN_MIN,                                                // pause_1_len_min: minimum length of pause with bit value 1
    SIEMENS_BIT_LEN_MAX,                                                // pause_1_len_max: maximum length of pause with bit value 1
    1,  // tricky: use this as stop bit length                          // pulse_0_len_min: minimum length of pulse with bit value 0
    1,                                                                  // pulse_0_len_max: maximum length of pulse with bit value 0
    1,                                                                  // pause_0_len_min: minimum length of pause with bit value 0
    1,                                                                  // pause_0_len_max: maximum length of pause with bit value 0
    SIEMENS_ADDRESS_OFFSET,                                             // address_offset:  address offset
    SIEMENS_ADDRESS_OFFSET + SIEMENS_ADDRESS_LEN,                       // address_end:     end of address
    SIEMENS_COMMAND_OFFSET,                                             // command_offset:  command offset
    SIEMENS_COMMAND_OFFSET + SIEMENS_COMMAND_LEN,                       // command_end:     end of command
    SIEMENS_COMPLETE_DATA_LEN,                                          // complete_len:    complete length of frame
    SIEMENS_STOP_BIT,                                                   // stop_bit:        flag: frame has stop bit
    SIEMENS_LSB,                                                        // lsb_first:       flag: LSB first
    SIEMENS_FLAGS                                                       // flags:           some flags
};

#endif

#if IRMP_SUPPORT_FDC_PROTOCOL == 1

static PROGMEM IRMP_PARAMETER fdc_param =
{
    IRMP_FDC_PROTOCOL,                                                  // protocol:        ir protocol
    FDC_PULSE_LEN_MIN,                                                  // pulse_1_len_min: minimum length of pulse with bit value 1
    FDC_PULSE_LEN_MAX,                                                  // pulse_1_len_max: maximum length of pulse with bit value 1
    FDC_1_PAUSE_LEN_MIN,                                                // pause_1_len_min: minimum length of pause with bit value 1
    FDC_1_PAUSE_LEN_MAX,                                                // pause_1_len_max: maximum length of pause with bit value 1
    FDC_PULSE_LEN_MIN,                                                  // pulse_0_len_min: minimum length of pulse with bit value 0
    FDC_PULSE_LEN_MAX,                                                  // pulse_0_len_max: maximum length of pulse with bit value 0
    FDC_0_PAUSE_LEN_MIN,                                                // pause_0_len_min: minimum length of pause with bit value 0
    FDC_0_PAUSE_LEN_MAX,                                                // pause_0_len_max: maximum length of pause with bit value 0
    FDC_ADDRESS_OFFSET,                                                 // address_offset:  address offset
    FDC_ADDRESS_OFFSET + FDC_ADDRESS_LEN,                               // address_end:     end of address
    FDC_COMMAND_OFFSET,                                                 // command_offset:  command offset
    FDC_COMMAND_OFFSET + FDC_COMMAND_LEN,                               // command_end:     end of command
    FDC_COMPLETE_DATA_LEN,                                              // complete_len:    complete length of frame
    FDC_STOP_BIT,                                                       // stop_bit:        flag: frame has stop bit
    FDC_LSB,                                                            // lsb_first:       flag: LSB first
    FDC_FLAGS                                                           // flags:           some flags
};

#endif

#if IRMP_SUPPORT_RCCAR_PROTOCOL == 1

static PROGMEM IRMP_PARAMETER rccar_param =
{
    IRMP_RCCAR_PROTOCOL,                                                // protocol:        ir protocol
    RCCAR_PULSE_LEN_MIN,                                                // pulse_1_len_min: minimum length of pulse with bit value 1
    RCCAR_PULSE_LEN_MAX,                                                // pulse_1_len_max: maximum length of pulse with bit value 1
    RCCAR_1_PAUSE_LEN_MIN,                                              // pause_1_len_min: minimum length of pause with bit value 1
    RCCAR_1_PAUSE_LEN_MAX,                                              // pause_1_len_max: maximum length of pause with bit value 1
    RCCAR_PULSE_LEN_MIN,                                                // pulse_0_len_min: minimum length of pulse with bit value 0
    RCCAR_PULSE_LEN_MAX,                                                // pulse_0_len_max: maximum length of pulse with bit value 0
    RCCAR_0_PAUSE_LEN_MIN,                                              // pause_0_len_min: minimum length of pause with bit value 0
    RCCAR_0_PAUSE_LEN_MAX,                                              // pause_0_len_max: maximum length of pause with bit value 0
    RCCAR_ADDRESS_OFFSET,                                               // address_offset:  address offset
    RCCAR_ADDRESS_OFFSET + RCCAR_ADDRESS_LEN,                           // address_end:     end of address
    RCCAR_COMMAND_OFFSET,                                               // command_offset:  command offset
    RCCAR_COMMAND_OFFSET + RCCAR_COMMAND_LEN,                           // command_end:     end of command
    RCCAR_COMPLETE_DATA_LEN,                                            // complete_len:    complete length of frame
    RCCAR_STOP_BIT,                                                     // stop_bit:        flag: frame has stop bit
    RCCAR_LSB,                                                          // lsb_first:       flag: LSB first
    RCCAR_FLAGS                                                         // flags:           some flags
};

#endif

static uint8_t                              irmp_bit;                   // current bit position
static IRMP_PARAMETER                       irmp_param;

#if IRMP_SUPPORT_RC5_PROTOCOL == 1 && (IRMP_SUPPORT_FDC_PROTOCOL == 1 || IRMP_SUPPORT_RCCAR_PROTOCOL == 1)
static IRMP_PARAMETER                       irmp_param2;
#endif

static volatile uint8_t                     irmp_ir_detected;
static volatile uint8_t                     irmp_protocol;
static volatile uint16_t                    irmp_address;
static volatile uint16_t                    irmp_command;
static volatile uint16_t                    irmp_id;                    // only used for SAMSUNG protocol
static volatile uint8_t                     irmp_flags;

#ifdef ANALYZE
static uint8_t                              IRMP_PIN;
#endif

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 *  Initialize IRMP decoder
 *  @details  Configures IRMP input pin
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#if 0
#ifndef ANALYZE
void
_irmp_init (void)
{
#ifndef PIC_CCS_COMPILER
    IRMP_PORT &= ~(1<<IRMP_BIT);                                        // deactivate pullup
    IRMP_DDR &= ~(1<<IRMP_BIT);                                         // set pin to input
#endif // PIC_CCS_COMPILER

#if IRMP_LOGGING == 1
    irmp_uart_init ();
#endif
}
#endif
#endif
/*---------------------------------------------------------------------------------------------------------------------------------------------------
 *  Get IRMP data
 *  @details  gets decoded IRMP data
 *  @param    pointer in order to store IRMP data
 *  @return    TRUE: successful, FALSE: failed
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
uint8_t
irmp_get_data (IRMP_DATA * irmp_data_p)
{
    uint8_t   rtc = FALSE;

    if (irmp_ir_detected)
    {
        switch (irmp_protocol)
        {
#if IRMP_SUPPORT_SAMSUNG_PROTOCOL == 1
            case IRMP_SAMSUNG_PROTOCOL:
                if ((irmp_command >> 8) == (~irmp_command & 0x00FF))
                {
                    irmp_command &= 0xff;
                    irmp_command |= irmp_id << 8;
                    rtc = TRUE;
                }
                break;
#endif
#if IRMP_SUPPORT_NEC_PROTOCOL == 1
            case IRMP_NEC_PROTOCOL:
                if ((irmp_command >> 8) == (~irmp_command & 0x00FF))
                {
                    irmp_command &= 0xff;
                    rtc = TRUE;
                }
                else if (irmp_address == 0x87EE)
                {
                    ANALYZE_PRINTF ("Switching to APPLE protocol\n");
                    irmp_protocol = IRMP_APPLE_PROTOCOL;
                    irmp_address = (irmp_command & 0xFF00) >> 8;
                    irmp_command &= 0x00FF;
                    rtc = TRUE;
                }
                break;
#endif
#if IRMP_SUPPORT_RCCAR_PROTOCOL == 1
            case IRMP_RCCAR_PROTOCOL:
                // frame in irmp_data:
                // Bit 12 11 10 9  8  7  6  5  4  3  2  1  0
                //     V  D7 D6 D5 D4 D3 D2 D1 D0 A1 A0 C1 C0   //         10 9  8  7  6  5  4  3  2  1  0
                irmp_address = (irmp_command & 0x000C) >> 2;    // addr:   0  0  0  0  0  0  0  0  0  A1 A0
                irmp_command = ((irmp_command & 0x1000) >> 2) | // V-Bit:  V  0  0  0  0  0  0  0  0  0  0
                               ((irmp_command & 0x0003) << 8) | // C-Bits: 0  C1 C0 0  0  0  0  0  0  0  0
                               ((irmp_command & 0x0FF0) >> 4);  // D-Bits:          D7 D6 D5 D4 D3 D2 D1 D0
                rtc = TRUE;                                     // Summe:  V  C1 C0 D7 D6 D5 D4 D3 D2 D1 D0
                break;
#endif
            default:
                rtc = TRUE;
        }

        if (rtc)
        {
            irmp_data_p->protocol = irmp_protocol;
            irmp_data_p->address = irmp_address;
            irmp_data_p->command = irmp_command;
            irmp_data_p->flags   = irmp_flags;
            irmp_command = 0;
            irmp_address = 0;
            irmp_flags   = 0;
        }

        irmp_ir_detected = FALSE;
    }

    return rtc;
}

// these statics must not be volatile, because they are only used by irmp_store_bit(), which is called by irmp_ISR()
static uint16_t irmp_tmp_address;                                                       // ir address
static uint16_t irmp_tmp_command;                                                       // ir command

#if IRMP_SUPPORT_RC5_PROTOCOL == 1 && (IRMP_SUPPORT_FDC_PROTOCOL == 1 || IRMP_SUPPORT_RCCAR_PROTOCOL == 1)
static uint16_t irmp_tmp_address2;                                                      // ir address
static uint16_t irmp_tmp_command2;                                                      // ir command
#endif

#if IRMP_SUPPORT_SAMSUNG_PROTOCOL == 1
static uint16_t irmp_tmp_id;                                                            // ir id (only SAMSUNG)
#endif
#if IRMP_SUPPORT_KASEIKYO_PROTOCOL == 1
static uint8_t  xor_check[6];                                                           // check kaseikyo "parity" bits
#endif

static uint8_t  irmp_bit;                                                               // current bit position

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 *  store bit
 *  @details  store bit in temp address or temp command
 *  @param    value to store: 0 or 1
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
// verhindert, dass irmp_store_bit() inline compiliert wird:
// static void irmp_store_bit (uint8_t) __attribute__ ((noinline));

static void
irmp_store_bit (uint8_t value)
{

    if (irmp_bit >= irmp_param.address_offset && irmp_bit < irmp_param.address_end)
    {
        if (irmp_param.lsb_first)
        {
            irmp_tmp_address |= (((uint16_t) (value)) << (irmp_bit - irmp_param.address_offset));   // CV wants cast
        }
        else
        {
            irmp_tmp_address <<= 1;
            irmp_tmp_address |= value;
        }
    }
    else if (irmp_bit >= irmp_param.command_offset && irmp_bit < irmp_param.command_end)
    {
        if (irmp_param.lsb_first)
        {
            irmp_tmp_command |= (((uint16_t) (value)) << (irmp_bit - irmp_param.command_offset));   // CV wants cast
        }
        else
        {
            irmp_tmp_command <<= 1;
            irmp_tmp_command |= value;
        }
    }

#if IRMP_SUPPORT_SAMSUNG_PROTOCOL == 1
    else if (irmp_param.protocol == IRMP_SAMSUNG_PROTOCOL && irmp_bit >= SAMSUNG_ID_OFFSET && irmp_bit < SAMSUNG_ID_OFFSET + SAMSUNG_ID_LEN)
    {
        irmp_tmp_id |= (((uint16_t) (value)) << (irmp_bit - SAMSUNG_ID_OFFSET));                    // store with LSB first
    }
#endif

#if IRMP_SUPPORT_KASEIKYO_PROTOCOL == 1
    else if (irmp_param.protocol == IRMP_KASEIKYO_PROTOCOL && irmp_bit >= 20 && irmp_bit < 24)
    {
        irmp_tmp_command |= (((uint16_t) (value)) << (irmp_bit - 8));                   // store 4 system bits in upper nibble with LSB first
    }

    if (irmp_param.protocol == IRMP_KASEIKYO_PROTOCOL && irmp_bit < KASEIKYO_COMPLETE_DATA_LEN)
    {
        if (value)
        {
            xor_check[irmp_bit / 8] |= 1 << (irmp_bit % 8);
        }
        else
        {
            xor_check[irmp_bit / 8] &= ~(1 << (irmp_bit % 8));
        }
    }

#endif

    irmp_bit++;
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 *  store bit
 *  @details  store bit in temp address or temp command
 *  @param    value to store: 0 or 1
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#if IRMP_SUPPORT_RC5_PROTOCOL == 1 && (IRMP_SUPPORT_FDC_PROTOCOL == 1 || IRMP_SUPPORT_RCCAR_PROTOCOL == 1)
static void
irmp_store_bit2 (uint8_t value)
{
    uint8_t irmp_bit2;

    if (irmp_param.protocol)
    {
        irmp_bit2 = irmp_bit - 2;
    }
    else
    {
        irmp_bit2 = irmp_bit - 1;
    }

    if (irmp_bit2 >= irmp_param2.address_offset && irmp_bit2 < irmp_param2.address_end)
    {
        irmp_tmp_address2 |= (((uint16_t) (value)) << (irmp_bit2 - irmp_param2.address_offset));   // CV wants cast
    }
    else if (irmp_bit2 >= irmp_param2.command_offset && irmp_bit2 < irmp_param2.command_end)
    {
        irmp_tmp_command2 |= (((uint16_t) (value)) << (irmp_bit2 - irmp_param2.command_offset));   // CV wants cast
    }
}
#endif // IRMP_SUPPORT_RC5_PROTOCOL == 1 && (IRMP_SUPPORT_FDC_PROTOCOL == 1 || IRMP_SUPPORT_RCCAR_PROTOCOL == 1)

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 *  ISR routine
 *  @details  ISR routine, called 10000 times per second
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
uint8_t
#ifdef __AVR__
irmp_ISR (const uint8_t irmp_input)
#else
irmp_ISR (void)
#endif
{
    static uint8_t      irmp_start_bit_detected;                                // flag: start bit detected
    static uint8_t      wait_for_space;                                         // flag: wait for data bit space
    static uint8_t      wait_for_start_space;                                   // flag: wait for start bit space
    static uint8_t      irmp_pulse_time;                                        // count bit time for pulse
    static PAUSE_LEN    irmp_pause_time;                                        // count bit time for pause
    static uint16_t     last_irmp_address = 0xFFFF;                             // save last irmp address to recognize key repetition
    static uint16_t     last_irmp_command = 0xFFFF;                             // save last irmp command to recognize key repetition
    static uint16_t     repetition_len;                                         // SIRCS repeats frame 2-5 times with 45 ms pause
    static uint8_t      repetition_frame_number;
#if IRMP_SUPPORT_DENON_PROTOCOL == 1
    static uint16_t     last_irmp_denon_command;                                // save last irmp command to recognize DENON frame repetition
#endif
#if IRMP_SUPPORT_RC5_PROTOCOL == 1
    static uint8_t      rc5_cmd_bit6;                                           // bit 6 of RC5 command is the inverted 2nd start bit
#endif
#if IRMP_SUPPORT_MANCHESTER == 1
    static PAUSE_LEN    last_pause;                                             // last pause value
#endif
#if IRMP_SUPPORT_MANCHESTER == 1 || IRMP_SUPPORT_BANG_OLUFSEN_PROTOCOL == 1
    static uint8_t      last_value;                                             // last bit value
#endif
#ifndef __AVR__
    uint8_t             irmp_input;                                             // input value
#endif

#ifdef ANALYZE
    time_counter++;
#endif

#ifndef __AVR__
    irmp_input = input(IRMP_PIN);
#endif

    irmp_log(irmp_input);                                                       // log ir signal, if IRMP_LOGGING defined

    if (! irmp_ir_detected)                                                     // ir code already detected?
    {                                                                           // no...
        if (! irmp_start_bit_detected)                                          // start bit detected?
        {                                                                       // no...
            if (! irmp_input)                                                   // receiving burst?
            {                                                                   // yes...
#ifdef ANALYZE
                if (! irmp_pulse_time)
                {
                    ANALYZE_PRINTF("%8d [starting pulse]\n", time_counter);
                }
#endif
                irmp_pulse_time++;                                              // increment counter
            }
            else
            {                                                                   // no...
                if (irmp_pulse_time)                                            // it's dark....
                {                                                               // set flags for counting the time of darkness...
                    irmp_start_bit_detected = 1;
                    wait_for_start_space    = 1;
                    wait_for_space          = 0;
                    irmp_tmp_command        = 0;
                    irmp_tmp_address        = 0;

#if IRMP_SUPPORT_RC5_PROTOCOL == 1 && (IRMP_SUPPORT_FDC_PROTOCOL == 1 || IRMP_SUPPORT_RCCAR_PROTOCOL == 1)
                    irmp_tmp_command2       = 0;
                    irmp_tmp_address2       = 0;
#endif

                    irmp_bit                = 0xff;
                    irmp_pause_time         = 1;                                // 1st pause: set to 1, not to 0!
#if IRMP_SUPPORT_RC5_PROTOCOL == 1
                    rc5_cmd_bit6            = 0;                                // fm 2010-03-07: bugfix: reset it after incomplete RC5 frame!
#endif
                }
                else
                {
                    if (repetition_len < 0xFFFF)                                // avoid overflow of counter
                    {
                        repetition_len++;
                    }
                }
            }
        }
        else
        {
            if (wait_for_start_space)                                           // we have received start bit...
            {                                                                   // ...and are counting the time of darkness
                if (irmp_input)                                                 // still dark?
                {                                                               // yes
                    irmp_pause_time++;                                          // increment counter

                    if (irmp_pause_time > IRMP_TIMEOUT_LEN)                     // timeout?
                    {                                                           // yes...
#if IRMP_SUPPORT_JVC_PROTOCOL == 1
                        if (irmp_protocol == IRMP_JVC_PROTOCOL)                 // don't show eror if JVC protocol, irmp_pulse_time has been set below!
                        {
                            ;
                        }
                        else
#endif // IRMP_SUPPORT_JVC_PROTOCOL == 1
                        {
                            ANALYZE_PRINTF ("%8d error 1: pause after start bit pulse %d too long: %d\n", time_counter, irmp_pulse_time, irmp_pause_time);
                            ANALYZE_ONLY_NORMAL_PUTCHAR ('\n');
                        }
                        irmp_start_bit_detected = 0;                            // reset flags, let's wait for another start bit
                        irmp_pulse_time         = 0;
                        irmp_pause_time         = 0;
                    }
                }
                else
                {                                                               // receiving first data pulse!
                    IRMP_PARAMETER * irmp_param_p = (IRMP_PARAMETER *) 0;

#if IRMP_SUPPORT_RC5_PROTOCOL == 1 && (IRMP_SUPPORT_FDC_PROTOCOL == 1 || IRMP_SUPPORT_RCCAR_PROTOCOL == 1)
                    irmp_param2.protocol = 0;
#endif

                    ANALYZE_PRINTF ("%8d [start-bit: pulse = %2d, pause = %2d]\n", time_counter, irmp_pulse_time, irmp_pause_time);

#if IRMP_SUPPORT_SIRCS_PROTOCOL == 1
                    if (irmp_pulse_time >= SIRCS_START_BIT_PULSE_LEN_MIN && irmp_pulse_time <= SIRCS_START_BIT_PULSE_LEN_MAX &&
                        irmp_pause_time >= SIRCS_START_BIT_PAUSE_LEN_MIN && irmp_pause_time <= SIRCS_START_BIT_PAUSE_LEN_MAX)
                    {                                                           // it's SIRCS
                        ANALYZE_PRINTF ("protocol = SIRCS, start bit timings: pulse: %3d - %3d, pause: %3d - %3d\n",
                                        SIRCS_START_BIT_PULSE_LEN_MIN, SIRCS_START_BIT_PULSE_LEN_MAX,
                                        SIRCS_START_BIT_PAUSE_LEN_MIN, SIRCS_START_BIT_PAUSE_LEN_MAX);
                        irmp_param_p = (IRMP_PARAMETER *) (IRMP_PARAMETER *) &sircs_param;
                    }
                    else
#endif // IRMP_SUPPORT_SIRCS_PROTOCOL == 1

#if IRMP_SUPPORT_JVC_PROTOCOL == 1
                    if (irmp_protocol == IRMP_JVC_PROTOCOL &&                                                       // last protocol was JVC, awaiting repeat frame
                        irmp_pulse_time >= JVC_START_BIT_PULSE_LEN_MIN && irmp_pulse_time <= JVC_START_BIT_PULSE_LEN_MAX &&
                        irmp_pause_time >= JVC_REPEAT_START_BIT_PAUSE_LEN_MIN && irmp_pause_time <= JVC_REPEAT_START_BIT_PAUSE_LEN_MAX)
                    {
                        ANALYZE_PRINTF ("protocol = NEC or JVC repeat frame, start bit timings: pulse: %3d - %3d, pause: %3d - %3d\n",
                                        JVC_START_BIT_PULSE_LEN_MIN, JVC_START_BIT_PULSE_LEN_MAX,
                                        JVC_REPEAT_START_BIT_PAUSE_LEN_MIN, JVC_REPEAT_START_BIT_PAUSE_LEN_MAX);
                        irmp_param_p = (IRMP_PARAMETER *) &nec_param;                                               // tricky: use nec parameters
                    }
                    else
#endif // IRMP_SUPPORT_JVC_PROTOCOL == 1

#if IRMP_SUPPORT_NEC_PROTOCOL == 1
                    if (irmp_pulse_time >= NEC_START_BIT_PULSE_LEN_MIN && irmp_pulse_time <= NEC_START_BIT_PULSE_LEN_MAX &&
                        irmp_pause_time >= NEC_START_BIT_PAUSE_LEN_MIN && irmp_pause_time <= NEC_START_BIT_PAUSE_LEN_MAX)
                    {
                        ANALYZE_PRINTF ("protocol = NEC, start bit timings: pulse: %3d - %3d, pause: %3d - %3d\n",
                                        NEC_START_BIT_PULSE_LEN_MIN, NEC_START_BIT_PULSE_LEN_MAX,
                                        NEC_START_BIT_PAUSE_LEN_MIN, NEC_START_BIT_PAUSE_LEN_MAX);
                        irmp_param_p = (IRMP_PARAMETER *) &nec_param;
                    }
                    else if (irmp_pulse_time >= NEC_START_BIT_PULSE_LEN_MIN        && irmp_pulse_time <= NEC_START_BIT_PULSE_LEN_MAX &&
                             irmp_pause_time >= NEC_REPEAT_START_BIT_PAUSE_LEN_MIN && irmp_pause_time <= NEC_REPEAT_START_BIT_PAUSE_LEN_MAX)
                    {                                                           // it's NEC
                        ANALYZE_PRINTF ("protocol = NEC (repetition frame), start bit timings: pulse: %3d - %3d, pause: %3d - %3d\n",
                                        NEC_START_BIT_PULSE_LEN_MIN, NEC_START_BIT_PULSE_LEN_MAX,
                                        NEC_REPEAT_START_BIT_PAUSE_LEN_MIN, NEC_REPEAT_START_BIT_PAUSE_LEN_MAX);

                        irmp_param_p = (IRMP_PARAMETER *) &nec_rep_param;
                    }
                    else
#endif // IRMP_SUPPORT_NEC_PROTOCOL == 1

#if IRMP_SUPPORT_SAMSUNG_PROTOCOL == 1
                    if (irmp_pulse_time >= SAMSUNG_START_BIT_PULSE_LEN_MIN && irmp_pulse_time <= SAMSUNG_START_BIT_PULSE_LEN_MAX &&
                        irmp_pause_time >= SAMSUNG_START_BIT_PAUSE_LEN_MIN && irmp_pause_time <= SAMSUNG_START_BIT_PAUSE_LEN_MAX)
                    {                                                           // it's SAMSUNG
                        ANALYZE_PRINTF ("protocol = SAMSUNG, start bit timings: pulse: %3d - %3d, pause: %3d - %3d\n",
                                        SAMSUNG_START_BIT_PULSE_LEN_MIN, SAMSUNG_START_BIT_PULSE_LEN_MAX,
                                        SAMSUNG_START_BIT_PAUSE_LEN_MIN, SAMSUNG_START_BIT_PAUSE_LEN_MAX);
                        irmp_param_p = (IRMP_PARAMETER *) &samsung_param;
                    }
                    else
#endif // IRMP_SUPPORT_SAMSUNG_PROTOCOL == 1

#if IRMP_SUPPORT_MATSUSHITA_PROTOCOL == 1
                    if (irmp_pulse_time >= MATSUSHITA_START_BIT_PULSE_LEN_MIN && irmp_pulse_time <= MATSUSHITA_START_BIT_PULSE_LEN_MAX &&
                        irmp_pause_time >= MATSUSHITA_START_BIT_PAUSE_LEN_MIN && irmp_pause_time <= MATSUSHITA_START_BIT_PAUSE_LEN_MAX)
                    {                                                           // it's MATSUSHITA
                        ANALYZE_PRINTF ("protocol = MATSUSHITA, start bit timings: pulse: %3d - %3d, pause: %3d - %3d\n",
                                        MATSUSHITA_START_BIT_PULSE_LEN_MIN, MATSUSHITA_START_BIT_PULSE_LEN_MAX,
                                        MATSUSHITA_START_BIT_PAUSE_LEN_MIN, MATSUSHITA_START_BIT_PAUSE_LEN_MAX);
                        irmp_param_p = (IRMP_PARAMETER *) &matsushita_param;
                    }
                    else
#endif // IRMP_SUPPORT_MATSUSHITA_PROTOCOL == 1

#if IRMP_SUPPORT_KASEIKYO_PROTOCOL == 1
                    if (irmp_pulse_time >= KASEIKYO_START_BIT_PULSE_LEN_MIN && irmp_pulse_time <= KASEIKYO_START_BIT_PULSE_LEN_MAX &&
                        irmp_pause_time >= KASEIKYO_START_BIT_PAUSE_LEN_MIN && irmp_pause_time <= KASEIKYO_START_BIT_PAUSE_LEN_MAX)
                    {                                                           // it's KASEIKYO
                        ANALYZE_PRINTF ("protocol = KASEIKYO, start bit timings: pulse: %3d - %3d, pause: %3d - %3d\n",
                                        KASEIKYO_START_BIT_PULSE_LEN_MIN, KASEIKYO_START_BIT_PULSE_LEN_MAX,
                                        KASEIKYO_START_BIT_PAUSE_LEN_MIN, KASEIKYO_START_BIT_PAUSE_LEN_MAX);
                        irmp_param_p = (IRMP_PARAMETER *) &kaseikyo_param;
                    }
                    else
#endif // IRMP_SUPPORT_KASEIKYO_PROTOCOL == 1

#if IRMP_SUPPORT_RECS80_PROTOCOL == 1
                    if (irmp_pulse_time >= RECS80_START_BIT_PULSE_LEN_MIN && irmp_pulse_time <= RECS80_START_BIT_PULSE_LEN_MAX &&
                        irmp_pause_time >= RECS80_START_BIT_PAUSE_LEN_MIN && irmp_pause_time <= RECS80_START_BIT_PAUSE_LEN_MAX)
                    {                                                           // it's RECS80
                        ANALYZE_PRINTF ("protocol = RECS80, start bit timings: pulse: %3d - %3d, pause: %3d - %3d\n",
                                        RECS80_START_BIT_PULSE_LEN_MIN, RECS80_START_BIT_PULSE_LEN_MAX,
                                        RECS80_START_BIT_PAUSE_LEN_MIN, RECS80_START_BIT_PAUSE_LEN_MAX);
                        irmp_param_p = (IRMP_PARAMETER *) &recs80_param;
                    }
                    else
#endif // IRMP_SUPPORT_RECS80_PROTOCOL == 1

#if IRMP_SUPPORT_RC5_PROTOCOL == 1
                    if (((irmp_pulse_time >= RC5_START_BIT_LEN_MIN && irmp_pulse_time <= RC5_START_BIT_LEN_MAX) ||
                         (irmp_pulse_time >= 2 * RC5_START_BIT_LEN_MIN && irmp_pulse_time <= 2 * RC5_START_BIT_LEN_MAX)) &&
                        ((irmp_pause_time >= RC5_START_BIT_LEN_MIN && irmp_pause_time <= RC5_START_BIT_LEN_MAX) ||
                         (irmp_pause_time >= 2 * RC5_START_BIT_LEN_MIN && irmp_pause_time <= 2 * RC5_START_BIT_LEN_MAX)))
                    {                                                           // it's RC5
#if IRMP_SUPPORT_FDC_PROTOCOL == 1
                        if (irmp_pulse_time >= FDC_START_BIT_PULSE_LEN_MIN && irmp_pulse_time <= FDC_START_BIT_PULSE_LEN_MAX &&
                            irmp_pause_time >= FDC_START_BIT_PAUSE_LEN_MIN && irmp_pause_time <= FDC_START_BIT_PAUSE_LEN_MAX)
                        {
                            ANALYZE_PRINTF ("protocol = RC5 or FDC\n");
                            ANALYZE_PRINTF ("FDC start bit timings: pulse: %3d - %3d, pause: %3d - %3d\n",
                                            FDC_START_BIT_PULSE_LEN_MIN, FDC_START_BIT_PULSE_LEN_MAX,
                                            FDC_START_BIT_PAUSE_LEN_MIN, FDC_START_BIT_PAUSE_LEN_MAX);
                            ANALYZE_PRINTF ("RC5 start bit timings: pulse: %3d - %3d, pause: %3d - %3d\n",
                                            RC5_START_BIT_LEN_MIN, RC5_START_BIT_LEN_MAX,
                                            RC5_START_BIT_LEN_MIN, RC5_START_BIT_LEN_MAX);
                            memcpy_P (&irmp_param2, &fdc_param, sizeof (IRMP_PARAMETER));
                        }
                        else
#endif // IRMP_SUPPORT_FDC_PROTOCOL == 1
#if IRMP_SUPPORT_RCCAR_PROTOCOL == 1
                        if (irmp_pulse_time >= RCCAR_START_BIT_PULSE_LEN_MIN && irmp_pulse_time <= RCCAR_START_BIT_PULSE_LEN_MAX &&
                            irmp_pause_time >= RCCAR_START_BIT_PAUSE_LEN_MIN && irmp_pause_time <= RCCAR_START_BIT_PAUSE_LEN_MAX)
                        {
                            ANALYZE_PRINTF ("protocol = RC5 or RCCAR\n");
                            ANALYZE_PRINTF ("RCCAR start bit timings: pulse: %3d - %3d, pause: %3d - %3d\n",
                                            RCCAR_START_BIT_PULSE_LEN_MIN, RCCAR_START_BIT_PULSE_LEN_MAX,
                                            RCCAR_START_BIT_PAUSE_LEN_MIN, RCCAR_START_BIT_PAUSE_LEN_MAX);
                            ANALYZE_PRINTF ("RC5 start bit timings: pulse: %3d - %3d, pause: %3d - %3d\n",
                                            RC5_START_BIT_LEN_MIN, RC5_START_BIT_LEN_MAX,
                                            RC5_START_BIT_LEN_MIN, RC5_START_BIT_LEN_MAX);
                            memcpy_P (&irmp_param2, &rccar_param, sizeof (IRMP_PARAMETER));
                        }
                        else
#endif // IRMP_SUPPORT_RCCAR_PROTOCOL == 1
                        {
                            ANALYZE_PRINTF ("protocol = RC5, start bit timings: pulse: %3d - %3d, pause: %3d - %3d\n",
                                            RC5_START_BIT_LEN_MIN, RC5_START_BIT_LEN_MAX,
                                            RC5_START_BIT_LEN_MIN, RC5_START_BIT_LEN_MAX);
                        }

                        irmp_param_p = (IRMP_PARAMETER *) &rc5_param;
                        last_pause = irmp_pause_time;

                        if ((irmp_pulse_time > RC5_START_BIT_LEN_MAX && irmp_pulse_time <= 2 * RC5_START_BIT_LEN_MAX) ||
                            (irmp_pause_time > RC5_START_BIT_LEN_MAX && irmp_pause_time <= 2 * RC5_START_BIT_LEN_MAX))
                        {
                          last_value  = 0;
                          rc5_cmd_bit6 = 1<<6;
                        }
                        else
                        {
                          last_value  = 1;
                        }
                    }
                    else
#endif // IRMP_SUPPORT_RC5_PROTOCOL == 1

#if IRMP_SUPPORT_DENON_PROTOCOL == 1
                    if ( (irmp_pulse_time >= DENON_PULSE_LEN_MIN && irmp_pulse_time <= DENON_PULSE_LEN_MAX) &&
                        ((irmp_pause_time >= DENON_1_PAUSE_LEN_MIN && irmp_pause_time <= DENON_1_PAUSE_LEN_MAX) ||
                         (irmp_pause_time >= DENON_0_PAUSE_LEN_MIN && irmp_pause_time <= DENON_0_PAUSE_LEN_MAX)))
                    {                                                           // it's DENON
                        ANALYZE_PRINTF ("protocol = DENON, start bit timings: pulse: %3d - %3d, pause: %3d - %3d or %3d - %3d\n",
                                        DENON_PULSE_LEN_MIN, DENON_PULSE_LEN_MAX,
                                        DENON_1_PAUSE_LEN_MIN, DENON_1_PAUSE_LEN_MAX,
                                        DENON_0_PAUSE_LEN_MIN, DENON_0_PAUSE_LEN_MAX);
                        irmp_param_p = (IRMP_PARAMETER *) &denon_param;
                    }
                    else
#endif // IRMP_SUPPORT_DENON_PROTOCOL == 1

#if IRMP_SUPPORT_RC6_PROTOCOL == 1
                    if (irmp_pulse_time >= RC6_START_BIT_PULSE_LEN_MIN && irmp_pulse_time <= RC6_START_BIT_PULSE_LEN_MAX &&
                        irmp_pause_time >= RC6_START_BIT_PAUSE_LEN_MIN && irmp_pause_time <= RC6_START_BIT_PAUSE_LEN_MAX)
                    {                                                           // it's RC6
                        ANALYZE_PRINTF ("protocol = RC6, start bit timings: pulse: %3d - %3d, pause: %3d - %3d\n",
                                        RC6_START_BIT_PULSE_LEN_MIN, RC6_START_BIT_PULSE_LEN_MAX,
                                        RC6_START_BIT_PAUSE_LEN_MIN, RC6_START_BIT_PAUSE_LEN_MAX);
                        irmp_param_p = (IRMP_PARAMETER *) &rc6_param;
                        last_pause = 0;
                        last_value = 1;
                    }
                    else
#endif // IRMP_SUPPORT_RC6_PROTOCOL == 1

#if IRMP_SUPPORT_RECS80EXT_PROTOCOL == 1
                    if (irmp_pulse_time >= RECS80EXT_START_BIT_PULSE_LEN_MIN && irmp_pulse_time <= RECS80EXT_START_BIT_PULSE_LEN_MAX &&
                        irmp_pause_time >= RECS80EXT_START_BIT_PAUSE_LEN_MIN && irmp_pause_time <= RECS80EXT_START_BIT_PAUSE_LEN_MAX)
                    {                                                           // it's RECS80EXT
                        ANALYZE_PRINTF ("protocol = RECS80EXT, start bit timings: pulse: %3d - %3d, pause: %3d - %3d\n",
                                        RECS80EXT_START_BIT_PULSE_LEN_MIN, RECS80EXT_START_BIT_PULSE_LEN_MAX,
                                        RECS80EXT_START_BIT_PAUSE_LEN_MIN, RECS80EXT_START_BIT_PAUSE_LEN_MAX);
                        irmp_param_p = (IRMP_PARAMETER *) &recs80ext_param;
                    }
                    else
#endif // IRMP_SUPPORT_RECS80EXT_PROTOCOL == 1

#if IRMP_SUPPORT_NUBERT_PROTOCOL == 1
                    if (irmp_pulse_time >= NUBERT_START_BIT_PULSE_LEN_MIN && irmp_pulse_time <= NUBERT_START_BIT_PULSE_LEN_MAX &&
                        irmp_pause_time >= NUBERT_START_BIT_PAUSE_LEN_MIN && irmp_pause_time <= NUBERT_START_BIT_PAUSE_LEN_MAX)
                    {                                                           // it's NUBERT
                        ANALYZE_PRINTF ("protocol = NUBERT, start bit timings: pulse: %3d - %3d, pause: %3d - %3d\n",
                                        NUBERT_START_BIT_PULSE_LEN_MIN, NUBERT_START_BIT_PULSE_LEN_MAX,
                                        NUBERT_START_BIT_PAUSE_LEN_MIN, NUBERT_START_BIT_PAUSE_LEN_MAX);
                        irmp_param_p = (IRMP_PARAMETER *) &nubert_param;
                    }
                    else
#endif // IRMP_SUPPORT_NUBERT_PROTOCOL == 1

#if IRMP_SUPPORT_BANG_OLUFSEN_PROTOCOL == 1
                    if (irmp_pulse_time >= BANG_OLUFSEN_START_BIT1_PULSE_LEN_MIN && irmp_pulse_time <= BANG_OLUFSEN_START_BIT1_PULSE_LEN_MAX &&
                        irmp_pause_time >= BANG_OLUFSEN_START_BIT1_PAUSE_LEN_MIN && irmp_pause_time <= BANG_OLUFSEN_START_BIT1_PAUSE_LEN_MAX)
                    {                                                           // it's BANG_OLUFSEN
                        ANALYZE_PRINTF ("protocol = BANG_OLUFSEN\n");
                        ANALYZE_PRINTF ("start bit 1 timings: pulse: %3d - %3d, pause: %3d - %3d\n",
                                        BANG_OLUFSEN_START_BIT1_PULSE_LEN_MIN, BANG_OLUFSEN_START_BIT1_PULSE_LEN_MAX,
                                        BANG_OLUFSEN_START_BIT1_PAUSE_LEN_MIN, BANG_OLUFSEN_START_BIT1_PAUSE_LEN_MAX);
                        ANALYZE_PRINTF ("start bit 2 timings: pulse: %3d - %3d, pause: %3d - %3d\n",
                                        BANG_OLUFSEN_START_BIT2_PULSE_LEN_MIN, BANG_OLUFSEN_START_BIT2_PULSE_LEN_MAX,
                                        BANG_OLUFSEN_START_BIT2_PAUSE_LEN_MIN, BANG_OLUFSEN_START_BIT2_PAUSE_LEN_MAX);
                        ANALYZE_PRINTF ("start bit 3 timings: pulse: %3d - %3d, pause: %3d - %3d\n",
                                        BANG_OLUFSEN_START_BIT3_PULSE_LEN_MIN, BANG_OLUFSEN_START_BIT3_PULSE_LEN_MAX,
                                        BANG_OLUFSEN_START_BIT3_PAUSE_LEN_MIN, BANG_OLUFSEN_START_BIT3_PAUSE_LEN_MAX);
                        ANALYZE_PRINTF ("start bit 4 timings: pulse: %3d - %3d, pause: %3d - %3d\n",
                                        BANG_OLUFSEN_START_BIT4_PULSE_LEN_MIN, BANG_OLUFSEN_START_BIT4_PULSE_LEN_MAX,
                                        BANG_OLUFSEN_START_BIT4_PAUSE_LEN_MIN, BANG_OLUFSEN_START_BIT4_PAUSE_LEN_MAX);
                        irmp_param_p = (IRMP_PARAMETER *) &bang_olufsen_param;
                        last_value = 0;
                    }
                    else
#endif // IRMP_SUPPORT_BANG_OLUFSEN_PROTOCOL == 1

#if IRMP_SUPPORT_GRUNDIG_OR_NOKIA_PROTOCOL == 1
                    if (irmp_pulse_time >= GRUNDIG_OR_NOKIA_START_BIT_LEN_MIN && irmp_pulse_time <= GRUNDIG_OR_NOKIA_START_BIT_LEN_MAX &&
                        irmp_pause_time >= GRUNDIG_OR_NOKIA_PRE_PAUSE_LEN_MIN && irmp_pause_time <= GRUNDIG_OR_NOKIA_PRE_PAUSE_LEN_MAX)
                    {                                                           // it's GRUNDIG
                        ANALYZE_PRINTF ("protocol = GRUNDIG, pre bit timings: pulse: %3d - %3d, pause: %3d - %3d\n",
                                        GRUNDIG_OR_NOKIA_START_BIT_LEN_MIN, GRUNDIG_OR_NOKIA_START_BIT_LEN_MAX,
                                        GRUNDIG_OR_NOKIA_PRE_PAUSE_LEN_MIN, GRUNDIG_OR_NOKIA_PRE_PAUSE_LEN_MAX);
                        irmp_param_p = (IRMP_PARAMETER *) &grundig_param;
                        last_pause = irmp_pause_time;
                        last_value  = 1;
                    }
                    else
#endif // IRMP_SUPPORT_GRUNDIG_OR_NOKIA_PROTOCOL == 1

#if IRMP_SUPPORT_SIEMENS_PROTOCOL == 1
                    if (((irmp_pulse_time >= SIEMENS_START_BIT_LEN_MIN && irmp_pulse_time <= SIEMENS_START_BIT_LEN_MAX) ||
                         (irmp_pulse_time >= 2 * SIEMENS_START_BIT_LEN_MIN && irmp_pulse_time <= 2 * SIEMENS_START_BIT_LEN_MAX)) &&
                        ((irmp_pause_time >= SIEMENS_START_BIT_LEN_MIN && irmp_pause_time <= SIEMENS_START_BIT_LEN_MAX) ||
                         (irmp_pause_time >= 2 * SIEMENS_START_BIT_LEN_MIN && irmp_pause_time <= 2 * SIEMENS_START_BIT_LEN_MAX)))
                    {                                                           // it's SIEMENS
                        ANALYZE_PRINTF ("protocol = SIEMENS, start bit timings: pulse: %3d - %3d, pause: %3d - %3d\n",
                                        SIEMENS_START_BIT_LEN_MIN, SIEMENS_START_BIT_LEN_MAX,
                                        SIEMENS_START_BIT_LEN_MIN, SIEMENS_START_BIT_LEN_MAX);
                        irmp_param_p = (IRMP_PARAMETER *) &siemens_param;
                        last_pause = irmp_pause_time;
                        last_value  = 1;
                    }
                    else
#endif // IRMP_SUPPORT_SIEMENS_PROTOCOL == 1
#if IRMP_SUPPORT_FDC_PROTOCOL == 1
                    if (irmp_pulse_time >= FDC_START_BIT_PULSE_LEN_MIN && irmp_pulse_time <= FDC_START_BIT_PULSE_LEN_MAX &&
                        irmp_pause_time >= FDC_START_BIT_PAUSE_LEN_MIN && irmp_pause_time <= FDC_START_BIT_PAUSE_LEN_MAX)
                    {
                        ANALYZE_PRINTF ("protocol = FDC, start bit timings: pulse: %3d - %3d, pause: %3d - %3d\n",
                                        FDC_START_BIT_PULSE_LEN_MIN, FDC_START_BIT_PULSE_LEN_MAX,
                                        FDC_START_BIT_PAUSE_LEN_MIN, FDC_START_BIT_PAUSE_LEN_MAX);
                        irmp_param_p = (IRMP_PARAMETER *) &fdc_param;
                    }
                    else
#endif // IRMP_SUPPORT_FDC_PROTOCOL == 1
#if IRMP_SUPPORT_RCCAR_PROTOCOL == 1
                    if (irmp_pulse_time >= RCCAR_START_BIT_PULSE_LEN_MIN && irmp_pulse_time <= RCCAR_START_BIT_PULSE_LEN_MAX &&
                        irmp_pause_time >= RCCAR_START_BIT_PAUSE_LEN_MIN && irmp_pause_time <= RCCAR_START_BIT_PAUSE_LEN_MAX)
                    {
                        ANALYZE_PRINTF ("protocol = RCCAR, start bit timings: pulse: %3d - %3d, pause: %3d - %3d\n",
                                        RCCAR_START_BIT_PULSE_LEN_MIN, RCCAR_START_BIT_PULSE_LEN_MAX,
                                        RCCAR_START_BIT_PAUSE_LEN_MIN, RCCAR_START_BIT_PAUSE_LEN_MAX);
                        irmp_param_p = (IRMP_PARAMETER *) &rccar_param;
                    }
                    else
#endif // IRMP_SUPPORT_RCCAR_PROTOCOL == 1
                    {
                        ANALYZE_PRINTF ("protocol = UNKNOWN\n");
                        irmp_start_bit_detected = 0;                            // wait for another start bit...
                    }

                    if (irmp_start_bit_detected)
                    {
                        memcpy_P (&irmp_param, irmp_param_p, sizeof (IRMP_PARAMETER));

#ifdef ANALYZE
                        if (! (irmp_param.flags & IRMP_PARAM_FLAG_IS_MANCHESTER))
                        {
                            ANALYZE_PRINTF ("pulse_1: %3d - %3d\n", irmp_param.pulse_1_len_min, irmp_param.pulse_1_len_max);
                            ANALYZE_PRINTF ("pause_1: %3d - %3d\n", irmp_param.pause_1_len_min, irmp_param.pause_1_len_max);
                        }
                        else
                        {
                            ANALYZE_PRINTF ("pulse: %3d - %3d or %3d - %3d\n", irmp_param.pulse_1_len_min, irmp_param.pulse_1_len_max,
                                            irmp_param.pulse_1_len_max + 1, 2 * irmp_param.pulse_1_len_max);
                            ANALYZE_PRINTF ("pause: %3d - %3d or %3d - %3d\n", irmp_param.pause_1_len_min, irmp_param.pause_1_len_max,
                                            irmp_param.pause_1_len_max + 1, 2 * irmp_param.pause_1_len_max);
                        }

#if IRMP_SUPPORT_RC5_PROTOCOL == 1 && (IRMP_SUPPORT_FDC_PROTOCOL == 1 || IRMP_SUPPORT_RCCAR_PROTOCOL == 1)
                        if (irmp_param2.protocol)
                        {
                            ANALYZE_PRINTF ("pulse_0: %3d - %3d\n", irmp_param2.pulse_0_len_min, irmp_param2.pulse_0_len_max);
                            ANALYZE_PRINTF ("pause_0: %3d - %3d\n", irmp_param2.pause_0_len_min, irmp_param2.pause_0_len_max);
                            ANALYZE_PRINTF ("pulse_1: %3d - %3d\n", irmp_param2.pulse_1_len_min, irmp_param2.pulse_1_len_max);
                            ANALYZE_PRINTF ("pause_1: %3d - %3d\n", irmp_param2.pause_1_len_min, irmp_param2.pause_1_len_max);
                        }
#endif


#if IRMP_SUPPORT_RC6_PROTOCOL == 1
                        if (irmp_param.protocol == IRMP_RC6_PROTOCOL)
                        {
                            ANALYZE_PRINTF ("pulse_toggle: %3d - %3d\n", RC6_TOGGLE_BIT_LEN_MIN, RC6_TOGGLE_BIT_LEN_MAX);
                        }
#endif

                        if (! (irmp_param.flags & IRMP_PARAM_FLAG_IS_MANCHESTER))
                        {
                            ANALYZE_PRINTF ("pulse_0: %3d - %3d\n", irmp_param.pulse_0_len_min, irmp_param.pulse_0_len_max);
                            ANALYZE_PRINTF ("pause_0: %3d - %3d\n", irmp_param.pause_0_len_min, irmp_param.pause_0_len_max);
                        }

#if IRMP_SUPPORT_BANG_OLUFSEN_PROTOCOL == 1
                        if (irmp_param.protocol == IRMP_BANG_OLUFSEN_PROTOCOL)
                        {
                            ANALYZE_PRINTF ("pulse_r: %3d - %3d\n", irmp_param.pulse_0_len_min, irmp_param.pulse_0_len_max);
                            ANALYZE_PRINTF ("pause_r: %3d - %3d\n", BANG_OLUFSEN_R_PAUSE_LEN_MIN, BANG_OLUFSEN_R_PAUSE_LEN_MAX);
                        }
#endif

                        ANALYZE_PRINTF ("command_offset: %2d\n", irmp_param.command_offset);
                        ANALYZE_PRINTF ("command_len:    %3d\n", irmp_param.command_end - irmp_param.command_offset);
                        ANALYZE_PRINTF ("complete_len:   %3d\n", irmp_param.complete_len);
                        ANALYZE_PRINTF ("stop_bit:       %3d\n", irmp_param.stop_bit);
#endif // ANALYZE
                    }

                    irmp_bit = 0;

#if IRMP_SUPPORT_MANCHESTER == 1
                    if ((irmp_param.flags & IRMP_PARAM_FLAG_IS_MANCHESTER) && irmp_param.protocol != IRMP_RC6_PROTOCOL)    // Manchester, but not RC6
                    {
                        if (irmp_pause_time > irmp_param.pulse_1_len_max && irmp_pause_time <= 2 * irmp_param.pulse_1_len_max)
                        {
                            ANALYZE_PRINTF ("%8d [bit %2d: pulse = %3d, pause = %3d] ", time_counter, irmp_bit, irmp_pulse_time, irmp_pause_time);
                            ANALYZE_PUTCHAR ((irmp_param.flags & IRMP_PARAM_FLAG_1ST_PULSE_IS_1) ? '0' : '1');
                            ANALYZE_NEWLINE ();
                            irmp_store_bit ((irmp_param.flags & IRMP_PARAM_FLAG_1ST_PULSE_IS_1) ? 0 : 1);
                        }
                        else if (! last_value)  // && irmp_pause_time >= irmp_param.pause_1_len_min && irmp_pause_time <= irmp_param.pause_1_len_max)
                        {
                            ANALYZE_PRINTF ("%8d [bit %2d: pulse = %3d, pause = %3d] ", time_counter, irmp_bit, irmp_pulse_time, irmp_pause_time);

                            ANALYZE_PUTCHAR ((irmp_param.flags & IRMP_PARAM_FLAG_1ST_PULSE_IS_1) ? '1' : '0');
                            ANALYZE_NEWLINE ();
                            irmp_store_bit ((irmp_param.flags & IRMP_PARAM_FLAG_1ST_PULSE_IS_1) ? 1 : 0);
                        }
                    }
                    else
#endif // IRMP_SUPPORT_MANCHESTER == 1

#if IRMP_SUPPORT_DENON_PROTOCOL == 1
                    if (irmp_param.protocol == IRMP_DENON_PROTOCOL)
                    {
                        ANALYZE_PRINTF ("%8d [bit %2d: pulse = %3d, pause = %3d] ", time_counter, irmp_bit, irmp_pulse_time, irmp_pause_time);

                        if (irmp_pause_time >= DENON_1_PAUSE_LEN_MIN && irmp_pause_time <= DENON_1_PAUSE_LEN_MAX)
                        {                                                       // pause timings correct for "1"?
                          ANALYZE_PUTCHAR ('1');                                  // yes, store 1
                          ANALYZE_NEWLINE ();
                          irmp_store_bit (1);
                        }
                        else // if (irmp_pause_time >= DENON_0_PAUSE_LEN_MIN && irmp_pause_time <= DENON_0_PAUSE_LEN_MAX)
                        {                                                       // pause timings correct for "0"?
                          ANALYZE_PUTCHAR ('0');                                  // yes, store 0
                          ANALYZE_NEWLINE ();
                          irmp_store_bit (0);
                        }
                    }
                    else
#endif // IRMP_SUPPORT_DENON_PROTOCOL == 1
                    {
                        ;                                                       // else do nothing
                    }

                    irmp_pulse_time = 1;                                        // set counter to 1, not 0
                    irmp_pause_time = 0;
                    wait_for_start_space = 0;
                }
            }
            else if (wait_for_space)                                            // the data section....
            {                                                                   // counting the time of darkness....
                uint8_t got_light = FALSE;

                if (irmp_input)                                                 // still dark?
                {                                                               // yes...
                    if (irmp_bit == irmp_param.complete_len && irmp_param.stop_bit == 1)
                    {
                        if (irmp_pulse_time >= irmp_param.pulse_0_len_min && irmp_pulse_time <= irmp_param.pulse_0_len_max)
                        {
#ifdef ANALYZE
                            if (! (irmp_param.flags & IRMP_PARAM_FLAG_IS_MANCHESTER))
                            {
                                ANALYZE_PRINTF ("stop bit detected\n");
                            }
#endif
                            irmp_param.stop_bit = 0;
                        }
                        else
                        {
                            ANALYZE_PRINTF ("stop bit timing wrong\n");

                            irmp_start_bit_detected = 0;                        // wait for another start bit...
                            irmp_pulse_time         = 0;
                            irmp_pause_time         = 0;
                        }
                    }
                    else
                    {
                        irmp_pause_time++;                                      // increment counter

#if IRMP_SUPPORT_SIRCS_PROTOCOL == 1
                        if (irmp_param.protocol == IRMP_SIRCS_PROTOCOL &&       // Sony has a variable number of bits:
                            irmp_pause_time > SIRCS_PAUSE_LEN_MAX &&            // minimum is 12
                            irmp_bit >= 12 - 1)                                 // pause too long?
                        {                                                       // yes, break and close this frame
                            irmp_param.complete_len = irmp_bit + 1;             // set new complete length
                            got_light = TRUE;                                   // this is a lie, but helps (generates stop bit)
                            irmp_param.command_end = irmp_param.command_offset + irmp_bit + 1;        // correct command length
                            irmp_pause_time = SIRCS_PAUSE_LEN_MAX - 1;          // correct pause length
                        }
                        else
#endif
#if IRMP_SUPPORT_GRUNDIG_OR_NOKIA_PROTOCOL == 1
                        if (irmp_param.protocol == IRMP_GRUNDIG_PROTOCOL && !irmp_param.stop_bit)
                        {
                            if (irmp_pause_time > 2 * irmp_param.pause_1_len_max && irmp_bit >= GRUNDIG_COMPLETE_DATA_LEN - 2)
                            {                                                           // special manchester decoder
                                irmp_param.complete_len = GRUNDIG_COMPLETE_DATA_LEN;    // correct complete len
                                got_light = TRUE;                                       // this is a lie, but generates a stop bit ;-)
                                irmp_param.stop_bit = TRUE;                             // set flag
                            }
                            else if (irmp_bit >= GRUNDIG_COMPLETE_DATA_LEN)
                            {
                                ANALYZE_PRINTF ("Switching to NOKIA protocol\n");
                                irmp_param.protocol         = IRMP_NOKIA_PROTOCOL;      // change protocol
                                irmp_param.address_offset   = NOKIA_ADDRESS_OFFSET;
                                irmp_param.address_end      = NOKIA_ADDRESS_OFFSET + NOKIA_ADDRESS_LEN;
                                irmp_param.command_offset   = NOKIA_COMMAND_OFFSET;
                                irmp_param.command_end      = NOKIA_COMMAND_OFFSET + NOKIA_COMMAND_LEN;

                                if (irmp_tmp_command & 0x300)
                                {
                                    irmp_tmp_address = (irmp_tmp_command >> 8);
                                    irmp_tmp_command &= 0xFF;
                                }
                            }
                        }
                        else
#endif
#if IRMP_SUPPORT_MANCHESTER == 1
                        if ((irmp_param.flags & IRMP_PARAM_FLAG_IS_MANCHESTER) &&
                            irmp_pause_time > 2 * irmp_param.pause_1_len_max && irmp_bit >= irmp_param.complete_len - 2 && !irmp_param.stop_bit)
                        {                                                       // special manchester decoder
                            got_light = TRUE;                                   // this is a lie, but generates a stop bit ;-)
                            irmp_param.stop_bit = TRUE;                         // set flag
                        }
                        else
#endif // IRMP_SUPPORT_MANCHESTER == 1
                        if (irmp_pause_time > IRMP_TIMEOUT_LEN)                 // timeout?
                        {                                                       // yes...
                            if (irmp_bit == irmp_param.complete_len - 1 && irmp_param.stop_bit == 0)
                            {
                                irmp_bit++;
                            }
#if IRMP_SUPPORT_JVC_PROTOCOL == 1
                            else if (irmp_param.protocol == IRMP_NEC_PROTOCOL && (irmp_bit == 16 || irmp_bit == 17))      // it was a JVC stop bit
                            {
                                ANALYZE_PRINTF ("Switching to JVC protocol\n");
                                irmp_param.stop_bit     = TRUE;                                     // set flag
                                irmp_param.protocol     = IRMP_JVC_PROTOCOL;                        // switch protocol
                                irmp_param.complete_len = irmp_bit;                                 // patch length: 16 or 17
                                irmp_tmp_command        = (irmp_tmp_address >> 4);                  // set command: upper 12 bits are command bits
                                irmp_tmp_address        = irmp_tmp_address & 0x000F;                // lower 4 bits are address bits
                                irmp_start_bit_detected = 1;                                        // tricky: don't wait for another start bit...
                            }
#endif // IRMP_SUPPORT_JVC_PROTOCOL == 1
                            else
                            {
                                ANALYZE_PRINTF ("error 2: pause %d after data bit %d too long\n", irmp_pause_time, irmp_bit);
                                ANALYZE_ONLY_NORMAL_PUTCHAR ('\n');

                                irmp_start_bit_detected = 0;                    // wait for another start bit...
                                irmp_pulse_time         = 0;
                                irmp_pause_time         = 0;
                            }
                        }
                    }
                }
                else
                {                                                               // got light now!
                    got_light = TRUE;
                }

                if (got_light)
                {
                    ANALYZE_PRINTF ("%8d [bit %2d: pulse = %3d, pause = %3d] ", time_counter, irmp_bit, irmp_pulse_time, irmp_pause_time);

#if IRMP_SUPPORT_MANCHESTER == 1
                    if ((irmp_param.flags & IRMP_PARAM_FLAG_IS_MANCHESTER))                                     // Manchester
                    {
                        if (irmp_pulse_time > irmp_param.pulse_1_len_max /* && irmp_pulse_time <= 2 * irmp_param.pulse_1_len_max */)
                        {
#if IRMP_SUPPORT_RC6_PROTOCOL == 1
                            if (irmp_param.protocol == IRMP_RC6_PROTOCOL && irmp_bit == 4 && irmp_pulse_time > RC6_TOGGLE_BIT_LEN_MIN)         // RC6 toggle bit
                            {
                                ANALYZE_PUTCHAR ('T');
                                if (irmp_param.complete_len == RC6_COMPLETE_DATA_LEN_LONG)                      // RC6 mode 6A
                                {
                                    irmp_store_bit (1);
                                    last_value = 1;
                                }
                                else                                                                            // RC6 mode 0
                                {
                                    irmp_store_bit (0);
                                    last_value = 0;
                                }
                                ANALYZE_NEWLINE ();
                            }
                            else
#endif // IRMP_SUPPORT_RC6_PROTOCOL == 1
                            {
                                ANALYZE_PUTCHAR ((irmp_param.flags & IRMP_PARAM_FLAG_1ST_PULSE_IS_1) ? '0' : '1');
                                irmp_store_bit ((irmp_param.flags & IRMP_PARAM_FLAG_1ST_PULSE_IS_1) ? 0  :  1 );

#if IRMP_SUPPORT_RC6_PROTOCOL == 1
                                if (irmp_param.protocol == IRMP_RC6_PROTOCOL && irmp_bit == 4 && irmp_pulse_time > RC6_TOGGLE_BIT_LEN_MIN)      // RC6 toggle bit
                                {
                                    ANALYZE_PUTCHAR ('T');
                                    irmp_store_bit (1);

                                    if (irmp_pause_time > 2 * irmp_param.pause_1_len_max)
                                    {
                                        last_value = 0;
                                    }
                                    else
                                    {
                                        last_value = 1;
                                    }
                                    ANALYZE_NEWLINE ();
                                }
                                else
#endif // IRMP_SUPPORT_RC6_PROTOCOL == 1
                                {
                                    ANALYZE_PUTCHAR ((irmp_param.flags & IRMP_PARAM_FLAG_1ST_PULSE_IS_1) ? '1' : '0');
                                    irmp_store_bit ((irmp_param.flags & IRMP_PARAM_FLAG_1ST_PULSE_IS_1) ? 1 :   0 );
#if IRMP_SUPPORT_RC5_PROTOCOL == 1 && (IRMP_SUPPORT_FDC_PROTOCOL == 1 || IRMP_SUPPORT_RCCAR_PROTOCOL == 1)
                                    if (! irmp_param2.protocol)
#endif
                                    {
                                        ANALYZE_NEWLINE ();
                                    }
                                    last_value = (irmp_param.flags & IRMP_PARAM_FLAG_1ST_PULSE_IS_1) ? 1 : 0;
                                }
                            }
                        }
                        else if (irmp_pulse_time >= irmp_param.pulse_1_len_min && irmp_pulse_time <= irmp_param.pulse_1_len_max)
                        {
                            uint8_t manchester_value;

                            if (last_pause > irmp_param.pause_1_len_max && last_pause <= 2 * irmp_param.pause_1_len_max)
                            {
                                manchester_value = last_value ? 0 : 1;
                                last_value  = manchester_value;
                            }
                            else
                            {
                                manchester_value = last_value;
                            }

                            ANALYZE_PUTCHAR (manchester_value + '0');

#if IRMP_SUPPORT_RC5_PROTOCOL == 1 && (IRMP_SUPPORT_FDC_PROTOCOL == 1 || IRMP_SUPPORT_RCCAR_PROTOCOL == 1)
                            if (! irmp_param2.protocol)
#endif
                            {
                                ANALYZE_NEWLINE ();
                            }

#if IRMP_SUPPORT_RC6_PROTOCOL == 1
                            if (irmp_param.protocol == IRMP_RC6_PROTOCOL && irmp_bit == 1 && manchester_value == 1)     // RC6 mode != 0 ???
                            {
                                ANALYZE_PRINTF ("Switching to RC6A protocol\n");
                                irmp_param.complete_len = RC6_COMPLETE_DATA_LEN_LONG;
                                irmp_param.address_offset = 5;
                                irmp_param.address_end = irmp_param.address_offset + 15;
                                irmp_param.command_offset = irmp_param.address_end + 1;                                 // skip 1 system bit, changes like a toggle bit
                                irmp_param.command_end = irmp_param.command_offset + 16 - 1;
                                irmp_tmp_address = 1;                                                                   // addr 0 - 32767 --> 32768 - 65535
                            }
#endif // IRMP_SUPPORT_RC6_PROTOCOL == 1

                            irmp_store_bit (manchester_value);
                        }
                        else
                        {
#if IRMP_SUPPORT_RC5_PROTOCOL == 1 && IRMP_SUPPORT_FDC_PROTOCOL == 1
                            if (irmp_param2.protocol == IRMP_FDC_PROTOCOL &&
                                irmp_pulse_time >= FDC_PULSE_LEN_MIN && irmp_pulse_time <= FDC_PULSE_LEN_MAX &&
                                ((irmp_pause_time >= FDC_1_PAUSE_LEN_MIN && irmp_pause_time <= FDC_1_PAUSE_LEN_MAX) ||
                                 (irmp_pause_time >= FDC_0_PAUSE_LEN_MIN && irmp_pause_time <= FDC_0_PAUSE_LEN_MAX)))
                            {
                                ANALYZE_PUTCHAR ('?');
                                irmp_param.protocol = 0;                // switch to FDC, see below
                            }
                            else
#endif // IRMP_SUPPORT_FDC_PROTOCOL == 1
#if IRMP_SUPPORT_RC5_PROTOCOL == 1 && IRMP_SUPPORT_RCCAR_PROTOCOL == 1
                            if (irmp_param2.protocol == IRMP_RCCAR_PROTOCOL &&
                                irmp_pulse_time >= RCCAR_PULSE_LEN_MIN && irmp_pulse_time <= RCCAR_PULSE_LEN_MAX &&
                                ((irmp_pause_time >= RCCAR_1_PAUSE_LEN_MIN && irmp_pause_time <= RCCAR_1_PAUSE_LEN_MAX) ||
                                 (irmp_pause_time >= RCCAR_0_PAUSE_LEN_MIN && irmp_pause_time <= RCCAR_0_PAUSE_LEN_MAX)))
                            {
                                ANALYZE_PUTCHAR ('?');
                                irmp_param.protocol = 0;                // switch to RCCAR, see below
                            }
                            else
#endif // IRMP_SUPPORT_RCCAR_PROTOCOL == 1
                            {
                                ANALYZE_PUTCHAR ('?');
                                ANALYZE_NEWLINE ();
                                ANALYZE_PRINTF ("error 3 manchester: timing not correct: data bit %d,  pulse: %d, pause: %d\n", irmp_bit, irmp_pulse_time, irmp_pause_time);
                                ANALYZE_ONLY_NORMAL_PUTCHAR ('\n');
                                irmp_start_bit_detected = 0;                            // reset flags and wait for next start bit
                                irmp_pause_time         = 0;
                            }
                        }

#if IRMP_SUPPORT_RC5_PROTOCOL == 1 && IRMP_SUPPORT_FDC_PROTOCOL == 1
                        if (irmp_param2.protocol == IRMP_FDC_PROTOCOL && irmp_pulse_time >= FDC_PULSE_LEN_MIN && irmp_pulse_time <= FDC_PULSE_LEN_MAX)
                        {
                            if (irmp_pause_time >= FDC_1_PAUSE_LEN_MIN && irmp_pause_time <= FDC_1_PAUSE_LEN_MAX)
                            {
                                ANALYZE_PRINTF ("   1 (FDC)\n");
                                irmp_store_bit2 (1);
                            }
                            else if (irmp_pause_time >= FDC_0_PAUSE_LEN_MIN && irmp_pause_time <= FDC_0_PAUSE_LEN_MAX)
                            {
                                ANALYZE_PRINTF ("   0 (FDC)\n");
                                irmp_store_bit2 (0);
                            }

                            if (! irmp_param.protocol)
                            {
                                ANALYZE_PRINTF ("Switching to FDC protocol\n");
                                memcpy (&irmp_param, &irmp_param2, sizeof (IRMP_PARAMETER));
                                irmp_param2.protocol = 0;
                                irmp_tmp_address = irmp_tmp_address2;
                                irmp_tmp_command = irmp_tmp_command2;
                            }
                        }
#endif // IRMP_SUPPORT_FDC_PROTOCOL == 1
#if IRMP_SUPPORT_RC5_PROTOCOL == 1 && IRMP_SUPPORT_RCCAR_PROTOCOL == 1
                        if (irmp_param2.protocol == IRMP_RCCAR_PROTOCOL && irmp_pulse_time >= RCCAR_PULSE_LEN_MIN && irmp_pulse_time <= RCCAR_PULSE_LEN_MAX)
                        {
                            if (irmp_pause_time >= RCCAR_1_PAUSE_LEN_MIN && irmp_pause_time <= RCCAR_1_PAUSE_LEN_MAX)
                            {
                                ANALYZE_PRINTF ("   1 (RCCAR)\n");
                                irmp_store_bit2 (1);
                            }
                            else if (irmp_pause_time >= RCCAR_0_PAUSE_LEN_MIN && irmp_pause_time <= RCCAR_0_PAUSE_LEN_MAX)
                            {
                                ANALYZE_PRINTF ("   0 (RCCAR)\n");
                                irmp_store_bit2 (0);
                            }

                            if (! irmp_param.protocol)
                            {
                                ANALYZE_PRINTF ("Switching to RCCAR protocol\n");
                                memcpy (&irmp_param, &irmp_param2, sizeof (IRMP_PARAMETER));
                                irmp_param2.protocol = 0;
                                irmp_tmp_address = irmp_tmp_address2;
                                irmp_tmp_command = irmp_tmp_command2;
                            }
                        }
#endif // IRMP_SUPPORT_RCCAR_PROTOCOL == 1

                        last_pause      = irmp_pause_time;
                        wait_for_space  = 0;
                    }
                    else
#endif // IRMP_SUPPORT_MANCHESTER == 1


#if IRMP_SUPPORT_SAMSUNG_PROTOCOL == 1
                    if (irmp_param.protocol == IRMP_SAMSUNG_PROTOCOL && irmp_bit == 16)       // Samsung: 16th bit
                    {
                        if (irmp_pulse_time >= SAMSUNG_PULSE_LEN_MIN && irmp_pulse_time <= SAMSUNG_PULSE_LEN_MAX &&
                            irmp_pause_time >= SAMSUNG_START_BIT_PAUSE_LEN_MIN && irmp_pause_time <= SAMSUNG_START_BIT_PAUSE_LEN_MAX)
                        {
                            ANALYZE_PRINTF ("SYNC\n");
                            wait_for_space = 0;
                            irmp_tmp_id = 0;
                            irmp_bit++;
                        }
                        else  if (irmp_pulse_time >= SAMSUNG_PULSE_LEN_MIN && irmp_pulse_time <= SAMSUNG_PULSE_LEN_MAX)
                        {
                            irmp_param.protocol         = IRMP_SAMSUNG32_PROTOCOL;
                            irmp_param.command_offset   = SAMSUNG32_COMMAND_OFFSET;
                            irmp_param.command_end      = SAMSUNG32_COMMAND_OFFSET + SAMSUNG32_COMMAND_LEN;
                            irmp_param.complete_len     = SAMSUNG32_COMPLETE_DATA_LEN;

                            if (irmp_pause_time >= SAMSUNG_1_PAUSE_LEN_MIN && irmp_pause_time <= SAMSUNG_1_PAUSE_LEN_MAX)
                            {
                                ANALYZE_PUTCHAR ('1');
                                ANALYZE_NEWLINE ();
                                irmp_store_bit (1);
                                wait_for_space = 0;
                            }
                            else
                            {
                                ANALYZE_PUTCHAR ('0');
                                ANALYZE_NEWLINE ();
                                irmp_store_bit (0);
                                wait_for_space = 0;
                            }

                            ANALYZE_PRINTF ("Switching to SAMSUNG32 protocol\n");
                        }
                        else
                        {                                                           // timing incorrect!
                            ANALYZE_PRINTF ("error 3 Samsung: timing not correct: data bit %d,  pulse: %d, pause: %d\n", irmp_bit, irmp_pulse_time, irmp_pause_time);
                            ANALYZE_ONLY_NORMAL_PUTCHAR ('\n');
                            irmp_start_bit_detected = 0;                            // reset flags and wait for next start bit
                            irmp_pause_time         = 0;
                        }
                    }
                    else
#endif // IRMP_SUPPORT_SAMSUNG_PROTOCOL

#if IRMP_SUPPORT_BANG_OLUFSEN_PROTOCOL == 1
                    if (irmp_param.protocol == IRMP_BANG_OLUFSEN_PROTOCOL)
                    {
                        if (irmp_pulse_time >= BANG_OLUFSEN_PULSE_LEN_MIN && irmp_pulse_time <= BANG_OLUFSEN_PULSE_LEN_MAX)
                        {
                            if (irmp_bit == 1)                                      // Bang & Olufsen: 3rd bit
                            {
                                if (irmp_pause_time >= BANG_OLUFSEN_START_BIT3_PAUSE_LEN_MIN && irmp_pause_time <= BANG_OLUFSEN_START_BIT3_PAUSE_LEN_MAX)
                                {
                                    ANALYZE_PRINTF ("3rd start bit\n");
                                    wait_for_space = 0;
                                    irmp_bit++;
                                }
                                else
                                {                                                   // timing incorrect!
                                    ANALYZE_PRINTF ("error 3a B&O: timing not correct: data bit %d,  pulse: %d, pause: %d\n", irmp_bit, irmp_pulse_time, irmp_pause_time);
                                    ANALYZE_ONLY_NORMAL_PUTCHAR ('\n');
                                    irmp_start_bit_detected = 0;                    // reset flags and wait for next start bit
                                    irmp_pause_time         = 0;
                                }
                            }
                            else if (irmp_bit == 19)                                // Bang & Olufsen: trailer bit
                            {
                                if (irmp_pause_time >= BANG_OLUFSEN_TRAILER_BIT_PAUSE_LEN_MIN && irmp_pause_time <= BANG_OLUFSEN_TRAILER_BIT_PAUSE_LEN_MAX)
                                {
                                    ANALYZE_PRINTF ("trailer bit\n");
                                    wait_for_space = 0;
                                    irmp_bit++;
                                }
                                else
                                {                                                   // timing incorrect!
                                    ANALYZE_PRINTF ("error 3b B&O: timing not correct: data bit %d,  pulse: %d, pause: %d\n", irmp_bit, irmp_pulse_time, irmp_pause_time);
                                    ANALYZE_ONLY_NORMAL_PUTCHAR ('\n');
                                    irmp_start_bit_detected = 0;                    // reset flags and wait for next start bit
                                    irmp_pause_time         = 0;
                                }
                            }
                            else
                            {
                                if (irmp_pause_time >= BANG_OLUFSEN_1_PAUSE_LEN_MIN && irmp_pause_time <= BANG_OLUFSEN_1_PAUSE_LEN_MAX)
                                {                                                   // pulse & pause timings correct for "1"?
                                    ANALYZE_PUTCHAR ('1');
                                    ANALYZE_NEWLINE ();
                                    irmp_store_bit (1);
                                    last_value = 1;
                                    wait_for_space = 0;
                                }
                                else if (irmp_pause_time >= BANG_OLUFSEN_0_PAUSE_LEN_MIN && irmp_pause_time <= BANG_OLUFSEN_0_PAUSE_LEN_MAX)
                                {                                                   // pulse & pause timings correct for "0"?
                                    ANALYZE_PUTCHAR ('0');
                                    ANALYZE_NEWLINE ();
                                    irmp_store_bit (0);
                                    last_value = 0;
                                    wait_for_space = 0;
                                }
                                else if (irmp_pause_time >= BANG_OLUFSEN_R_PAUSE_LEN_MIN && irmp_pause_time <= BANG_OLUFSEN_R_PAUSE_LEN_MAX)
                                {
                                    ANALYZE_PUTCHAR (last_value + '0');
                                    ANALYZE_NEWLINE ();
                                    irmp_store_bit (last_value);
                                    wait_for_space = 0;
                                }
                                else
                                {                                                   // timing incorrect!
                                    ANALYZE_PRINTF ("error 3c B&O: timing not correct: data bit %d,  pulse: %d, pause: %d\n", irmp_bit, irmp_pulse_time, irmp_pause_time);
                                    ANALYZE_ONLY_NORMAL_PUTCHAR ('\n');
                                    irmp_start_bit_detected = 0;                    // reset flags and wait for next start bit
                                    irmp_pause_time         = 0;
                                }
                            }
                        }
                        else
                        {                                                           // timing incorrect!
                            ANALYZE_PRINTF ("error 3d B&O: timing not correct: data bit %d,  pulse: %d, pause: %d\n", irmp_bit, irmp_pulse_time, irmp_pause_time);
                            ANALYZE_ONLY_NORMAL_PUTCHAR ('\n');
                            irmp_start_bit_detected = 0;                            // reset flags and wait for next start bit
                            irmp_pause_time         = 0;
                        }
                    }
                    else
#endif // IRMP_SUPPORT_BANG_OLUFSEN_PROTOCOL

                    if (irmp_pulse_time >= irmp_param.pulse_1_len_min && irmp_pulse_time <= irmp_param.pulse_1_len_max &&
                        irmp_pause_time >= irmp_param.pause_1_len_min && irmp_pause_time <= irmp_param.pause_1_len_max)
                    {                                                               // pulse & pause timings correct for "1"?
                        ANALYZE_PUTCHAR ('1');
                        ANALYZE_NEWLINE ();
                        irmp_store_bit (1);
                        wait_for_space = 0;
                    }
                    else if (irmp_pulse_time >= irmp_param.pulse_0_len_min && irmp_pulse_time <= irmp_param.pulse_0_len_max &&
                             irmp_pause_time >= irmp_param.pause_0_len_min && irmp_pause_time <= irmp_param.pause_0_len_max)
                    {                                                               // pulse & pause timings correct for "0"?
                        ANALYZE_PUTCHAR ('0');
                        ANALYZE_NEWLINE ();
                        irmp_store_bit (0);
                        wait_for_space = 0;
                    }
                    else
                    {                                                               // timing incorrect!
                        ANALYZE_PRINTF ("error 3: timing not correct: data bit %d,  pulse: %d, pause: %d\n", irmp_bit, irmp_pulse_time, irmp_pause_time);
                        ANALYZE_ONLY_NORMAL_PUTCHAR ('\n');
                        irmp_start_bit_detected = 0;                                // reset flags and wait for next start bit
                        irmp_pause_time         = 0;
                    }

                    irmp_pulse_time = 1;                                            // set counter to 1, not 0
                }
            }
            else
            {                                                                       // counting the pulse length ...
                if (! irmp_input)                                                   // still light?
                {                                                                   // yes...
                    irmp_pulse_time++;                                              // increment counter
                }
                else
                {                                                                   // now it's dark!
                    wait_for_space  = 1;                                            // let's count the time (see above)
                    irmp_pause_time = 1;                                            // set pause counter to 1, not 0
                }
            }

            if (irmp_start_bit_detected && irmp_bit == irmp_param.complete_len && irmp_param.stop_bit == 0)    // enough bits received?
            {
                if (last_irmp_command == irmp_tmp_command && repetition_len < AUTO_FRAME_REPETITION_LEN)
                {
                    repetition_frame_number++;
                }
                else
                {
                    repetition_frame_number = 0;
                }

#if IRMP_SUPPORT_SIRCS_PROTOCOL == 1
                // if SIRCS protocol and the code will be repeated within 50 ms, we will ignore 2nd and 3rd repetition frame
                if (irmp_param.protocol == IRMP_SIRCS_PROTOCOL && (repetition_frame_number == 1 || repetition_frame_number == 2))
                {
                    ANALYZE_PRINTF ("code skipped: SIRCS auto repetition frame #%d, counter = %d, auto repetition len = %d\n",
                                    repetition_frame_number + 1, repetition_len, AUTO_FRAME_REPETITION_LEN);
                    repetition_len = 0;
                }
                else
#endif

#if IRMP_SUPPORT_KASEIKYO_PROTOCOL == 1
                // if KASEIKYO protocol and the code will be repeated within 50 ms, we will ignore 2nd repetition frame
                if (irmp_param.protocol == IRMP_KASEIKYO_PROTOCOL && repetition_frame_number == 1)
                {
                    ANALYZE_PRINTF ("code skipped: KASEIKYO auto repetition frame #%d, counter = %d, auto repetition len = %d\n",
                                    repetition_frame_number + 1, repetition_len, AUTO_FRAME_REPETITION_LEN);
                    repetition_len = 0;
                }
                else
#endif

#if IRMP_SUPPORT_SAMSUNG_PROTOCOL == 1
                // if SAMSUNG32 protocol and the code will be repeated within 50 ms, we will ignore every 2nd frame
                if (irmp_param.protocol == IRMP_SAMSUNG32_PROTOCOL && (repetition_frame_number & 0x01))
                {
                    ANALYZE_PRINTF ("code skipped: SAMSUNG32 auto repetition frame #%d, counter = %d, auto repetition len = %d\n",
                                    repetition_frame_number + 1, repetition_len, AUTO_FRAME_REPETITION_LEN);
                    repetition_len = 0;
                }
                else
#endif

#if IRMP_SUPPORT_NUBERT_PROTOCOL == 1
                // if NUBERT protocol and the code will be repeated within 50 ms, we will ignore every 2nd frame
                if (irmp_param.protocol == IRMP_NUBERT_PROTOCOL && (repetition_frame_number & 0x01))
                {
                    ANALYZE_PRINTF ("code skipped: NUBERT auto repetition frame #%d, counter = %d, auto repetition len = %d\n",
                                    repetition_frame_number + 1, repetition_len, AUTO_FRAME_REPETITION_LEN);
                    repetition_len = 0;
                }
                else
#endif

                {
                    ANALYZE_PRINTF ("%8d code detected, length = %d\n", time_counter, irmp_bit);
                    irmp_ir_detected = TRUE;

#if IRMP_SUPPORT_DENON_PROTOCOL == 1
                    if (irmp_param.protocol == IRMP_DENON_PROTOCOL)
                    {                                                               // check for repetition frame
                        if ((~irmp_tmp_command & 0x3FF) == last_irmp_denon_command) // command bits must be inverted
                        {
                            irmp_tmp_command = last_irmp_denon_command;             // use command received before!

                            irmp_protocol = irmp_param.protocol;                    // store protocol
                            irmp_address = irmp_tmp_address;                        // store address
                            irmp_command = irmp_tmp_command ;                       // store command
                        }
                        else
                        {
                            ANALYZE_PRINTF ("waiting for inverted command repetition\n");
                            irmp_ir_detected = FALSE;
                            last_irmp_denon_command = irmp_tmp_command;
                        }
                    }
                    else
#endif // IRMP_SUPPORT_DENON_PROTOCOL

#if IRMP_SUPPORT_GRUNDIG_PROTOCOL == 1
                    if (irmp_param.protocol == IRMP_GRUNDIG_PROTOCOL && irmp_tmp_command == 0x01ff)
                    {                                                               // Grundig start frame?
                        ANALYZE_PRINTF ("Detected GRUNDIG start frame, ignoring it\n");
                        irmp_ir_detected = FALSE;
                    }
                    else
#endif // IRMP_SUPPORT_GRUNDIG_PROTOCOL

#if IRMP_SUPPORT_NOKIA_PROTOCOL == 1
                    if (irmp_param.protocol == IRMP_NOKIA_PROTOCOL && irmp_tmp_address == 0x00ff && irmp_tmp_command == 0x00fe)
                    {                                                               // Nokia start frame?
                        ANALYZE_PRINTF ("Detected NOKIA start frame, ignoring it\n");
                        irmp_ir_detected = FALSE;
                    }
                    else
#endif // IRMP_SUPPORT_NOKIA_PROTOCOL
                    {
#if IRMP_SUPPORT_NEC_PROTOCOL == 1
                        if (irmp_param.protocol == IRMP_NEC_PROTOCOL && irmp_bit == 0)  // repetition frame
                        {
                            if (repetition_len < NEC_FRAME_REPEAT_PAUSE_LEN_MAX)
                            {
                                ANALYZE_PRINTF ("Detected NEC repetition frame, repetition_len = %d\n", repetition_len);
                                irmp_tmp_address = last_irmp_address;                   // address is last address
                                irmp_tmp_command = last_irmp_command;                   // command is last command
                                irmp_flags |= IRMP_FLAG_REPETITION;
                                repetition_len = 0;
                            }
                            else
                            {
                                ANALYZE_PRINTF ("Detected NEC repetition frame, ignoring it: timeout occured, repetition_len = %d > %d\n",
                                                repetition_len, NEC_FRAME_REPEAT_PAUSE_LEN_MAX);
                                irmp_ir_detected = FALSE;
                            }
                        }
#endif // IRMP_SUPPORT_NEC_PROTOCOL

#if IRMP_SUPPORT_KASEIKYO_PROTOCOL == 1
                        if (irmp_param.protocol == IRMP_KASEIKYO_PROTOCOL)
                        {
                            uint8_t xor;
                            // ANALYZE_PRINTF ("0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n",
                            //                 xor_check[0], xor_check[1], xor_check[2], xor_check[3], xor_check[4], xor_check[5]);

                            xor = (xor_check[0] & 0x0F) ^ ((xor_check[0] & 0xF0) >> 4) ^ (xor_check[1] & 0x0F) ^ ((xor_check[1] & 0xF0) >> 4);

                            if (xor != (xor_check[2] & 0x0F))
                            {
                                ANALYZE_PRINTF ("error 4: wrong XOR check for customer id: 0x%1x 0x%1x\n", xor, xor_check[2] & 0x0F);
                                irmp_ir_detected = FALSE;
                            }

                            xor = xor_check[2] ^ xor_check[3] ^ xor_check[4];

                            if (xor != xor_check[5])
                            {
                                ANALYZE_PRINTF ("error 4: wrong XOR check for data bits: 0x%02x 0x%02x\n", xor, xor_check[5]);
                                irmp_ir_detected = FALSE;
                            }
                        }
#endif // IRMP_SUPPORT_KASEIKYO_PROTOCOL == 1

#if IRMP_SUPPORT_RC6_PROTOCOL == 1
                        if (irmp_param.protocol == IRMP_RC6_PROTOCOL && irmp_param.complete_len == RC6_COMPLETE_DATA_LEN_LONG)     // RC6 mode = 6?
                        {
                            irmp_protocol = IRMP_RC6A_PROTOCOL;
                        }
                        else
#endif // IRMP_SUPPORT_RC6_PROTOCOL == 1

                        irmp_protocol = irmp_param.protocol;

#if IRMP_SUPPORT_FDC_PROTOCOL == 1
                        if (irmp_param.protocol == IRMP_FDC_PROTOCOL)
                        {
                            if (irmp_tmp_command & 0x000F)                          // released key?
                            {
                                irmp_tmp_command = (irmp_tmp_command >> 4) | 0x80;  // yes, set bit 7
                            }
                            else
                            {
                                irmp_tmp_command >>= 4;                             // no, it's a pressed key
                            }
                            irmp_tmp_command |= (irmp_tmp_address << 2) & 0x0F00;   // 000000CCCCAAAAAA -> 0000CCCC00000000
                            irmp_tmp_address &= 0x003F;
                        }
#endif

                        irmp_address = irmp_tmp_address;                            // store address
#if IRMP_SUPPORT_NEC_PROTOCOL == 1
                        last_irmp_address = irmp_tmp_address;                       // store as last address, too
#endif

#if IRMP_SUPPORT_RC5_PROTOCOL == 1
                        if (irmp_param.protocol == IRMP_RC5_PROTOCOL)
                        {
                            irmp_tmp_command |= rc5_cmd_bit6;                       // store bit 6
                        }
#endif
                        irmp_command = irmp_tmp_command;                            // store command

#if IRMP_SUPPORT_SAMSUNG_PROTOCOL == 1
                        irmp_id = irmp_tmp_id;
#endif
                    }
                }

                if (irmp_ir_detected)
                {
                    if (last_irmp_command == irmp_command &&
                        last_irmp_address == irmp_address &&
                        repetition_len < IRMP_KEY_REPETITION_LEN)
                    {
                        irmp_flags |= IRMP_FLAG_REPETITION;
                    }

                    last_irmp_address = irmp_tmp_address;                           // store as last address, too
                    last_irmp_command = irmp_tmp_command;                           // store as last command, too

                    repetition_len = 0;
                }
                else
                {
                    ANALYZE_ONLY_NORMAL_PUTCHAR ('\n');
                }

                irmp_start_bit_detected = 0;                                        // and wait for next start bit
                irmp_tmp_command        = 0;
                irmp_pulse_time         = 0;
                irmp_pause_time         = 0;

#if IRMP_SUPPORT_JVC_PROTOCOL == 1
                if (irmp_protocol == IRMP_JVC_PROTOCOL)                             // the stop bit of JVC frame is also start bit of next frame
                {                                                                   // set pulse time here!
                    irmp_pulse_time = ((uint8_t)(F_INTERRUPTS * JVC_START_BIT_PULSE_TIME));
                }
#endif // IRMP_SUPPORT_JVC_PROTOCOL == 1
            }
        }
    }
    return (irmp_ir_detected);
}

#ifdef ANALYZE

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * main functions - for Unix/Linux + Windows only!
 *
 * AVR: see main.c!
 *
 * Compile it under linux with:
 * cc irmp.c -o irmp
 *
 * usage: ./irmp [-v|-s|-a|-l|-p] < file
 *
 * options:
 *   -v verbose
 *   -s silent
 *   -a analyze
 *   -l list pulse/pauses
 *   -p print timings
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */

static void
print_timings (void)
{
    printf ("IRMP_TIMEOUT_LEN:        %d [%d byte(s)]\n", IRMP_TIMEOUT_LEN, sizeof (PAUSE_LEN));
    printf ("IRMP_KEY_REPETITION_LEN  %d\n", IRMP_KEY_REPETITION_LEN);
    puts ("");
    printf ("PROTOCOL       S  S-PULSE    S-PAUSE    PULSE-0    PAUSE-0    PULSE-1    PAUSE-1\n");
    printf ("====================================================================================\n");
    printf ("SIRCS          1  %3d - %3d  %3d - %3d  %3d - %3d  %3d - %3d  %3d - %3d  %3d - %3d\n",
            SIRCS_START_BIT_PULSE_LEN_MIN, SIRCS_START_BIT_PULSE_LEN_MAX, SIRCS_START_BIT_PAUSE_LEN_MIN, SIRCS_START_BIT_PAUSE_LEN_MAX,
            SIRCS_0_PULSE_LEN_MIN, SIRCS_0_PULSE_LEN_MAX, SIRCS_PAUSE_LEN_MIN, SIRCS_PAUSE_LEN_MAX,
            SIRCS_1_PULSE_LEN_MIN, SIRCS_1_PULSE_LEN_MAX, SIRCS_PAUSE_LEN_MIN, SIRCS_PAUSE_LEN_MAX);

    printf ("NEC            1  %3d - %3d  %3d - %3d  %3d - %3d  %3d - %3d  %3d - %3d  %3d - %3d\n",
            NEC_START_BIT_PULSE_LEN_MIN, NEC_START_BIT_PULSE_LEN_MAX, NEC_START_BIT_PAUSE_LEN_MIN, NEC_START_BIT_PAUSE_LEN_MAX,
            NEC_PULSE_LEN_MIN, NEC_PULSE_LEN_MAX, NEC_0_PAUSE_LEN_MIN, NEC_0_PAUSE_LEN_MAX,
            NEC_PULSE_LEN_MIN, NEC_PULSE_LEN_MAX, NEC_1_PAUSE_LEN_MIN, NEC_1_PAUSE_LEN_MAX);

    printf ("NEC (rep)      1  %3d - %3d  %3d - %3d  %3d - %3d  %3d - %3d  %3d - %3d  %3d - %3d\n",
            NEC_START_BIT_PULSE_LEN_MIN, NEC_START_BIT_PULSE_LEN_MAX, NEC_REPEAT_START_BIT_PAUSE_LEN_MIN, NEC_REPEAT_START_BIT_PAUSE_LEN_MAX,
            NEC_PULSE_LEN_MIN, NEC_PULSE_LEN_MAX, NEC_0_PAUSE_LEN_MIN, NEC_0_PAUSE_LEN_MAX,
            NEC_PULSE_LEN_MIN, NEC_PULSE_LEN_MAX, NEC_1_PAUSE_LEN_MIN, NEC_1_PAUSE_LEN_MAX);

    printf ("SAMSUNG        1  %3d - %3d  %3d - %3d  %3d - %3d  %3d - %3d  %3d - %3d  %3d - %3d\n",
            SAMSUNG_START_BIT_PULSE_LEN_MIN, SAMSUNG_START_BIT_PULSE_LEN_MAX, SAMSUNG_START_BIT_PAUSE_LEN_MIN, SAMSUNG_START_BIT_PAUSE_LEN_MAX,
            SAMSUNG_PULSE_LEN_MIN, SAMSUNG_PULSE_LEN_MAX, SAMSUNG_0_PAUSE_LEN_MIN, SAMSUNG_0_PAUSE_LEN_MAX,
            SAMSUNG_PULSE_LEN_MIN, SAMSUNG_PULSE_LEN_MAX, SAMSUNG_1_PAUSE_LEN_MIN, SAMSUNG_1_PAUSE_LEN_MAX);

    printf ("MATSUSHITA     1  %3d - %3d  %3d - %3d  %3d - %3d  %3d - %3d  %3d - %3d  %3d - %3d\n",
            MATSUSHITA_START_BIT_PULSE_LEN_MIN, MATSUSHITA_START_BIT_PULSE_LEN_MAX, MATSUSHITA_START_BIT_PAUSE_LEN_MIN, MATSUSHITA_START_BIT_PAUSE_LEN_MAX,
            MATSUSHITA_PULSE_LEN_MIN, MATSUSHITA_PULSE_LEN_MAX, MATSUSHITA_0_PAUSE_LEN_MIN, MATSUSHITA_0_PAUSE_LEN_MAX,
            MATSUSHITA_PULSE_LEN_MIN, MATSUSHITA_PULSE_LEN_MAX, MATSUSHITA_1_PAUSE_LEN_MIN, MATSUSHITA_1_PAUSE_LEN_MAX);

    printf ("KASEIKYO       1  %3d - %3d  %3d - %3d  %3d - %3d  %3d - %3d  %3d - %3d  %3d - %3d\n",
            KASEIKYO_START_BIT_PULSE_LEN_MIN, KASEIKYO_START_BIT_PULSE_LEN_MAX, KASEIKYO_START_BIT_PAUSE_LEN_MIN, KASEIKYO_START_BIT_PAUSE_LEN_MAX,
            KASEIKYO_PULSE_LEN_MIN, KASEIKYO_PULSE_LEN_MAX, KASEIKYO_0_PAUSE_LEN_MIN, KASEIKYO_0_PAUSE_LEN_MAX,
            KASEIKYO_PULSE_LEN_MIN, KASEIKYO_PULSE_LEN_MAX, KASEIKYO_1_PAUSE_LEN_MIN, KASEIKYO_1_PAUSE_LEN_MAX);

    printf ("RECS80         1  %3d - %3d  %3d - %3d  %3d - %3d  %3d - %3d  %3d - %3d  %3d - %3d\n",
            RECS80_START_BIT_PULSE_LEN_MIN, RECS80_START_BIT_PULSE_LEN_MAX, RECS80_START_BIT_PAUSE_LEN_MIN, RECS80_START_BIT_PAUSE_LEN_MAX,
            RECS80_PULSE_LEN_MIN, RECS80_PULSE_LEN_MAX, RECS80_0_PAUSE_LEN_MIN, RECS80_0_PAUSE_LEN_MAX,
            RECS80_PULSE_LEN_MIN, RECS80_PULSE_LEN_MAX, RECS80_1_PAUSE_LEN_MIN, RECS80_1_PAUSE_LEN_MAX);

    printf ("RC5            1  %3d - %3d  %3d - %3d  %3d - %3d\n",
            RC5_START_BIT_LEN_MIN, RC5_START_BIT_LEN_MAX, RC5_START_BIT_LEN_MIN, RC5_START_BIT_LEN_MAX,
            RC5_BIT_LEN_MIN, RC5_BIT_LEN_MAX);

    printf ("DENON          1  %3d - %3d             %3d - %3d  %3d - %3d  %3d - %3d  %3d - %3d\n",
            DENON_PULSE_LEN_MIN, DENON_PULSE_LEN_MAX,
            DENON_PULSE_LEN_MIN, DENON_PULSE_LEN_MAX, DENON_0_PAUSE_LEN_MIN, DENON_0_PAUSE_LEN_MAX,
            DENON_PULSE_LEN_MIN, DENON_PULSE_LEN_MAX, DENON_1_PAUSE_LEN_MIN, DENON_1_PAUSE_LEN_MAX);

    printf ("RC6            1  %3d - %3d  %3d - %3d  %3d - %3d  %3d - %3d\n",
            RC6_START_BIT_PULSE_LEN_MIN, RC6_START_BIT_PULSE_LEN_MAX, RC6_START_BIT_PAUSE_LEN_MIN, RC6_START_BIT_PAUSE_LEN_MAX,
            RC6_BIT_PULSE_LEN_MIN, RC6_BIT_PULSE_LEN_MAX, RC6_BIT_PAUSE_LEN_MIN, RC6_BIT_PAUSE_LEN_MAX);

    printf ("RECS80EXT      1  %3d - %3d  %3d - %3d  %3d - %3d  %3d - %3d  %3d - %3d  %3d - %3d\n",
            RECS80EXT_START_BIT_PULSE_LEN_MIN, RECS80EXT_START_BIT_PULSE_LEN_MAX, RECS80EXT_START_BIT_PAUSE_LEN_MIN, RECS80EXT_START_BIT_PAUSE_LEN_MAX,
            RECS80EXT_PULSE_LEN_MIN, RECS80EXT_PULSE_LEN_MAX, RECS80EXT_0_PAUSE_LEN_MIN, RECS80EXT_0_PAUSE_LEN_MAX,
            RECS80EXT_PULSE_LEN_MIN, RECS80EXT_PULSE_LEN_MAX, RECS80EXT_1_PAUSE_LEN_MIN, RECS80EXT_1_PAUSE_LEN_MAX);

    printf ("NUBERT         1  %3d - %3d  %3d - %3d  %3d - %3d  %3d - %3d  %3d - %3d  %3d - %3d\n",
            NUBERT_START_BIT_PULSE_LEN_MIN, NUBERT_START_BIT_PULSE_LEN_MAX, NUBERT_START_BIT_PAUSE_LEN_MIN, NUBERT_START_BIT_PAUSE_LEN_MAX,
            NUBERT_0_PULSE_LEN_MIN, NUBERT_0_PULSE_LEN_MAX, NUBERT_0_PAUSE_LEN_MIN, NUBERT_0_PAUSE_LEN_MAX,
            NUBERT_1_PULSE_LEN_MIN, NUBERT_1_PULSE_LEN_MAX, NUBERT_1_PAUSE_LEN_MIN, NUBERT_1_PAUSE_LEN_MAX);

    printf ("BANG_OLUFSEN   1  %3d - %3d  %3d - %3d\n",
            BANG_OLUFSEN_START_BIT1_PULSE_LEN_MIN, BANG_OLUFSEN_START_BIT1_PULSE_LEN_MAX,
            BANG_OLUFSEN_START_BIT1_PAUSE_LEN_MIN, BANG_OLUFSEN_START_BIT1_PAUSE_LEN_MAX);

    printf ("BANG_OLUFSEN   2  %3d - %3d  %3d - %3d\n",
            BANG_OLUFSEN_START_BIT2_PULSE_LEN_MIN, BANG_OLUFSEN_START_BIT2_PULSE_LEN_MAX,
            BANG_OLUFSEN_START_BIT2_PAUSE_LEN_MIN, BANG_OLUFSEN_START_BIT2_PAUSE_LEN_MAX);

    printf ("BANG_OLUFSEN   3  %3d - %3d  %3d - %3d\n",
            BANG_OLUFSEN_START_BIT3_PULSE_LEN_MIN, BANG_OLUFSEN_START_BIT3_PULSE_LEN_MAX,
            BANG_OLUFSEN_START_BIT3_PAUSE_LEN_MIN, BANG_OLUFSEN_START_BIT3_PAUSE_LEN_MAX);

    printf ("BANG_OLUFSEN   4  %3d - %3d  %3d - %3d\n",
            BANG_OLUFSEN_START_BIT4_PULSE_LEN_MIN, BANG_OLUFSEN_START_BIT4_PULSE_LEN_MAX,
            BANG_OLUFSEN_START_BIT4_PAUSE_LEN_MIN, BANG_OLUFSEN_START_BIT4_PAUSE_LEN_MAX);

    printf ("BANG_OLUFSEN   -                        %3d - %3d  %3d - %3d  %3d - %3d  %3d - %3d\n",
            BANG_OLUFSEN_PULSE_LEN_MIN, BANG_OLUFSEN_PULSE_LEN_MAX, BANG_OLUFSEN_0_PAUSE_LEN_MIN, BANG_OLUFSEN_0_PAUSE_LEN_MAX,
            BANG_OLUFSEN_PULSE_LEN_MIN, BANG_OLUFSEN_PULSE_LEN_MAX, BANG_OLUFSEN_1_PAUSE_LEN_MIN, BANG_OLUFSEN_1_PAUSE_LEN_MAX);

    printf ("GRUNDIG/NOKIA  1  %3d - %3d  %3d - %3d  %3d - %3d\n",
            GRUNDIG_OR_NOKIA_START_BIT_LEN_MIN, GRUNDIG_OR_NOKIA_START_BIT_LEN_MAX, GRUNDIG_OR_NOKIA_PRE_PAUSE_LEN_MIN, GRUNDIG_OR_NOKIA_PRE_PAUSE_LEN_MAX,
            GRUNDIG_OR_NOKIA_BIT_LEN_MIN, GRUNDIG_OR_NOKIA_BIT_LEN_MAX);

    printf ("SIEMENS        1  %3d - %3d  %3d - %3d  %3d - %3d\n",
            SIEMENS_START_BIT_LEN_MIN, SIEMENS_START_BIT_LEN_MAX, SIEMENS_START_BIT_LEN_MIN, SIEMENS_START_BIT_LEN_MAX,
            SIEMENS_BIT_LEN_MIN, SIEMENS_BIT_LEN_MAX);

    printf ("FDC            1  %3d - %3d  %3d - %3d  %3d - %3d  %3d - %3d  %3d - %3d  %3d - %3d\n",
            FDC_START_BIT_PULSE_LEN_MIN, FDC_START_BIT_PULSE_LEN_MAX, FDC_START_BIT_PAUSE_LEN_MIN, FDC_START_BIT_PAUSE_LEN_MAX,
            FDC_PULSE_LEN_MIN, FDC_PULSE_LEN_MAX, FDC_0_PAUSE_LEN_MIN, FDC_0_PAUSE_LEN_MAX,
            FDC_PULSE_LEN_MIN, FDC_PULSE_LEN_MAX, FDC_1_PAUSE_LEN_MIN, FDC_1_PAUSE_LEN_MAX);

    printf ("RCCAR          1  %3d - %3d  %3d - %3d  %3d - %3d  %3d - %3d  %3d - %3d  %3d - %3d\n",
            RCCAR_START_BIT_PULSE_LEN_MIN, RCCAR_START_BIT_PULSE_LEN_MAX, RCCAR_START_BIT_PAUSE_LEN_MIN, RCCAR_START_BIT_PAUSE_LEN_MAX,
            RCCAR_PULSE_LEN_MIN, RCCAR_PULSE_LEN_MAX, RCCAR_0_PAUSE_LEN_MIN, RCCAR_0_PAUSE_LEN_MAX,
            RCCAR_PULSE_LEN_MIN, RCCAR_PULSE_LEN_MAX, RCCAR_1_PAUSE_LEN_MIN, RCCAR_1_PAUSE_LEN_MAX);
}

void
print_spectrum (char * text, int * buf, int is_pulse)
{
    int     i;
    int     j;
    int     min;
    int     max;
    int     max_value = 0;
    int     value;
    int     sum = 0;
    int     counter = 0;
    double  average = 0;
    double  tolerance;

    puts ("-------------------------------------------------------------------------------");
    printf ("%s:\n", text);

    for (i = 0; i < 256; i++)
    {
        if (buf[i] > max_value)
        {
            max_value = buf[i];
        }
    }

    for (i = 0; i < 100; i++)
    {
        if (buf[i] > 0)
        {
            printf ("%3d ", i);
            value = (buf[i] * 60) / max_value;

            for (j = 0; j < value; j++)
            {
                putchar ('o');
            }
            printf (" %d\n", buf[i]);

            sum += i * buf[i];
            counter += buf[i];
        }
        else
        {
            max = i - 1;

            if (counter > 0)
            {
                average = (float) sum / (float) counter;

                if (is_pulse)
                {
                    printf ("pulse ");
                }
                else
                {
                    printf ("pause ");
                }

                printf ("avg: %4.1f=%6.1f us, ", average, (1000000. * average) / (float) F_INTERRUPTS);
                printf ("min: %2d=%6.1f us, ", min, (1000000. * min) / (float) F_INTERRUPTS);
                printf ("max: %2d=%6.1f us, ", max, (1000000. * max) / (float) F_INTERRUPTS);

                tolerance = (max - average);

                if (average - min > tolerance)
                {
                    tolerance = average - min;
                }

                tolerance = tolerance * 100 / average;
                printf ("tol: %4.1f%%\n", tolerance);
            }

            counter = 0;
            sum = 0;
            min = i + 1;
        }
    }
}

#define STATE_LEFT_SHIFT    0x01
#define STATE_RIGHT_SHIFT   0x02
#define STATE_LEFT_CTRL     0x04
#define STATE_LEFT_ALT      0x08
#define STATE_RIGHT_ALT     0x10

#define KEY_ESCAPE          0x1B            // keycode = 0x006e
#define KEY_MENUE           0x80            // keycode = 0x0070
#define KEY_BACK            0x81            // keycode = 0x0071
#define KEY_FORWARD         0x82            // keycode = 0x0072
#define KEY_ADDRESS         0x83            // keycode = 0x0073
#define KEY_WINDOW          0x84            // keycode = 0x0074
#define KEY_1ST_PAGE        0x85            // keycode = 0x0075
#define KEY_STOP            0x86            // keycode = 0x0076
#define KEY_MAIL            0x87            // keycode = 0x0077
#define KEY_FAVORITES       0x88            // keycode = 0x0078
#define KEY_NEW_PAGE        0x89            // keycode = 0x0079
#define KEY_SETUP           0x8A            // keycode = 0x007a
#define KEY_FONT            0x8B            // keycode = 0x007b
#define KEY_PRINT           0x8C            // keycode = 0x007c
#define KEY_ON_OFF          0x8E            // keycode = 0x007c

#define KEY_INSERT          0x90            // keycode = 0x004b
#define KEY_DELETE          0x91            // keycode = 0x004c
#define KEY_LEFT            0x92            // keycode = 0x004f
#define KEY_HOME            0x93            // keycode = 0x0050
#define KEY_END             0x94            // keycode = 0x0051
#define KEY_UP              0x95            // keycode = 0x0053
#define KEY_DOWN            0x96            // keycode = 0x0054
#define KEY_PAGE_UP         0x97            // keycode = 0x0055
#define KEY_PAGE_DOWN       0x98            // keycode = 0x0056
#define KEY_RIGHT           0x99            // keycode = 0x0059
#define KEY_MOUSE_1         0x9E            // keycode = 0x0400
#define KEY_MOUSE_2         0x9F            // keycode = 0x0800

static uint8_t
get_fdc_key (uint16_t cmd)
{
    static uint8_t key_table[128] =
    {
     // 0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F
        0,  '^', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', 'ß', '´',  0,  '\b',
       '\t','q', 'w', 'e', 'r', 't', 'z', 'u', 'i', 'o', 'p', 'ü', '+',  0,   0,  'a',
       's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', 'ö', 'ä', '#',  '\r', 0,  '<', 'y', 'x',
       'c', 'v', 'b', 'n', 'm', ',', '.', '-',  0,   0,   0,   0,   0,  ' ',  0,   0,

        0,  '°', '!', '"', '§', '$', '%', '&', '/', '(', ')', '=', '?', '`',  0,  '\b',
       '\t','Q', 'W', 'E', 'R', 'T', 'Z', 'U', 'I', 'O', 'P', 'Ü', '*',  0,   0,  'A',
       'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', 'Ö', 'Ä', '\'','\r', 0,  '>', 'Y', 'X',
       'C', 'V', 'B', 'N', 'M', ';', ':', '_',  0,   0,   0,   0,   0,  ' ',  0,   0
    };
    static uint8_t state;

    uint8_t key = 0;

    switch (cmd)
    {
        case 0x002C: state |=  STATE_LEFT_SHIFT;    break;              // pressed left shift
        case 0x00AC: state &= ~STATE_LEFT_SHIFT;    break;              // released left shift
        case 0x0039: state |=  STATE_RIGHT_SHIFT;   break;              // pressed right shift
        case 0x00B9: state &= ~STATE_RIGHT_SHIFT;   break;              // released right shift
        case 0x003A: state |=  STATE_LEFT_CTRL;     break;              // pressed left ctrl
        case 0x00BA: state &= ~STATE_LEFT_CTRL;     break;              // released left ctrl
        case 0x003C: state |=  STATE_LEFT_ALT;      break;              // pressed left alt
        case 0x00BC: state &= ~STATE_LEFT_ALT;      break;              // released left alt
        case 0x003E: state |=  STATE_RIGHT_ALT;     break;              // pressed left alt
        case 0x00BE: state &= ~STATE_RIGHT_ALT;     break;              // released left alt

        case 0x006e: key = KEY_ESCAPE;              break;
        case 0x004b: key = KEY_INSERT;              break;
        case 0x004c: key = KEY_DELETE;              break;
        case 0x004f: key = KEY_LEFT;                break;
        case 0x0050: key = KEY_HOME;                break;
        case 0x0051: key = KEY_END;                 break;
        case 0x0053: key = KEY_UP;                  break;
        case 0x0054: key = KEY_DOWN;                break;
        case 0x0055: key = KEY_PAGE_UP;             break;
        case 0x0056: key = KEY_PAGE_DOWN;           break;
        case 0x0059: key = KEY_RIGHT;               break;
        case 0x0400: key = KEY_MOUSE_1;             break;
        case 0x0800: key = KEY_MOUSE_2;             break;

        default:
        {
            if (!(cmd & 0x80))                      // pressed key
            {
                if (cmd >= 0x70 && cmd <= 0x7F)     // function keys
                {
                    key = cmd + 0x10;               // 7x -> 8x
                }
                else if (cmd < 64)                  // key listed in key_table
                {
                    if (state & (STATE_LEFT_ALT | STATE_RIGHT_ALT))
                    {
                        switch (cmd)
                        {
                            case 0x0003: key = '²';     break;
                            case 0x0008: key = '{';     break;
                            case 0x0009: key = '[';     break;
                            case 0x000A: key = ']';     break;
                            case 0x000B: key = '}';     break;
                            case 0x000C: key = '\\';    break;
                            case 0x001C: key = '~';     break;
                            case 0x002D: key = '|';     break;
                            case 0x0034: key = 'µ';     break;
                        }
                    }
                    else if (state & (STATE_LEFT_CTRL))
                    {
                        if (key_table[cmd] >= 'a' && key_table[cmd] <= 'z')
                        {
                            key = key_table[cmd] - 'a' + 1;
                        }
                        else
                        {
                            key = key_table[cmd];
                        }
                    }
                    else
                    {
                        int idx = cmd + ((state & (STATE_LEFT_SHIFT | STATE_RIGHT_SHIFT)) ? 64 : 0);

                        if (key_table[idx])
                        {
                            key = key_table[idx];
                        }
                    }
                }
            }
            break;
        }
    }

    return (key);
}

static int         analyze = FALSE;
static int         list = FALSE;
static IRMP_DATA   irmp_data;

static void
next_tick (void)
{
    if (! analyze && ! list)
    {
        (void) irmp_ISR ();

        if (irmp_get_data (&irmp_data))
        {
            uint8_t key;

            ANALYZE_ONLY_NORMAL_PUTCHAR (' ');

            if (verbose)
            {
                printf ("%8d ", time_counter);
            }

            if (irmp_data.protocol == IRMP_FDC_PROTOCOL && (key = get_fdc_key (irmp_data.command)) != 0)
            {
                if ((key >= 0x20 && key < 0x7F) || key >= 0xA0)
                {
                    printf ("p = %2d, a = 0x%04x, c = 0x%04x, f = 0x%02x, asc = 0x%02x, key = '%c'\n",
                            irmp_data.protocol, irmp_data.address, irmp_data.command, irmp_data.flags, key, key);
                }
                else if (key == '\r' || key == '\t' || key == KEY_ESCAPE || (key >= 0x80 && key <= 0x9F))                 // function keys
                {
                    char * p = (char *) NULL;

                    switch (key)
                    {
                        case '\t'                : p = "TAB";           break;
                        case '\r'                : p = "CR";            break;
                        case KEY_ESCAPE          : p = "ESCAPE";        break;
                        case KEY_MENUE           : p = "MENUE";         break;
                        case KEY_BACK            : p = "BACK";          break;
                        case KEY_FORWARD         : p = "FORWARD";       break;
                        case KEY_ADDRESS         : p = "ADDRESS";       break;
                        case KEY_WINDOW          : p = "WINDOW";        break;
                        case KEY_1ST_PAGE        : p = "1ST_PAGE";      break;
                        case KEY_STOP            : p = "STOP";          break;
                        case KEY_MAIL            : p = "MAIL";          break;
                        case KEY_FAVORITES       : p = "FAVORITES";     break;
                        case KEY_NEW_PAGE        : p = "NEW_PAGE";      break;
                        case KEY_SETUP           : p = "SETUP";         break;
                        case KEY_FONT            : p = "FONT";          break;
                        case KEY_PRINT           : p = "PRINT";         break;
                        case KEY_ON_OFF          : p = "ON_OFF";        break;

                        case KEY_INSERT          : p = "INSERT";        break;
                        case KEY_DELETE          : p = "DELETE";        break;
                        case KEY_LEFT            : p = "LEFT";          break;
                        case KEY_HOME            : p = "HOME";          break;
                        case KEY_END             : p = "END";           break;
                        case KEY_UP              : p = "UP";            break;
                        case KEY_DOWN            : p = "DOWN";          break;
                        case KEY_PAGE_UP         : p = "PAGE_UP";       break;
                        case KEY_PAGE_DOWN       : p = "PAGE_DOWN";     break;
                        case KEY_RIGHT           : p = "RIGHT";         break;
                        case KEY_MOUSE_1         : p = "KEY_MOUSE_1";   break;
                        case KEY_MOUSE_2         : p = "KEY_MOUSE_2";   break;
                        default                  : p = "<UNKNWON>";     break;
                    }

                    printf ("p = %2d, a = 0x%04x, c = 0x%04x, f = 0x%02x, asc = 0x%02x, key = %s\n",
                            irmp_data.protocol, irmp_data.address, irmp_data.command, irmp_data.flags, key, p);
                }
                else
                {
                    printf ("p = %2d, a = 0x%04x, c = 0x%04x, f = 0x%02x, asc = 0x%02x\n",
                            irmp_data.protocol, irmp_data.address, irmp_data.command, irmp_data.flags, key);
                }
            }
            else
            {
                printf ("p = %2d, a = 0x%04x, c = 0x%04x, f = 0x%02x\n",
                        irmp_data.protocol, irmp_data.address, irmp_data.command, irmp_data.flags);
            }
        }
    }
}

int
main (int argc, char ** argv)
{
    int         i;
    int         ch;
    int         last_ch = 0;
    int         pulse = 0;
    int         pause = 0;

    int         start_pulses[256];
    int         start_pauses[256];
    int         pulses[256];
    int         pauses[256];

    int         first_pulse = TRUE;
    int         first_pause = TRUE;

    if (argc == 2)
    {
        if (! strcmp (argv[1], "-v"))
        {
            verbose = TRUE;
        }
        else if (! strcmp (argv[1], "-l"))
        {
            list = TRUE;
        }
        else if (! strcmp (argv[1], "-a"))
        {
            analyze = TRUE;
        }
        else if (! strcmp (argv[1], "-s"))
        {
            silent = TRUE;
        }
        else if (! strcmp (argv[1], "-p"))
        {
            print_timings ();
            return (0);
        }
    }

    for (i = 0; i < 256; i++)
    {
        start_pulses[i] = 0;
        start_pauses[i] = 0;
        pulses[i] = 0;
        pauses[i] = 0;
    }

    IRMP_PIN = 0xFF;

    while ((ch = getchar ()) != EOF)
    {
        if (ch == '_' || ch == '0')
        {
            if (last_ch != ch)
            {
                if (pause > 0)
                {
                    if (list)
                    {
                        printf ("pause: %d\n", pause);
                    }

                    if (analyze)
                    {
                        if (first_pause)
                        {
                            if (pause < 256)
                            {
                                start_pauses[pause]++;
                            }
                            first_pause = FALSE;
                        }
                        else
                        {
                            if (pause < 256)
                            {
                                pauses[pause]++;
                            }
                        }
                    }
                }
                pause = 0;
            }
            pulse++;
            IRMP_PIN = 0x00;
        }
        else if (ch == 0xaf || ch == '-' || ch == '1')
        {
            if (last_ch != ch)
            {
                if (list)
                {
                    printf ("pulse: %d ", pulse);
                }

                if (analyze)
                {
                    if (first_pulse)
                    {
                        if (pulse < 256)
                        {
                            start_pulses[pulse]++;
                        }
                        first_pulse = FALSE;
                    }
                    else
                    {
                        if (pulse < 256)
                        {
                            pulses[pulse]++;
                        }
                    }
                }
                pulse = 0;
            }

            pause++;
            IRMP_PIN = 0xff;
        }
        else if (ch == '\n')
        {
            IRMP_PIN = 0xff;

            if (list && pause > 0)
            {
                printf ("pause: %d\n", pause);
            }
            pause = 0;

            if (! analyze)
            {
                for (i = 0; i < (int) ((8000.0 * F_INTERRUPTS) / 10000); i++)               // newline: long pause of 800 msec
                {
                    next_tick ();
                }
            }
            first_pulse = TRUE;
            first_pause = TRUE;
        }
        else if (ch == '#')
        {
            if (analyze)
            {
                while ((ch = getchar()) != '\n' && ch != EOF)
                {
                    ;
                }
            }
            else
            {
                puts ("-------------------------------------------------------------------");
                putchar (ch);

                while ((ch = getchar()) != '\n' && ch != EOF)
                {
                    if (ch != '\r')                                                         // ignore CR in DOS/Windows files
                    {
                        putchar (ch);
                    }
                }
                putchar ('\n');
            }

        }

        last_ch = ch;

        next_tick ();
    }

    if (analyze)
    {
        print_spectrum ("START PULSES", start_pulses, TRUE);
        print_spectrum ("START PAUSES", start_pauses, FALSE);
        print_spectrum ("PULSES", pulses, TRUE);
        print_spectrum ("PAUSES", pauses, FALSE);
        puts ("-------------------------------------------------------------------------------");
    }
    return 0;
}

#endif // ANALYZE
