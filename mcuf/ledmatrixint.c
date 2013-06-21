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
}}} */

#include "mcuf.h"
#include "ledmatrixint.h"
#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "config.h"


#ifdef LEDRG_SUPPORT

#if (use_low_colors == 0)
//welcher Durchlauf, wird bei use_low_colors nicht benoetigt.
uint8_t volatile gdurchlauf;
#endif
uint8_t volatile gzeile;       //Welche Zeile gerade behandelt wird

/*256Byte fuer die Graphic Daten (12,5% des gesamten SRAMs bei screenx und
  screeny = 16 und 2KB RAM)
  Die Daten fuers Display[y;Zeile][x;Spalte]
*/
//uint8_t volatile gdata[screeny][screenx];

void init_led_display(void) {
  /*Setzen der Data Direction Regiser auf Ausgang
  LED_RESET_DDR |= (1<<LED_RESET_PIN);
  LED_SELECT_DDR |= (1<<LED_SELECT_PIN);
  LED_BRIGHT_DDR |= (1<<LED_BRIGHT_PIN);
  LED_RED_DDR |= (1<<LED_RED_PIN);
  LED_GREEN_DDR |= (1<<LED_GREEN_PIN);
  LED_CLOCK_DDR |= (1<<LED_CLOCK_PIN);
  */
  TC2_COUNTER_CURRENT = 0;
  TC2_INT_OVERFLOW_ON;
  TC2_PRESCALER_32;
  PIN_SET(LED_RESET);      //LED_RESET_PORT |= (1<<LED_RESET_PIN); //Reset high
  gzeile = 0;                   //beginne mit Zeile 0
  PIN_CLEAR(LED_RESET); //Reset low
  PIN_SET(LED_SELECT);     //LED_SELECT_PORT |= (1<<LED_SELECT_PIN); //Select auf high
}

void resync_led_display(void) {
  //Setzt einmal Reset
  cli();
  PIN_SET(LED_RESET); //Reset high
  gzeile = 0;
  PIN_CLEAR(LED_RESET); //Reset lo
  sei();
}

#if (use_low_colors == 0)
//Verwende Variante mit 4Bit Farben (16 moegliche Farbtoene)

//4+24+19+(12+7+9)*16+7+5+4+27=538Takte pro Interrupt
ISR(TC2_VECTOR_OVERFLOW)
{
  uint8_t gspalte;
  uint8_t gtemp,gdbyte;
  uint8_t gdurchlauf_t;
  uint8_t gzeile_t;

  TC2_COUNTER_CURRENT = (uint8_t)timerset;
  PIN_SET(LED_BRIGHT); //Bright auf high
  _delay_us(4);
  //gdurchlauf_t und gzeile_t sind nicht volatile -> Speicher und Platz Ersparnis
  gdurchlauf_t = gdurchlauf;
  gzeile_t = gzeile;
  gspalte = 0;
  while (gspalte != screenx) {
    gdbyte = gdata[gzeile_t][gspalte]; //gdata ist volatile, gtemp nicht
    PIN_CLEAR(LED_RED); //Rote LED Leitung sicher aus
    PIN_CLEAR(LED_GREEN); //Gruene LED Leitung sicher aus
    /* aus gtemp werden die fuer die rote LED wichtigen Bits extrahiert und die
      LED in Abhaengikeit von gdurchlauf_t entweder ein oder Ausgeschaltet.
      So koennen durch schnelles Ein- und Ausschalten drei verschiedene
      Helligkeitsstufen (+ganz aus) angezeigt werden. */
    gtemp = gdbyte & 0x03; //Dies in der If Verzweigung ->16Bit ->langsamer+groesser
    if (gdurchlauf_t < gtemp) {
      PIN_SET(LED_RED);//Rote LED an
    }
    gdbyte = (gdbyte>>4) & 0x03;
    if (gdurchlauf_t < gdbyte)  {
      PIN_SET(LED_GREEN);//Gruene LED an
    }
    //Clock Leitung auf high, Datenuebernahme durch Takt
    PIN_SET(LED_CLOCK);
    PIN_CLEAR(LED_CLOCK); //Clock Leitung auf low
    gspalte++;
  }
  _delay_us(1);
  PIN_CLEAR(LED_BRIGHT); //Bright auf Low
  gzeile_t++;
  if (gzeile_t == screeny) {
    PIN_SET(LED_RESET); //Reset high
    gzeile_t = 0;
    gdurchlauf_t++;
    PIN_CLEAR(LED_RESET); //Reset lo
  }
  if (gdurchlauf_t == 4) {
    gdurchlauf_t = 0;
  }
  //zurueck in die volatile Variablen schreiben
  gdurchlauf = gdurchlauf_t;
  gzeile = gzeile_t;
}

#else
/*Verwende Variante mit 2Bit Farben (4 moegliche Farbtoene)
  2Bit Farben ist das, was das LED Panel eigentlich nur darstellen kann und
  deshalb nicht durch Software PWM simuliert werden muss. Allerdings sieht die
  Demo dann nicht annaehernd so gut aus. Da Software PWM weg faellt, koennte der
  MCU erheblich im Bezug auf Rechenleistung entlastet werden und so problemlos
  mehrere LED Panels ansteuern. Diese Moeglichkeit der reduzierten Rechenleistung
  wurde hier jedoch nicht realisiert. Je nachdem auf welchen Wert gdurchlauf_t
  gesetzt wurde, werden die dunklen Farben mit maximaler Helligkeit angezeigt
  oder ganz ausgelassen!. gdurchlauf_t = 0 zeigt auch die dunkelste Farbe an,
  gdurchlauf_t = 3 nur die allerhellsten.
*/

ISR(TC2_VECTOR_OVERFLOW) {
  uint8_t gspalte;
  uint8_t gtemp,gdbyte;
  const uint8_t gdurchlauf_t = 1;
  uint8_t gzeile_t;

  TC2_COUNTER_CURRENT = (uint8_t)timerset;
  PIN_SET(LED_BRIGHT); //Bright auf high
  _delay_us(9);
  //gdurchlauf_t und gzeile_t sind nicht volatile -> Speicher und Platz Ersparnis
  gzeile_t = gzeile;
  gspalte = 0;
  while (gspalte != screenx) {
    gdbyte = gdata[gzeile_t][gspalte]; //gdata ist volatile, gtemp nicht
    PIN_CLEAR(LED_RED); //Rote LED Leitung sicher aus
    PIN_CLEAR(LED_GREEN); //Gruene LED Leitung sicher auss
    /* aus gtemp werden die fuer die rote LED wichtigen Bits extrahiert und die
      LED in Abhaengikeit von gdurchlauf_t entweder ein oder Ausgeschaltet.
    */
    gtemp = gdbyte & 0x03; //Dies in der If Verzweigung ->16Bit ->langsamer+groesser
    if (gdurchlauf_t < gtemp) {
      PIN_SET(LED_RED);//Rote LED an
    }
    gdbyte = (gdbyte>>4) & 0x03;
    if (gdurchlauf_t < gdbyte)  {
      PIN_SET(LED_GREEN);//Gruene LED an
    }
    //Clock Leitung auf high, Datenuebernahme durch Takt
    PIN_SET(LED_CLOCK);
    PIN_CLEAR(LED_CLOCK); //Clock Leitung auf low
    gspalte++;
  }
  PIN_CLEAR(LED_BRIGHT); //Bright auf Low
  _delay_us(3);
  gzeile_t++;
  if (gzeile_t == screeny) {
    gzeile_t = 0;
  }
  //zurueck in die volatile Variablen schreiben
  gzeile = gzeile_t;
}

#endif
#endif /* LEDRG_SUPPORT */
