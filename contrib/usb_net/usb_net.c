 /* Copyright(C) 2008 Christian Dietrich <stettberger@dokucode.de>
    Copyright(C) 2008 Stefan Siegl <stesie@brokenpipe.de>

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
#include <termios.h>
#include <getopt.h>
#include <errno.h>
#include <unistd.h>
#include <usb.h>


#include <linux/if.h>
#include <linux/if_tun.h>

#define USB_REQUEST_NET_SEND 10
#define USB_REQUEST_NET_RECV 11

#define max(a,b) ((a) > (b) ? (a) : (b))

struct global_t {
  usb_dev_handle *usb_handle;
  char *usbid;

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

usb_dev_handle *
usb_find(const char *device)
{
  struct usb_bus *bus;
  unsigned int idVendor;
  unsigned int idProduct;
  if (sscanf(device, "%04x%04x", &idVendor, &idProduct) == 2)
  {
    usb_init();
    usb_find_busses();
    usb_find_devices();
    for (bus = usb_busses; bus; bus = bus->next)
    {
      struct usb_device *dev;
      for (dev = bus->devices; dev; dev = dev->next)
      {
        usb_dev_handle *usb_bus_dev;
        usb_bus_dev = usb_open(dev);
        if (usb_bus_dev)
        {
          if (dev->descriptor.idVendor == idVendor &&
              dev->descriptor.idProduct == idProduct)
          {
            printf ("gefunden! devnr: %i %04X - %04X\n",1,
                    dev->descriptor.idVendor, dev->descriptor.idProduct);
            return usb_bus_dev;
          }
        }
        if (usb_bus_dev)
          usb_close(usb_bus_dev);
      }
    }
  }
  fprintf(stderr, "Kein passendes USB Device gefunden\n");
  return 0;
}

void
cleanup(void)
{
  if (global.tun_fd != -1)
    close(global.tun_fd);
  if (global.usb_handle)
    usb_close(global.usb_handle);

}


void 
usage(void)
{
  fprintf(stderr, 
          "Usage: %s [OPTIONEN]\n"
          " -h --help      Print this help and exit\n"
          " -a --address   adds of the interface in the form 192.168.8.1/24\n"
          " -d --device    usb id (e.g. 16c005dc) \n"
          " -m --mtu       set mtu\n"
          " -u --up        call this command, when the interface is up\n", global.argv0);

}

int
usb_send(usb_dev_handle *handle, const char *data, int len)
{
  int ret = usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | 
                            USB_ENDPOINT_OUT, USB_REQUEST_NET_SEND, len, 0, (char*) data, len, 500);

  return ret; /* > 0 = ok */
}


int
usb_recv(usb_dev_handle *handle, char *buf, int len)
{
  int ret = usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE |  USB_ENDPOINT_IN, 
                            USB_REQUEST_NET_RECV, 0, 0, (char*)buf, len, 500);

  return ret;
}





int 
main(int argc, char *argv[])
{
  global.argv0 = argv[0];
  global.tun_fd = -1;
  global.usb_handle = NULL;
  global.usbid = (char *)"16c005dc";
  global.address = (char *)"192.168.8.1/24";
  global.mtu = 192;
  global.up = NULL;
  atexit(cleanup);

  fd_set fds;
  char netbuf[1600], c;
  int recvlen = 0;

  const struct option longopts[] = {
    {"help", no_argument, 0, 'h'},
    {"address", required_argument, 0, 'a'},
    {"device", required_argument, 0, 'd'},
    {"mtu", required_argument, 0, 'm'},
    {"up", required_argument, 0, 'u'},
    {0, 0, 0, 0}
  };

  while ((c = getopt_long(argc, argv, "ha:d:u:", longopts, 0)) != -1) {
    switch(c) {
    case 'h':
        usage();
        exit(EXIT_SUCCESS);
    case 'a':
        global.address = optarg;
        break;
    case 'd':
        global.usbid = optarg;
        break;
    case 'u':
        global.up = optarg;
        break;
    default:
        die("Try `--help' for more information.");
    }
  }

  open_tun("usb%d", global.address);
  global.usb_handle = NULL;

  /* Execute the up command */
  if (global.up)
    system(global.up);

  int fm = global.tun_fd + 1;
  struct timeval tv;

  while(1){
     FD_ZERO(&fds);
     FD_SET(global.tun_fd, &fds);

     tv.tv_sec = 0;
     tv.tv_usec = 100000;

     select(fm, &fds, NULL, NULL, &tv);
     if(global.usb_handle == NULL)
       global.usb_handle = usb_find(global.usbid);
     if(global.usb_handle == NULL)
       continue;

     // Outgoing packets
     if( FD_ISSET(global.tun_fd, &fds) ) {
       int l = read(global.tun_fd, netbuf, sizeof(netbuf));
       int r = usb_send(global.usb_handle, netbuf, l);
       printf ("sent: %d:%d\n", l, r);
       if (r < 0){
         usb_close(global.usb_handle);
         global.usb_handle = NULL;
         continue;
       }

     }

     int l = usb_recv (global.usb_handle, netbuf, sizeof (netbuf));
     if (l > 0) {
       printf ("recv: %d\n", l);
       write (global.tun_fd, netbuf, l);
     }
  }
  return 0;
}

