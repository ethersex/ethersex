/*
*
* Copyright (c) 2009 by Stefan Riepenhausen <rhn@gmx.net>
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

#ifndef _PWM_MELODY_H
#define _PWM_MELODY_H

#ifdef PWM_MELODY_SUPPORT

// Notes-Frequency
// note, scale, 	real Frequency,       tone#

#define PWMBASEFAKTOR 0.5

#define	C	262 * PWMBASEFAKTOR			// 262Hz -> Tone: -9
#define	Cis	277 * PWMBASEFAKTOR			// 277Hz -> Tone: -8
#define	D	294 * PWMBASEFAKTOR			// 294Hz -> Tone: -7
#define	Dis	311 * PWMBASEFAKTOR			// 311Hz -> Tone: -6
#define	E	330 * PWMBASEFAKTOR			// 330Hz -> Tone: -5
#define	F	349 * PWMBASEFAKTOR			// 349Hz -> Tone: -4
#define	Fis	370 * PWMBASEFAKTOR			// 370Hz -> Tone: -3
#define	G	392 * PWMBASEFAKTOR			// 392Hz -> Tone: -2
#define	Gis	415 * PWMBASEFAKTOR			// 425Hz -> Tone: -1
#define	A	440 * PWMBASEFAKTOR			// 440Hz -> Tone: 0
#define	xA	466 * PWMBASEFAKTOR			// 466Hz -> Tone: 1
#define	Ax	466 * PWMBASEFAKTOR			// 466Hz -> Tone: 1
#define	B	493 * PWMBASEFAKTOR			// 493Hz -> Tone: 2
#define	C0	523 * PWMBASEFAKTOR			// 523Hz -> Tone: 3
#define	xC0	554 * PWMBASEFAKTOR			// 554Hz -> Tone: 4
#define	Cx0	554 * PWMBASEFAKTOR			// 554Hz -> Tone: 4
#define	D0	587 * PWMBASEFAKTOR			// 587Hz -> Tone: 5
#define	xD0	622 * PWMBASEFAKTOR			// 622Hz -> Tone: 6
#define	Dx0	622 * PWMBASEFAKTOR			// 622Hz -> Tone: 6
#define	E0	659 * PWMBASEFAKTOR			// 659Hz -> Tone: 7
#define	F0	698 * PWMBASEFAKTOR			// 698Hz -> Tone: 8
#define	xF0	739 * PWMBASEFAKTOR			// 739Hz -> Tone: 9
#define	Fx0	739 * PWMBASEFAKTOR			// 739Hz -> Tone: 9
#define	G0	783 * PWMBASEFAKTOR			// 783Hz -> Tone: 10
#define	xG0	830 * PWMBASEFAKTOR			// 830Hz -> Tone: 11
#define	Gx0	830 * PWMBASEFAKTOR			// 830Hz -> Tone: 11
#define	A0	880 * PWMBASEFAKTOR			// 880Hz -> Tone: 12
#define	xA0	932 * PWMBASEFAKTOR			// 932Hz -> Tone: 13
#define	Ax0	932 * PWMBASEFAKTOR			// 932Hz -> Tone: 13
#define	B0	987 * PWMBASEFAKTOR			// 987Hz -> Tone: 14
#define	C1	1046 * PWMBASEFAKTOR			// 1046Hz -> Tone: 15
#define	xC1	1108 * PWMBASEFAKTOR			// 1108Hz -> Tone: 16
#define	Cx1	1108 * PWMBASEFAKTOR			// 1108Hz -> Tone: 16
#define	D1	1174 * PWMBASEFAKTOR			// 1174Hz -> Tone: 17
#define	xD1	1244 * PWMBASEFAKTOR			// 1244Hz -> Tone: 18
#define	Dx1	1244 * PWMBASEFAKTOR			// 1244Hz -> Tone: 18
#define	E1	1318 * PWMBASEFAKTOR			// 1318Hz -> Tone: 19
#define	F1	1396 * PWMBASEFAKTOR			// 1396Hz -> Tone: 20
#define	xF1	1479 * PWMBASEFAKTOR			// 1479Hz -> Tone: 21
#define	Fx1	1479 * PWMBASEFAKTOR			// 1479Hz -> Tone: 21
#define	G1	1567 * PWMBASEFAKTOR			// 1567Hz -> Tone: 22
#define	xG1	1661 * PWMBASEFAKTOR			// 1661Hz -> Tone: 23
#define	Gx1	1661 * PWMBASEFAKTOR			// 1661Hz -> Tone: 23
#define	A1	1760 * PWMBASEFAKTOR			// 1760Hz -> Tone: 24
#define	xA1	1864 * PWMBASEFAKTOR			// 1864Hz -> Tone: 25
#define	Ax1	1864 * PWMBASEFAKTOR			// 1864Hz -> Tone: 25
#define	B1	1975 * PWMBASEFAKTOR			// 1975Hz -> Tone: 26
#define	C2	2093 * PWMBASEFAKTOR			// 2093Hz -> Tone: 27
#define	xC2	2217 * PWMBASEFAKTOR			// 2217Hz -> Tone: 28
#define	Cx2	2217 * PWMBASEFAKTOR			// 2217Hz -> Tone: 28
#define	D2	2349 * PWMBASEFAKTOR			// 2349Hz -> Tone: 29
#define	xD2	2489 * PWMBASEFAKTOR			// 2489Hz -> Tone: 30
#define	Dx2	2489 * PWMBASEFAKTOR			// 2489Hz -> Tone: 30
#define	E2	2637 * PWMBASEFAKTOR			// 2637Hz -> Tone: 31
#define	F2	2793 * PWMBASEFAKTOR			// 2793Hz -> Tone: 32
#define	xF2	2959 * PWMBASEFAKTOR			// 2959Hz -> Tone: 33
#define	Fx2	2959 * PWMBASEFAKTOR			// 2959Hz -> Tone: 33
#define	G2	3135 * PWMBASEFAKTOR			// 3135Hz -> Tone: 34
#define	xG2	3322 * PWMBASEFAKTOR			// 3322Hz -> Tone: 35
#define	Gx2	3322 * PWMBASEFAKTOR			// 3322Hz -> Tone: 35
#define	A2	3520 * PWMBASEFAKTOR			// 3520Hz -> Tone: 36
#define	xA2	3729 * PWMBASEFAKTOR			// 3729Hz -> Tone: 37
#define	Ax2	3729 * PWMBASEFAKTOR			// 3729Hz -> Tone: 37
#define	B2	3951 * PWMBASEFAKTOR			// 3951Hz -> Tone: 38
#define	C3	4186 * PWMBASEFAKTOR			// 4186Hz -> Tone: 39
#define	xC3	4434 * PWMBASEFAKTOR			// 4434Hz -> Tone: 40
#define	Cx3	4434 * PWMBASEFAKTOR			// 4434Hz -> Tone: 40
#define	D3	4698 * PWMBASEFAKTOR			// 4698Hz -> Tone: 41
#define	xD3	4978 * PWMBASEFAKTOR			// 4978Hz -> Tone: 42
#define	Dx3	4978 * PWMBASEFAKTOR			// 4978Hz -> Tone: 42
#define	E3	5274 * PWMBASEFAKTOR			// 5274Hz -> Tone: 43
#define	F3	5587 * PWMBASEFAKTOR			// 5587Hz -> Tone: 44
#define	xF3	5919 * PWMBASEFAKTOR			// 5919Hz -> Tone: 45
#define	Fx3	5919 * PWMBASEFAKTOR			// 5919Hz -> Tone: 45
#define	G3	6271 * PWMBASEFAKTOR			// 6271Hz -> Tone: 46
#define	xG3	6644 * PWMBASEFAKTOR			// 6644Hz -> Tone: 47
#define	Gx3	6644 * PWMBASEFAKTOR			// 6644Hz -> Tone: 47
#define	A3	7040 * PWMBASEFAKTOR			// 7040Hz -> Tone: 48
#define	xA3	7458 * PWMBASEFAKTOR			// 7458Hz -> Tone: 49
#define	Ax3	7458 * PWMBASEFAKTOR			// 7458Hz -> Tone: 49
#define	B3	7902 * PWMBASEFAKTOR			// 7902Hz -> Tone: 50
#define	C4	8372 * PWMBASEFAKTOR			// 8372Hz -> Tone: 51
#define	xC4	8869 * PWMBASEFAKTOR			// 8869Hz -> Tone: 52
#define	Cx4	8869 * PWMBASEFAKTOR			// 8869Hz -> Tone: 52
#define	D4	9397 * PWMBASEFAKTOR			// 9397Hz -> Tone: 53
#define	xD4	9956 * PWMBASEFAKTOR			// 9956Hz -> Tone: 54
#define	Dx4	9956 * PWMBASEFAKTOR			// 9956Hz -> Tone: 54
#define	E4	10548 * PWMBASEFAKTOR			// 10548Hz -> Tone: 55
#define	F4	11175 * PWMBASEFAKTOR			// 11175Hz -> Tone: 56
#define	xF4	11839 * PWMBASEFAKTOR			// 11839Hz -> Tone: 57
#define	Fx4	11839 * PWMBASEFAKTOR			// 11839Hz -> Tone: 57
#define	G4	12543 * PWMBASEFAKTOR			// 12543Hz -> Tone: 58
#define	xG4	13289 * PWMBASEFAKTOR			// 13289Hz -> Tone: 59
#define	Gx4	13289 * PWMBASEFAKTOR			// 13289Hz -> Tone: 59
#define	A4	14080 * PWMBASEFAKTOR			// 14080Hz -> Tone: 60
#define P 0   // break

// same with small letters
#define	c	262 * PWMBASEFAKTOR			// 262Hz -> Tone: -9
#define	cis	277 * PWMBASEFAKTOR			// 277Hz -> Tone: -8
#define	d	294 * PWMBASEFAKTOR			// 294Hz -> Tone: -7
#define	dis	311 * PWMBASEFAKTOR			// 311Hz -> Tone: -6
#define	e	330 * PWMBASEFAKTOR			// 330Hz -> Tone: -5
#define	f	349 * PWMBASEFAKTOR			// 349Hz -> Tone: -4
#define	fis	370 * PWMBASEFAKTOR			// 370Hz -> Tone: -3
#define	g	392 * PWMBASEFAKTOR			// 392Hz -> Tone: -2
#define	gis	415 * PWMBASEFAKTOR			// 425Hz -> Tone: -1
#define	a	440 * PWMBASEFAKTOR			// 440Hz -> Tone: 0
#define	xa	466 * PWMBASEFAKTOR			// 466Hz -> Tone: 1
#define	ax	466 * PWMBASEFAKTOR			// 466Hz -> Tone: 1
#define	b	493 * PWMBASEFAKTOR			// 493Hz -> Tone: 2
#define	c0	523 * PWMBASEFAKTOR			// 523Hz -> Tone: 3
#define	xc0	554 * PWMBASEFAKTOR			// 554Hz -> Tone: 4
#define	cx0	554 * PWMBASEFAKTOR			// 554Hz -> Tone: 4
#define	d0	587 * PWMBASEFAKTOR			// 587Hz -> Tone: 5
#define	xd0	622 * PWMBASEFAKTOR			// 622Hz -> Tone: 6
#define	dx0	622 * PWMBASEFAKTOR			// 622Hz -> Tone: 6
#define	e0	659 * PWMBASEFAKTOR			// 659Hz -> Tone: 7
#define	f0	698 * PWMBASEFAKTOR			// 698Hz -> Tone: 8
#define	xf0	739 * PWMBASEFAKTOR			// 739Hz -> Tone: 9
#define	fx0	739 * PWMBASEFAKTOR			// 739Hz -> Tone: 9
#define	g0	783 * PWMBASEFAKTOR			// 783Hz -> Tone: 10
#define	xg0	830 * PWMBASEFAKTOR			// 830Hz -> Tone: 11
#define	gx0	830 * PWMBASEFAKTOR			// 830Hz -> Tone: 11
#define	a0	880 * PWMBASEFAKTOR			// 880Hz -> Tone: 12
#define	xa0	932 * PWMBASEFAKTOR			// 932Hz -> Tone: 13
#define	ax0	932 * PWMBASEFAKTOR			// 932Hz -> Tone: 13
#define	b0	987 * PWMBASEFAKTOR			// 987Hz -> Tone: 14
#define	c1	1046 * PWMBASEFAKTOR			// 1046Hz -> Tone: 15
#define	xc1	1108 * PWMBASEFAKTOR			// 1108Hz -> Tone: 16
#define	cx1	1108 * PWMBASEFAKTOR			// 1108Hz -> Tone: 16
#define	d1	1174 * PWMBASEFAKTOR			// 1174Hz -> Tone: 17
#define	xd1	1244 * PWMBASEFAKTOR			// 1244Hz -> Tone: 18
#define	dx1	1244 * PWMBASEFAKTOR			// 1244Hz -> Tone: 18
#define	e1	1318 * PWMBASEFAKTOR			// 1318Hz -> Tone: 19
#define	f1	1396 * PWMBASEFAKTOR			// 1396Hz -> Tone: 20
#define	xf1	1479 * PWMBASEFAKTOR			// 1479Hz -> Tone: 21
#define	fx1	1479 * PWMBASEFAKTOR			// 1479Hz -> Tone: 21
#define	g1	1567 * PWMBASEFAKTOR			// 1567Hz -> Tone: 22
#define	xg1	1661 * PWMBASEFAKTOR			// 1661Hz -> Tone: 23
#define	gx1	1661 * PWMBASEFAKTOR			// 1661Hz -> Tone: 23
#define	a1	1760 * PWMBASEFAKTOR			// 1760Hz -> Tone: 24
#define	xa1	1864 * PWMBASEFAKTOR			// 1864Hz -> Tone: 25
#define	ax1	1864 * PWMBASEFAKTOR			// 1864Hz -> Tone: 25
#define	b1	1975 * PWMBASEFAKTOR			// 1975Hz -> Tone: 26
#define	c2	2093 * PWMBASEFAKTOR			// 2093Hz -> Tone: 27
#define	xc2	2217 * PWMBASEFAKTOR			// 2217Hz -> Tone: 28
#define	cx2	2217 * PWMBASEFAKTOR			// 2217Hz -> Tone: 28
#define	d2	2349 * PWMBASEFAKTOR			// 2349Hz -> Tone: 29
#define	xd2	2489 * PWMBASEFAKTOR			// 2489Hz -> Tone: 30
#define	dx2	2489 * PWMBASEFAKTOR			// 2489Hz -> Tone: 30
#define	e2	2637 * PWMBASEFAKTOR			// 2637Hz -> Tone: 31
#define	f2	2793 * PWMBASEFAKTOR			// 2793Hz -> Tone: 32
#define	xf2	2959 * PWMBASEFAKTOR			// 2959Hz -> Tone: 33
#define	fx2	2959 * PWMBASEFAKTOR			// 2959Hz -> Tone: 33
#define	g2	3135 * PWMBASEFAKTOR			// 3135Hz -> Tone: 34
#define	xg2	3322 * PWMBASEFAKTOR			// 3322Hz -> Tone: 35
#define	gx2	3322 * PWMBASEFAKTOR			// 3322Hz -> Tone: 35
#define	a2	3520 * PWMBASEFAKTOR			// 3520Hz -> Tone: 36
#define	xa2	3729 * PWMBASEFAKTOR			// 3729Hz -> Tone: 37
#define	ax2	3729 * PWMBASEFAKTOR			// 3729Hz -> Tone: 37
#define	b2	3951 * PWMBASEFAKTOR			// 3951Hz -> Tone: 38
#define	c3	4186 * PWMBASEFAKTOR			// 4186Hz -> Tone: 39
#define	xc3	4434 * PWMBASEFAKTOR			// 4434Hz -> Tone: 40
#define	cx3	4434 * PWMBASEFAKTOR			// 4434Hz -> Tone: 40
#define	d3	4698 * PWMBASEFAKTOR			// 4698Hz -> Tone: 41
#define	xd3	4978 * PWMBASEFAKTOR			// 4978Hz -> Tone: 42
#define	dx3	4978 * PWMBASEFAKTOR			// 4978Hz -> Tone: 42
#define	e3	5274 * PWMBASEFAKTOR			// 5274Hz -> Tone: 43
#define	f3	5587 * PWMBASEFAKTOR			// 5587Hz -> Tone: 44
#define	xf3	5919 * PWMBASEFAKTOR			// 5919Hz -> Tone: 45
#define	fx3	5919 * PWMBASEFAKTOR			// 5919Hz -> Tone: 45
#define	g3	6271 * PWMBASEFAKTOR			// 6271Hz -> Tone: 46
#define	xg3	6644 * PWMBASEFAKTOR			// 6644Hz -> Tone: 47
#define	gx3	6644 * PWMBASEFAKTOR			// 6644Hz -> Tone: 47
#define	a3	7040 * PWMBASEFAKTOR			// 7040Hz -> Tone: 48
#define	xa3	7458 * PWMBASEFAKTOR			// 7458Hz -> Tone: 49
#define	ax3	7458 * PWMBASEFAKTOR			// 7458Hz -> Tone: 49
#define	b3	7902 * PWMBASEFAKTOR			// 7902Hz -> Tone: 50
#define	c4	8372 * PWMBASEFAKTOR			// 8372Hz -> Tone: 51
#define	xc4	8869 * PWMBASEFAKTOR			// 8869Hz -> Tone: 52
#define	cx4	8869 * PWMBASEFAKTOR			// 8869Hz -> Tone: 52
#define	d4	9397 * PWMBASEFAKTOR			// 9397Hz -> Tone: 53
#define	xd4	9956 * PWMBASEFAKTOR			// 9956Hz -> Tone: 54
#define	dx4	9956 * PWMBASEFAKTOR			// 9956Hz -> Tone: 54
#define	e4	10548 * PWMBASEFAKTOR			// 10548Hz -> Tone: 55
#define	f4	11175 * PWMBASEFAKTOR			// 11175Hz -> Tone: 56
#define	xf4	11839 * PWMBASEFAKTOR			// 11839Hz -> Tone: 57
#define	fx4	11839 * PWMBASEFAKTOR			// 11839Hz -> Tone: 57
#define	g4	12543 * PWMBASEFAKTOR			// 12543Hz -> Tone: 58
#define	xg4	13289 * PWMBASEFAKTOR			// 13289Hz -> Tone: 59
#define	gx4	13289 * PWMBASEFAKTOR			// 13289Hz -> Tone: 59
#define	a4	14080 * PWMBASEFAKTOR			// 14080Hz -> Tone: 60
#define p 0   // break


// define the length of the notes
#define GAP             1
#define ONESIXTEEN      2   
#define EIGHTS          4   
#define QUARTER         8   
#define HALF           16   
#define THREEQUARTERS  24 
#define FULL           32 

struct notes_duration_t
{
  uint16_t note;
  uint16_t duration;
};

struct song_t
{
  const PGM_P title;
  const uint8_t delay;
  const uint8_t transpose;
  const struct notes_duration_t *notes;
  const uint16_t size;
};

void 
pwm_melody_init(uint8_t nr);
#endif //PWM_MELODY_SUPPORT

#endif /* _PWM_MELODY_H */

