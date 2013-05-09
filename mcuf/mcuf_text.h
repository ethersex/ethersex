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

#ifndef MCUF_TEXT_H
#define MCUF_TEXT_H


extern char textbuff[37];          //Text Puffer

void load_text(PGM_VOID_P x);


/* draw_char prüft, vor dem schreiben auf das Display ob die Position
innerhalb des göltigen Bereiches ist. Daher sind für posx und posy auch negative
oder zu große Werte kein Problem. Die Buchstaben sind dann normalerweise nicht
sichtbar.
Im schlimmsten Fall werden sie an die falsche Stelle auf dem Display gezeichnet.
*/
uint8_t draw_char(uint8_t zeichen, uint8_t posx, uint8_t posy, uint8_t color,
                 uint8_t transparency, uint8_t shrink);

void draw_string(uint8_t posx, uint8_t posy, uint8_t color,
                 uint8_t transparency, uint8_t shrink);

void scrolltext(uint8_t posy, uint8_t color, uint8_t bcolor, uint8_t waittime);

void draw_tinydigit(uint8_t ziffer, uint8_t posx,uint8_t posy, uint8_t color);

void draw_tinynumber(uint16_t value, uint8_t posx, uint8_t posy, uint8_t color);

#endif /* MCUF_TEXT_H */
