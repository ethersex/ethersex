/* vim:fdm=marker ts=4 et ai
 * {{{
 *
 * (c) by Alexander Neumann <alexander@bumpern.de>
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
 }}} */

#ifndef _HTTPD_H
#define _HTTPD_H

#include <stdint.h>

#ifdef AVR
#include "../uip/uip.h"
#endif

/* constants */
#define HTTPD_PORT 80
#define HTTPD_ALTERNATE_PORT 8000

#define HTTPD_INDEX "idx.ht"
#define ECMD_INDEX "ecmd"

/* prototypes */
void httpd_init(void);
void httpd_main(void);

/* headers */
extern char PROGMEM httpd_header_200[];
extern char PROGMEM httpd_header_ct_css[];
extern char PROGMEM httpd_header_ct_html[];
extern char PROGMEM httpd_header_ct_xhtml[];
extern char PROGMEM httpd_header_200_ecmd[];
extern char PROGMEM httpd_header_400[];
extern char PROGMEM httpd_header_gzip[];
extern char PROGMEM httpd_header_401[];
extern char PROGMEM httpd_body_401[];
extern char PROGMEM httpd_body_400[];
extern char PROGMEM httpd_header_404[];
extern char PROGMEM httpd_body_404[];
extern char PROGMEM httpd_header_length[];

#endif
