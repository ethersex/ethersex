/*
Author: David Gräff <david.graeff@web.de>
Date: Dec, 2009
Purpose: For the module "motorCurtain" and the protocol "udpCurtain".
   This test program will send the positions from 0..4 to the ethersex firmware.

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
#include <sys/fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */
#include <time.h>

////////////////// UDP Helper /////////////////
int
udp_open(const char *ip, int port) //ip, port
{
	// create an udp socket
	struct sockaddr_in remote;
	int fd;

	if ((fd=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1) return 0;

	memset((char *) &remote, 0, sizeof(remote));
	remote.sin_family = AF_INET;
	remote.sin_port = htons(port);
	remote.sin_addr.s_addr = inet_addr(ip);

	int flags = fcntl(fd, F_GETFL);
	if(fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) return 0;

	// connect to the socket
	if (connect(fd, (struct sockaddr*)&remote, sizeof(remote)) == -1) return 0;

	return fd;
}

int
udp_recv(int fd, char *buf, int len)
{
    int ret = recv(fd, buf, len, 0);
    if (ret < 0)
		return 0;
    return ret; /* > 0 = ok */
}

int
udp_send(int fd ,const char *data, int len)
{
	int ret = send(fd, data, len, 0);
    if (ret <= 0)
		return 0;
	return ret; /* > 0 = ok */
}

////////////////////////////////////////////////

// UDP Socket file handle
int fd;

void sendcmd(char str)
{
	printf("Sending: %i\n",str);
	if (udp_send(fd, &str, sizeof(str)) == 0)
		printf("Sending failed!\n");
}

int main(int argc, const char* argv[])
{
	printf("Starting\n");
	
	// Establish an udp connection to 192.168.1.8 on the port 2704
	fd = udp_open("192.168.1.8",2704);
	if (!fd) {
	  printf("Connection failed!\n");
	  return 1;
	}
	
	sendcmd(0);
	sleep(2);

	sendcmd(1);
	sleep(3);

	sendcmd(2);
	sleep(3);

	sendcmd(3);
	sleep(3);

	sendcmd(4);
	sleep(3);

	sendcmd(0);
	sleep(2);

	printf("Finished\n");
	
	close(fd);
	
	return 0;
}