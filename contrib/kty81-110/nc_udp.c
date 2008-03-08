/*  Copyright(C) 2007,2008 Christian Dietrich <stettberger@dokucode.de>

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software Foundation,
 Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
*/
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>


void 
error(char *msg)
{
  fprintf(stderr, "%s\n", msg);
  exit(EXIT_FAILURE);
}

int  
open_udp_socket(int port)
{
  struct sockaddr_in addr;

  int sock  = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock < 0) error("socket");

  /* Make the socket reusable */
  int x;
  if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &x, sizeof (x)) == -1)
    error("setsockopt (SO_REUSEADDR)");

  /* Bind to an local port */
  if (port) {
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1)
      error("bind");
  }

  /* Make the socket non blocking */
  fcntl(sock, F_SETFL, O_NONBLOCK);

  return sock;
}

struct sockaddr_in*
get_sockaddr_in(const char *hostname, int port)
{
  struct hostent *hp;
  struct sockaddr_in *addr = malloc(sizeof(struct sockaddr_in));
  if (!addr) error("malloc");

  addr->sin_family = AF_INET;

  /* Get hostname */
  hp = gethostbyname(hostname);
  if (hp == 0) error("Unknown host");

  memcpy(&addr->sin_addr, hp->h_addr, hp->h_length);

  addr->sin_port = htons(port);

  return addr;
}

void
send_message(int sock, struct sockaddr_in* target, 
             char *msg, int datalen) 
{
  int n = sendto(sock, msg, datalen , 0, (struct sockaddr *)target, 
                 sizeof(struct sockaddr_in));
  if (n < 0) 
    error("sendto");
}

int
recv_message(int sock, int timeout, char **message, int *len)
{
  char *msg = malloc(1024);
  *message = msg;
  *len = 0;
  if (! msg) 
    error("malloc");

  int useconds = 0;
  struct sockaddr_in remote;
  int remote_len = sizeof(remote);

  while (useconds < timeout) {
    char status;
    int n = recvfrom(sock, (char *)msg, 1024, 0, 
                     (struct sockaddr *)&remote, &remote_len);
    if (n > 0) {
      *len = n;
      return 1;
    }
    usleep(5000);
    useconds += 5;
  }
  return 0;

}

void
usage() 
{
  fprintf(stderr, "Options:\n"
          " -H <host>  -  host to connect to\n"
          " -P <port>  -  remote port to connect to\n"
          " -L <port>  -  local to send from\n\n"
          "All following data will be sent as message body\n"
          );
}


int 
main(int argc, char *argv[]) 
{
  int i, len = 0;
  char *host = NULL;
  int rport = 2701;
  int lport = 0;
  for (i = 1; i < argc; i++) {
    if (argv[i][0] == '-') {

      if (argv[i][1] == 'P') 
        rport = strtol(argv[i+1], NULL, 10);
      if (argv[i][1] == 'H') 
        host = argv[i+1];
      if (argv[i][1] == 'L') 
        lport = strtol(argv[i+1], NULL, 10);
      
      if (argv[i][1] == 'h') {
        usage();
        exit(EXIT_SUCCESS);
      }
      i += 2;
    } else {
      i++;
    }
  }

  /* test remote options */
  if (!host || !rport) {
    usage();
    exit(EXIT_FAILURE);
  }

  int fd;
  fd = open_udp_socket(lport);
  struct sockaddr_in *addr = get_sockaddr_in(host, rport);
  char input[1024];
  if ((len = read(STDIN_FILENO, input, 1024)) < 0)
    error("read");

  send_message(fd, addr, input, len);

  /* answer */
  char *msg;
  if (recv_message(fd, 1000, &msg, &len)) {
    for (i = 0; i < len; i++)
      putchar(msg[i]);
  } else
    exit(EXIT_FAILURE);
  close(fd);
  free(addr);
  free(msg);

  return 0;
}



