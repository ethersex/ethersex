dnl
dnl  Copyright (c) 2009 by Stefan Siegl <stesie@brokenpipe.de>
dnl
dnl  This program is free software; you can redistribute it and/or modify
dnl  it under the terms of the GNU General Public License as published by
dnl  the Free Software Foundation; either version 3 of the License, or
dnl  (at your option) any later version.
dnl
dnl  This program is distributed in the hope that it will be useful,
dnl  but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl  GNU General Public License for more details.
dnl
dnl  You should have received a copy of the GNU General Public License
dnl  along with this program; if not, write to the Free Software
dnl  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
dnl
dnl  For more information on the GPL, please go to:
dnl  http://www.gnu.org/copyleft/gpl.html
dnl

dnl ==========================================================================
dnl SENDMAIL(IP,PORT,FROM,TO,SUBJECT,TEXT)
dnl ==========================================================================
define(`SENDMAIL', `
    TCP_HANDLER(`sendmail'__line__)
	TCP_EXPECT("220");
	TCP_SEND("HELO Ethersex\r\n");

	TCP_EXPECT("250");
	TCP_SEND("MAIL FROM: <"$3">\r\n");

	TCP_EXPECT("250");
	TCP_SEND("RCPT TO: <"$4">\r\n");

	TCP_EXPECT("250");
	TCP_SEND("DATA\r\n");

	TCP_EXPECT("354");
	TCP_SEND("From: \"Ethersex\" <"$3">\r\n"
		"To: <"$4">\r\n"
		"Subject: "$5"\r\n"
		"\r\n"$6"\r\n.\r\n");
    TCP_HANDLER_END()
    TCP_CONNECT($1,$2,`sendmail'__line__)
')
