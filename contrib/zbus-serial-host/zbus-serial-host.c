 /* Copyright(C) 2008 Christian Dietrich <stettberger@dokucode.de>

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

#define _XOPEN_SOURCE  520
#define _BSD_SOURCE

#include <stdarg.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <getopt.h>
#include <errno.h>


#include <linux/if.h>
#include <linux/if_tun.h>

#define MAX_MTU 1500

#define max(a,b) ((a) > (b) ? (a) : (b))

struct global_t {
  struct termios restore_serial;
  int tty_fd;
  int baudrate;
  char *tty;

  char *argv0;

  int tun_fd;
  char tun_name[15]; /* Interface name */
  char *address;
  int mtu;

  char *up;
} global;

void 
die(const char *msg, ...)
{
  va_list va;
  va_start(va, msg);
  fprintf(stderr, "%s: ", global.argv0);
  vfprintf(stderr, msg, va);
  putchar('\n');
  va_end(va);
  exit (-1);
}

int tun_alloc(char *dev)
{
  struct ifreq ifr;
  int fd, err;

  if( (fd = open("/dev/net/tun", O_RDWR)) < 0 )
    return -1;

  memset(&ifr, 0, sizeof(ifr));

  /* Flags: IFF_TUN   - TUN device (no Ethernet headers) 
   *        IFF_TAP   - TAP device  
   *
   *        IFF_NO_PI - Do not provide packet information  
   */ 
  ifr.ifr_flags = IFF_TUN | IFF_NO_PI; 
  if( *dev )
    strncpy(ifr.ifr_name, dev, IFNAMSIZ);

  if( (err = ioctl(fd, TUNSETIFF, (void *) &ifr)) < 0 ){
    close(fd);
    return err;
  }
  strcpy(dev, ifr.ifr_name);
  return fd;
}              

void
open_tty(char *dev, int baudrate)
{
  struct termios attr;

  if ((global.tty_fd = open(dev, O_RDWR | O_NONBLOCK)) == -1)
    die("Couldn't open device: %s", dev);




  tcgetattr(global.tty_fd, &global.restore_serial);


  switch (baudrate) {
  case 50:     global.baudrate = B50; break;
  case 75:     global.baudrate = B75; break;
  case 110:    global.baudrate = B110; break;
  case 134:    global.baudrate = B134; break;
  case 150:    global.baudrate = B150; break;
  case 200:    global.baudrate = B200; break;
  case 300:    global.baudrate = B300; break;
  case 600:    global.baudrate = B600; break;
  case 1200:   global.baudrate = B1200; break;
  case 1800:   global.baudrate = B1800; break;
  case 2400:   global.baudrate = B2400; break;
  case 4800:   global.baudrate = B4800; break;
  case 9600:   global.baudrate = B9600; break;
  case 19200:  global.baudrate = B19200; break;
  case 38400:  global.baudrate = B38400; break;
  case 57600:  global.baudrate = B57600; break;
  case 115200: global.baudrate = B115200; break;
  case 230400: global.baudrate = B230400; break;
  case 460800: global.baudrate = B460800; break;
  case 500000: global.baudrate = B500000; break;
  case 576000: global.baudrate = B576000; break;
  case 921600: global.baudrate = B921600; break;
  case 1000000: global.baudrate = B1000000; break;
  case 1152000: global.baudrate = B1152000; break;
  case 1500000: global.baudrate = B1500000; break;
  case 2000000: global.baudrate = B2000000; break;
  case 2500000: global.baudrate = B2500000; break;
  default:  die("invalid baudrate %d\n", baudrate);
  }

  memcpy(&attr, &global.restore_serial, sizeof(struct termios));

  /* set baudrate, 8n1, as-raw-as-possible ... */
  attr.c_cflag = CS8 | CREAD | CLOCAL ; //| CRTSCTS;
  attr.c_lflag = 0;

  attr.c_iflag = IGNBRK | IMAXBEL;
  attr.c_oflag = 0;

  attr.c_cc[VMIN] = 1;
  attr.c_cc[VTIME] = 5;

  cfsetispeed(&attr, global.baudrate);
  cfsetospeed(&attr, global.baudrate);

  int mcs = 0;
  ioctl(global.tty_fd, TIOCMGET, &mcs);
  mcs |= TIOCM_RTS;
  ioctl(global.tty_fd, TIOCMSET, &mcs);

  tcsetattr(global.tty_fd, TCSANOW, &attr);

}

void 
open_tun(const char *dev, char *address)
{

  strcpy(global.tun_name, dev);
  global.tun_fd = tun_alloc(global.tun_name);
  if (global.tun_fd < 0)
    die("Couldn't open tun device");

  int ret;
  char *cmd = malloc(strlen("ip addr add %s dev %s") + strlen(address) + strlen(dev) + 1);
  if (!cmd)
    die("Malloc failed");

  sprintf(cmd, "ip addr add %s dev %s", address, global.tun_name);
  ret = system(cmd);
  if (ret != 0)
    die("Setting ip address failed with %d", ret);

  sprintf(cmd, "ip link set %s mtu %d", global.tun_name, global.mtu);
  ret = system(cmd);
  if (ret != 0)
    die("Failed to set the mtu with %d", ret);

  sprintf(cmd, "ip link set %s up", global.tun_name);
  ret = system(cmd);
  if (ret != 0)
    die("Bringing up the tun device failed with %d", ret);

  free(cmd);

}

void
cleanup(void)
{
  if (global.tty_fd != -1) {
    tcsetattr(global.tty_fd, 0, &global.restore_serial);
    close(global.tty_fd);
  }
  if (global.tun_fd != -1)
    close(global.tun_fd);

}

void 
set_rts(int fd, int high)
{
  int mcs = 0;
  ioctl(fd, TIOCMGET, &mcs);
  if (high)
    mcs &= ~(TIOCM_RTS);
  else
    mcs |= TIOCM_RTS;
  ioctl(fd, TIOCMSET, &mcs);
}

void write_blocking(int fd, const void* buf, size_t count)
{
    ssize_t ret;

    while ((ret=write(fd,buf,count)) < (ssize_t)count)
    {
        if (ret < 0)
        {
            if (errno != EAGAIN && errno != EINTR)
                die("write to device failed: %s\n", strerror(errno));
        }
        else
        {
            // less than count or nothing was written
            buf=(const char*)buf+ret;
            count-=ret;
        }
    }
}

void read_tty(void)
{
    static char netbuf[1600];
    static char recvbuf[1600];
    static int recvlen = 0;

    int packet_ended = 0, attached = 0;
    int timeout = 0, first_packet = 1;
    int packet_began = 0, escaped = 0;

    while(1)
    {
        // Read from device
        int i, l = read(global.tty_fd, netbuf, sizeof(netbuf));
        if (l == 0)
        {
            fprintf(stderr, "%s: read from device returned 0 bytes (terminating)\n",
                global.argv0);
            exit(EXIT_SUCCESS);
        }
        else if (l < 0)
        {
            if (errno != EAGAIN)
                die("read from device failed: %s\n", strerror(errno));
            else
            {
                timeout += 1;
                if (timeout > 10)
                {
                    printf("Packet timeout\n");
                    break;
                }
                usleep(3000);
                continue;
            }
        }
        timeout = 0;

        // decoder
        attached = 0;
        if (l == 1 && escaped && netbuf[0] == '1')
        {
            packet_ended = 1;
            attached = 1;
            recvlen -= 1;
        }

        for (i = 0; i < l - 1; i++)
        {
            if (netbuf[i] == '\\' && netbuf[i+1] == '0')
            {
                recvlen = l - i - 2;
                memcpy(recvbuf, netbuf + i + 2, l - i - 2);
                attached = 1;
                packet_began  = 1;

            }
            else if (netbuf[i] == '\\' && netbuf[i+1] == '1')
            {
                if (attached)
                {
                    recvlen = i - 2;
                    packet_ended = 1;
                }
                else
                {
                    if ((unsigned int)(recvlen+i) > sizeof(recvbuf))
                    {
                        // data too large for recvbuf -> clean buffer and forget
                        recvlen = 0;
                        packet_ended = 0;
                        break;
                    }

                    memcpy(recvbuf + recvlen , netbuf, i);
                    recvlen += i;
                    packet_ended = 1;
                    attached = 1;
                }
            }
        }

        if (! attached )
        {
            if (netbuf[l - 1] == '\\' && netbuf[l - 2] != '\\')
                escaped = 1;
            else
                escaped = 0;

            if ((unsigned int)(recvlen+l) > sizeof(recvbuf))
            {
                // data too large for recvbuf -> clean buffer and forget
                recvlen = 0;
                packet_ended = 0;
                break;
            }

            memcpy(recvbuf + recvlen , netbuf, l);
            recvlen += l;
        }

        if (packet_ended) break;
        if (first_packet && ! packet_began) break;

        first_packet = 0;
    }

    if (packet_ended)
    {
        int p1, p2 ;
        for (p1 = 0, p2 = 0; p1 < recvlen; p1++)
        {
            if(recvbuf[p1] == '\\')
                p1 ++;
            recvbuf[p2++] = recvbuf[p1];
        }

        write(global.tun_fd, recvbuf, p2);
    }
}

void write_tty(void)
{
    static char netbuf[MAX_MTU];

    // start+endmarker and worst case: all bytes need to be escaped
    static char writebuf[(MAX_MTU*2)+4];
    char *w = writebuf;

    int l = read(global.tun_fd, netbuf, sizeof(netbuf));
    char *p = netbuf;

    // start-marker
    *w++='\\';
    *w++='0';

    while (l > 0)
    {
        // escape char if it is backslash
        if (*p == '\\')
            *w++='\\';

        *w++=*p++;

        l--;
    }

    // end-marker
    *w++='\\';
    *w++='1';

    // write out the packet in one go
    set_rts(global.tty_fd, 1);
    write_blocking(global.tty_fd, writebuf, (w-writebuf));
    set_rts(global.tty_fd, 0);
}

void 
usage(void)
{
  fprintf(stderr, 
          "Usage: %s [OPTIONEN]\n"
          " -h --help      Print this help and exit\n"
          " -r --baudrate  set baudrate\n"
          " -a --address   adds of the interface in the form 192.168.8.1/24\n"
          " -d --device    set the ttyS* interface to use\n"
          " -m --mtu       set mtu\n"
          " -u --up        call this command, when the interface is up\n", global.argv0);

}

int 
main(int argc, char *argv[])
{
  global.argv0 = argv[0];
  global.tun_fd = -1;
  global.tty_fd = -1;
  global.address = (char *)"192.168.8.1/24";
  global.tty = (char *)"/dev/ttyS0";
  global.baudrate = 19200;
  global.mtu = 174;
  global.up = NULL;
  atexit(cleanup);

  fd_set fds;
  char c;

  const struct option longopts[] = {
    {"help", no_argument, 0, 'h'},
    {"baudrate", required_argument, 0, 'r'},
    {"address", required_argument, 0, 'a'},
    {"device", required_argument, 0, 'd'},
    {"mtu", required_argument, 0, 'm'},
    {"up", required_argument, 0, 'u'},
    {0, 0, 0, 0}
  };

  while ((c = getopt_long(argc, argv, "hr:a:d:u:m:", longopts, 0)) != -1) {
    switch(c) {
    case 'h':
        usage();
        exit(EXIT_SUCCESS);
    case 'r':
        global.baudrate = atoi(optarg);
        break;
    case 'a':
        global.address = optarg;
        break;
    case 'd':
        global.tty = optarg;
        break;
    case 'u':
        global.up = optarg;
        break;
    case 'm':
    {
        char* endptr;
        global.mtu=strtoul(optarg,&endptr,0);
        if (*endptr != 0 || global.mtu == 0 || global.mtu > MAX_MTU)
            die("illegal value for mtu given, see `--help'");
        break;
    }
    default:
        die("Try `--help' for more information.");
    }
  }

  open_tun("zbus%d", global.address);
  open_tty(global.tty, global.baudrate);

  /* Execute the up command */
  if (global.up)
    system(global.up);

  int fm = max(global.tun_fd, global.tty_fd) + 1;

  while(1){
     FD_ZERO(&fds);
     FD_SET(global.tun_fd, &fds);
     FD_SET(global.tty_fd, &fds);

     select(fm, &fds, NULL, NULL, NULL);

     /* Incoming Packets
      * WARNING: This piece of code grew */
     if( FD_ISSET(global.tty_fd, &fds) )
        read_tty();

     // Outgoing packets
     if( FD_ISSET(global.tun_fd, &fds) )
        write_tty();
  }
  return 0;
}
