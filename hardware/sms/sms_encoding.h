/*
 * Copyright (c) 2009 by Benjamin Sackenreuter <benson@zerties.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
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

#ifndef _SMS_ENCODING_H
#define _SMS_ENCODING_H

void sms_get_text(uint8_t *sms_text, uint8_t *encoded_text, uint8_t *text_len);
void sms_get_number(uint8_t *rufnummer, uint8_t * encoded_num);

#endif
