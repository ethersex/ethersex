/*
*
* Copyright (c) 2010 by Stefan Riepenhausen <rhn@gmx.net>
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
* For more information on the GPL, please go to:
* http://www.gnu.org/copyleft/gpl.html
*/

#ifndef _PWM_COMMON_H
#define _PWM_COMMON_H

#ifdef DEBUG_PWM
# include "core/debug.h"
# define PWMDEBUG(a...)  debug_printf("pwm: " a)
#else
# define PWMDEBUG(a...)
#endif

// Notes-Frequency
// note, scale, 	real Frequency,       tone#

#define PWMBASEFAKTOR 0.5

#define _C	262 * PWMBASEFAKTOR			// 262Hz -> Tone: -9
#define _Cis	277 * PWMBASEFAKTOR			// 277Hz -> Tone: -8
#define _D	294 * PWMBASEFAKTOR			// 294Hz -> Tone: -7
#define _Dis	311 * PWMBASEFAKTOR			// 311Hz -> Tone: -6
#define _E	330 * PWMBASEFAKTOR			// 330Hz -> Tone: -5
#define _F	349 * PWMBASEFAKTOR			// 349Hz -> Tone: -4
#define _Fis	370 * PWMBASEFAKTOR			// 370Hz -> Tone: -3
#define _G	392 * PWMBASEFAKTOR			// 392Hz -> Tone: -2
#define _Gis	415 * PWMBASEFAKTOR			// 425Hz -> Tone: -1
#define _A	440 * PWMBASEFAKTOR			// 440Hz -> Tone: 0
#define _xA	466 * PWMBASEFAKTOR			// 466Hz -> Tone: 1
#define _Ax	466 * PWMBASEFAKTOR			// 466Hz -> Tone: 1
#define _B	493 * PWMBASEFAKTOR			// 493Hz -> Tone: 2
#define _C0	523 * PWMBASEFAKTOR			// 523Hz -> Tone: 3
#define _xC0	554 * PWMBASEFAKTOR			// 554Hz -> Tone: 4
#define _Cx0	554 * PWMBASEFAKTOR			// 554Hz -> Tone: 4
#define _D0	587 * PWMBASEFAKTOR			// 587Hz -> Tone: 5
#define _xD0	622 * PWMBASEFAKTOR			// 622Hz -> Tone: 6
#define _Dx0	622 * PWMBASEFAKTOR			// 622Hz -> Tone: 6
#define _E0	659 * PWMBASEFAKTOR			// 659Hz -> Tone: 7
#define _F0	698 * PWMBASEFAKTOR			// 698Hz -> Tone: 8
#define _xF0	739 * PWMBASEFAKTOR			// 739Hz -> Tone: 9
#define _Fx0	739 * PWMBASEFAKTOR			// 739Hz -> Tone: 9
#define _G0	783 * PWMBASEFAKTOR			// 783Hz -> Tone: 10
#define _xG0	830 * PWMBASEFAKTOR			// 830Hz -> Tone: 11
#define _Gx0	830 * PWMBASEFAKTOR			// 830Hz -> Tone: 11
#define _A0	880 * PWMBASEFAKTOR			// 880Hz -> Tone: 12
#define _xA0	932 * PWMBASEFAKTOR			// 932Hz -> Tone: 13
#define _Ax0	932 * PWMBASEFAKTOR			// 932Hz -> Tone: 13
#define _B0	987 * PWMBASEFAKTOR			// 987Hz -> Tone: 14
#define _C1	1046 * PWMBASEFAKTOR			// 1046Hz -> Tone: 15
#define _xC1	1108 * PWMBASEFAKTOR			// 1108Hz -> Tone: 16
#define _Cx1	1108 * PWMBASEFAKTOR			// 1108Hz -> Tone: 16
#define _D1	1174 * PWMBASEFAKTOR			// 1174Hz -> Tone: 17
#define _xD1	1244 * PWMBASEFAKTOR			// 1244Hz -> Tone: 18
#define _Dx1	1244 * PWMBASEFAKTOR			// 1244Hz -> Tone: 18
#define _E1	1318 * PWMBASEFAKTOR			// 1318Hz -> Tone: 19
#define _F1	1396 * PWMBASEFAKTOR			// 1396Hz -> Tone: 20
#define _xF1	1479 * PWMBASEFAKTOR			// 1479Hz -> Tone: 21
#define _Fx1	1479 * PWMBASEFAKTOR			// 1479Hz -> Tone: 21
#define _G1	1567 * PWMBASEFAKTOR			// 1567Hz -> Tone: 22
#define _xG1	1661 * PWMBASEFAKTOR			// 1661Hz -> Tone: 23
#define _Gx1	1661 * PWMBASEFAKTOR			// 1661Hz -> Tone: 23
#define _A1	1760 * PWMBASEFAKTOR			// 1760Hz -> Tone: 24
#define _xA1	1864 * PWMBASEFAKTOR			// 1864Hz -> Tone: 25
#define _Ax1	1864 * PWMBASEFAKTOR			// 1864Hz -> Tone: 25
#define _B1	1975 * PWMBASEFAKTOR			// 1975Hz -> Tone: 26
#define _C2	2093 * PWMBASEFAKTOR			// 2093Hz -> Tone: 27
#define _xC2	2217 * PWMBASEFAKTOR			// 2217Hz -> Tone: 28
#define _Cx2	2217 * PWMBASEFAKTOR			// 2217Hz -> Tone: 28
#define _D2	2349 * PWMBASEFAKTOR			// 2349Hz -> Tone: 29
#define _xD2	2489 * PWMBASEFAKTOR			// 2489Hz -> Tone: 30
#define _Dx2	2489 * PWMBASEFAKTOR			// 2489Hz -> Tone: 30
#define _E2	2637 * PWMBASEFAKTOR			// 2637Hz -> Tone: 31
#define _F2	2793 * PWMBASEFAKTOR			// 2793Hz -> Tone: 32
#define _xF2	2959 * PWMBASEFAKTOR			// 2959Hz -> Tone: 33
#define _Fx2	2959 * PWMBASEFAKTOR			// 2959Hz -> Tone: 33
#define _G2	3135 * PWMBASEFAKTOR			// 3135Hz -> Tone: 34
#define _xG2	3322 * PWMBASEFAKTOR			// 3322Hz -> Tone: 35
#define _Gx2	3322 * PWMBASEFAKTOR			// 3322Hz -> Tone: 35
#define _A2	3520 * PWMBASEFAKTOR			// 3520Hz -> Tone: 36
#define _xA2	3729 * PWMBASEFAKTOR			// 3729Hz -> Tone: 37
#define _Ax2	3729 * PWMBASEFAKTOR			// 3729Hz -> Tone: 37
#define _B2	3951 * PWMBASEFAKTOR			// 3951Hz -> Tone: 38
#define _C3	4186 * PWMBASEFAKTOR			// 4186Hz -> Tone: 39
#define _xC3	4434 * PWMBASEFAKTOR			// 4434Hz -> Tone: 40
#define _Cx3	4434 * PWMBASEFAKTOR			// 4434Hz -> Tone: 40
#define _D3	4698 * PWMBASEFAKTOR			// 4698Hz -> Tone: 41
#define _xD3	4978 * PWMBASEFAKTOR			// 4978Hz -> Tone: 42
#define _Dx3	4978 * PWMBASEFAKTOR			// 4978Hz -> Tone: 42
#define _E3	5274 * PWMBASEFAKTOR			// 5274Hz -> Tone: 43
#define _F3	5587 * PWMBASEFAKTOR			// 5587Hz -> Tone: 44
#define _xF3	5919 * PWMBASEFAKTOR			// 5919Hz -> Tone: 45
#define _Fx3	5919 * PWMBASEFAKTOR			// 5919Hz -> Tone: 45
#define _G3	6271 * PWMBASEFAKTOR			// 6271Hz -> Tone: 46
#define _xG3	6644 * PWMBASEFAKTOR			// 6644Hz -> Tone: 47
#define _Gx3	6644 * PWMBASEFAKTOR			// 6644Hz -> Tone: 47
#define _A3	7040 * PWMBASEFAKTOR			// 7040Hz -> Tone: 48
#define _xA3	7458 * PWMBASEFAKTOR			// 7458Hz -> Tone: 49
#define _Ax3	7458 * PWMBASEFAKTOR			// 7458Hz -> Tone: 49
#define _B3	7902 * PWMBASEFAKTOR			// 7902Hz -> Tone: 50
#define _C4	8372 * PWMBASEFAKTOR			// 8372Hz -> Tone: 51
#define _xC4	8869 * PWMBASEFAKTOR			// 8869Hz -> Tone: 52
#define _Cx4	8869 * PWMBASEFAKTOR			// 8869Hz -> Tone: 52
#define _D4	9397 * PWMBASEFAKTOR			// 9397Hz -> Tone: 53
#define _xD4	9956 * PWMBASEFAKTOR			// 9956Hz -> Tone: 54
#define _Dx4	9956 * PWMBASEFAKTOR			// 9956Hz -> Tone: 54
#define _E4	10548 * PWMBASEFAKTOR			// 10548Hz -> Tone: 55
#define _F4	11175 * PWMBASEFAKTOR			// 11175Hz -> Tone: 56
#define _xF4	11839 * PWMBASEFAKTOR			// 11839Hz -> Tone: 57
#define _Fx4	11839 * PWMBASEFAKTOR			// 11839Hz -> Tone: 57
#define _G4	12543 * PWMBASEFAKTOR			// 12543Hz -> Tone: 58
#define _xG4	13289 * PWMBASEFAKTOR			// 13289Hz -> Tone: 59
#define _Gx4	13289 * PWMBASEFAKTOR			// 13289Hz -> Tone: 59
#define _A4	14080 * PWMBASEFAKTOR			// 14080Hz -> Tone: 60
#define _P 0   // break

// same with small letters
#define _c	262 * PWMBASEFAKTOR			// 262Hz -> Tone: -9
#define _cis	277 * PWMBASEFAKTOR			// 277Hz -> Tone: -8
#define _d	294 * PWMBASEFAKTOR			// 294Hz -> Tone: -7
#define _dis	311 * PWMBASEFAKTOR			// 311Hz -> Tone: -6
#define _e	330 * PWMBASEFAKTOR			// 330Hz -> Tone: -5
#define _f	349 * PWMBASEFAKTOR			// 349Hz -> Tone: -4
#define _fis	370 * PWMBASEFAKTOR			// 370Hz -> Tone: -3
#define _g	392 * PWMBASEFAKTOR			// 392Hz -> Tone: -2
#define _gis	415 * PWMBASEFAKTOR			// 425Hz -> Tone: -1
#define _a	440 * PWMBASEFAKTOR			// 440Hz -> Tone: 0
#define _xa	466 * PWMBASEFAKTOR			// 466Hz -> Tone: 1
#define _ax	466 * PWMBASEFAKTOR			// 466Hz -> Tone: 1
#define _b	493 * PWMBASEFAKTOR			// 493Hz -> Tone: 2
#define _c0	523 * PWMBASEFAKTOR			// 523Hz -> Tone: 3
#define _xc0	554 * PWMBASEFAKTOR			// 554Hz -> Tone: 4
#define _cx0	554 * PWMBASEFAKTOR			// 554Hz -> Tone: 4
#define _d0	587 * PWMBASEFAKTOR			// 587Hz -> Tone: 5
#define _xd0	622 * PWMBASEFAKTOR			// 622Hz -> Tone: 6
#define _dx0	622 * PWMBASEFAKTOR			// 622Hz -> Tone: 6
#define _e0	659 * PWMBASEFAKTOR			// 659Hz -> Tone: 7
#define _f0	698 * PWMBASEFAKTOR			// 698Hz -> Tone: 8
#define _xf0	739 * PWMBASEFAKTOR			// 739Hz -> Tone: 9
#define _fx0	739 * PWMBASEFAKTOR			// 739Hz -> Tone: 9
#define _g0	783 * PWMBASEFAKTOR			// 783Hz -> Tone: 10
#define _xg0	830 * PWMBASEFAKTOR			// 830Hz -> Tone: 11
#define _gx0	830 * PWMBASEFAKTOR			// 830Hz -> Tone: 11
#define _a0	880 * PWMBASEFAKTOR			// 880Hz -> Tone: 12
#define _xa0	932 * PWMBASEFAKTOR			// 932Hz -> Tone: 13
#define _ax0	932 * PWMBASEFAKTOR			// 932Hz -> Tone: 13
#define _b0	987 * PWMBASEFAKTOR			// 987Hz -> Tone: 14
#define _c1	1046 * PWMBASEFAKTOR			// 1046Hz -> Tone: 15
#define _xc1	1108 * PWMBASEFAKTOR			// 1108Hz -> Tone: 16
#define _cx1	1108 * PWMBASEFAKTOR			// 1108Hz -> Tone: 16
#define _d1	1174 * PWMBASEFAKTOR			// 1174Hz -> Tone: 17
#define _xd1	1244 * PWMBASEFAKTOR			// 1244Hz -> Tone: 18
#define _dx1	1244 * PWMBASEFAKTOR			// 1244Hz -> Tone: 18
#define _e1	1318 * PWMBASEFAKTOR			// 1318Hz -> Tone: 19
#define _f1	1396 * PWMBASEFAKTOR			// 1396Hz -> Tone: 20
#define _xf1	1479 * PWMBASEFAKTOR			// 1479Hz -> Tone: 21
#define _fx1	1479 * PWMBASEFAKTOR			// 1479Hz -> Tone: 21
#define _g1	1567 * PWMBASEFAKTOR			// 1567Hz -> Tone: 22
#define _xg1	1661 * PWMBASEFAKTOR			// 1661Hz -> Tone: 23
#define _gx1	1661 * PWMBASEFAKTOR			// 1661Hz -> Tone: 23
#define _a1	1760 * PWMBASEFAKTOR			// 1760Hz -> Tone: 24
#define _xa1	1864 * PWMBASEFAKTOR			// 1864Hz -> Tone: 25
#define _ax1	1864 * PWMBASEFAKTOR			// 1864Hz -> Tone: 25
#define _b1	1975 * PWMBASEFAKTOR			// 1975Hz -> Tone: 26
#define _c2	2093 * PWMBASEFAKTOR			// 2093Hz -> Tone: 27
#define _xc2	2217 * PWMBASEFAKTOR			// 2217Hz -> Tone: 28
#define _cx2	2217 * PWMBASEFAKTOR			// 2217Hz -> Tone: 28
#define _d2	2349 * PWMBASEFAKTOR			// 2349Hz -> Tone: 29
#define _xd2	2489 * PWMBASEFAKTOR			// 2489Hz -> Tone: 30
#define _dx2	2489 * PWMBASEFAKTOR			// 2489Hz -> Tone: 30
#define _e2	2637 * PWMBASEFAKTOR			// 2637Hz -> Tone: 31
#define _f2	2793 * PWMBASEFAKTOR			// 2793Hz -> Tone: 32
#define _xf2	2959 * PWMBASEFAKTOR			// 2959Hz -> Tone: 33
#define _fx2	2959 * PWMBASEFAKTOR			// 2959Hz -> Tone: 33
#define _g2	3135 * PWMBASEFAKTOR			// 3135Hz -> Tone: 34
#define _xg2	3322 * PWMBASEFAKTOR			// 3322Hz -> Tone: 35
#define _gx2	3322 * PWMBASEFAKTOR			// 3322Hz -> Tone: 35
#define _a2	3520 * PWMBASEFAKTOR			// 3520Hz -> Tone: 36
#define _xa2	3729 * PWMBASEFAKTOR			// 3729Hz -> Tone: 37
#define _ax2	3729 * PWMBASEFAKTOR			// 3729Hz -> Tone: 37
#define _b2	3951 * PWMBASEFAKTOR			// 3951Hz -> Tone: 38
#define _c3	4186 * PWMBASEFAKTOR			// 4186Hz -> Tone: 39
#define _xc3	4434 * PWMBASEFAKTOR			// 4434Hz -> Tone: 40
#define _cx3	4434 * PWMBASEFAKTOR			// 4434Hz -> Tone: 40
#define _d3	4698 * PWMBASEFAKTOR			// 4698Hz -> Tone: 41
#define _xd3	4978 * PWMBASEFAKTOR			// 4978Hz -> Tone: 42
#define _dx3	4978 * PWMBASEFAKTOR			// 4978Hz -> Tone: 42
#define _e3	5274 * PWMBASEFAKTOR			// 5274Hz -> Tone: 43
#define _f3	5587 * PWMBASEFAKTOR			// 5587Hz -> Tone: 44
#define _xf3	5919 * PWMBASEFAKTOR			// 5919Hz -> Tone: 45
#define _fx3	5919 * PWMBASEFAKTOR			// 5919Hz -> Tone: 45
#define _g3	6271 * PWMBASEFAKTOR			// 6271Hz -> Tone: 46
#define _xg3	6644 * PWMBASEFAKTOR			// 6644Hz -> Tone: 47
#define _gx3	6644 * PWMBASEFAKTOR			// 6644Hz -> Tone: 47
#define _a3	7040 * PWMBASEFAKTOR			// 7040Hz -> Tone: 48
#define _xa3	7458 * PWMBASEFAKTOR			// 7458Hz -> Tone: 49
#define _ax3	7458 * PWMBASEFAKTOR			// 7458Hz -> Tone: 49
#define _b3	7902 * PWMBASEFAKTOR			// 7902Hz -> Tone: 50
#define _c4	8372 * PWMBASEFAKTOR			// 8372Hz -> Tone: 51
#define _xc4	8869 * PWMBASEFAKTOR			// 8869Hz -> Tone: 52
#define _cx4	8869 * PWMBASEFAKTOR			// 8869Hz -> Tone: 52
#define _d4	9397 * PWMBASEFAKTOR			// 9397Hz -> Tone: 53
#define _xd4	9956 * PWMBASEFAKTOR			// 9956Hz -> Tone: 54
#define _dx4	9956 * PWMBASEFAKTOR			// 9956Hz -> Tone: 54
#define _e4	10548 * PWMBASEFAKTOR			// 10548Hz -> Tone: 55
#define _f4	11175 * PWMBASEFAKTOR			// 11175Hz -> Tone: 56
#define _xf4	11839 * PWMBASEFAKTOR			// 11839Hz -> Tone: 57
#define _fx4	11839 * PWMBASEFAKTOR			// 11839Hz -> Tone: 57
#define _g4	12543 * PWMBASEFAKTOR			// 12543Hz -> Tone: 58
#define _xg4	13289 * PWMBASEFAKTOR			// 13289Hz -> Tone: 59
#define _gx4	13289 * PWMBASEFAKTOR			// 13289Hz -> Tone: 59
#define _a4	14080 * PWMBASEFAKTOR			// 14080Hz -> Tone: 60
#define _p 0   // break


// define the length of the notes
#define _GAP             1
#define _ONESIXTEEN      2   
#define _EIGHTS          4   
#define _QUARTER         8   
#define _HALF           16   
#define _THREEQUARTERS  24 
#define _FULL           32 

#endif /* _PWM_COMMON_H */

