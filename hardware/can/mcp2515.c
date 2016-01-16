// coding: utf-8
// ----------------------------------------------------------------------------
/*
 * Copyright (c) 2007 Fabian Greif, Roboterclub Aachen e.V.
 * Copyright (c) 2016 by Michael Wagner <mw@iot-make.de>
 *  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $Id: mcp2515.c 8086 2009-07-14 14:08:25Z fabian $
 */
// ----------------------------------------------------------------------------
/* ---- Beispiel zum Einstellen des Bit Timings ----
 *	
 *	Fosc		= 16MHz
 *	BRP			= 7
 *	TQ 			= 2 * (BRP + 1) / Fosc
 *				= 1 uS
 *
 *	Sync Seg	= 					= 1 TQ
 *	Prop Seg	= (PRSEG + 1) * TQ	= 1 TQ
 *	Phase Seg1	= (PHSEG1 + 1) * TQ	= 3 TQ
 *	Phase Seg2	= (PHSEG2 + 1) * TQ = 3 TQ
 *									--------
 *									  8 TQ
 *	
 *	Bus speed	= 1 / ((Total # of TQ) * TQ)
 *				= 1 / (8 * TQ) = 125 kHz
 */
// -------------------------------------------------------------------------
#include <avr/pgmspace.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <inttypes.h>
#include <stdbool.h>

#include "core/spi.h"
#include "core/bool.h"
#include "can.h"
#include "mcp2515_defs.h"

#ifndef	MCP2515_CLKOUT_PRESCALER
#define	MCP2515_CLKOUT_PRESCALER	0
#endif

// Prototypes
void mcp2515_write_register(uint8_t adress, uint8_t data);
uint8_t mcp2515_read_status(uint8_t type);
uint8_t mcp2515_read_register(uint8_t adress);
void mcp2515_bit_modify(uint8_t adress, uint8_t mask, uint8_t data);

#if	SUPPORT_EXTENDED_CANID
void mcp2515_write_id(const uint32_t *id, uint8_t extended);
uint8_t mcp2515_read_id(uint32_t *id);
#else
void mcp2515_write_id(const uint16_t *id);
uint8_t mcp2515_read_id(uint16_t *id);
#endif	// USE_EXTENDED_CANID

#ifndef	MCP2515_CLKOUT_PRESCALER
#error	MCP2515_CLKOUT_PRESCALER not defined!
#elif MCP2515_CLKOUT_PRESCALER == 0
#define	CLKOUT_PRESCALER_	0x0
#elif MCP2515_CLKOUT_PRESCALER == 1
#define	CLKOUT_PRESCALER_	0x4
#elif MCP2515_CLKOUT_PRESCALER == 2
#define	CLKOUT_PRESCALER_	0x5
#elif MCP2515_CLKOUT_PRESCALER == 4
#define	CLKOUT_PRESCALER_	0x6
#elif MCP2515_CLKOUT_PRESCALER == 8
#define	CLKOUT_PRESCALER_	0x7
#else
#error	invaild value of MCP2515_CLKOUT_PRESCALER
#endif

// CS
#define cs_low_mcp2515()  PIN_CLEAR(SPI_CS_MCP2515)
#define cs_high_mcp2515() PIN_SET(SPI_CS_MCP2515)

const uint8_t _mcp2515_cnf[8][3] PROGMEM =
{
// 10 kbps
		{ 0x04, 0xb6, 0xe7 },
		// 20 kbps
		{ 0x04, 0xb6, 0xd3 },
		// 50 kbps
		{ 0x04, 0xb6, 0xc7 },
		// 100 kbps
		{ 0x04, 0xb6, 0xc3 },
		// 125 kbps
		{ (1 << PHSEG21),					// CNF3
				(1 << BTLMODE) | (1 << PHSEG11),		// CNF2
				(1 << BRP2) | (1 << BRP1) | (1 << BRP0)	// CNF1
		},
		// 250 kbps
		{ 0x03, 0xac, 0x81 },
		// 500 kbps
		{ 0x03, 0xac, 0x80 },
		// 1 Mbps
		{ (1 << PHSEG21), (1 << BTLMODE) | (1 << PHSEG11), 0 } };

// -------------------------------------------------------------------------
// CAN init
bool mcp2515_init(can_bitrate_t bitrate)
{
	if (bitrate >= 8)
		return false;

	// Interrupts sperren
	uint8_t sreg = SREG;
	cli();

	// MCP2515 per Software Reset zuruecksetzten,
	// danach ist er automatisch im Konfigurations Modus
	cs_low_mcp2515();
	spi_send(SPI_RESET);

	_delay_ms(1);

	cs_high_mcp2515();

	// ein bisschen warten bis der MCP2515 sich neu gestartet hat
	_delay_ms(10);

	// CNF1..3 Register laden (Bittiming)
	cs_low_mcp2515();
	spi_send(SPI_WRITE);
	spi_send(CNF3);
	for (uint8_t i = 0; i < 3; i++)
	{
		spi_send(pgm_read_byte(&_mcp2515_cnf[bitrate][i]));
	}
	// aktivieren/deaktivieren der Interrupts
	spi_send((1 << RX1IE) | (1 << RX0IE));
	cs_high_mcp2515();

	// TXnRTS Bits als Inputs schalten
	mcp2515_write_register(TXRTSCTRL, 0);

#ifdef MCP2515_TRANSCEIVER_SLEEP
	// activate the pin RX1BF as GPIO which is connected
	// to RS of MCP2551 and set it to 0
	mcp2515_write_register(BFPCTRL, (1<<B1BFE));
#else
#if MCP2515_RXnBF_OUTPUT
	// activate the pin RX0BF/RX1BF as GPIO
	mcp2515_write_register(BFPCTRL, (1 << B0BFE) | (1 << B1BFE));
#else
	// Deaktivieren der Pins RXnBF Pins (High Impedance State)
	mcp2515_write_register(BFPCTRL, 0);
#endif
#endif
	// Testen ob das auf die beschreibenen Register zugegriffen werden kann
	// (=> ist der Chip ueberhaupt ansprechbar?)
	bool error = false;
	if (mcp2515_read_register(CNF2) != pgm_read_byte(&_mcp2515_cnf[bitrate][1]))
	{
		error = true;
	}

	// Device zurueck in den normalen Modus versetzten
	// und aktivieren/deaktivieren des Clkout-Pins
	mcp2515_write_register(CANCTRL, CLKOUT_PRESCALER_);

	if (error)
	{
		SREG = sreg;
		return false;
	}
	else
	{
		while ((mcp2515_read_register(CANSTAT) & 0xe0) != 0)
		{
			// warten bis der neue Modus uebernommen wurde
		}
		SREG = sreg;
		return true;
	}
}

// ----------------------------------------------------------------------------
// Send message
uint8_t mcp2515_send_message(const can_t *msg, uint8_t tx_buffer)
{
	uint8_t address = 0;
	uint8_t sreg = SREG;

	// Interrupts sperren
	cli();

	if (tx_buffer == 1)
	{
		address = 0x00;
	}
	else if (tx_buffer == 2)
	{
		address = 0x02;
	}
	else if (tx_buffer == 3)
	{
		address = 0x04;
	}

	cs_low_mcp2515();
	spi_send(SPI_WRITE_TX | address);

#if SUPPORT_EXTENDED_CANID
	mcp2515_write_id(&msg->id, msg->flags.extended);
#else
	mcp2515_write_id(&msg->id);
#endif

	uint8_t length = msg->length & 0x0f;

	// Ist die Nachricht ein "Remote Transmit Request" ?
	if (msg->flags.rtr)
	{
		// Ein RTR hat zwar eine Laenge, enthaelt aber keine Daten
		// Nachrichten Laenge + RTR einstellen
		spi_send((1 << RTR) | length);
	}
	else
	{
		// Nachrichten Laenge einstellen
		spi_send(length);

		// Daten
		for (uint8_t i = 0; i < length; i++)
		{
			spi_send(msg->data[i]);
		}
	}
	cs_high_mcp2515();

	_delay_us(1);

	// CAN Nachricht verschicken
	// die letzten drei Bit im RTS Kommando geben an welcher
	// Puffer gesendet werden soll.
	cs_low_mcp2515();
	address = (address == 0) ? 1 : address;
	spi_send(SPI_RTS | address);
	cs_high_mcp2515();

	// clear interrupt flag

	/*if (bit_is_clear(status, 2)) {
	 mcp2515_bit_modify(CANINTF, (1<<TX0IF), 0x00);
	 } else if (bit_is_clear(status, 4)) {
	 mcp2515_bit_modify(CANINTF, (1<<TX1IF), 0x00);
	 } else if (bit_is_clear(status, 6)) {
	 mcp2515_bit_modify(CANINTF, (1<<TX2IF), 0x00);
	 }*/

	SREG = sreg;

	return address;
}

// ----------------------------------------------------------------------------
// Get message
uint8_t mcp2515_get_message(can_t *msg)
{
	uint8_t addr;
	uint8_t sreg = SREG;

	// Interrupts sperren
	cli();

	// read status
	uint8_t status = mcp2515_read_status(SPI_RX_STATUS);

	//debug_printf("status: %02x\n", status);

	if (bit_is_set(status, 6))
	{
		// message in buffer 0
		addr = SPI_READ_RX;
	}
	else if (bit_is_set(status, 7))
	{
		// message in buffer 1
		addr = SPI_READ_RX | 0x04;
	}
	else
	{
		// Error: no message available
		SREG = sreg;
		return 0;
	}

	cs_low_mcp2515();
	spi_send(addr);

	// CAN ID auslesen und ueberpruefen
	uint8_t tmp = mcp2515_read_id(&msg->id);

	//debug_printf("TMP: %0x\n", tmp);

#if SUPPORT_EXTENDED_CANID
	msg->flags.extended = tmp & 0x01;
#else
	if (tmp & 0x01)
	{
		// Nachrichten mit extended ID verwerfen
		cs_high_mcp2515();;

		if (bit_is_set(status, 6))

		mcp2515_bit_modify(CANINTF, (1 << RX0IF), 0);
		else
		mcp2515_bit_modify(CANINTF, (1 << RX1IF), 0);

		SREG = sreg;
		return 0;
	}
#endif

	// read DLC
	uint8_t length = spi_send(0xff);

	msg->flags.rtr = (bit_is_set(status, 3)) ? 1 : 0;

	length &= 0x0f;
	msg->length = length;
	// read data
	for (uint8_t i = 0; i < length; i++)
	{
		msg->data[i] = spi_send(0xff);
	}
	cs_high_mcp2515();

	// clear interrupt flag
	if (bit_is_set(status, 6))
		mcp2515_bit_modify(CANINTF, (1 << RX0IF), 0);
	else
		mcp2515_bit_modify(CANINTF, (1 << RX1IF), 0);

	SREG = sreg;
	return (status & 0x07) + 1;
}

// ----------------------------------------------------------------------------
// check if there are any new messages waiting

bool mcp2515_check_message(void)
{
#if defined(HAVE_SPI_CS_MCP2515)
	return (!(PIN_HIGH(MCP2515_INT)) ? true : false);
#else
	return ((mcp2515_read_status(SPI_RX_STATUS) & 0xC0) ? true : false);
#endif

}

// ----------------------------------------------------------------------------
// Filter setzen

void mcp2515_static_filter(const uint8_t *filter)
{
	uint8_t sreg = SREG;

	// Interrupts sperren
	cli();

	// change to configuration mode
	mcp2515_bit_modify(CANCTRL, 0xe0, (1 << REQOP2));
	while ((mcp2515_read_register(CANSTAT) & 0xe0) != (1 << REQOP2))
		;

	mcp2515_write_register(RXB0CTRL, (1 << BUKT));
	mcp2515_write_register(RXB1CTRL, 0);

	uint8_t i, j;
	for (i = 0; i < 0x30; i += 0x10)
	{
		cs_low_mcp2515();
		spi_send(SPI_WRITE);
		spi_send(i);

		for (j = 0; j < 12; j++)
		{
			if (i == 0x20 && j >= 0x08)
				break;

			spi_send(pgm_read_byte(filter++));
		}
		cs_high_mcp2515();
	}

	mcp2515_bit_modify(CANCTRL, 0xe0, 0);

	SREG = sreg;
}

// ----------------------------------------------------------------------------
// check if there is a free buffer to send messages

bool mcp2515_check_free_buffer(void)
{
	uint8_t sreg = SREG;

	// Interrupts sperren
	cli();

	uint8_t status = mcp2515_read_status(SPI_READ_STATUS);

	if ((status & 0x54) == 0x54)
	{
		return false;		// all buffers used
		SREG = sreg;
	}
	else
	{
		SREG = sreg;
		return true;
	}
}

// ----------------------------------------------------------------------------
// Buffer status

uint8_t mcp2515_get_free_buffer(void)
{
	uint8_t sreg = SREG;
	uint8_t status;

	// Interrupts sperren
	cli();

	status = mcp2515_read_status(SPI_READ_STATUS);

	SREG = sreg;

	return status;
}

// ----------------------------------------------------------------------------
can_error_register_t mcp2515_read_error_register(void)
{
	uint8_t sreg = SREG;
	can_error_register_t error;

	// Interrupts sperren
	cli();

	error.tx = mcp2515_read_register(TEC);
	error.rx = mcp2515_read_register(REC);

	SREG = sreg;

	return error;
}

// ----------------------------------------------------------------------------
uint8_t mcp2515_read_error_flags(void)
{
	uint8_t sreg = SREG;
	uint8_t errorflags;

	// Interrupts sperren
	cli();

	errorflags = mcp2515_read_register(EFLG);

	SREG = sreg;

	return errorflags;
}
// ----------------------------------------------------------------------------
// Internal functions
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Liest eine ID aus dem Registern des MCP2515 (siehe auch mcp2515_write_id())

#if	SUPPORT_EXTENDED_CANID

uint8_t mcp2515_read_id(uint32_t *id)
{
	uint8_t first;
	uint8_t tmp;

	first = spi_send(0xff);
	tmp = spi_send(0xff);

	if (tmp & (1 << IDE))
	{
		*((uint16_t *) id + 1) = (uint16_t) first << 5;
		*((uint8_t *) id + 1) = spi_send(0xff);
		*((uint8_t *) id + 2) |= (tmp >> 3) & 0x1C;
		*((uint8_t *) id + 2) |= tmp & 0x03;
		*((uint8_t *) id) = spi_send(0xff);

		return TRUE;
	}
	else
	{
		spi_send(0xff);

		*((uint8_t *) id + 3) = 0;
		*((uint8_t *) id + 2) = 0;
		*((uint16_t *) id) = (uint16_t) first << 3;

		spi_send(0xff);
		*((uint8_t *) id) |= tmp >> 5;

		return FALSE;
	}
}

#else

uint8_t mcp2515_read_id(uint16_t *id)
{
	uint8_t first;
	uint8_t tmp;

	first = spi_send(0xff);
	tmp = spi_send(0xff);

	if (tmp & (1 << IDE))
	{
		spi_send(0xff);
		spi_send(0xff);

		return 1;			// extended-frame
	}
	else
	{
		spi_send(0xff);
		*id = (uint16_t) first << 3;
		spi_send(0xff);
		*((uint8_t *) id) |= tmp >> 5;

		if (tmp & (1 << SRR))
		return 2;		// RTR-frame
		else
		return 0;// normal-frame
	}
}

#endif	// SUPPORT_EXTENDED_CANID

// ----------------------------------------------------------------------------
void mcp2515_set_mode(can_mode_t mode)
{
	uint8_t reg = 0;

	if (mode == LISTEN_ONLY_MODE)
	{
		reg = (1 << REQOP1) | (1 << REQOP0);
	}
	else if (mode == LOOPBACK_MODE)
	{
		reg = (1 << REQOP1);
	}
	else if (mode == SLEEP_MODE)
	{
		reg = (1 << REQOP0);
	}

	// set the new mode
	mcp2515_bit_modify(CANCTRL, (1 << REQOP2) | (1 << REQOP1) | (1 << REQOP0),
			reg);
	while ((mcp2515_read_register(CANSTAT) & 0xe0) != reg)
	{
		// wait for the new mode to become active
	}
}

#if MCP2515_RXnBF_OUTPUT
// ----------------------------------------------------------------------------
void mcp2515_rx0bf(uint8_t level)
{
	uint8_t sreg = SREG;

	// Interrupts sperren
	cli();

	if (level)
		mcp2515_bit_modify(BFPCTRL, (1 << B0BFS), (1 << B0BFS));
	else
		mcp2515_bit_modify(BFPCTRL, (1 << B0BFS), 0);

	SREG = sreg;
}

// ----------------------------------------------------------------------------
void mcp2515_rx1bf(uint8_t level)
{
	uint8_t sreg = SREG;

	// Interrupts sperren
	cli();

	if (level)
		mcp2515_bit_modify(BFPCTRL, (1 << B1BFS), (1 << B1BFS));
	else
		mcp2515_bit_modify(BFPCTRL, (1 << B1BFS), 0);

	SREG = sreg;
}
#endif

// ----------------------------------------------------------------------------
void mcp2515_sleep(void)
{
	// put also the 2551 in standby mode
	// for this, connect RX1BF to the RS pin of the 2551
	mcp2515_bit_modify(BFPCTRL, (1 << B1BFS), (1 << B1BFS));

	// put the 2515 in sleep more
	mcp2515_set_mode(SLEEP_MODE);

	// enable generating an interrupt for wakeup when activity on bus
	mcp2515_bit_modify(CANINTE, (1 << WAKIE), (1 << WAKIE));
}

// ----------------------------------------------------------------------------
void mcp2515_wakeup(void)
{
	// reset int enable and cancel the interrupt flag
	mcp2515_bit_modify(CANINTE, (1 << WAKIE), 0);
	mcp2515_bit_modify(CANINTF, (1 << WAKIF), 0);

	// re-enable the 2551
	mcp2515_bit_modify(BFPCTRL, (1 << B1BFS), 0);

	// when we get up of sleep, we are in listen mode, return into normal mode
	mcp2515_set_mode(NORMAL_MODE);
}

// ----------------------------------------------------------------------------
/* Schreibt eine CAN ID in die Register des MCP2515
 *
 * Die Funktion setzt eine offene Verbindung zum MCP2515 vorraus
 * und schreibt dann die CAN ID per SPI in die folgenden vier
 * Register des MCP2515.
 *
 * ACHTUNG: die Funktion wurde "optimiert", damit nicht ständig unnötige
 * 			32-Bit Operationen verwendet werden :)
 *
 * Funktionell aequivalent zu:
 *
 *	static void mcp2515_write_id(uint32_t *id, uint8_t extended)
 *	{
 *		if (extended) {
 *			spi_send(*id >> 21);
 *			spi_send(((*id >> 13) & 0xe0) | (1<<IDE) | ((*id >> 16) & 0x3));
 *			spi_send(*id >> 8);
 *			spi_send(*id);
 *		}
 *		else {
 *			spi_send(*id >> 3);
 *			spi_send(*id << 5);
 *			spi_send(0);
 *			spi_send(0);
 *		}
 *	}
 */

#if SUPPORT_EXTENDED_CANID

void mcp2515_write_id(const uint32_t *id, uint8_t extended)
{
	uint8_t tmp;

	if (extended)
	{
		spi_send(*((uint16_t *) id + 1) >> 5);

		// naechsten Werte berechnen
		tmp = (*((uint8_t *) id + 2) << 3) & 0xe0;
		tmp |= (1 << IDE);
		tmp |= (*((uint8_t *) id + 2)) & 0x03;

		// warten bis der vorherige Werte geschrieben wurde
		//spi_wait();

		// restliche Werte schreiben
		spi_send(tmp);
		spi_send(*((uint8_t *) id + 1));
		spi_send(*((uint8_t *) id));
	}
	else
	{
		spi_send(*((uint16_t *) id) >> 3);

		// naechsten Werte berechnen
		tmp = *((uint8_t *) id) << 5;
		//spi_wait();

		spi_send(tmp);
		spi_send(0);
		spi_send(0);
	}
}

#else

void mcp2515_write_id(const uint16_t *id)
{
	uint8_t tmp;

	spi_send(*id >> 3);
	tmp = *((uint8_t *) id) << 5;
	//spi_wait();

	spi_send(tmp);
	spi_send(0);
	spi_send(0);
}

#endif	// USE_EXTENDED_CANID

// -------------------------------------------------------------------------
void mcp2515_write_register(uint8_t adress, uint8_t data)
{
	cs_low_mcp2515();

	spi_send(SPI_WRITE);
	spi_send(adress);
	spi_send(data);

	cs_high_mcp2515();
}

// -------------------------------------------------------------------------
uint8_t mcp2515_read_register(uint8_t adress)
{
	uint8_t data;

	cs_low_mcp2515();

	spi_send(SPI_READ);
	spi_send(adress);

	data = spi_send(0xff);

	cs_high_mcp2515();

	return data;
}

// -------------------------------------------------------------------------
void mcp2515_bit_modify(uint8_t adress, uint8_t mask, uint8_t data)
{
	cs_low_mcp2515();

	spi_send(SPI_BIT_MODIFY);
	spi_send(adress);
	spi_send(mask);
	spi_send(data);

	cs_high_mcp2515();
}

// ----------------------------------------------------------------------------
uint8_t mcp2515_read_status(uint8_t type)
{
	uint8_t data;

	cs_low_mcp2515();

	spi_send(type);
	data = spi_send(0xff);

	cs_high_mcp2515();

	return data;
}
