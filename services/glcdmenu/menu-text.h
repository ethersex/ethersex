/*
   Gamebox
    Copyright (C) 2004-2006  by Malte Marwedel
    m.marwedel AT onlinehome dot de

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef _TEXT_H
 #define _TEXT_H

#include "menu-interpreter.h"
/* draw_char pr�ft, vor dem schreiben auf das Display ob die Position
innerhalb des g�ltigen Bereiches ist. Daher sind f�r posx und posy auch negative
oder zu gro�e Werte kein Problem. Die Buchstaben sind dann normalerweise nicht
sichtbar.
Im schlimmsten Fall werden sie an die falsche Stelle auf dem Display gezeichnet.
*/
unsigned char menu_char_draw(SCREENPOS posx, SCREENPOS posy, unsigned char font, unsigned char cdraw);

unsigned char menu_font_heigth(unsigned char font);

#endif
