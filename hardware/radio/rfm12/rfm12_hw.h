/**** RFM 12 library for Atmel AVR Microcontrollers *******
 *
 * This software is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License,
 * or (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA.
 *
 * @author Peter Fuhrmann, Hans-Gert Dahmen, Soeren Heisrath
 */

#ifndef __RFM12_HW_H
#define __RFM12_HW_H

/* Configuration setting command
	Bit el enables the internal data register.
	Bit ef enables the FIFO mode. If ef=0 then DATA (pin 6) and DCLK (pin 7) are used for data and data clock output.
	x3 x2 x1 x0 Crystal Load Capacitance [pF]
	0 0 0 0 8.5
	0 0 0 1 9.0
	0 0 1 0 9.5
	0 0 1 1 10.0
	....
	1 1 1 0 15.5
	1 1 1 1 16.0
*/
#define RFM12_CMD_CFG 0x8000
#	define RFM12_CFG_EL 0x80
#	define RFM12_CFG_EF 0x40
#	define RFM12_BAND_315 0x00
#	define RFM12_BAND_433 0x10
#	define RFM12_BAND_868 0x20
#	define RFM12_BAND_915 0x30
#	define RFM12_XTAL_12PF 0x07
#	define RFM12_XTAL_135PF 0x0A

/*
	2. Power Management Command
	Bit Function of the control bit Related blocks
	er Enables the whole receiver chain RF front end, baseband, synthesizer, oscillator
	ebb The receiver baseband circuit can be separately switched on Baseband
	et Switches on the PLL, the power amplifier, and starts the
	transmission (If TX register is enabled) Power amplifier, synthesizer, oscillator
	es Turns on the synthesizer Synthesizer
	ex Turns on the crystal oscillator Crystal oscillator
	eb Enables the low battery detector Low battery detector
	ew Enables the wake-up timer Wake-up timer
	dc Disables the clock output (pin 8) Clock output buffer
*/
#define RFM12_CMD_PWRMGT 0x8200
#define RFM12_PWRMGT_ER 0x80
#define RFM12_PWRMGT_EBB 0x40
#define RFM12_PWRMGT_ET 0x20
#define RFM12_PWRMGT_ES 0x10
#define RFM12_PWRMGT_EX 0x08
#define RFM12_PWRMGT_EB 0x04
#define RFM12_PWRMGT_EW 0x02
#define RFM12_PWRMGT_DC 0x01

/*
	3. Frequency Setting Command
	Bit 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0 POR
	1 0 1 0 f11 f10 f9 f8 f7 f6 f5 f4 f3 f2 f1 f0 A680h
	The 12-bit parameter F (bits f11 to f0) should be in the range
	of 96 and 3903. When F value sent is out of range, the
	previous value is kept. The synthesizer center frequency f0 can
	be calculated as:
	f0 = 10 * C1 * (C2 + F/4000) [MHz]
	The constants C1 and C2 are determined by
	the selected band as:
	Band [MHz] C1 C2
	433 		1 43
	868 		2 43
	915 		3 30

	Frequency in 433 Band can be from 430.24MHz to 439.7575MHz in 2.5kHz increments.
*/

#define RFM12_CMD_FREQUENCY 0xA000
#define RFM12_FREQUENCY_CALC_433(f) (((f)-430000000UL)/2500UL)
#define RFM12_FREQUENCY_CALC_868(f) (((f)-860000000UL)/5000UL)
#define RFM12_FREQUENCY_CALC_915(f) (((f)-900000000UL)/7500UL)


/*
	4. Data Rate Command
	Bit 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0 POR
	1 1 0 0 0 1 1 0 cs r6 r5 r4 r3 r2 r1 r0 C623h
	The actual bit rate in transmit mode and the expected bit rate of the received data stream in receive mode is determined by the 7-bit
	parameter R (bits r6 to r0) and bit cs.
	BR = 10000 / 29 / (R+1) / (1+cs*7) [kbps]
	In the receiver set R according to the next function:
	R = (10000 / 29 / (1+cs*7) / BR) – 1, where BR is the expected bit rate in kbps.
	Apart from setting custom values, the standard bit rates from 600 bps to 115.2 kbps can be approximated with small error.
	Data rate accuracy requirements:
	Clock recovery in slow mode: ΔBR/BR < 1/(29*Nbit) Clock recovery in fast mode: ΔBR/BR < 3/(29*Nbit)
	BR is the bit rate set in the receiver and ΔBR is the bit rate difference between the transmitter and the receiver. Nbit is the maximum
	number of consecutive ones or zeros in the data stream. It is recommended for long data packets to include enough 1/0 and 0/1
	transitions, and to be careful to use the same division ratio in the receiver and in the transmitter.
*/

#define RFM12_CMD_DATARATE 0xC600
#define RFM12_DATARATE_CS 0x80
//calculate setting for datarates >= 2700 Baud
#define RFM12_DATARATE_CALC_HIGH(d) ((uint8_t)((10000000.0/29.0/d)-0.5))
//calculate setting for datarates < 2700 Baud
#define RFM12_DATARATE_CALC_LOW(d) (RFM12_DATARATE_CS|(uint8_t)((10000000.0/29.0/8.0/d)-0.5))

/*
	5. Receiver Control Command
	Bit 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0 POR
	1 0 0 1 0 p16 d1 d0 i2 i1 i0 g1 g0 r2 r1 r0 9080h
	Bit 10 (p16): pin16 function select
	
	p16 Function of pin 16
	0 Interrupt input
	1 VDI output
	
	Bits 9-8 (d1 to d0): VDI (valid data indicator) signal response time setting:
	d1 d0 Response
	0 0 Fast
	0 1 Medium
	1 0 Slow
	1 1 Always on

	Bits 7-5 (i2 to i0): Receiver baseband bandwidth (BW) select:
	i2 i1 i0 BW [kHz]
	0 0 0 reserved
	0 0 1 400
	0 1 0 340
	0 1 1 270
	1 0 0 200
	1 0 1 134
	1 1 0 67
	1 1 1 reserved

	Bits 4-3 (g1 to g0): LNA gain select:
	g1 g0 relative to maximum [dB]
	0 0 0
	0 1 -6
	1 0 -14
	1 1 -20

	Bits 2-0 (r2 to r0): RSSI detector threshold:
	r2 r1 r0 RSSIsetth [dBm]
	0 0 0 -103
	0 0 1 -97
	0 1 0 -91
	0 1 1 -85
	1 0 0 -79
	1 0 1 -73
	1 1 0 Reserved
	1 1 1 Reserved
	The RSSI threshold depends on the LNA gain, the real RSSI threshold can be calculated:
	RSSIth=RSSIsetth+GLNA

*/

#define RFM12_CMD_RXCTRL 0x9000
#define RFM12_RXCTRL_P16_VDI 0x400
#define RFM12_RXCTRL_VDI_FAST 0x000
#define RFM12_RXCTRL_VDI_MEDIUM 0x100
#define RFM12_RXCTRL_VDI_SLOW 0x200
#define RFM12_RXCTRL_VDI_ALWAYS_ON 0x300
#define RFM12_RXCTRL_BW_400 0x20
#define RFM12_RXCTRL_BW_340 0x40
#define RFM12_RXCTRL_BW_270 0x60
#define RFM12_RXCTRL_BW_200 0x80
#define RFM12_RXCTRL_BW_134 0xA0
#define RFM12_RXCTRL_BW_67 0xC0
#define RFM12_RXCTRL_LNA_0 0x00
#define RFM12_RXCTRL_LNA_6 0x08
#define RFM12_RXCTRL_LNA_14 0x10
#define RFM12_RXCTRL_LNA_20 0x18
#define RFM12_RXCTRL_RSSI_103 0x00
#define RFM12_RXCTRL_RSSI_97 0x01
#define RFM12_RXCTRL_RSSI_91 0x02
#define RFM12_RXCTRL_RSSI_85 0x03
#define RFM12_RXCTRL_RSSI_79 0x04
#define RFM12_RXCTRL_RSSI_73 0x05
#define RFM12_RXCTRL_RSSI_67 0x06
#define RFM12_RXCTRL_RSSI_61 0x07



/*
	6. Data Filter Command
	Bit 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0 POR
	1 1 0 0 0 0 1 0 al ml 1 s 1 f2 f1 f0 C22Ch
	
	Bit 7 (al): Clock recovery (CR) auto lock control, if set.
	CR will start in fast mode, then after locking it will automatically switch to slow mode.
	
	Bit 6 (ml): Clock recovery lock control
	1: fast mode, fast attack and fast release (4 to 8 bit preamble (1010...) is recommended)
	0: slow mode, slow attack and slow release (12 to 16 bit preamble is recommended)
	Using the slow mode requires more accurate bit timing (see Data Rate Command).
	
	Bits 4 (s): Select the type of the data filter:
	s Filter Type
	0 Digital filter
	1 Analog RC filter
	Digital: This is a digital realization of an analog RC filter followed by a comparator with hysteresis. The time constant is
	automatically adjusted to the bit rate defined by the Data Rate Command.
	Note: Bit rate can not exceed 115 kpbs in this mode.
	Analog RC filter: The demodulator output is fed to pin 7 over a 10 kOhm resistor. The filter cut-off frequency is set by the
	external capacitor connected to this pin and VSS.
	
	Bits 2-0 (f2 to f0): DQD threshold parameter.
	Note: To let the DQD report "good signal quality" the threshold parameter should be 4 in cases where the bitrate is close to the
	deviation. At higher deviation/bitrate settings, a higher threshold parameter can report "good signal quality" as well.
*/

#define RFM12_CMD_DATAFILTER 0xC228
#define RFM12_DATAFILTER_AL 0x80
#define RFM12_DATAFILTER_ML 0x40
#define RFM12_DATAFILTER_S 0x10


/*
	7. FIFO and Reset Mode Command
	Bit 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0 POR
	1 1 0 0 1 0 1 0 f3 f2 f1 f0 sp al ff dr CA80h
	
	Bits 7-4 (f3 to f0): FIFO IT level. The FIFO generates IT when the number of received data bits reaches this level.
	
	Bit 3 (sp): Select the length of the synchron pattern:
	sp 		Byte1 		Byte0 (POR) 	Synchron Pattern (Byte1+Byte0)
	0 		2Dh 		D4h 			2DD4h
	1 		Not used 	D4h 			D4h
	Note: Byte0 can be programmed by the Synchron Pattern Command.
	
	Bit 2 (al): Set the input of the FIFO fill start condition:
	al
	0 Synchron pattern
	1 Always fill

	Bit 1 (ff): FIFO fill will be enabled after synchron pattern reception. The FIFO fill stops when this bit is cleared.
	Bit 0 (dr): Disables the highly sensitive RESET mode.

*/
#define RFM12_CMD_FIFORESET 0xCA00
#define RFM12_FIFORESET_SP 0x08
#define RFM12_FIFORESET_AL 0x04
#define RFM12_FIFORESET_FF 0x02
#define RFM12_FIFORESET_DR 0x01

/*
	8. Synchron Pattern Command
	Bit 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0 POR
	1 1 0 0 1 1 1 0 b7 b6 b5 b4 b3 b2 b1 b0 CED4h
	The Byte0 used for synchron pattern detection can be reprogrammed by B <b7:b0>.
*/
#define RFM12_CMD_SYNCPATTERN 0xCE00

/*
	9. Receiver FIFO Read Command
	Bit 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0 POR
	1 0 1 1 0 0 0 0 0 0 0 0 0 0 0 0 B000h
	With this command, the controller can read 8 bits from the receiver FIFO. Bit 6 (ef) must be set in Configuration Setting Command.

	Note:: During FIFO access fSCK cannot be higher than fref /4, where fref is the crystal oscillator frequency. When the duty-cycle of the
	clock signal is not 50 % the shorter period of the clock pulse width should be at least 2/fref .
*/

#define RFM12_CMD_READ 0xB000

/*
	10. AFC Command
	Bit 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0 POR
	1 1 0 0 0 1 0 0 a1 a0 rl1 rl0 st fi oe en C4F7h

	Bit 7-6 (a1 to a0): Automatic operation mode selector:
	a1 a0
	0 0 Auto mode off (Strobe is controlled by microcontroller)
	0 1 Runs only once after each power-up
	1 0 Keep the foffset only during receiving (VDI=high)
	1 1 Keep the foffset value independently from the state of the VDI signal

	Bit 5-4 (rl1 to rl0): Range limit. Limits the value of the frequency offset register to the next values:
	rl1 rl0 Max deviation
	0 0 No restriction (+63 fres to -64 fres)
	0 1 +15 fres to -16 fres
	1 0 +7 fres to -8 fres
	1 1 +3 fres to -4 fres
	fres:
	433 MHz bands: 2.5 kHz
	868 MHz band: 5 kHz
	915 MHz band: 7.5 kHz
	
	Bit 3 (st): Strobe edge, when st goes to high, the actual latest calculated frequency error is stored into the offset register of the AFC
	block.
	Bit 2 (fi): Switches the circuit to high accuracy (fine) mode. In this case, the processing time is about twice as long, but the measurement
	uncertainty is about half.
	Bit 1 (oe): Enables the frequency offset register. It allows the addition of the offset register to the frequency control word of the PLL.
	Bit 0 (en): Enables the calculation of the offset frequency by the AFC circuit.
	
	In automatic operation mode (no strobe signal is needed from the microcontroller to update the output offset register) the AFC circuit
	is automatically enabled when the VDI indicates potential incoming signal during the whole measurement cycle and the circuit
	measures the same result in two subsequent cycles.
	There are three operation modes, examples from the possible application:
	1, (a1=0, a0=1) The circuit measures the frequency offset only once after power up. This way, extended TX-RX maximum distance
	can be achieved.
	Possible application:
	In the final application, when the user inserts the battery, the circuit measures and compensates for the frequency offset caused by
	the crystal tolerances. This method allows for the use of a cheaper quartz in the application and provides protection against tracking
	an interferer.
	2a, (a1=1, a0=0) The circuit automatically measures the frequency offset during an initial effective low data rate pattern –easier to
	receive- (i.e.: 00110011) of the package and changes the receiving frequency accordingly. The further part of the package can be
	received by the corrected frequency settings.
	2b, (a1=1, a0=0) The transmitter must transmit the first part of the packet with a step higher deviation and later there is a possibility
	of reducing it.
	In both cases (2a and 2b), when the VDI indicates poor receiving conditions (VDI goes low), the output register is automatically
	cleared. Use these settings when receiving signals from different transmitters transmitting in the same nominal frequencies.
	3, (a1=1, a0=1) It’s the same as 2a and 2b modes, but suggested to use when a receiver operates with only one transmitter. After a
	complete measuring cycle, the measured value is kept independently of the state of the VDI signal.
*/

#define RFM12_CMD_AFC 0xC400
#define RFM12_AFC_AUTO_OFF 0x00
#define RFM12_AFC_AUTO_ONCE 0x40
#define RFM12_AFC_AUTO_VDI 0x80
#define RFM12_AFC_AUTO_KEEP 0xC0
#define RFM12_AFC_LIMIT_OFF 0x00        /* 64 */
#define RFM12_AFC_LIMIT_16 0x10
#define RFM12_AFC_LIMIT_8 0x20
#define RFM12_AFC_LIMIT_4 0x30
#define RFM12_AFC_ST 0x08
#define RFM12_AFC_FI 0x04
#define RFM12_AFC_OE 0x02
#define RFM12_AFC_EN 0x01

/*
	11. TX Configuration Control Command
	Bit 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0 POR
	1 0 0 1 1 0 0 mp m3 m2 m1 m0 0 p2 p1 p0 9800h

	Bits 8-4 (mp, m3 to m0): FSK modulation parameters:
	The resulting output frequency can be calculated as:
	fout = f0 + (-1)SIGN * (M + 1) * (15 kHz)
	where:
	f0 is the channel center frequency (see the
	Frequency Setting Command)
	M is the four bit binary number <m3 : m0>
	SIGN = (mp) XOR FSK
	
	Bits 2-0 (p2 to p0): Output power:
	p2 p1 p0 Relative Output Power [dB]
	0 0 0 0
	0 0 1 -2.5
	0 1 0 -5
	0 1 1 -7.5
	1 0 0 -10
	1 0 1 -12.5
	1 1 0 -15
	1 1 1 -17.5
	
*/

#define RFM12_CMD_TXCONF 0x9800
#define RFM12_TXCONF_MP 0x100
#define RFM12_TXCONF_POWER_0 	0x00
#define RFM12_TXCONF_POWER_2_5 	0x01
#define RFM12_TXCONF_POWER_5 	0x02
#define RFM12_TXCONF_POWER_7_5 	0x03
#define RFM12_TXCONF_POWER_10 	0x04
#define RFM12_TXCONF_POWER_12_5 0x05
#define RFM12_TXCONF_POWER_15 	0x06
#define RFM12_TXCONF_POWER_17_5 0x07
#define RFM12_TXCONF_FS_CALC(f) (((f/15000UL)-1)<<4)



/*
	12. PLL Setting Command
	Bit 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0 POR
	1 1 0 0 1 1 0 0 0 ob1 ob0 0 ddy ddit 1 bw0 CC67h
	Note: POR default setting of the register carefully selected to cover almost all typical applications.
	Bit 6-5 (ob1-ob0): Microcontroller output clock buffer rise and fall time control.
	ob1 ob0 Selected uC CLK frequency
	0 0 5 or 10 MHz (recommended)
	0 1 3.3 MHz
	1 X 2.5 MHz or less
	
	(Typ conditions: Top = 27 oC; Vdd = Voc = 2.7 V, Crystal ESR = 30 Ohm)

	Bit 3 (ddy): Switches on the delay in the phase detector when this bit is set.
	Bit 2 (ddit): When set, disables the dithering in the PLL loop.
	Bit 0 (bw0): PLL bandwidth can be set for optimal TX RF performance.
	
	bw0 	Max bit rate [kbps]		Phase noise at 1MHz offset [dBc/Hz]
	0 		86.2 					-107
	1 		256 					-102
	
	Note: Needed for optimization of the RF
	performance. Optimal settings can vary
	according to the external load capacitance.
*/

#define RFM12_CMD_PLL 	0xCC02
#define RFM12_PLL_DDY 	0x08
#define RFM12_PLL_DDIT 	0x04
#define RFM12_PLL_BW0 	0x01

/*
	13. Transmitter Register Write Command
	Bit 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0 POR
	1 0 1 1 1 0 0 0 t7 t6 t5 t4 t3 t2 t1 t0 B8AAh
	With this command, the controller can write 8 bits (t7 to t0) to the transmitter data register. Bit 7 (el) must be set in Configuration
	Setting Command.
*/

#define RFM12_CMD_TX 0xB800

/*
	14. Wake-Up Timer Command
	Bit 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0 POR
	1 1 1 r4 r3 r2 r1 r0 m7 m6 m5 m4 m3 m2 m1 m0 E196h
	The wake-up time period can be calculated by (m7 to m0) and (r4 to r0):
	Twake-up = 1.03 * M * 2R + 0.5 [ms]
	Note:
	• For continual operation the ew bit should be cleared and set at the end of every cycle.
	• For future compatibility, use R in a range of 0 and 29.
*/
#define RFM12_CMD_WAKEUP 0xE000


/*	
	15. Low Duty-Cycle Command
	Bit 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0 POR
	1 1 0 0 1 0 0 0 d6 d5 d4 d3 d2 d1 d0 en C80Eh
	With this command, Low Duty-Cycle operation can be set in order to decrease the average power consumption in receiver mode.
	The time cycle is determined by the Wake-Up Timer Command.
	The Duty-Cycle can be calculated by using (d6 to d0) and M. (M is parameter in a Wake-Up Timer Command.)
	Duty-Cycle= (D * 2 +1) / M *100%
	The on-cycle is automatically extended while DQD indicates good received signal condition (FSK transmission is detected in the
	frequency range determined by Frequency Setting Command plus and minus the baseband filter bandwidth determined by the
	Receiver Control Command).
*/
#define RFM12_CMD_DUTYCYCLE 0xC800
#define RFM12_DUTYCYCLE_ENABLE 0x01

/*
	16. Low Battery Detector and Microcontroller Clock Divider Command
	Bit 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0 POR
	1 1 0 0 0 0 0 0 d2 d1 d0 0 v3 v2 v1 v0 C000h
	The 4 bit parameter (v3 to v0) represents the value V, which defines the threshold voltage Vlb of the detector:
	Vlb= 2.25 + V * 0.1 [V]
	Clock divider configuration:
	Clock Output
	Frequency [MHz]
	0 0 0 1
	0 0 1 1.25
	0 1 0 1.66
	0 1 1 2
	1 0 0 2.5
	1 0 1 3.33
	1 1 0 5
	1 1 1 10
	d2 d1 d0
	The low battery detector and the clock output can be enabled or disabled by bits eb and dc, respectively, using the Power
	Management Command.
*/
#define RFM12_CMD_LBDMCD 0xC000

/*
	17. Status Read Command
	The read command starts with a zero, whereas all other control commands start with a one. If a read command is identified, the
	status bits will be clocked out on the SDO pin as follows:
	
	bitnumber
	15	RGIT TX register is ready to receive the next byte (Can be cleared by Transmitter Register Write Command)
		FFIT The number of data bits in the RX FIFO has reached the pre-programmed limit (Can be cleared by any of the
		FIFO read methods)
	14	POR Power-on reset (Cleared after Status Read Command)
	13	RGUR TX register under run, register over write (Cleared after Status Read Command)
		FFOV RX FIFO overflow (Cleared after Status Read Command)
	12	WKUP Wake-up timer overflow (Cleared after Status Read Command)
	11	EXT Logic level on interrupt pin (pin 16) changed to low (Cleared after Status Read Command)
	10	LBD Low battery detect, the power supply voltage is below the pre-programmed limit
	9	FFEM FIFO is empty
	8	ATS Antenna tuning circuit detected strong enough RF signal
		RSSI The strength of the incoming signal is above the pre-programmed limit
	7	DQD Data quality detector output
	6	CRL Clock recovery locked
	5	ATGL Toggling in each AFC cycle
	4	OFFS(6) MSB of the measured frequency offset (sign of the offset value)
	3	OFFS(3) -OFFS(0) Offset value to be added to the value of the frequency control parameter (Four LSB bits)
	2
	1
	0
*/

#define RFM12_CMD_STATUS 	0x0000
#define RFM12_STATUS_RGIT 	0x8000
#define RFM12_STATUS_FFIT 	0x8000
#define RFM12_STATUS_POR 	0x4000
#define RFM12_STATUS_RGUR 	0x2000
#define RFM12_STATUS_FFOV 	0x2000
#define RFM12_STATUS_WKUP 	0x1000
#define RFM12_STATUS_EXT 	0x0800
#define RFM12_STATUS_LBD 	0x0400
#define RFM12_STATUS_FFEM 	0x0200
#define RFM12_STATUS_ATS 	0x0100
#define RFM12_STATUS_RSSI 	0x0100
#define RFM12_STATUS_DQD 	0x0080
#define RFM12_STATUS_CRL 	0x0040
#define RFM12_STATUS_ATGL 	0x0020

#endif /* __RFM12_HW_H */
