/*
 *
 * Copyright (c) 2009 by Dirk Pannenbecker <dp@sd-gp.de>
 *
 * taken from:
 *   http://www.marwedels.de/malte/ledmatrix/ledmatrix.html
 *  Gamebox
 *  Copyright (C) 2004-2006  by Malte Marwedel
 *  m.marwedel AT onlinehome dot de
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

//Funktionsprototypen fuer Graphic
#ifndef LEDMATRIXINT_H
#define LEDMATRIXINT_H

#include "mcuf.h"

//Pin Belegung des LED Displays

//Die Reset Leitung
// #define LED_RESET_PORT  PORTA
// #define LED_RESET_DDR   DDRA
// #define LED_RESET_PIN   4
//Die Select Leitung
// #define LED_SELECT_PORT PORTA
// #define LED_SELECT_DDR  DDRA
// #define LED_SELECT_PIN  5
//Die Bright Leitung
// #define LED_BRIGHT_PORT PORTA
// #define LED_BRIGHT_DDR  DDRA
// #define LED_BRIGHT_PIN  3
//Die rote LED Datenleitung
// #define LED_RED_PORT    PORTA
// #define LED_RED_DDR     DDRA
// #define LED_RED_PIN     0
//Die gruene LED Datenleitung
// #define LED_GREEN_PORT  PORTA
// #define LED_GREEN_DDR   DDRA
// #define LED_GREEN_PIN   1
//Die Clock Leitung
// #define LED_CLOCK_PORT  PORTA
// #define LED_CLOCK_DDR   DDRA
// #define LED_CLOCK_PIN   2

//Konstanten, Aendern nur mit Vorsicht

/*Definieren der LED Feld Groesse.
Achtung: Alle Funktionen wurden nur mit screenx = 16 und screeny = 16 getestet.
Manche Funktionen sind nicht explizit dafuer ausgelegt, mit groesseren
'Aufloesungen' zu funktionieren. Mit 'Aufloesungen' die mehr als 254 Pixel
Kantenlaege haben, werden die Funktionieren nicht funktionieren.
*/
#define screenx MCUF_MIN_SCREEN_WIDTH
#define screeny MCUF_MAX_SCREEN_HEIGHT

#if (screenx > screeny)
#define maxscreen screenx
#else
#define maxscreen screeny
#endif

/* Wird use_low_colors auf eins gesetzt, so wird kein Software PWM zum steuern
   der LED Helligkeit verwendet. Die Farbtiefe wird folglich von 4 auf 2 Bit
   reduziert. Die Demo sieht mit 2Bit Farben doch recht langweilig aus.
*/
#define use_low_colors 0

/* check if cpu speed is defined */
#ifndef F_CPU
#error "please define F_CPU!"
#endif
/*Berechnung der Timer Geschwindigkeit.
Der Timer2 soll alle 4800mal pro Sekunde ausloesen. (100Hz*16Zeilen*3Durchlaeufe)
Der Prescaler des Timers ist 8. Der timer laeuft bis 255 und ruft dann den
Interrupt auf. Daraus folgt:
4800 = (F_CPU/8)/(255-timerset)
umgestellt:
255-timerset = (F_CPU/8)/4800
255 = (FCPU/8)/4800)+timerset
255-((FCPU/8)/4800) = timerset
*/
#define timerset_test (255-(F_CPU/32/4800))

#if (timerset_test < 0)
#define timerset 0
#else
#define timerset (uint8_t)timerset_test
#endif
extern uint8_t volatile gdurchlauf;   //welcher Durchlauf
extern uint8_t volatile gzeile;       //Welche Zeile gerade behandelt wird
//Die Daten fuers Display[y;Zeile][x;Spalte]
// extern uint8_t volatile gdata[screeny][screenx];


void init_led_display(void); //Initialisiert das LED Display
void resync_led_display(void); //kurz ein Reset bei eventuellen Fehlern

#endif /* LEDMATRIXINT_H */
