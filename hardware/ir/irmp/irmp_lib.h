/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * irmp.h
 *
 * Copyright (c) 2009-2010 Frank Meyer - frank(at)fli4l.de
 *
 * $Id: irmp.h,v 1.43 2010/08/31 15:22:24 fm Exp $
 *
 * ATMEGA88 @ 8 MHz
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */

#ifndef _WC_IRMP_H_
#define _WC_IRMP_H_

#ifdef __cplusplus
extern "C"
{
#endif

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * timing constants:
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#define IRMP_TIMEOUT_TIME                       16500.0e-6                  // timeout after 16.5 ms darkness
#define IRMP_TIMEOUT_TIME_MS                    16500L                      // timeout after 16.5 ms darkness

#if (F_INTERRUPTS * IRMP_TIMEOUT_TIME_MS) / 1000000 >= 254
typedef uint16_t    PAUSE_LEN;
#else
typedef uint8_t     PAUSE_LEN;
#endif

#define IRMP_TIMEOUT_LEN                        (PAUSE_LEN)(F_INTERRUPTS * IRMP_TIMEOUT_TIME + 0.5)

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * IR protocols
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#define IRMP_SIRCS_PROTOCOL                      1              // Sony
#define IRMP_NEC_PROTOCOL                        2              // NEC, Pioneer, JVC, Toshiba, NoName etc.
#define IRMP_SAMSUNG_PROTOCOL                    3              // Samsung
#define IRMP_MATSUSHITA_PROTOCOL                 4              // Matsushita
#define IRMP_KASEIKYO_PROTOCOL                   5              // Kaseikyo (Panasonic etc)
#define IRMP_RECS80_PROTOCOL                     6              // Philips, Thomson, Nordmende, Telefunken, Saba
#define IRMP_RC5_PROTOCOL                        7              // Philips etc
#define IRMP_DENON_PROTOCOL                      8              // Denon
#define IRMP_RC6_PROTOCOL                        9              // Philips etc
#define IRMP_SAMSUNG32_PROTOCOL                 10              // Samsung32: no sync pulse at bit 16, length 32 instead of 37
#define IRMP_APPLE_PROTOCOL                     11              // Apple, very similar to NEC
#define IRMP_RECS80EXT_PROTOCOL                 12              // Philips, Technisat, Thomson, Nordmende, Telefunken, Saba
#define IRMP_NUBERT_PROTOCOL                    13              // Nubert
#define IRMP_BANG_OLUFSEN_PROTOCOL              14              // Bang & Olufsen
#define IRMP_GRUNDIG_PROTOCOL                   15              // Grundig
#define IRMP_NOKIA_PROTOCOL                     16              // Nokia
#define IRMP_SIEMENS_PROTOCOL                   17              // Siemens, e.g. Gigaset
#define IRMP_FDC_PROTOCOL                       18              // FDC keyboard
#define IRMP_RCCAR_PROTOCOL                     19              // RC Car
#define IRMP_JVC_PROTOCOL                       20              // JVC
#define IRMP_RC6A_PROTOCOL                      21              // RC6A, e.g. Kathrein, XBOX

// some flags of struct IRMP_PARAMETER:
#define IRMP_PARAM_FLAG_IS_MANCHESTER           0x01
#define IRMP_PARAM_FLAG_1ST_PULSE_IS_1          0x02

#define SIRCS_START_BIT_PULSE_TIME              2400.0e-6                       // 2400 usec pulse
#define SIRCS_START_BIT_PAUSE_TIME               600.0e-6                       //  600 usec pause
#define SIRCS_1_PULSE_TIME                      1200.0e-6                       // 1200 usec pulse
#define SIRCS_0_PULSE_TIME                       600.0e-6                       //  600 usec pulse
#define SIRCS_PAUSE_TIME                         600.0e-6                       //  600 usec pause
#define SIRCS_FRAMES                            3                               // SIRCS sends each frame 3 times
#define SIRCS_AUTO_REPETITION_PAUSE_TIME          25.0e-3                       // auto repetition after 25ms
#define SIRCS_FRAME_REPEAT_PAUSE_TIME             25.0e-3                       // frame repeat after 25ms
#define SIRCS_ADDRESS_OFFSET                    15                              // skip 15 bits
#define SIRCS_ADDRESS_LEN                       5                               // read up to 5 address bits
#define SIRCS_COMMAND_OFFSET                    0                               // skip 0 bits
#define SIRCS_COMMAND_LEN                       15                              // read 12-15 command bits
#define SIRCS_MINIMUM_DATA_LEN                  12                              // minimum data length
#define SIRCS_COMPLETE_DATA_LEN                 20                              // complete length - may be up to 20
#define SIRCS_STOP_BIT                          0                               // has no stop bit
#define SIRCS_LSB                               1                               // LSB...MSB
#define SIRCS_FLAGS                             0                               // flags

#define NEC_START_BIT_PULSE_TIME                9000.0e-6                       // 9000 usec pulse
#define NEC_START_BIT_PAUSE_TIME                4500.0e-6                       // 4500 usec pause
#define NEC_REPEAT_START_BIT_PAUSE_TIME         2250.0e-6                       // 2250 usec pause
#define NEC_PULSE_TIME                           560.0e-6                       //  560 usec pulse
#define NEC_1_PAUSE_TIME                        1690.0e-6                       // 1690 usec pause
#define NEC_0_PAUSE_TIME                         560.0e-6                       //  560 usec pause
#define NEC_FRAME_REPEAT_PAUSE_TIME               40.0e-3                       // frame repeat after 40ms
#define NEC_ADDRESS_OFFSET                       0                              // skip 0 bits
#define NEC_ADDRESS_LEN                         16                              // read 16 address bits
#define NEC_COMMAND_OFFSET                      16                              // skip 16 bits (8 address + 8 /address)
#define NEC_COMMAND_LEN                         16                              // read 16 bits (8 command + 8 /command)
#define NEC_COMPLETE_DATA_LEN                   32                              // complete length
#define NEC_STOP_BIT                            1                               // has stop bit
#define NEC_LSB                                 1                               // LSB...MSB
#define NEC_FLAGS                               0                               // flags

#define SAMSUNG_START_BIT_PULSE_TIME            4500.0e-6                       // 4500 usec pulse
#define SAMSUNG_START_BIT_PAUSE_TIME            4500.0e-6                       // 4500 usec pause
#define SAMSUNG_PULSE_TIME                       550.0e-6                       //  550 usec pulse
#define SAMSUNG_1_PAUSE_TIME                    1450.0e-6                       // 1450 usec pause
#define SAMSUNG_0_PAUSE_TIME                     450.0e-6                       //  450 usec pause
#define SAMSUNG_FRAME_REPEAT_PAUSE_TIME           45.0e-3                       // frame repeat after 45ms
#define SAMSUNG_ADDRESS_OFFSET                   0                              // skip 0 bits
#define SAMSUNG_ADDRESS_LEN                     16                              // read 16 address bits
#define SAMSUNG_ID_OFFSET                       17                              // skip 16 + 1 sync bit
#define SAMSUNG_ID_LEN                          4                               // read 4 id bits
#define SAMSUNG_COMMAND_OFFSET                  21                              // skip 16 + 1 sync + 4 data bits
#define SAMSUNG_COMMAND_LEN                     16                              // read 16 command bits
#define SAMSUNG_COMPLETE_DATA_LEN               37                              // complete length
#define SAMSUNG_STOP_BIT                        1                               // has stop bit
#define SAMSUNG_LSB                             1                               // LSB...MSB?
#define SAMSUNG_FLAGS                           0                               // flags

#define SAMSUNG32_COMMAND_OFFSET                16                              // skip 16 bits
#define SAMSUNG32_COMMAND_LEN                   16                              // read 16 command bits
#define SAMSUNG32_COMPLETE_DATA_LEN             32                              // complete length
#define SAMSUNG32_FRAMES                        2                               // SAMSUNG32 sends each frame 2 times
#define SAMSUNG32_AUTO_REPETITION_PAUSE_TIME    47.0e-3                         // repetition after 47 ms
#define SAMSUNG32_FRAME_REPEAT_PAUSE_TIME       47.0e-3                         // frame repeat after 40ms

#define MATSUSHITA_START_BIT_PULSE_TIME         3488.0e-6                       // 3488 usec pulse
#define MATSUSHITA_START_BIT_PAUSE_TIME         3488.0e-6                       // 3488 usec pause
#define MATSUSHITA_PULSE_TIME                    872.0e-6                       //  872 usec pulse
#define MATSUSHITA_1_PAUSE_TIME                 2616.0e-6                       // 2616 usec pause
#define MATSUSHITA_0_PAUSE_TIME                  872.0e-6                       //  872 usec pause
#define MATSUSHITA_FRAME_REPEAT_PAUSE_TIME        45.0e-3                       // frame repeat after 45ms
#define MATSUSHITA_ADDRESS_OFFSET               12                              // skip 12 bits
#define MATSUSHITA_ADDRESS_LEN                  12                              // read 12 address bits
#define MATSUSHITA_COMMAND_OFFSET               0                               // skip 0 bits
#define MATSUSHITA_COMMAND_LEN                  12                              // read 12 bits (6 custom + 6 command)
#define MATSUSHITA_COMPLETE_DATA_LEN            24                              // complete length
#define MATSUSHITA_STOP_BIT                     1                               // has stop bit
#define MATSUSHITA_LSB                          1                               // LSB...MSB?
#define MATSUSHITA_FLAGS                        0                               // flags

#define KASEIKYO_START_BIT_PULSE_TIME           3380.0e-6                       // 3380 usec pulse
#define KASEIKYO_START_BIT_PAUSE_TIME           1690.0e-6                       // 1690 usec pause
#define KASEIKYO_PULSE_TIME                      423.0e-6                       //  525 usec pulse
#define KASEIKYO_1_PAUSE_TIME                   1269.0e-6                       //  525 usec pause
#define KASEIKYO_0_PAUSE_TIME                    423.0e-6                       // 1690 usec pause
#define KASEIKYO_AUTO_REPETITION_PAUSE_TIME       74.0e-3                       // repetition after 74 ms
#define KASEIKYO_FRAME_REPEAT_PAUSE_TIME          74.0e-3                       // frame repeat after 74 ms
#define KASEIKYO_ADDRESS_OFFSET                  0                              // skip 0 bits
#define KASEIKYO_ADDRESS_LEN                    16                              // read 16 address bits
#define KASEIKYO_COMMAND_OFFSET                 28                              // skip 28 bits (16 manufacturer & 4 parity & 8 genre)
#define KASEIKYO_COMMAND_LEN                    12                              // read 12 command bits (10 real command & 2 id)
#define KASEIKYO_COMPLETE_DATA_LEN              48                              // complete length
#define KASEIKYO_STOP_BIT                       1                               // has stop bit
#define KASEIKYO_LSB                            1                               // LSB...MSB?
#define KASEIKYO_FRAMES                         2                               // KASEIKYO sends 1st frame 2 times
#define KASEIKYO_FLAGS                          0                               // flags

#define RECS80_START_BIT_PULSE_TIME              158.0e-6                       //  158 usec pulse
#define RECS80_START_BIT_PAUSE_TIME             7432.0e-6                       // 7432 usec pause
#define RECS80_PULSE_TIME                        158.0e-6                       //  158 usec pulse
#define RECS80_1_PAUSE_TIME                     7432.0e-6                       // 7432 usec pause
#define RECS80_0_PAUSE_TIME                     4902.0e-6                       // 4902 usec pause
#define RECS80_FRAME_REPEAT_PAUSE_TIME            45.0e-3                       // frame repeat after 45ms
#define RECS80_ADDRESS_OFFSET                   2                               // skip 2 bits (2nd start + 1 toggle)
#define RECS80_ADDRESS_LEN                      3                               // read 3 address bits
#define RECS80_COMMAND_OFFSET                   5                               // skip 5 bits (2nd start + 1 toggle + 3 address)
#define RECS80_COMMAND_LEN                      6                               // read 6 command bits
#define RECS80_COMPLETE_DATA_LEN                11                              // complete length
#define RECS80_STOP_BIT                         1                               // has stop bit
#define RECS80_LSB                              0                               // MSB...LSB
#define RECS80_FLAGS                            0                               // flags

#define RC5_BIT_TIME                             889.0e-6                       // 889 usec pulse/pause
#define RC5_FRAME_REPEAT_PAUSE_TIME               45.0e-3                       // frame repeat after 45ms
#define RC5_ADDRESS_OFFSET                      2                               // skip 2 bits (2nd start + 1 toggle)
#define RC5_ADDRESS_LEN                         5                               // read 5 address bits
#define RC5_COMMAND_OFFSET                      7                               // skip 5 bits (2nd start + 1 toggle + 5 address)
#define RC5_COMMAND_LEN                         6                               // read 6 command bits
#define RC5_COMPLETE_DATA_LEN                   13                              // complete length
#define RC5_STOP_BIT                            0                               // has no stop bit
#define RC5_LSB                                 0                               // MSB...LSB
#define RC5_FLAGS                               IRMP_PARAM_FLAG_IS_MANCHESTER   // flags

#define DENON_PULSE_TIME                        275.0e-6                        //  275 usec pulse
#define DENON_1_PAUSE_TIME                      1900.0e-6                       // 1900 usec pause
#define DENON_0_PAUSE_TIME                       775.0e-6                       //  775 usec pause
#define DENON_FRAMES                            2                               // DENON sends each frame 2 times
#define DENON_AUTO_REPETITION_PAUSE_TIME          65.0e-3                       // inverted repetition after 65ms
#define DENON_FRAME_REPEAT_PAUSE_TIME             65.0e-3                       // frame repeat after 65ms
#define DENON_ADDRESS_OFFSET                    0                               // skip 0 bits
#define DENON_ADDRESS_LEN                       5                               // read 5 address bits
#define DENON_COMMAND_OFFSET                    5                               // skip 5
#define DENON_COMMAND_LEN                       10                              // read 10 command bits
#define DENON_COMPLETE_DATA_LEN                 15                              // complete length
#define DENON_STOP_BIT                          1                               // has stop bit
#define DENON_LSB                               0                               // MSB...LSB
#define DENON_FLAGS                             0                               // flags

#define RC6_START_BIT_PULSE_TIME                2666.0e-6                       // 2.666 msec pulse
#define RC6_START_BIT_PAUSE_TIME                 889.0e-6                       // 889 usec pause
#define RC6_TOGGLE_BIT_TIME                      889.0e-6                       // 889 msec pulse/pause
#define RC6_BIT_TIME                             444.0e-6                       // 889 usec pulse/pause
#define RC6_FRAME_REPEAT_PAUSE_TIME               45.0e-3                       // frame repeat after 45ms
#define RC6_ADDRESS_OFFSET                      5                               // skip "1" + 3 mode bits + 1 toggle bit
#define RC6_ADDRESS_LEN                         8                               // read 8 address bits
#define RC6_COMMAND_OFFSET                      13                              // skip 12 bits ("1" + 3 mode + 1 toggle + 8 address)
#define RC6_COMMAND_LEN                         8                               // read 8 command bits
#define RC6_COMPLETE_DATA_LEN_SHORT             21                              // complete length
#define RC6_COMPLETE_DATA_LEN_LONG              36                              // complete length
#define RC6_STOP_BIT                            0                               // has no stop bit
#define RC6_LSB                                 0                               // MSB...LSB
#define RC6_FLAGS                               (IRMP_PARAM_FLAG_IS_MANCHESTER | IRMP_PARAM_FLAG_1ST_PULSE_IS_1)   // flags

#define RECS80EXT_START_BIT_PULSE_TIME           158.0e-6                       //  158 usec pulse
#define RECS80EXT_START_BIT_PAUSE_TIME          3637.0e-6                       // 3637 usec pause
#define RECS80EXT_PULSE_TIME                     158.0e-6                       //  158 usec pulse
#define RECS80EXT_1_PAUSE_TIME                  7432.0e-6                       // 7432 usec pause
#define RECS80EXT_0_PAUSE_TIME                  4902.0e-6                       // 4902 usec pause
#define RECS80EXT_FRAME_REPEAT_PAUSE_TIME         45.0e-3                       // frame repeat after 45ms
#define RECS80EXT_ADDRESS_OFFSET                2                               // skip 2 bits (2nd start + 1 toggle)
#define RECS80EXT_ADDRESS_LEN                   4                               // read 3 address bits
#define RECS80EXT_COMMAND_OFFSET                6                               // skip 6 bits (2nd start + 1 toggle + 4 address)
#define RECS80EXT_COMMAND_LEN                   6                               // read 6 command bits
#define RECS80EXT_COMPLETE_DATA_LEN             12                              // complete length
#define RECS80EXT_STOP_BIT                      1                               // has stop bit
#define RECS80EXT_LSB                           0                               // MSB...LSB
#define RECS80EXT_FLAGS                         0                               // flags

#define NUBERT_START_BIT_PULSE_TIME             1340.0e-6                       // 1340 usec pulse
#define NUBERT_START_BIT_PAUSE_TIME              340.0e-6                       //  340 usec pause
#define NUBERT_1_PULSE_TIME                     1340.0e-6                       // 1340 usec pulse
#define NUBERT_1_PAUSE_TIME                      340.0e-6                       //  340 usec pause
#define NUBERT_0_PULSE_TIME                      500.0e-6                       //  500 usec pulse
#define NUBERT_0_PAUSE_TIME                     1300.0e-6                       // 1300 usec pause
#define NUBERT_FRAMES                           2                               // Nubert sends 2 frames
#define NUBERT_AUTO_REPETITION_PAUSE_TIME         35.0e-3                       // auto repetition after 35ms
#define NUBERT_FRAME_REPEAT_PAUSE_TIME            35.0e-3                       // frame repeat after 45ms
#define NUBERT_ADDRESS_OFFSET                   0                               // skip 0 bits
#define NUBERT_ADDRESS_LEN                      0                               // read 0 address bits
#define NUBERT_COMMAND_OFFSET                   0                               // skip 0 bits
#define NUBERT_COMMAND_LEN                      10                              // read 10 bits
#define NUBERT_COMPLETE_DATA_LEN                10                              // complete length
#define NUBERT_STOP_BIT                         1                               // has stop bit
#define NUBERT_LSB                              0                               // MSB?
#define NUBERT_FLAGS                            0                               // flags

#define BANG_OLUFSEN_START_BIT1_PULSE_TIME       200.0e-6                       //   200 usec pulse
#define BANG_OLUFSEN_START_BIT1_PAUSE_TIME      3125.0e-6                       //  3125 usec pause
#define BANG_OLUFSEN_START_BIT2_PULSE_TIME       200.0e-6                       //   200 usec pulse
#define BANG_OLUFSEN_START_BIT2_PAUSE_TIME      3125.0e-6                       //  3125 usec pause
#define BANG_OLUFSEN_START_BIT3_PULSE_TIME       200.0e-6                       //   200 usec pulse
#define BANG_OLUFSEN_START_BIT3_PAUSE_TIME     15625.0e-6                       // 15625 usec pause
#define BANG_OLUFSEN_START_BIT4_PULSE_TIME       200.0e-6                       //   200 usec pulse
#define BANG_OLUFSEN_START_BIT4_PAUSE_TIME      3125.0e-6                       //  3125 usec pause
#define BANG_OLUFSEN_PULSE_TIME                  200.0e-6                       //   200 usec pulse
#define BANG_OLUFSEN_1_PAUSE_TIME               9375.0e-6                       //  9375 usec pause
#define BANG_OLUFSEN_0_PAUSE_TIME               3125.0e-6                       //  3125 usec pause
#define BANG_OLUFSEN_R_PAUSE_TIME               6250.0e-6                       //  6250 usec pause (repeat last bit)
#define BANG_OLUFSEN_TRAILER_BIT_PAUSE_TIME    12500.0e-6                       // 12500 usec pause (trailer bit)
#define BANG_OLUFSEN_FRAME_REPEAT_PAUSE_TIME      45.0e-3                       // frame repeat after 45ms
#define BANG_OLUFSEN_ADDRESS_OFFSET             0                               // no address bits
#define BANG_OLUFSEN_ADDRESS_LEN                0                               // no address bits
#define BANG_OLUFSEN_COMMAND_OFFSET             3                               // skip startbits 2, 3, 4
#define BANG_OLUFSEN_COMMAND_LEN                16                              // read 16 command bits
#define BANG_OLUFSEN_COMPLETE_DATA_LEN          20                              // complete length: startbits 2, 3, 4 + 16 data bits + trailer bit
#define BANG_OLUFSEN_STOP_BIT                   1                               // has stop bit
#define BANG_OLUFSEN_LSB                        0                               // MSB...LSB
#define BANG_OLUFSEN_FLAGS                      0                               // flags

#define GRUNDIG_OR_NOKIA_BIT_TIME               528.0e-6                        // 528 usec pulse/pause
#define GRUNDIG_OR_NOKIA_PRE_PAUSE_TIME         2639.0e-6                       // 2639 usec pause after pre bit
#define GRUNDIG_OR_NOKIA_FRAME_REPEAT_PAUSE_TIME  117.76e-3                     // info frame repeat after 117.76 ms
#define GRUNDIG_OR_NOKIA_STOP_BIT               0                               // has no stop bit
#define GRUNDIG_OR_NOKIA_LSB                    1                               // MSB...LSB
#define GRUNDIG_OR_NOKIA_FLAGS                  (IRMP_PARAM_FLAG_IS_MANCHESTER | IRMP_PARAM_FLAG_1ST_PULSE_IS_1)  // flags

#define GRUNDIG_FRAMES                          2                               // GRUNDIG sends each frame 1+1 times
#define GRUNDIG_AUTO_REPETITION_PAUSE_TIME        20.0e-3                       // repetition after 20ms
#define GRUNDIG_ADDRESS_OFFSET                  0                               // no address
#define GRUNDIG_ADDRESS_LEN                     0                               // no address
#define GRUNDIG_COMMAND_OFFSET                  1                               // skip 1 start bit
#define GRUNDIG_COMMAND_LEN                     9                               // read 9 command bits
#define GRUNDIG_COMPLETE_DATA_LEN               10                              // complete length: 1 start bit + 9 data bits

#define NOKIA_FRAMES                            3                               // NOKIA sends each frame 1 + 1 + 1 times
#define NOKIA_AUTO_REPETITION_PAUSE_TIME          20.0e-3                       // repetition after 20ms
#define NOKIA_ADDRESS_OFFSET                    9                               // skip 9 bits (1 start bit + 8 data bits)
#define NOKIA_ADDRESS_LEN                       8                               // 7 address bits
#define NOKIA_COMMAND_OFFSET                    1                               // skip 1 bit (1 start bit)
#define NOKIA_COMMAND_LEN                       8                               // read 8 command bits
#define NOKIA_COMPLETE_DATA_LEN                 17                              // complete length: 1 start bit + 8 address bits + 8 command bits

#define SIEMENS_BIT_TIME                        250.0e-6                        // 250 usec pulse/pause
#define SIEMENS_FRAME_REPEAT_PAUSE_TIME          45.0e-3                        // frame repeat after 45ms
#define SIEMENS_ADDRESS_OFFSET                  2                               // skip 2 start bits
#define SIEMENS_ADDRESS_LEN                     12                              // read 12 address bits
#define SIEMENS_COMMAND_OFFSET                  15                              // skip 15 bits (2 start bits + 12 address bits + 1 inverted bit)
#define SIEMENS_COMMAND_LEN                     7                               // read 7 command bits
#define SIEMENS_COMPLETE_DATA_LEN               23                              // complete length
#define SIEMENS_STOP_BIT                        0                               // has no stop bit
#define SIEMENS_LSB                             0                               // MSB...LSB
#define SIEMENS_FLAGS                           (IRMP_PARAM_FLAG_IS_MANCHESTER | IRMP_PARAM_FLAG_1ST_PULSE_IS_1)  // flags

#define FDC_START_BIT_PULSE_TIME                 2085.0e-6                      // 2085 usec pulse
#define FDC_START_BIT_PAUSE_TIME                  966.0e-6                      //  966 usec pause
#define FDC_PULSE_TIME                            300.0e-6                      //  300 usec pulse
#define FDC_1_PAUSE_TIME                          715.0e-6                      //  715 usec pause
#define FDC_0_PAUSE_TIME                          220.0e-6                      //  220 usec pause
#define FDC_FRAME_REPEAT_PAUSE_TIME                60.0e-3                      // frame repeat after 60ms
#define FDC_ADDRESS_OFFSET                       0                              // skip 0 bits
#define FDC_ADDRESS_LEN                         14                              // read 14 address bits, but use only 6, shift 8 into command
#define FDC_COMMAND_OFFSET                      20                              // skip 20 bits
#define FDC_COMMAND_LEN                         12                              // read 12 bits
#define FDC_COMPLETE_DATA_LEN                   40                              // complete length
#define FDC_STOP_BIT                            1                               // has stop bit
#define FDC_LSB                                 1                               // LSB...MSB
#define FDC_FLAGS                               0                               // flags

#define RCCAR_START_BIT_PULSE_TIME               2000.0e-6                      // 2000 usec pulse
#define RCCAR_START_BIT_PAUSE_TIME               2000.0e-6                      // 2000 usec pause
#define RCCAR_PULSE_TIME                          600.0e-6                      //  360 usec pulse
#define RCCAR_1_PAUSE_TIME                        450.0e-6                      //  650 usec pause
#define RCCAR_0_PAUSE_TIME                        900.0e-6                      //  180 usec pause
#define RCCAR_FRAME_REPEAT_PAUSE_TIME              40.0e-3                      // frame repeat after 40ms
#define RCCAR_ADDRESS_OFFSET                     0                              // skip 0 bits
#define RCCAR_ADDRESS_LEN                        0                              // read 0 address bits
#define RCCAR_COMMAND_OFFSET                     0                              // skip 0 bits
#define RCCAR_COMMAND_LEN                       13                              // read 13 bits
#define RCCAR_COMPLETE_DATA_LEN                 13                              // complete length
#define RCCAR_STOP_BIT                          1                               // has stop bit
#define RCCAR_LSB                               1                               // LSB...MSB
#define RCCAR_FLAGS                             0                               // flags

#define JVC_START_BIT_PULSE_TIME                9000.0e-6                       // 9000 usec pulse
#define JVC_START_BIT_PAUSE_TIME                4500.0e-6                       // 4500 usec pause
#define JVC_PULSE_TIME                           560.0e-6                       //  560 usec pulse
#define JVC_1_PAUSE_TIME                        1690.0e-6                       // 1690 usec pause
#define JVC_0_PAUSE_TIME                         560.0e-6                       //  560 usec pause
#define JVC_FRAME_REPEAT_PAUSE_TIME               22.0e-3                       // frame repeat after 22ms
#define JVC_ADDRESS_OFFSET                       0                              // skip 0 bits
#define JVC_ADDRESS_LEN                          4                              // read 4 address bits
#define JVC_COMMAND_OFFSET                       4                              // skip 4 bits
#define JVC_COMMAND_LEN                         12                              // read 12 bits
#define JVC_COMPLETE_DATA_LEN                   16                              // complete length
#define JVC_STOP_BIT                            1                               // has stop bit
#define JVC_LSB                                 1                               // LSB...MSB
#define JVC_FLAGS                               0                               // flags

#define AUTO_FRAME_REPETITION_TIME              80.0e-3                         // SIRCS/SAMSUNG32/NUBERT: automatic repetition after 25-50ms
                                                                                // KASEIKYO: automatic repetition after 75ms

#define TRUE                                    1
#define FALSE                                   0

#define IRMP_FLAG_REPETITION                    0x01

#ifndef IRMP_USE_AS_LIB
typedef struct
{
  uint8_t               protocol;                                               // protocol, i.e. NEC_PROTOCOL
  uint16_t              address;                                                // address
  uint16_t              command;                                                // command
  uint8_t               flags;                                                  // flags, e.g. repetition
} IRMP_DATA;
#endif


/**
 *  Initialize IRMP decoder
 *  @details  Configures IRMP input pin
 */
#ifndef IRMP_USE_AS_LIB
extern void                           irmp_init (void);
#endif

/**
 *  Get IRMP data
 *  @details  gets decoded IRMP data
 *  @param    pointer in order to store IRMP data
 *  @return    TRUE: successful, FALSE: failed
 */
extern uint8_t                        irmp_get_data (IRMP_DATA *);

/**
 *  ISR routine
 *  @details  ISR routine, called 10000 times per second
 */
#ifdef IRMP_USE_AS_LIB
extern uint8_t                        irmp_ISR (const uint8_t);
#else
extern uint8_t                        irmp_ISR (void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* _WC_IRMP_H_ */
