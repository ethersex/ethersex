/*
 * Copyright (c) 2009 by Stefan Siegl <stesie@brokenpipe.de>
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
#include "protocols/ecmd/parser.h"
#include "protocols/ecmd/ecmd-base.h"
#include "httpd.h"

static const char PROGMEM httpd_header_500_xml[] =
"HTTP/1.1 500 Server Error\n"
"Connection: close\n";

static const char PROGMEM httpd_header_ct_xml[] =
"Content-Type: text/xml; charset=utf-8\n\n";


void
httpd_handle_soap (void)
{
    if (uip_newdata()) {
	/* We've received new data (maybe even the first time).  We'll
	   receive something like this:

	   POST /~stesie/tmp/hibye.cgi HTTP/1.1
	   TE: deflate,gzip;q=0.3
	   Connection: TE, close
	   [...]

	   <?xml version="1.0" encoding="UTF-8"?><soap:Envelope */

	/* Make sure it's zero-terminated, so we can safely use strstr */
	char *ptr = (char *)uip_appdata;
	ptr[uip_len] = 0;

	SOAP_DEBUG("newdata: len=%d, parsing=%d.\n",
		   uip_len, STATE->u.soap.parsing);

	/* Don't parse before the <? xml preamble. */
	if (!STATE->u.soap.parsing) {
	    ptr = strstr_P (ptr, PSTR("\r\n\r\n"));

	    if (!ptr) {
		SOAP_DEBUG ("neither in parsing-mode, nor CRLFCRLF found.\n");
		return;		/* Wait for more data to come. */
	    }

	    ptr += 4;		/* Ignore \r\n\r\n */
	    STATE->u.soap.parsing = 1;
	}

	uint16_t len = uip_len - (ptr - (char *)uip_appdata);
	if (len)
	    soap_parse (&STATE->u.soap, ptr, len);
    }

    if (uip_acked ()) {
	uip_close ();
	return;
    }

    if (STATE->u.soap.parsing_complete
	&& !STATE->u.soap.error
	&& !STATE->u.soap.evaluated)
	soap_evaluate (&STATE->u.soap);


    if (STATE->u.soap.parsing_complete) {
	PASTE_RESET ();

	if (STATE->u.soap.error)
	    PASTE_P (httpd_header_500_xml);
	else
	    PASTE_P (httpd_header_200);

	PASTE_P (httpd_header_ct_xml);
	soap_paste_result (&STATE->u.soap);
	PASTE_SEND ();
    }
}
