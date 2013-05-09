 /* Copyright(C) 2008 Christian Dietrich <stettberger@dokucode.de>
                 2009 Gerd v. Egidy <gerd@egidy.de>

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
  unsigned int mtu;

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
    int timeout = 0;

    while ((ret=write(fd,buf,count)) < (ssize_t)count)
    {
        if (ret < 0)
        {
            if (errno == EAGAIN)
            {
                // Packet timeout: max 30 msec transfer time per packet
                if (++timeout > 10)
                {
                    fprintf(stderr,"Packet timeout\n");
                    break;
                }
                usleep(3000);
                continue;
            }
            else if (errno == EINTR)
            {
                // just try again, do not wait or increase timeout
                continue;
            }
            else
            {
                // some severe error during io
                die("write to device failed: %s\n", strerror(errno));
            }
        }
        else
        {
            // less than count or nothing was written
            buf=(const char*)buf+ret;
            count-=ret;
        }
    }
}

// parse some bytes received via zbus
// returns number of bytes we have from a still incomplete packet
unsigned int parse_zbus(char* xferbuf, unsigned int xfersize)
{
    static char recvpktbuf[MAX_MTU];          // just for correct packet data, already descaped etc.
    static unsigned int recvpktpos = 0;       // current pos within recvpktbuf
    static unsigned int escaped = 0;          // must be static as the last byte in xferbuf could be the esc sequence
    static unsigned int pkt_started = 0;      // must be static as the packet could just be started with the last byte

    unsigned int i;
    for (i=0; i < xfersize; i++)
    {
        if (!escaped && xferbuf[i] == '\\')
        {
            escaped=1;
            continue;
        }

        if (!pkt_started)
        {
            // packet has not started yet

            if (escaped && xferbuf[i] == '0')
            {
                // start sequence detected
                escaped=0;
                pkt_started=1;
            }

            // something else was escaped, we don't really care
            if (escaped)
                escaped=0;

            // ignore all rubbish between packets
            continue;
        }
        else
        {
            // we are within a packet

            if (escaped && xferbuf[i] == '1')
            {
                // end sequence detected, packet is complete, send it off
                write(global.tun_fd, recvpktbuf,recvpktpos);

                escaped=0;
                pkt_started=0;
                recvpktpos=0;
                continue;
            }

            // something else was escaped, probably backslash, just append it
            if (escaped)
                escaped=0;

            // try to append data
            if (recvpktpos+1 > global.mtu)
            {
                // packet larger than our MRU (we assume MTU == MRU)

                // we should probably send some ICMP message to inform the sender.
                // But uIP currently can't refragment, so this would only help if
                // data from something other than ethersex is routed over ZBUS.
                // If someone wants to do that, add your code here

                // throw it all away
                pkt_started=0;
                recvpktpos=0;
            }
            else
            {
                recvpktbuf[recvpktpos++]=xferbuf[i];
            }
        }
    }

    // maybe there are no "real" bytes in the buffer but a packet has just been started
    if (!recvpktpos && pkt_started)
        return 1;
    else
        return recvpktpos;
}

void read_tty(void)
{
    char xferbuf[256];                        // one read is stored here and then parsed

    // read_tty is only called if there is really data to read, so it's got to be parsed
    unsigned int left_to_be_parsed=1;

    // loop until we have a complete packet and no more data waiting (or timeout)
    while(1)
    {
        int timeout = 0;
        int got;

        // Read from device
        got = read(global.tty_fd, xferbuf, sizeof(xferbuf));
        if (got == 0)
        {
            // someone closed (e.g. unplugged) the device, terminate the program
            fprintf(stderr, "%s: read from device returned 0 bytes (terminating)\n",
                global.argv0);
            exit(EXIT_SUCCESS);
        }
        else if (got < 0)
        {
            if (errno == EAGAIN)
            {
                if (!left_to_be_parsed)
                {
                    // nothing to be parsed anymore, no more data waiting
                    // -> we have done our job reading so return to main loop
                    break;
                }

                // Packet timeout: max 30 msec transfer time per packet
                if (++timeout > 10)
                {
                    fprintf(stderr,"Packet timeout\n");
                    break;
                }
                usleep(3000);
                continue;
            }
            else if (errno == EINTR)
            {
                // just try again, do not wait or increase timeout
                continue;
            }
            else
            {
                // some severe error during io
                die("write to device failed: %s\n", strerror(errno));
            }
        }

        left_to_be_parsed=parse_zbus(xferbuf,got);
    }
}

void write_tty(void)
{
    char netbuf[MAX_MTU];

    // start+endmarker and worst case: all bytes need to be escaped
    char writebuf[(MAX_MTU*2)+4];
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

    // switch to sendig mode (needed for RS485)
    set_rts(global.tty_fd, 1);

    // write out the packet in one go
    write_blocking(global.tty_fd, writebuf, (w-writebuf));

    // switch off sendig mode
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
  int c;

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

     if( FD_ISSET(global.tty_fd, &fds) )
        read_tty();

     // Outgoing packets
     if( FD_ISSET(global.tun_fd, &fds) )
        write_tty();
  }
  return 0;
}
