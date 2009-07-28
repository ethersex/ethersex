/*
Author: David Gräff <david.graeff@web.de>
Date: Feb, 2009
Purpose: Access ethersex project based firmware via rs232, usb, udp or tcp. (www.ethersex.de)

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 or version 3 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */

#ifndef LIBECMD_H
#define LIBECMD_H

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <usb.h>
#include <sys/fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */
#include <time.h>

#define _BV(bit) (1 << (bit))
#define USB_REQUEST_ECMD 0

#define USB_DEFAULT_DATA "16c0" "05dc"
#define TCP_DEFAULT_DATA 2701
#define UDP_DEFAULT_DATA 2701
#define RS232_DEFAULT_DATA 19200

#define response_buffer_len 500
char response_buffer[response_buffer_len];

struct usb_esex_device {
    usb_dev_handle *fd;
    struct usb_esex_device* next;
};

struct rs232_esex_device {
	int fd;
    struct rs232_esex_device* next;
};

struct udp_esex_device {
	int fd;
    struct udp_esex_device* next;
};

struct tcp_esex_device {
	int fd;
    struct tcp_esex_device* next;
};

struct connection {
  struct usb_esex_device* usb;
  struct rs232_esex_device* rs232;
  struct udp_esex_device* udp;
  struct tcp_esex_device* tcp;
};

struct connection* ecmd_init();
int ecmd_add_usb_device(struct connection*, char* vendor_product_id);
int ecmd_add_rs232_device(struct connection*, char* device_file, int baudrate);
int ecmd_add_udp_device(struct connection*, char* ip, int port);
int ecmd_add_tcp_device(struct connection*, char* ip, int port);
int ecmd_add(struct connection*, char* conn_string);
void ecmd_close(struct connection*);
char* ecmd_execute(struct connection*,char* ecmd,int len,int timeout);

#endif
