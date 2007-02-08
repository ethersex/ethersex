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
#include <stdint.h>
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
void send_message(struct ethcmd_msg_t *msg);
void parse_message(struct ethcmd_msg_t *msg);
void request_procotol_version(void);
void parse_commands(void);

void print_usage(void)
/* {{{ */ {

     printf("USAGE: ethcmd [options]\n"
           "  -h, --help                display this help and exit\n"
           "  -v, --verbose=[LEVEL]     set verbosity level (range: 0 <= LEVEL <= 255)\n"
           "                            if LEVEL is not given, the verbosity level is set to 1\n"
           "  -H, --host=HOST           set remote host (default: %s)\n"
           "  -P, --port=PORT           set remote port (default: %d)\n"
           "  -c, --command=CMD         execute command\n"
           "\n"
           " available commands:\n"
           // "  onewire_discover          discover the onewire bus for devices, print ids\n"
           // "  fs20_send                 send fs20 command, parameters: housecode, address, command\n"
           "   (none documented yet...)\n"
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
        errx(EXIT_FAILURE, "no address found for \"%s\"", host);

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

void send_message(struct ethcmd_msg_t *msg)
/* {{{ */ {

    DEBUG_PRINTF("send_message: writing data: ");
#   ifdef DEBUG
    char *p = (char *)msg;
    for (int i = 0; i < sizeof(struct ethcmd_msg_t); i++) {
        printf(" %02x", *p++);
    }

    printf("\n");

    int len = write(cfg.sock, msg, sizeof(struct ethcmd_msg_t));

#   else
    write(cfg.sock, msg, sizeof(struct ethcmd_msg_t));
#   endif

    DEBUG_PRINTF("send_message: wrote %d bytes\n", len);

}  /* }}} */

void parse_message(struct ethcmd_msg_t *msg)
/* {{{ */ {

    DEBUG_PRINTF("parse_message: subsystem %d\n", msg->sys);

    switch (msg->sys) {

        case ETHCMD_SYS_VERSION:  {
                                      struct ethcmd_msg_version_t *v = (struct ethcmd_msg_version_t *)msg;
                                      VERBOSE_PRINTF("server speaks protocol version %d.%d\n", v->major, v->minor);
                                      VERBOSE_PRINTF("sending command\r");
                                      parse_commands();
                                      break;
                                  }

        case ETHCMD_SYS_RESPONSE: {
                                      struct ethcmd_response_t *r = (struct ethcmd_response_t *)msg;
                                      VERBOSE_PRINTF("received response from system %d, status: %d\n", r->old_sys, r->status);
                                      parse_commands();
                                      break;
                                  }

        default:
                                  warn("parse_message: unknown subsystem %d", msg->sys);
                                  break;

    }

}  /* }}} */

void request_procotol_version(void)
/* {{{ */ {

    /* build protocol request message */
    struct ethcmd_msg_version_t *v = malloc(sizeof(struct ethcmd_msg_version_t));

    if (v == NULL)
        errx(EXIT_FAILURE, "malloc()");

    v->sys = ETHCMD_SYS_VERSION;
    v->cmd = ETHCMD_VERSION_REQUEST;

    send_message((struct ethcmd_msg_t *)v);

    free(v);

} /* }}} */

void parse_commands(void)
/* {{{ */ {

    VERBOSE_PRINTF("parsing %d arguments\n", cfg.argc);

    if (cfg.argc == 0) {
        VERBOSE_PRINTF("nothing to do, exiting\n");
        exit(EXIT_SUCCESS);
    }

    for (int i = 0; i < cfg.argc; i++)
        DEBUG_PRINTF("argument: \"%s\"\n", cfg.argv[i]);

    if (strncasecmp(cfg.argv[0], "fs20_send", strlen("fs20_send")) == 0) {

        if (cfg.argc < 4)
            errx(1, "need more parameters for fs20_send: housecode address command");

        VERBOSE_PRINTF("sending fs20 command\n");

        struct ethcmd_msg_fs20_t *msg = malloc(sizeof(struct ethcmd_msg_fs20_t));

        if (msg == NULL)
            errx(EXIT_FAILURE, "malloc()");

        msg->sys = ETHCMD_SYS_FS20;
        msg->cmd = ETHCMD_FS20_SEND;
        msg->housecode = htons(strtol(cfg.argv[1], NULL, 16));
        msg->address = strtol(cfg.argv[2], NULL, 16);
        msg->command = strtol(cfg.argv[3], NULL, 16);

        cfg.argv = &cfg.argv[4];
        cfg.argc -= 4;

        VERBOSE_PRINTF("fs20 parameters: 0x%04x 0x%02x 0x%02x\n",
                msg->housecode,
                msg->address,
                msg->command);

        send_message((struct ethcmd_msg_t *)msg);

        free(msg);
    } else if (strncasecmp(cfg.argv[0], "onewire_list", strlen("onewire_list")) == 0) {

        VERBOSE_PRINTF("requesting onewire device id list\n");
        VERBOSE_PRINTF("(not implemented yet)\n");

        cfg.argv = &cfg.argv[1];
        cfg.argc -= 1;

    } else if (strncasecmp(cfg.argv[0], "file_upload", strlen("file_upload")) == 0) {

        if (cfg.argc < 3)
            errx(1, "need more parameters for fs20_send: housecode address command");

        VERBOSE_PRINTF("requesting onewire device id list\n");
        VERBOSE_PRINTF("(not implemented yet)\n");

        cfg.argv = &cfg.argv[1];
        cfg.argc -= 1;

    } else
        errx(1, "unknown command: \"%s\"", cfg.argv[0]);

} /* }}} */

int main(int argc, char *argv[])
/* {{{ */ {

    cfg.verbose = 0;
    cfg.sock = -1;
    cfg.host = DEFAULT_HOST;
    cfg.port = DEFAULT_PORT;
    cfg.argv = NULL;
    cfg.argc = 0;

    /* read commandline arguments */
    char c;

    DEBUG_PRINTF("activated\n");

    while ( (c = getopt_long(argc, argv, "hv:H:P:", longopts, 0)) != -1 ) {
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

    if (argc - optind > 0) {
        cfg.argv = &argv[optind];
        cfg.argc = argc - optind;
    }

    DEBUG_PRINTF("verbosity level is %d\n", cfg.verbose);
    DEBUG_PRINTF("remote is %s:%d\n", cfg.host, cfg.port);

    cfg.sock = connect_host(cfg.host, cfg.port);

    DEBUG_PRINTF("connected.\n");

    request_procotol_version();

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
                    VERBOSE_PRINTF("received %d bytes\n", len);

                    struct ethcmd_msg_t *msg = (struct ethcmd_msg_t *)receive_buffer;
                    parse_message(msg);
                } else if (len < 0)
                    errx(EXIT_FAILURE, "read()");
                else {
                    VERBOSE_PRINTF("connection closed\n");
                    break;
                }

            }
        } else
            errx(EXIT_FAILURE, "select()");
    }

    return EXIT_SUCCESS;

} /* }}} */
