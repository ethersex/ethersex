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

#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <err.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>

#include "ethcmd.h"

/* global variables */
struct global_config_t cfg;

/* structure for getopt */
const struct option longopts[] = {
    {"help", no_argument, 0, 'h'},
    {"verbose", optional_argument, 0, 'v'},
    {"host", required_argument, 0, 'H'},
    {"port", required_argument, 0, 'P'},
    {0, 0, 0, 0},
};

/* module-local prototypes */
void print_usage(void);
int connect_host(char *host, int port);
void send_message(void);
void parse_message(struct ethcmd_message_t *msg);

void print_usage(void)
/* {{{ */ {

     printf("USAGE: ethcmd [options]\n"
           "  -h, --help                display this help and exit\n"
           "  -v, --verbose=[LEVEL]     set verbosity level (range: 0 <= LEVEL <= 255)\n"
           "                            if LEVEL is not given, the verbosity level is set to 1\n"
           "  -H, --host=HOST           set remote host (default: %s)\n"
           "  -P, --port=PORT           set remote port (default: %d)\n"
           , DEFAULT_HOST, DEFAULT_PORT
          );

} /* }}} */

int connect_host(char *host, int port)
/* {{{ */ {

     /* socket fd and structures */
    int socket_fd;
    struct sockaddr_in server_address;

    /* create socket */
    if ((socket_fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        err(1, "socket()");

    /* search host address */
    struct hostent *host_address;

    VERBOSE_PRINTF("resolving host...\n");
    if ((host_address = gethostbyname(host)) == NULL)
        err(1, "no valid address found for \"%s\"", host);

    if (host_address->h_length == 0)
        errx(1, "no address found for \"%s\"", host);

    struct in_addr host_ip_address;
    host_ip_address.s_addr = *(unsigned long *)host_address->h_addr_list[0];
    VERBOSE_PRINTF("connecting to server %s, port %d\n", inet_ntoa(host_ip_address), port);

    /* init structure */
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = *(unsigned long *)host_address->h_addr_list[0];
    server_address.sin_port = htons(port);

    /* connect */
    if (connect(socket_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
        err(1, "connect()");

    return socket_fd;

} /* }}} */

void send_message(void)
/* {{{ */ {

    struct ethcmd_message_t msg;

    msg.length = htons(0);
    msg.message_type = htons(ETHCMD_MESSAGE_TYPE_ONEWIRE);

    int len = write(cfg.sock, &msg, sizeof(struct ethcmd_message_t));

    printf("wrote %d bytes\n", len);

}  /* }}} */

void parse_message(struct ethcmd_message_t *msg)
/* {{{ */ {

    printf("length: %d, message type %d\n", ntohs(msg->length), ntohs(msg->message_type));

    printf("data:");

    for (int i = 0; i < ntohs(msg->length) - sizeof(struct ethcmd_message_t); i++)
        printf(" %02x", msg->data[i]);

    printf("\n");

}  /* }}} */

int main(int argc, char *argv[])
/* {{{ */ {

    cfg.verbose = 0;
    cfg.sock = -1;
    cfg.host = DEFAULT_HOST;
    cfg.port = DEFAULT_PORT;

    /* read commandline arguments */
    char c;

    DEBUG_PRINTF("activated\n");

    while ( (c = getopt_long(argc, argv, "hv::H:P:", longopts, 0)) != -1 ) {
        switch (c) {

            case 'h':
                      print_usage();
                      exit(EXIT_SUCCESS);
                      break;

            case 'v':
                      if (optarg != NULL)
                          cfg.verbose = atoi(optarg);
                      else
                          cfg.verbose = 1;
                      break;

            case 'H':
                      cfg.host = optarg;
                      break;

            case 'P':
                      cfg.port = atoi(optarg);
                      break;

            case '?':
                      errx(EXIT_FAILURE, "try --help");
                      break;

        }
    }

    DEBUG_PRINTF("verbosity level is %d\n", cfg.verbose);
    DEBUG_PRINTF("remote is %s:%d\n", cfg.host, cfg.port);

    cfg.sock = connect_host(cfg.host, cfg.port);

    DEBUG_PRINTF("connected.\n");

    send_message();

    /* main select loop */
    char receive_buffer[BUFSIZE];
    fd_set fds;

    FD_ZERO(&fds);
    FD_SET(cfg.sock, &fds);

    while (1) {
        if (select(cfg.sock+1, &fds, NULL, NULL, NULL) >= 0) {

            if (FD_ISSET(cfg.sock, &fds)) { /* data on network */
                int len = read(cfg.sock, receive_buffer, BUFSIZE);

                if (len > 0) {
                    printf("received %d bytes\n", len);

                    struct ethcmd_message_t *msg = (struct ethcmd_message_t *)receive_buffer;

                    if (ntohs(msg->length) != len)
                        printf("packet with invalid length received!\n");
                    else
                        parse_message(msg);
                } else if (len < 0)
                    errx(EXIT_FAILURE, "read()");
                else {
                    printf("connection closed\n");
                    break;
                }

            }
        } else
            errx(EXIT_FAILURE, "select()");
    }

    return EXIT_SUCCESS;

} /* }}} */
