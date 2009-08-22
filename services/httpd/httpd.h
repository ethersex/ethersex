/*
 * (c) by Alexander Neumann <alexander@bumpern.de>
 * Copyright (C) 2009 Stefan Siegl <stesie@brokenpipe.de>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License (either version 2 or
 * version 3) as published by the Free Software Foundation.
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

#ifndef _HTTPD_H
#define _HTTPD_H

#include <stdint.h>

#ifdef AVR
#include "protocols/uip/uip.h"
#endif

/* constants */
#define HTTPD_PORT 80
#define HTTPD_ALTERNATE_PORT 8000

#define HTTPD_INDEX "idx.ht"
#define ECMD_INDEX "ecmd"

/* prototypes */
void httpd_init (void);
void httpd_main (void);
void httpd_cleanup (void);

void httpd_handle_400 (void);
void httpd_handle_401 (void);
void httpd_handle_404 (void);

void httpd_handle_vfs (void);
void httpd_handle_sd_dir (void);
void httpd_handle_sd_dir_redirect (void);
void httpd_handle_soap (void);

void httpd_handle_ecmd_setup (char *encoded_cmd);
void httpd_handle_ecmd (void);

const PGM_P httpd_mimetype_detect (const uint8_t *);

/* headers */
extern char httpd_header_200[];
extern char httpd_header_ct_css[];
extern char httpd_header_ct_html[];
extern char httpd_header_ct_xhtml[];
extern char httpd_header_ecmd[];
extern char httpd_header_400[];
extern char httpd_header_gzip[];
extern char httpd_header_401[];
extern char httpd_body_401[];
extern char httpd_body_400[];
extern char httpd_header_404[];
extern char httpd_body_404[];
extern char httpd_header_length[];
extern char httpd_header_end[];

#include <stdio.h>
#include <avr/pgmspace.h>

#define PASTE_RESET()   (((unsigned char *)uip_appdata)[0] = 0)
#define PASTE_P(a)      strcat_P(uip_appdata, a)
#define PASTE_PF(a...)	sprintf_P(uip_appdata + strlen(uip_appdata), a)

#include "config.h"
#ifdef VFS_TEENSY
#  define PASTE_LEN(a)    sprintf_P(uip_appdata + strlen(uip_appdata),	\
				    PSTR ("%u\n"), a)
#else
#  define PASTE_LEN(a)    sprintf_P(uip_appdata + strlen(uip_appdata),	\
				    PSTR ("%lu\n"), a)
#endif

#define PASTE_LEN_P(a)    sprintf_P(uip_appdata + strlen(uip_appdata),	\
				    PSTR ("%u\n"), strlen_P(a))

/* FIXME maybe check uip_mss and emit warning on debugging console. */
#define PASTE_SEND()    uip_send(uip_appdata, strlen(uip_appdata))


#define STATE (&uip_conn->appstate.httpd)

#endif /* _HTTPD_H */
