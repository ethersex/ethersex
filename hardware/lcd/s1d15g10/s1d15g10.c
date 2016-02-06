/*
 *
 * Copyright (c) 2009 by Dirk Pannenbecker <dp@sd-gp.de>
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

/*
 * Nokia 6100 LCD driver
 *   based on Sparkfun code.
 *   http://www.sparkfun.com/datasheets/LCD/SFE_lcd_driver_c.zip
 *
 * Slightly modified
 * - use only standard data types (uint8_t and uint16_t) no more char and int.
 * - modified initialization.
 * - added handling for RST
 *
 * Pinout for SPI is:
 * PORTD
 *   7: CS
 *   6: SCLK
 *   5: SDATA
 * PORTB
 *   0: RST
 *
 * 	Remember if running this code on a 5V AVR, make sure to add resistors in 
 * 	series so as not to blow the LCD!
 */

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include "config.h"
#include "version.h"
#include "s1d15g10.h"
#include "core/spi.h"

/* module local macros */
#ifdef RFM12_IP_SUPPORT
/* RFM12 uses interrupts which do SPI interaction, therefore
   we have to disable interrupts if support is enabled */
#  define cs_low()  uint8_t sreg = SREG; cli(); PIN_CLEAR(S1D15G10_CS); 
#  define cs_high() PIN_SET(S1D15G10_CS); SREG = sreg;
#else
#  define cs_low()  PIN_CLEAR(S1D15G10_CS)
#  define cs_high() PIN_SET(S1D15G10_CS)
#endif

#include "5x9.txt"

#define DISON       0xaf
#define DISOFF      0xae
#define DISNOR      0xa6
#define DISINV      0xa7
#define COMSCN      0xbb
#define DISCTL      0xca
#define SLPIN       0x95
#define SLPOUT      0x94
#define PASET       0x75
#define CASET       0x15
#define DATCTL      0xbc
#define RGBSET8     0xce
#define RAMWR       0x5c
#define RAMRD       0x5d
#define PTLIN       0xa8
#define PTLOUT      0xa9
#define RMWIN       0xe0
#define RMWOUT      0xee
#define ASCSET      0xaa
#define SCSTART     0xab
#define OSCON       0xd1
#define OSCOFF      0xd2
#define PWRCTR      0x20
#define VOLCTR      0x81
#define VOLUP       0xd6
#define VOLDOWN     0xd7
#define TMPGRD      0x82
#define EPCTIN      0xcd
#define EPCOUT      0xcc
#define EPMWR       0xfc
#define EPMRD       0xfd
#define EPSRRD1     0x7c
#define EPSRRD2     0x7d
#define NOP         0x25

#define dxLCDScreen 130
#define dyLCDScreen 130
static const uint8_t xlMin = 0;
static const uint8_t ylMin = 2;

uint8_t lcd_init_flag = 0;

void sendByte(uint8_t cmd, uint8_t data) {
  // enable chip_sel
  PIN_CLEAR(S1D15G10_CS);

  // Send the command flag bit and data bit 7->1
  spi_send ((cmd << 7) | (data >> 1));
		
  // Send bit 0 of data
  spi_send(data << 7);

  // disable chip_sel
  PIN_SET(S1D15G10_CS);
}

uint16_t cpxSetAndValidateLCDRect(uint8_t x, uint8_t y, uint8_t dx, uint8_t dy) {
  uint8_t xlFirst, ylFirst, xlLast, ylLast;	// LCD coordinates

  /* check upper left corner */
  /* (x and y aren't too low since unsigned can't be < 0!) */
  if (x >= dxLCDScreen || y >= dyLCDScreen)	// completely off-screen
    return 0;
  
  /* check lower right corner */
  if (x + dx > dxLCDScreen)
    dx = dxLCDScreen - x;
  if (y + dy > dyLCDScreen)
    dy = dyLCDScreen - y;

  /* convert to LCD coordinates */
  xlLast = (xlFirst = xlMin + x) + dx - 1;
  ylLast = (ylFirst = ylMin + y) + dy - 1;

  /* note: for PASET/CASET, docs say that start must be < end,
     but <= appears to be OK; end is a "last" not "lim" value */
  sendByte(lctCmd, PASET);	// Page Address Set
  sendByte(lctData, ylFirst);	// start page (line)
  sendByte(lctData, ylLast);	// end page
  sendByte(lctCmd, CASET);	// Column Address Set
  sendByte(lctData, xlFirst);	// start address
  sendByte(lctData, xlLast);	// end address
  sendByte(lctCmd, RAMWR);	// Memory Write
  
  return (uint16_t)dx * dy;
}

void lcd_set_brightness_parms(uint8_t a, uint8_t b) {
  sendByte(lctCmd, VOLCTR);
  sendByte(lctData, a);
  sendByte(lctData, b);
}

void fillrect(uint8_t sx, uint8_t sy, uint8_t dx, uint8_t dy, uint8_t color) {
	
  uint16_t cpx = cpxSetAndValidateLCDRect(sx, sy, dx, dy);
  while (cpx-- > 0)
    sendByte(lctData, color);
}

void setup_pix_blit(uint8_t sx, uint8_t sy, uint8_t dx, uint8_t dy) {
  cpxSetAndValidateLCDRect(sx,sy,dx,dy);	
}

void pix_blit(uint8_t color){
  sendByte(lctData, color);
}

void init_lcd(void) {
  uint8_t brightness = 23;

  // reset the display
  PIN_CLEAR(S1D15G10_RESET);
  wdt_kick();
  _delay_ms(10);
  PIN_SET(S1D15G10_RESET);
  wdt_kick();

  sendByte(lctCmd, DISCTL);	// Display Control
  sendByte(lctData, 0x00);	// default
  sendByte(lctData, 0x20);	// (32 + 1) * 4 = 132 lines (of which 130 are visible)
  // sendByte(lctData, 0x0a);	// default
  sendByte(lctData, 0x00);	// 
  
  sendByte(lctCmd, COMSCN);	// Common Scan
  sendByte(lctData, 0x01);	// COM1-80: 1->80; COM81-160: 160<-81 (otherwise bottom of screen is upside down)

  sendByte(lctCmd, TMPGRD);      // Temperature gradient
  sendByte(lctData, 0);		// default
  sendByte(lctCmd, OSCON);	// Oscillation On
  sendByte(lctCmd, SLPOUT);	// Sleep Out (exit sleep mode)

  sendByte(lctCmd, VOLCTR);	// Electronic Volume Control (brightness/contrast)
  sendByte(lctData, brightness);
  sendByte(lctData, 3);

  sendByte(lctCmd, PWRCTR);	// Power Control Set
  sendByte(lctData, 0x0f);

  wdt_kick();
  _delay_ms(100);

  sendByte(lctCmd, DISINV);	// Inverse Display, you have to invert to display colors correctly 
  sendByte(lctCmd, PTLOUT);	// Partial Out (no partial display)

  sendByte(lctCmd, DATCTL);	// Data Control
  sendByte(lctData, 0x00);	// normal orientation; scan across cols, then rows
  sendByte(lctData, 0x00);	// RGB arrangement (RGB all rows/cols)
  sendByte(lctData, 0x01);	// 8 bit-color display
  
  /* if 256-color mode, bytes represent RRRGGGBB; the following
     maps to 4-bit color for each value in range (0-7 R/G, 0-3 B) */
  sendByte(lctCmd, RGBSET8);	// 256-color position set
  sendByte(lctData, 0x00);	// 000 RED
  sendByte(lctData, 0x02);	// 001  
  sendByte(lctData, 0x04);	// 010
  sendByte(lctData, 0x06);	// 011
  sendByte(lctData, 0x08);	// 100
  sendByte(lctData, 0x0a);	// 101
  sendByte(lctData, 0x0c);	// 110
  sendByte(lctData, 0x0f);	// 111
  sendByte(lctData, 0x00);	// 000 GREEN
  sendByte(lctData, 0x02);	// 001  
  sendByte(lctData, 0x04);	// 010
  sendByte(lctData, 0x06);	// 011
  sendByte(lctData, 0x08);	// 100
  sendByte(lctData, 0x0a);	// 101
  sendByte(lctData, 0x0c);	// 110
  sendByte(lctData, 0x0f);	// 111
  sendByte(lctData, 0x00);	//  00 BLUE
  sendByte(lctData, 0x06);	//  01
  sendByte(lctData, 0x09);	//  10
  sendByte(lctData, 0x0f);	//  11

  uint16_t cpx = cpxSetAndValidateLCDRect(0,0,dxLCDScreen,dyLCDScreen);		
  while (cpx-- > 0) {
    sendByte(lctData, 0xFF);
  }
  wdt_kick();
  _delay_ms(300);

  sendByte(lctCmd, DISON);	// Display On

  fillrect(0, 0, 130, 130, 0x00);
  uint16_t size = cpxSetAndValidateLCDRect(130-42, 0, 42, 42);
  for (uint16_t i = 0; i < size; i++) {
    sendByte(lctData, pgm_read_byte(bunnie+i));
  }
  lcd_init_flag = 1;
  putstr_pgm(0, 0,  PSTR("booting...\n"), 0x03, 0x00);
  putstr_pgm(0, 10,  PSTR("ethersex v." VERSION_STRING "\n"), 0xE0, 0x00);
  putstr_pgm(0, 20,  PSTR("www.ethersex.de\n"), 0xFC, 0x00);
  putstr_pgm(0, 30,  PSTR("S1D15G10 LCD\n"), 0x1C, 0x00);

}

uint8_t DxForIch(uint8_t ich, uint16_t *pcbit) {
  uint16_t ax[2];
  uint16_t dx;

  memcpy_P(&ax, &s_mpchxFont5x9[ich], 2 * sizeof(uint16_t));
  
  if ((dx = ax[1] - ax[0]) == 0)
    return 0;
	
  *pcbit = ax[0] * dyFont5x9;
  return (uint8_t)dx;
}

uint8_t * PbForCh(char ch, uint8_t *pf, uint8_t *pdx) {
  uint8_t ich;
  uint16_t cbit;

  /* make sure given ch in range, else use space */
  if (ch < ' ' || (ich = ch - ' ') >= cchFont5x9) {
    *pdx = DxForIch(0, &cbit);
  }
  else {
    /* get char width offset and bit offset */
    *pdx = DxForIch(ich, &cbit);
    if (*pdx == 0) {
      /* no bitmap for this char; pick an alternative;
	 try uppercase for lowercase char, else use space */
      if (ch < 'a' || ch > 'z'
	  || (*pdx = DxForIch(ich - ('a'-'A'), &cbit)) == 0) {
	/* use space instead of unknown/unmapped character */
	*pdx = DxForIch(0, &cbit);
      }
    }
  }
  /* bits: 01234567 89... */
  *pf = 0x80 >> (cbit % 8);
  return ( uint8_t *)&s_apxFont5x9[cbit / 8];
}

#define dxLeading 1
#define dyLeading 1
uint8_t putch(uint8_t x, uint8_t y, char c, uint8_t fg, uint8_t bg) {
  uint8_t dx, dy;
  uint16_t cpx;
  uint8_t f;
  uint8_t grf;
  uint8_t *pb;


  dy = dyFont5x9;
  pb = PbForCh(c, &f, &dx);

  cpx = cpxSetAndValidateLCDRect(x, y, dx, dy);
  if (cpx != (uint16_t)dx * dy)		// not enough room
    return 0;

  memcpy_P(&grf, pb, sizeof(uint8_t));

	
  /* area set above; just fill in the pixels */
  while (cpx-- > 0) {
    sendByte(lctData, (grf & f) ? fg : bg);
    if ((f >>= 1) == 0) {
      memcpy_P(&grf, ++pb, sizeof(uint8_t));
      f = 0x80;
    }
  }
  /* fill in background for leading below text */
  fillrect(x, y + dy, dx + dxLeading, dyLeading, bg);

  /* update current position */
  x += dx;
  /* fill in background for leading after character */
  fillrect(x, y, dxLeading, dy, bg);
  x += dxLeading;
  return x;
}

uint8_t putstr_ram(uint8_t x, uint8_t y, char * p, uint8_t fg, uint8_t bg) {
  
  uint8_t c;
  while ((c = *p++))
    x = putch(x, y, c, fg, bg);	
  return x;
}

uint8_t putstr_pgm(uint8_t x, uint8_t y, PGM_P p, uint8_t fg, uint8_t bg) {
	
  uint8_t c;
  while ((c = pgm_read_byte(p++)))
    x = putch(x, y, c, fg, bg);
	
  return x;
}

uint8_t printDecimal(uint8_t x, uint8_t y, uint32_t val, uint8_t fg, uint8_t bg) {
  char buf[9];
  buf[8] = 0;
  char * ptr = &buf[8];
	
  while(val > 0) {
    ptr--;
    *ptr = val%10 + '0';
    val /= 10;
  }
  if (ptr == &buf[8])
    *(ptr = &buf[7]) = '0';

  return putstr_ram(x,y,ptr,fg,bg);
}

void lcd_putch(char d) {
  static uint8_t x_t = 0;
  static uint8_t y_t = 42;
  if (lcd_init_flag == 0)
    return;
  if (d == '\r') {
    y_t+=10;
  } else if (d == '\n') {
    x_t = 0;
  } else if (d == 8) {
    y_t+=10;
    x_t -= 5;
  } else {        // unknown command
    x_t=putch(x_t, y_t, d, 0x1C, 0x00);
    if (x_t>(130-42+9) && y_t < 42) {
      y_t+= 9;
      x_t = 0;
    } else if (x_t>125) {
      y_t+= 9;
      x_t = 0;
    }

    if (y_t>125) {
      y_t = 0;
      x_t = 0;
    }
  }
}

/*
  -- Ethersex META --
  header(hardware/lcd/s1d15g10/s1d15g10.h)
  init(init_lcd)
*/
