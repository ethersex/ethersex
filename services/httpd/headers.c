/*
 * (c) by Alexander Neumann <alexander@bumpern.de>
 * Copyright (c) 2008,2009 by Stefan Siegl <stesie@brokenpipe.de>
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

#include "config.h"
#include "httpd.h"


const char PROGMEM httpd_header_200[] =
"HTTP/1.1 200 OK\n"
"Connection: close\n";


const char PROGMEM httpd_header_ct_css[] =
"Content-Type: text/css; charset=utf-8\n\n";


const char PROGMEM httpd_header_ct_html[] =
"Content-Type: text/html; charset=utf-8\n\n";


const char PROGMEM httpd_header_ct_xhtml[] =
"Content-Type: application/xhtml+xml; charset=utf-8\n\n";

#ifdef HTTP_FAVICON_SUPPORT
const char PROGMEM httpd_header_ct_xicon[] =
"Content-Type: image/x-icon\n\n";
#endif

#ifdef ECMD_PARSER_SUPPORT
const char PROGMEM httpd_header_ecmd[] =
"Cache-Control: no-cache\n"
"Cache-Control: must-revalidate\n"
"Content-Type: text/plain; charset=utf-8\n\n";
#endif	/* ECMD_PARSER_SUPPORT */


const char PROGMEM httpd_header_400[] =
"HTTP/1.1 400 Bad Request\n"
"Connection: close\n"
"Content-Type: text/plain; charset=utf-8\n";


const char PROGMEM httpd_header_gzip[] =
"Content-Encoding: gzip\n";


#ifdef HTTPD_AUTH_SUPPORT
const char PROGMEM httpd_header_401[] =
"HTTP/1.1 401 UNAUTHORIZED\n"
"Connection: close\n"
"WWW-Authenticate: Basic realm=\"Secure Area\"\n"
"Content-Type: text/plain; charset=utf-8\n";


const char PROGMEM httpd_body_401[] =
"Authentification required\n";
#endif	/* HTTPD_AUTH_SUPPORT */


const char PROGMEM httpd_body_400[] =
"Bad Request\n";


const char PROGMEM httpd_header_404[] =
"HTTP/1.1 404 File Not Found\n"
"Connection: close\n"
"Content-Type: text/plain; charset=utf-8\n";


const char PROGMEM httpd_body_404[] =
"File Not Found\n";


const char PROGMEM httpd_header_length[] = "Content-Length: ";


const char PROGMEM httpd_header_end[] = "\n";
