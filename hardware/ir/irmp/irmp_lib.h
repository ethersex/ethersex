/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * irmp.h
 *
 * Copyright (c) 2009-2010 Frank Meyer - frank(at)fli4l.de
 *
 * $Id: irmp.h,v 1.58 2011/02/25 15:24:06 fm Exp $
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

#if IRMP_SUPPORT_NIKON_PROTOCOL == 1
#define IRMP_TIMEOUT_NIKON_TIME                 29500.0e-6                  // 2nd timeout after 29.5 ms darkness (only for NIKON!)
#define IRMP_TIMEOUT_NIKON_TIME_MS              29500L                      // 2nd timeout after 29.5 ms darkness
typedef uint16_t    PAUSE_LEN;
#define IRMP_TIMEOUT_NIKON_LEN                  (PAUSE_LEN)(F_INTERRUPTS * IRMP_TIMEOUT_NIKON_TIME + 0.5)
#else
#if (F_INTERRUPTS * IRMP_TIMEOUT_TIME_MS) / 1000000 >= 254
typedef uint16_t    PAUSE_LEN;
#else
typedef uint8_t     PAUSE_LEN;
#endif
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
#define IRMP_DENON_PROTOCOL                      8              // Denon, Sharp
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
#define IRMP_NIKON_PROTOCOL                     22              // Nikon
#define IRMP_RUWIDO_PROTOCOL                    23              // Ruwido, e.g. T-Home Mediareceiver
#define IRMP_IR60_PROTOCOL                      24              // IR60 (SAB2008)
#define IRMP_KATHREIN_PROTOCOL                  25              // Kathrein
#define IRMP_NETBOX_PROTOCOL                    26              // Netbox keyboard (bitserial)
#define IRMP_NEC16_PROTOCOL                     27              // NEC with 16 bits
#define IRMP_IMON_PROTOCOL                      99              // Imon (bitserial) PROTOTYPE!

// some flags of struct IRMP_PARAMETER:
#define IRMP_PARAM_FLAG_IS_MANCHESTER           0x01
#define IRMP_PARAM_FLAG_1ST_PULSE_IS_1          0x02
#define IRMP_PARAM_FLAG_IS_SERIAL               0x04

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

#define NEC16_ADDRESS_OFFSET                    0                               // skip 0 bits
#define NEC16_ADDRESS_LEN                       8                               // read 8 address bits
#define NEC16_COMMAND_OFFSET                    8                               // skip 8 bits (8 address)
#define NEC16_COMMAND_LEN                       8                               // read 8 bits (8 command)
#define NEC16_COMPLETE_DATA_LEN                 16                              // complete length
#define NEC16_STOP_BIT                          1                               // has stop bit
#define NEC16_LSB                               1                               // LSB...MSB
#define NEC16_FLAGS                             0                               // flags

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
#define RECS80_ADDRESS_OFFSET                   1                               // skip 1 bit (toggle bit)
#define RECS80_ADDRESS_LEN                      3                               // read 3 address bits
#define RECS80_COMMAND_OFFSET                   4                               // skip 4 bits (1 toggle + 3 address)
#define RECS80_COMMAND_LEN                      6                               // read 6 command bits
#define RECS80_COMPLETE_DATA_LEN                10                              // complete length
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

#define DENON_PULSE_TIME                         310.0e-6                       //  310 usec pulse in practice,  275 in theory
#define DENON_1_PAUSE_TIME                      1780.0e-6                       // 1780 usec pause in practice, 1900 in theory
#define DENON_0_PAUSE_TIME                       745.0e-6                       //  745 usec pause in practice,  775 in theory
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
#define RECS80EXT_ADDRESS_LEN                   4                               // read 4 address bits
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

#define GRUNDIG_NOKIA_IR60_BIT_TIME             528.0e-6                        // 528 usec pulse/pause
#define GRUNDIG_NOKIA_IR60_PRE_PAUSE_TIME       2639.0e-6                       // 2639 usec pause after pre bit
#define GRUNDIG_NOKIA_IR60_FRAME_REPEAT_PAUSE_TIME  117.76e-3                   // info frame repeat after 117.76 ms
#define GRUNDIG_NOKIA_IR60_STOP_BIT             0                               // has no stop bit
#define GRUNDIG_NOKIA_IR60_LSB                  1                               // MSB...LSB
#define GRUNDIG_NOKIA_IR60_FLAGS                (IRMP_PARAM_FLAG_IS_MANCHESTER | IRMP_PARAM_FLAG_1ST_PULSE_IS_1)  // flags

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

#define IR60_TIMEOUT_TIME                       5000.0e-6                       // timeout grundig frame, switch to IR60
#define IR60_ADDRESS_OFFSET                     0                               // skip 1 bits
#define IR60_ADDRESS_LEN                        0                               // read 0 address bits
#define IR60_COMMAND_OFFSET                     0                               // skip 1 bit (start bit after pre bit, always 1)
#define IR60_COMMAND_LEN                        7                               // read 6 command bits
#define IR60_COMPLETE_DATA_LEN                  7                               // complete length

#define SIEMENS_OR_RUWIDO_START_BIT_PULSE_TIME    275.0e-6                      //  275 usec pulse
#define SIEMENS_OR_RUWIDO_START_BIT_PAUSE_TIME    550.0e-6                      //  550 usec pause
#define SIEMENS_OR_RUWIDO_BIT_PULSE_TIME          275.0e-6                      //  275 usec short pulse
#define SIEMENS_OR_RUWIDO_BIT_PULSE_TIME_2        550.0e-6                      //  550 usec long pulse
#define SIEMENS_OR_RUWIDO_BIT_PAUSE_TIME          275.0e-6                      //  275 usec short pause
#define SIEMENS_OR_RUWIDO_BIT_PAUSE_TIME_2        550.0e-6                      //  550 usec long pause
#define SIEMENS_OR_RUWIDO_FRAME_REPEAT_PAUSE_TIME 45.0e-3                       // frame repeat after 45ms
#define SIEMENS_OR_RUWIDO_STOP_BIT                0                             // has no stop bit
#define SIEMENS_OR_RUWIDO_LSB                     0                             // MSB...LSB
#define SIEMENS_OR_RUWIDO_FLAGS                   (IRMP_PARAM_FLAG_IS_MANCHESTER | IRMP_PARAM_FLAG_1ST_PULSE_IS_1)  // flags

#define RUWIDO_ADDRESS_OFFSET                   0                               // skip 0 bits
#define RUWIDO_ADDRESS_LEN                      9                               // read 9 address bits
#define RUWIDO_COMMAND_OFFSET                   9                               // skip 9 bits
#define RUWIDO_COMMAND_LEN                      8                               // read 7 + 1 command bits, last bit is only check bit
#define RUWIDO_COMPLETE_DATA_LEN                17                              // complete length

#define SIEMENS_ADDRESS_OFFSET                  0                               // skip 0 bits
#define SIEMENS_ADDRESS_LEN                     11                              // read 11 bits
#define SIEMENS_COMMAND_OFFSET                  11                              // skip 11 bits
#define SIEMENS_COMMAND_LEN                     11                              // read 10 + 1 command bits, last bit is only check bit
#define SIEMENS_COMPLETE_DATA_LEN               22                              // complete length

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

#define NIKON_START_BIT_PULSE_TIME              2200.0e-6                       //  2200 usec pulse
#define NIKON_START_BIT_PAUSE_TIME             27100.0e-6                       // 27100 usec pause
#define NIKON_PULSE_TIME                         500.0e-6                       //   520 usec pulse
#define NIKON_1_PAUSE_TIME                      3500.0e-6                       //  3500 usec pause
#define NIKON_0_PAUSE_TIME                      1500.0e-6                       //  1500 usec pause
#define NIKON_FRAME_REPEAT_PAUSE_TIME             60.0e-3                       // frame repeat after 60ms
#define NIKON_ADDRESS_OFFSET                    0                               // skip 0 bits
#define NIKON_ADDRESS_LEN                       0                               // read 0 address bits
#define NIKON_COMMAND_OFFSET                    0                               // skip 0 bits
#define NIKON_COMMAND_LEN                       2                               // read 2 bits
#define NIKON_COMPLETE_DATA_LEN                 2                               // complete length
#define NIKON_STOP_BIT                          1                               // has stop bit
#define NIKON_LSB                               0                               // LSB...MSB
#define NIKON_FLAGS                             0                               // flags

#define KATHREIN_START_BIT_PULSE_TIME            210.0e-6                       // 1340 usec pulse
#define KATHREIN_START_BIT_PAUSE_TIME           6218.0e-6                       //  340 usec pause
#define KATHREIN_1_PULSE_TIME                    210.0e-6                       // 1340 usec pulse
#define KATHREIN_1_PAUSE_TIME                   3000.0e-6                       //  340 usec pause
#define KATHREIN_0_PULSE_TIME                    210.0e-6                       //  500 usec pulse
#define KATHREIN_0_PAUSE_TIME                   1400.0e-6                       // 1300 usec pause
#define KATHREIN_SYNC_BIT_PAUSE_LEN_TIME        4600.0e-6                       // 4600 usec sync (on 6th and/or 8th bit)
#define KATHREIN_FRAMES                         1                               // Kathrein sends 1 frame
#define KATHREIN_AUTO_REPETITION_PAUSE_TIME     35.0e-3                         // auto repetition after 35ms
#define KATHREIN_FRAME_REPEAT_PAUSE_TIME        35.0e-3                         // frame repeat after 35ms
#define KATHREIN_ADDRESS_OFFSET                 1                               // skip 1 bits
#define KATHREIN_ADDRESS_LEN                    4                               // read 4 address bits
#define KATHREIN_COMMAND_OFFSET                 5                               // skip 5 bits
#define KATHREIN_COMMAND_LEN                    7                               // read 7 bits
#define KATHREIN_COMPLETE_DATA_LEN              13                              // complete length
#define KATHREIN_STOP_BIT                       1                               // has stop bit
#define KATHREIN_LSB                            0                               // MSB
#define KATHREIN_FLAGS                          0                               // flags

#define NETBOX_START_BIT_PULSE_TIME             2400.0e-6                       // 2400 usec pulse
#define NETBOX_START_BIT_PAUSE_TIME              800.0e-6                       //  800 usec pause
#define NETBOX_PULSE_TIME                        800.0e-6                       //  800 usec pulse
#define NETBOX_PAUSE_TIME                        800.0e-6                       //  800 usec pause
#define NETBOX_FRAMES                           1                               // Netbox sends 1 frame
#define NETBOX_AUTO_REPETITION_PAUSE_TIME       35.0e-3                         // auto repetition after 35ms
#define NETBOX_FRAME_REPEAT_PAUSE_TIME          35.0e-3                         // frame repeat after 35ms
#define NETBOX_ADDRESS_OFFSET                   0                               // skip 0 bits
#define NETBOX_ADDRESS_LEN                      3                               // read 3 address bits
#define NETBOX_COMMAND_OFFSET                   3                               // skip 3 bits
#define NETBOX_COMMAND_LEN                      13                              // read 13 bits
#define NETBOX_COMPLETE_DATA_LEN                16                              // complete length
#define NETBOX_STOP_BIT                         0                               // has no stop bit
#define NETBOX_LSB                              1                               // LSB
#define NETBOX_FLAGS                            IRMP_PARAM_FLAG_IS_SERIAL       // flags

#define IMON_START_BIT_PULSE_TIME               1333.0e-6                       // 1333 usec pulse
#define IMON_START_BIT_PAUSE_TIME               1172.0e-6                       // 1333 usec pause
#define IMON_PULSE_TIME                          500.0e-6                       //  500 usec pulse
#define IMON_PAUSE_TIME                          500.0e-6                       //  500 usec pause
#define IMON_FRAMES                             1                               // Imon sends 1 frame
#define IMON_AUTO_REPETITION_PAUSE_TIME         35.0e-3                         // auto repetition after 35ms
#define IMON_FRAME_REPEAT_PAUSE_TIME            35.0e-3                         // frame repeat after 35ms
#define IMON_ADDRESS_OFFSET                     0                               // skip 0 bits
#define IMON_ADDRESS_LEN                        0                               // read 0 address bits
#define IMON_COMMAND_OFFSET                     26                              // skip 26 bits
#define IMON_COMMAND_LEN                        16                              // read last 16 bits, ignore rest
#define IMON_COMPLETE_DATA_LEN                  42                              // complete length, last is stop bit
#define IMON_STOP_BIT                           1                               // has stop bit
#define IMON_LSB                                1                               // LSB
#define IMON_FLAGS                              IRMP_PARAM_FLAG_IS_SERIAL       // flags

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
