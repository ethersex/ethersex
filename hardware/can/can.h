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
 */

#ifndef _CAN_H
#define _CAN_H

#include <avr/pgmspace.h>
#include <stdint.h>
#include <stdbool.h>
#include "config.h"

// Bitraten fuer den CAN-Bus
typedef enum
{
	BITRATE_10_KBPS = 0,	// ungetestet
	BITRATE_20_KBPS = 1,	// ungetestet
	BITRATE_50_KBPS = 2,	// ungetestet
	BITRATE_100_KBPS = 3,	// ungetestet
	BITRATE_125_KBPS = 4,
	BITRATE_250_KBPS = 5,	// ungetestet
	BITRATE_500_KBPS = 6,	// ungetestet
	BITRATE_1_MBPS = 7,		// ungetestet
} can_bitrate_t;

// Symbol um auf alle Filter zuzugreifen
#define	CAN_ALL_FILTER		0xff

// Set to 1 for use RX0BF and RX1BF as GPIO
#define MCP2515_RXnBF_OUTPUT 	0

#if SUPPORT_EXTENDED_CANID
#define MCP2515_FILTER_EXTENDED(id)	\
				(uint8_t)  ((uint32_t) (id) >> 21), \
				(uint8_t)((((uint32_t) (id) >> 13) & 0xe0) | (1<<3) | \
					(((uint32_t) (id) >> 16) & 0x3)), \
				(uint8_t)  ((uint32_t) (id) >> 8), \
				(uint8_t)  ((uint32_t) (id))
#endif

#define	MCP2515_FILTER(id) \
			(uint8_t)((uint32_t) id >> 3), \
			(uint8_t)((uint32_t) id << 5), \
			0, \
			0

// Datenstruktur zum Aufnehmen von CAN Nachrichten
typedef struct
{
#if SUPPORT_EXTENDED_CANID
	uint32_t id;				//!< ID der Nachricht (11 oder 29 Bit)
	struct
	{
		int rtr : 1;			//!< Remote-Transmit-Request-Frame?
		int extended : 1;		//!< extended ID?
	}flags;
#else
	uint16_t id;				//!< ID der Nachricht (11 Bit)
	struct
	{
		int rtr :1;				//!< Remote-Transmit-Request-Frame?
	} flags;
#endif

	uint8_t length;				//!< Anzahl der Datenbytes
	uint8_t data[8];			//!< Die Daten der CAN Nachricht

} can_t;

// Inhalt der Fehler-Register
typedef struct
{
	uint8_t rx;					//!< Empfangs-Register
	uint8_t tx;					//!< Sende-Register
} can_error_register_t;

// Modus des CAN Interfaces
typedef enum
{
	LISTEN_ONLY_MODE,		//!< der CAN Contoller empfängt nur und verhält sich völlig passiv
	LOOPBACK_MODE,			//!< alle Nachrichten direkt auf die Empfangsregister umleiten ohne sie zu senden
	NORMAL_MODE,			//!< normaler Modus, CAN Controller ist aktiv
	SLEEP_MODE				// sleep mode
} can_mode_t;

//Gewuenschte Geschwindigkeit des CAN Interfaces
extern bool mcp2515_init(can_bitrate_t bitrate);

// ----------------------------------------------------------------------------
/**
 * CAN interface to sleep and wake up
 *
 * MCP2515 active : 5mA
 * MCP2515 sleep  : 1µA
 *
 * MCP2551 active : 10mA+
 * MCP2551 sleep  : 400µA
 *
 * Code:
 * // before we are going to sleep, enable the interrupt that will wake us up
 // attach interrupt 1 to the routine
 EICRA = 0;  // int on low level
 // Enable the interrupt1
 EIMSK = _BV(INT1);

 // put the MCP2515 to sleep
 can_sleep();

 // enable atmega sleep mode
 cli();
 sleep_enable();
 // turn off BOD
 sleep_bod_disable();
 // and we go to sleep
 sei();
 sleep_cpu();

 // here int1 has been executed and we are woken up
 sleep_disable();

 // disable int1
 EIMSK = 0;

 // re-enable 2515 and 2551
 can_wake();
 *
 */
extern void mcp2515_sleep(void);

extern void mcp2515_wakeup(void);

// Filter setzten
extern void mcp2515_static_filter(const uint8_t *filter_array);

// Ueberpruefen ob neue CAN Nachrichten vorhanden sind
extern bool mcp2515_check_message(void);

// Check Buffer
extern bool mcp2515_check_free_buffer(void);

// Zustand Buffer
extern uint8_t mcp2515_get_free_buffer(void);

// Versendet eine Nachricht über den CAN Bus
extern uint8_t mcp2515_send_message(const can_t *msg, uint8_t tx_buffer);

// Liest eine Nachricht aus den Empfangspuffern des CAN Controllers
extern uint8_t mcp2515_get_message(can_t *msg);

//  Liest den Inhalt der Fehler-Register
extern can_error_register_t mcp2515_read_error_register(void);

//  Liest den Inhalt der Fehlerflags-Register
extern uint8_t mcp2515_read_error_flags(void);

// Setzt den Operations-Modus
extern void mcp2515_set_mode(can_mode_t mode);

#ifdef DEBUG_CAN
# include "core/debug.h"
# define CANDEBUG(a...)  debug_printf("CAN: " a)
#else
# define CANDEBUG(a...)
#endif

// CAN Init
extern void can_init(void);
// Sende CAN Message
extern bool can_send_message(can_t *message);
// Lese CAN Message
extern bool can_get_message(can_t *message);
// Versendet CAN-Message aus dem FIFO (für den zyklischen Aufruf)
extern void can_send_tx_fifo(void);

// Anzahl Messages in TX-FIFO
extern uint8_t can_get_messages_in_tx_fifo(void);
// Ausgabe einer CAN-Nachricht
extern void can_print_message(const can_t *message);

// Gesamtzahl empfangener CAN Messages
extern uint32_t can_get_counter_messages_received(void);
// Gesamtzahl gesendeter CAN Messages
extern uint32_t can_get_counter_messages_send(void);
// Max. Anzahl Messages im TX-FIFO
extern uint8_t can_get_counter_fifo_max_tx(void);
// Stellt Counter zurück
extern void can_reset_counter(void);


#ifdef MCP2515_INTERRUPT
// Anzahl Messages in RX-FIFO
extern uint8_t can_get_messages_in_rx_fifo(void);
// Daten im RX-FIFO
extern uint8_t can_check_rx_fifo(void);
// Max. Anzahl Messages im RX-FIFO
extern uint8_t can_get_counter_fifo_max_rx(void);
#endif // MCP2515_INTERRUPT

#if MCP2515_RXnBF_OUTPUT
// Set Level Pin RX0BF
void mcp2515_rx0bf(uint8_t level);
// Set Level Pin RX1BF
void mcp2515_rx1bf(uint8_t level);
#endif // MCP2515_RXnBF_OUTPUT

#endif // _CAN_H
