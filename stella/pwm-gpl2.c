/*
 Copyright(C) 2006 Jochen Roessner <jochen@lugrot.de>

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */

#define F_CPU 8000000L
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/io.h>
#include <avr/eeprom.h>
#include <inttypes.h>

/* avr-libc 1.4.x !!!! */

#define PINS 3
#define VOFFSET 16

/*
  Aufbau des Timetable's
{{Zeit, Bitmask},....}
*/
volatile uint8_t timetable[PINS][2];
volatile uint8_t i_timetable[PINS][2];
volatile uint8_t length;
volatile uint8_t i_length;
volatile uint8_t now = 0;
volatile uint8_t overflow_mask = 0;
volatile uint8_t i_overflow_mask = 0;
volatile uint8_t update_table = 0;


/*i2c vars*/
volatile uint8_t byteanzahl; //zaehler der bytes beim twi empfang
volatile uint8_t runbootload;//statuszaehler fuer den sprung zum bootloader
volatile uint8_t runbootseq[3] = {0x01,0x02,0xFF}; //startsequenz zum bootloader start
volatile uint8_t i2ckommando; //kommando das ueber i2c empfangen wurden
volatile uint8_t smbuscommand; //smbus command byte
volatile uint8_t smbuscount; //bytezaehler des smbus 
volatile uint8_t Checksum; //checksumme beim eeprom schreiben
volatile uint16_t eepaddr; //eeprom schreib-start-adresse
volatile uint8_t buf[33]; //datenbuffer des smbus
volatile uint8_t bufaddr; //zeiger (zaehler) auf bufferbyte



enum colors {
	C_RED, C_GREEN, C_BLUE
};

void sort(uint8_t color[]);


void
blink(uint8_t blinki, uint8_t puls, uint8_t pause){
  uint8_t pulstemp = puls;
  uint8_t pausetemp = pause;
  while(blinki > 0){
    pulstemp = puls;
    pausetemp = pause;
    PORTB |= _BV(PB0);
    while(pulstemp > 0){
      _delay_ms(25);
      pulstemp--;
    }
    PORTB &= ~_BV(PB0);
    while(pausetemp > 0){
      _delay_ms(25);
      pausetemp--;
    }
    blinki--;
  }
}

/* Interruptroutine des TWI
 */
SIGNAL (SIG_2WIRE_SERIAL)
{
  
  if((TWSR & 0xF8) == 0x80){
    /* Datenbyte wurde empfangen
    * TWDR auslesen
    */
    if (byteanzahl == 0){
      smbuscommand = TWDR;
    }
    if (byteanzahl == 1){
      smbuscount = TWDR;
    }
    if(smbuscommand == 0xF0 && smbuscount == 3){
      if(runbootseq[byteanzahl-2] == TWDR){
        runbootload++;
      }
      else{
        runbootload = 0;
      }
    }
    else if (smbuscommand == 0x81){
      if (byteanzahl == 0){
        bufaddr = 0;
        Checksum = 0;
      }
      else if (byteanzahl == 2){
        eepaddr = TWDR;
      }
      else if (byteanzahl == 3){
        eepaddr |= TWDR<<8;
      }
    }

    else if (smbuscommand == 0x82 && byteanzahl > 1 && smbuscount > 0){
      buf[bufaddr++] = TWDR;
      Checksum = (Checksum + TWDR) & 0xFF;
      smbuscount--;
      if(bufaddr == 16){
        eeprom_write_block(&buf, (void *) eepaddr, 16);
        bufaddr = 0;
        smbuscommand = 0;
        eepaddr+=16;
      }
    }
    else if (smbuscommand == 0x40){
      if (byteanzahl > 1){
        buf[byteanzahl-2] = TWDR;
        if (--smbuscount == 0)
          i2ckommando = buf[0];
      }
    }
    byteanzahl++;
  }
  else if((TWSR & 0xF8) == 0x60){
    /* Der Avr wurde mit seiner Adresse angesprochen  */
    byteanzahl = 0;
    runbootload = 0;
    PORTB |= _BV(PB0);
  }

  /* hier wird an den Master gesendet */
  else if((TWSR & 0xF8) == 0xA8){
    if(smbuscommand == 0x83){
      TWDR = 1;
    }
    else if(smbuscommand == 0x84){
      TWDR = 0x10;
      smbuscount = 0x00;
    }
    else if(smbuscommand == 0x44){
      TWDR = 0x20;
      smbuscount = 0x00;
    }
    else
      TWDR = 0x10; //zur demo den zaehler txbyte senden
  }
  else if((TWSR & 0xF8) == 0xB8){
    if(smbuscommand == 0x83){
      TWDR = Checksum;
    }
    else if(smbuscommand == 0x84){
      if(smbuscount < 0x10){
        TWDR = eeprom_read_byte((uint8_t *)eepaddr++);
        smbuscount++;
      }
    }
    else if(smbuscommand == 0x44){
      if(smbuscount < 0x20){
        TWDR = buf[smbuscount++];
      }
    }
    else
      TWDR++; //zur demo den zaehler txbyte senden
    
    byteanzahl++;
  }
  else{
    PORTB &= ~_BV(PB0);
  }

  TWCR |= (1<<TWINT); //TWI wieder aktivieren

}

void
init_twi(void){
  /* INIT fuer den TWI i2c
  * hier wird die Addresse des µC festgelegt
  * (in den oberen 7 Bit, das LSB(niederwertigstes Bit)
  * steht dafür ob der µC auf einen general callreagiert
  */ 
  TWAR = 0x06<<1;
  
  /* TWI Control Register, hier wird der TWI aktiviert, 
   * der Interrupt aktiviert und solche Sachen
   */
  TWCR = (1<<TWIE) | (1<<TWEN) | (1<<TWEA); 
  
  /* TWI Status Register init */
  TWSR &= 0xFC; 
}



void init_timer(void) {
  /* Normal PWM Mode */
  /* da gibts nichts zu setzen ;-) */
  /* 256 Prescaler */
  TCCR2|=_BV(CS21);
  TCCR2|=_BV(CS22);
  /* Int. bei Overflow und CompareMatch einschalten */
  TIMSK |= _BV(TOIE2) | _BV(OCIE2); 
}

SIGNAL(SIG_OUTPUT_COMPARE2) {
  PORTD &=~i_timetable[now][1];
  now++;
  if (now>=i_length)
    now=0;
  OCR2=i_timetable[now][0];
  
}

SIGNAL(SIG_OVERFLOW2) {
  if(update_table == 1){
    uint8_t i;
    for (i=0; i<length; i++) {
      i_timetable[i][0]=timetable[i][0];
      i_timetable[i][1]=timetable[i][1];
    }
    i_length=length;
    i_overflow_mask = overflow_mask;
    now = 0;
    update_table = 0;
  }
  PORTD |= i_overflow_mask;
}

int 
main(void)
{
  void (*bootptr)( void ) = (void *) 0x0C00;
  DDRB |= _BV(PB0); //LED pin auf ausgang
  init_twi();
  runbootload = 0;
  blink(1, 20, 10);
  blink(2, 3, 10);
  
  DDRD |= _BV(PD5) | _BV(PD6) | _BV(PD7);
  sei();
  TWCR |= (1<<TWINT); //TWI-Modul aktiv (unbedingt nach sei() )
  OCR2=0x00;
  uint8_t i; //univeral zaehler var
  uint8_t test[]={0,0,0};
  while(1) {
    if (runbootload == 3){
      TIMSK &= ~(_BV(TOIE2)|_BV(OCIE2)); /* Int. fuer Timer ausschalten */
      runbootload = 0;
      bootptr();
    }
    else if (i2ckommando == 0x3F){
      test[0] = buf[1];
      test[1] = buf[2];
      test[2] = buf[3];
      
      sort(test);
      
      init_timer();

      i2ckommando = 0;
    }
    else if (i2ckommando == 0x3E){
      for(i=0;i<3;i++){
        if(buf[VOFFSET+i] == 0 && test[0+i] < buf[1+i]){
          test[0+i]++;
          if(test[0+i] >= buf[1+i]){
            buf[VOFFSET+i] = 1;
          }
        }
        else{
          test[0+i]--;
          if(test[0+i] == 0){
            buf[VOFFSET+i] = 0;
          }
        }
      }
      _delay_ms(buf[4]);
      sort(test);
      
    }
    else if (i2ckommando == 0x3D){
      buf[VOFFSET] = 0;
      for(i=0;i<3;i++){
        if(test[0+i] < buf[1+i]){
          test[0+i]++;
        }
        else if(test[0+i] > buf[1+i]){
          test[0+i]--;
        }
        else{
          buf[VOFFSET]++;
        }
      }
      if(buf[VOFFSET] == 3)
        i2ckommando = 0;
      
      _delay_ms(buf[4]);
      sort(test);
      
    }
    else if (i2ckommando == 0x31){
      init_timer();
      i2ckommando = 0;
    }
    else if (i2ckommando == 0x30){
      TIMSK &= ~(_BV(TOIE2)|_BV(OCIE2)); /* Int. fuer Timer ausschalten */
      i2ckommando = 0;
    }
    
    /*
    test[0]++;
    sort(test);
    _delay_ms(10);
    */
  }
  return 0;
}

void sort(uint8_t color[]) {
  uint8_t i,y,x = 0;
  uint8_t  temp[PINS][2]={{0,0},{0,0},{0,0}};
  /* Schauen ob schon vorhanden */
  for (i=0; i< PINS; i++) {
    uint8_t vorhanden=0;
    for (y=0; y < x; y++) {
      if (color[i]==temp[y][0])
        vorhanden=1;
    }
    if (! vorhanden ) {
      temp[x][0]=color[i];
      x++;
    }
  }
  /* Sotieren */
  if (x!=1) {
    uint8_t t;
    for (y=0; y < x-1; y++) {
      for (i=0; i < (x-1)-y; i++) {
        if (temp[i][0]>temp[i+1][0]) {
          t=temp[i][0];
          //if(debug < 0x20)
          //  buf[debug++] = t;
          temp[i][0]=temp[i+1][0];
          temp[i+1][0]=t;
        }
      }
    }
  }
  /* Eintragen */
  for (i=0; i<PINS; i++) {
    for(y=0; y<x; y++) {
      if (color[i]==temp[y][0]) {
        temp[y][1]|=(1<<(i+5));
      }
    }
  }
  /* So etz noch die alten Werte ersetzen */
  if(update_table == 0){
    for (i=0; i<x; i++) {
      timetable[i][0]=temp[i][0];
      timetable[i][1]=temp[i][1];
    }
    //now = 0;
    //TCNT2 = 0;
    length=x;
    //OCR2=timetable[0][0];
    /* Overflow_mask neu bilden */
    overflow_mask = _BV(PD5) | _BV(PD6) | _BV(PD7);
    if ( timetable[0][0] == 0)
      overflow_mask &= ~timetable[0][1];
  } 
  update_table = 1;
}

