/*
 * Copyright (c) 2007 by Jochen Roessner <jochen@lugrot.de>
 * Copyright (c) 2008 by Jochen Roessner <jochen@lugrot.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
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

#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>

#include "../net/sensormodul_state.h"
#include "../net/sensormodul_net.h"

#include "../uip/uip.h"
#include "../config.h"
#include "sensormodul.h"

#ifdef SENSORMODUL_SUPPORT
#define SENSORMODUL_DEBUG
#ifdef SENSORMODUL_DEBUG
#define NIBBLE_TO_HEX(a) ((a) < 10 ? (a) + '0' : ((a) - 10 + 'A'))
#endif
#include "lcd.h"
#include "../lcd/hd44780.h"
#include "kty81.h"

extern unsigned short rfm12_t_status;
extern uint8_t RFM12_akt_status;
//static uint16_t sensorwert[4];
static uint8_t sensor_i = 0;
//static uint8_t countdown = 0;
//static uint8_t startok = 0;

enum ledstate{
  CONDITION_GREEN,
  CONDITION_OFF,
  CONDITION_YELLOW,
  CONDITION_RED
};

//enum ledstate STATS.sensors.ledstate_akt=CONDITION_OFF;
uint8_t EEMEM maxfeuchte_div_eep[5];

#define BUF ((struct uip_udpip_hdr *) (uip_appdata - UIP_IPUDPH_LEN))
#define STATS (uip_udp_conn->appstate.sensormodul)

void 
sensormodul_core_init(uip_udp_conn_t *sensormodul_conn)
{
  PORTD |= _BV(PD3); // Taster Pullup einschalten
  DDRB |= _BV(PB0) | _BV(PB1); //LED Alarm ausgang
  /* Init des ADC mit Taktteiler von 64 */
  ADCSRA = _BV(ADEN) | _BV(ADPS2) | _BV(ADPS1);
  
  /* Aktivierung des Pin 0 (ADC0) fr die Messung 
  */

  sensor_i = SENSORMODUL_ADCMAX - 1;
  ADMUX = sensor_i;
#ifndef HD44780_SUPPORT
  lcd_init();
#endif
  //lcd_print("Japaadappadu :-)");
  //lcd_goto_ddram(LCD_SECOND_LINE);
  for(sensor_i=0;sensor_i<sizeof(sensormodul_conn->appstate.sensormodul.sensors.maxfeuchte_div);sensor_i++){
   if(eeprom_read_byte(&maxfeuchte_div_eep[sensor_i]) != 0xFF)
     sensormodul_conn->appstate.sensormodul.sensors.maxfeuchte_div[sensor_i] = eeprom_read_byte(&maxfeuchte_div_eep[sensor_i]);
  }
  sensormodul_conn->appstate.sensormodul.sensors.ledstate_akt=CONDITION_OFF;
}

void
sensormodul_core_newdata(void)
{
  struct sensormodul_request_t *REQ = uip_appdata;

  if ( uip_datalen() == sizeof(struct sensormodul_request_t) && REQ->type > '0' && REQ->type < '6' ){
    STATS.sensors.maxfeuchte_div[(REQ->type & 0x07) - 1] = ((REQ->digit[2] & 0x0F) + (REQ->digit[1] & 0x0F) * 10 + (REQ->digit[0] & 0x0F) * 100) & 0xFF;
    eeprom_write_byte(&maxfeuchte_div_eep[(REQ->type & 0x07) - 1], STATS.sensors.maxfeuchte_div[(REQ->type & 0x07) - 1]);
  }
  if ( uip_datalen() == 2 && REQ->type == '!'){
    PORTB = PORTB & 0xFC | (REQ->digit[0] & 0x03);
  }
  if ( uip_datalen() == 2 && REQ->type == '?'){
    STATS.sensors.led_blink = REQ->digit[0] & 0x03;
  }
  if ( uip_datalen() == sizeof(struct sensormodul_request_t) && REQ->type == 'c'){
    STATS.sensors.countdown = ((REQ->digit[2] & 0x0F) + (REQ->digit[1] & 0x0F) * 10 + (REQ->digit[0] & 0x0F) * 100) & 0xFF;
  }
#ifndef SENSORMODUL_DEBUG
  if (uip_datalen() <= LCD_PYSICAL_LINELEN * 2 + 1 && REQ->type == ':'){
    uip_ipaddr_copy(STATS.ripaddr, BUF->srcipaddr);
    STATS.rport = BUF->srcport;
    STATS.sensors.tastersend = 0;
    lcd_clear();
    lcd_home();
    REQ->data[uip_datalen()-1] = 0;
    if (uip_datalen() > LCD_PYSICAL_LINELEN + 1){
#ifdef HD44780_SUPPORT
      hd44780_goto(1, 0);
#else
      lcd_goto_ddram(LCD_SECOND_LINE);
#endif
      lcd_print(REQ->data+LCD_PYSICAL_LINELEN);
    }
#ifdef HD44780_SUPPORT
    hd44780_goto(0, 0);
#else
    lcd_goto_ddram(0);
#endif
    lcd_print(REQ->data);
    STATS.sensors.lcd_blocked = 1;
  }
#endif /*SENSORMODUL_DEBUG*/
}

void 
sensormodul_core_periodic(void)
{
  //if(start < 100)
  //  start++;
  //else{

  if((PIND & _BV(PD3)) == 0){
    STATS.sensors.tastersend++;
    STATS.sensors.lcd_blocked = 1;
    STATS.sensors.led_blink = 0;
    STATS.sensors.countdown = 5;
#ifndef SENSORMODUL_DEBUG
    lcd_clear();
    lcd_home();
    uint8_t i;
    for(i=0;i<sizeof(STATS.sensors.maxfeuchte_div);i++){
      temp2text(STATS.sensors.sensor[2].valuetext, STATS.sensors.maxfeuchte_div[i]);
      lcd_print(STATS.sensors.sensor[2].valuetext);
      if(i == 2)
#ifdef HD44780_SUPPORT
        hd44780_goto(1, 0);
#else
        lcd_goto_ddram(LCD_SECOND_LINE);
#endif
    }
#endif /* SENSORMODUL_DEBUG */
    uip_udp_conn_t return_conn;
    uip_ipaddr_copy(return_conn.ripaddr, STATS.ripaddr);
    return_conn.rport = STATS.rport;
    return_conn.lport = HTONS(SENSORMODUL_PORT);

    uip_send (&STATS, sizeof(struct sensormodul_datas_t));
    uip_udp_conn = &return_conn;
    /* Send immediately */
    uip_process(UIP_UDP_SEND_CONN);
    fill_llh_and_transmit();
    uip_slen = 0;
  }
  else{
    /* Start der Konvertierung */
    ADCSRA |= _BV(ADSC);
    while (ADCSRA & _BV(ADSC));

    STATS.sensors.sensor[sensor_i].value = ADC; //sensorwert[sensor_i] = ADC;

    if(sensor_i == 0) {//  && start != 0){
      temp2text(STATS.sensors.sensor[sensor_i].valuetext, temperatur(STATS.sensors.sensor[sensor_i].value));
      if(!STATS.sensors.lcd_blocked){
#ifdef HD44780_SUPPORT
        hd44780_goto(0, 0);
#else
        lcd_goto_ddram(0);
#endif
        lcd_print("Ra");
        lcd_print(STATS.sensors.sensor[sensor_i].valuetext);
      }
    }
    if(sensor_i == 1) {//  && start != 0){
      temp2text(STATS.sensors.sensor[sensor_i].valuetext, temperatur(STATS.sensors.sensor[sensor_i].value));
      if(!STATS.sensors.lcd_blocked){
#ifdef HD44780_SUPPORT
        hd44780_goto(0, 7);
#else
        lcd_goto_ddram(7);
#endif
        lcd_print(" Eck");
        lcd_print(STATS.sensors.sensor[sensor_i].valuetext);
      }
    }
    if(sensor_i == 3) {//  && start != 0){
      //char textbuf[6];
      uint16_t promille = STATS.sensors.sensor[sensor_i].value - (STATS.sensors.sensor[sensor_i].value >> 6) - (STATS.sensors.sensor[sensor_i].value >> 7) - (STATS.sensors.sensor[sensor_i].value >> 8);
      temp2text(STATS.sensors.sensor[sensor_i].valuetext, promille);
      /* warn leds */
      int16_t Trel_raum = temperatur(STATS.sensors.sensor[0].value);
      int16_t Trel_wand = temperatur(STATS.sensors.sensor[1].value);
      Trel_raum = (Trel_raum >> 3) + (Trel_raum >> 5) + (Trel_raum >> 4) + Trel_raum;
      Trel_wand = (Trel_wand >> 3) + (Trel_wand >> 5) + (Trel_wand >> 4) + Trel_wand;
      //Tx1=T/8+T/64+T/16+T
      //Tx2=T/8+T/64+T/16+T
      //feuchte(promille)=1000*Tx1/Tx2
  
      int16_t maxfeuchte = (100*Trel_wand)/(Trel_raum/10)-50;
      if(promille > (maxfeuchte - STATS.sensors.maxfeuchte_div[0]) && STATS.sensors.ledstate_akt == CONDITION_YELLOW){ //feuchte zu hoch
        PORTB |= _BV(PB0);
        PORTB &= ~_BV(PB1);
        STATS.sensors.ledstate_akt = CONDITION_RED;
        STATS.sensors.lcd_blocked = 0;
      }
      else if((promille > (maxfeuchte - STATS.sensors.maxfeuchte_div[1]) && STATS.sensors.ledstate_akt == CONDITION_OFF) || (promille < (maxfeuchte - (STATS.sensors.maxfeuchte_div[1] + STATS.sensors.maxfeuchte_div[4])) &&  STATS.sensors.ledstate_akt == CONDITION_RED)) { // feuchte hoch
        PORTB |= _BV(PB0);
        PORTB |= _BV(PB1);
        STATS.sensors.ledstate_akt = CONDITION_YELLOW;
        STATS.sensors.lcd_blocked = 0;
      }
      else if((promille > (maxfeuchte - STATS.sensors.maxfeuchte_div[2]) && STATS.sensors.ledstate_akt == CONDITION_GREEN) || (promille < (maxfeuchte - (STATS.sensors.maxfeuchte_div[1] + STATS.sensors.maxfeuchte_div[4])) &&  STATS.sensors.ledstate_akt == CONDITION_YELLOW)) { // feuchte ok
        PORTB &= ~_BV(PB0);
        PORTB &= ~_BV(PB1);
        STATS.sensors.ledstate_akt = CONDITION_OFF;
      }else if(promille < (maxfeuchte - STATS.sensors.maxfeuchte_div[3]) && STATS.sensors.ledstate_akt == CONDITION_OFF){ //feuchte ist sehr niedrig
        PORTB &= ~_BV(PB0);
        PORTB |= _BV(PB1);
        STATS.sensors.ledstate_akt = CONDITION_GREEN;
      }
      temp2text(STATS.sensors.sensor[2].valuetext, maxfeuchte);
  
      if(!STATS.sensors.lcd_blocked){
#ifdef HD44780_SUPPORT
        hd44780_goto(1, 7);
#else
        lcd_goto_ddram(LCD_SECOND_LINE + 7);
#endif

#ifdef SENSORMODUL_DEBUG
	char lcdbuf[6];
	lcdbuf[0] = NIBBLE_TO_HEX((rfm12_t_status >> 12) & 0x0F);
	lcdbuf[1] = NIBBLE_TO_HEX((rfm12_t_status >> 8) & 0x0F);
	lcdbuf[2] = NIBBLE_TO_HEX((rfm12_t_status >> 4) & 0x0F);
	lcdbuf[3] = NIBBLE_TO_HEX(rfm12_t_status & 0x0F);
	lcdbuf[4] = NIBBLE_TO_HEX(RFM12_akt_status & 0x0F);
	lcdbuf[5] = 0;
	lcd_print(lcdbuf);
#else
        lcd_print(" Max");
        lcd_print(STATS.sensors.sensor[2].valuetext);
#endif /* SENSORMODUL_DEBUG */
#ifdef HD44780_SUPPORT
        hd44780_goto(1, 0);
#else
        lcd_goto_ddram(LCD_SECOND_LINE);
#endif
        lcd_print("F% ");
        lcd_print(STATS.sensors.sensor[sensor_i].valuetext+1);
      }
      else if (STATS.sensors.led_blink){
        if((PORTB & _BV(PB0)) != 0)
          PORTB &= ~_BV(PB0) & ~_BV(PB1);
        else
          PORTB |= STATS.sensors.led_blink;
      }
      if(STATS.sensors.countdown > 0){
        STATS.sensors.countdown--;
      }
      if (STATS.sensors.countdown == 1){
        STATS.sensors.lcd_blocked = 0;
        PORTB &= ~_BV(PB0) & ~_BV(PB1);
      }
    }
  
    if(++sensor_i >= SENSORMODUL_ADCMAX) sensor_i = 0;
    ADMUX = sensor_i;
    
  }
/*  if (start != startok){
    lcd_clear();
    lcd_home();
      //text[5] = i2cbuf[2];
      //_delay_ms(2);
    lcd_goto_ddram(0);
    lcd_print("T");
    lcd_goto_ddram(8);
    lcd_print("W");
    char textbuf[6] = { 0 };
    lcd_goto_ddram(LCD_SECOND_LINE);
    uint8_t i;
    for(i=0;i<4;i++) {
      temp2text(textbuf, STATS.sensors.maxfeuchte_div[i]);
      lcd_print(textbuf+1);
    }*/
    //startok = start;
  //}
  //}
}

#if 0
void 
sensormodul_setlcdtext(char *text, uint8_t len)
{
  uint8_t i;
  //start=1;
  lcd_goto_ddram(LCD_SECOND_LINE+7);
  for(i = 0; i<len; i++)
  {
    lcd_data(text[i]);
  }

}
#endif

#endif
