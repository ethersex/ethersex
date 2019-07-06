/**********************************************************
 * Copyright (C) 2006 by Jochen Roessner <jochen@lugrot.de>, Germany
 * Copyright (C) 2008 by Christian Dietrich <stettberger@dokucode.de

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */

/* A little Wrapper around libusb to use the usb ecmd feature of ethersex */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <getopt.h>
#include <usb.h>

#include "protocols/usb/requests.h"

static usb_dev_handle *
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
            printf("Found it! devnr: %i %04X - %04X\n", 1,
                   dev->descriptor.idVendor, dev->descriptor.idProduct);
            return usb_bus_dev;
          }
          usb_close(usb_bus_dev);
        }
      }
    }
  }
  fprintf(stderr, "No suitable USB device found!\n");
  return NULL;
}

static int
usb_recv(usb_dev_handle * handle, char *buf, int len)
{
  int ret = usb_control_msg(handle,
                            USB_TYPE_VENDOR | USB_RECIP_DEVICE |
                            USB_ENDPOINT_IN,
                            USB_REQUEST_ECMD, 0, 0,
                            buf, len, 500);
  if (ret < 0)
    fprintf(stderr, "usb_recv: ret=%i, %s\n", ret, usb_strerror());

  return ret;                   /* > 0 = ok */
}



static int
usb_send(usb_dev_handle * handle, const char *data, int len)
{
  int ret = usb_control_msg(handle,
                            USB_TYPE_VENDOR | USB_RECIP_DEVICE |
                            USB_ENDPOINT_OUT,
                            USB_REQUEST_ECMD, 1, 0,
                            (char *) data, len, 500);
  if (ret < 0)
    fprintf(stderr, "usb_send: ret=%i, %s\n", ret, usb_strerror());

  return ret;                   /* > 0 = ok */
}

static void
print_usage(void)
{
  fprintf(stderr, "Usage: ecmd_usb [-d <device id>] [-v]\n");
  exit(EXIT_FAILURE);
}

int
main(int argc, char **argv)
{
  int index, option;
  int verbose = 0;
  char *device_id = "16c005dc";

  while ((option = getopt(argc, argv, "d:v")) != -1)
  {
    switch (option)
    {
      case 'd':
        device_id = optarg;
        break;
      case '?':
        if (optopt == 'd')
          fprintf(stderr, "Option -%c requires an argument.\n", optopt);
        else if (isprint(optopt))
          fprintf(stderr, "Unknown option `-%c'.\n", optopt);
        else
          fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
        print_usage();
        break;
      case 'v':
        verbose = 1;
        break;
      default:
        print_usage();
        break;
    }
  }

  for (index = optind; index < argc; index++)
    printf("Non-option argument %s\n", argv[index]);

  if (verbose)
    usb_set_debug(3);
  usb_init();

  usb_dev_handle *handle = usb_find(device_id);
  if (handle == 0)
    exit(EXIT_FAILURE);


  /* Start the Shell */
  char *line = NULL;
  size_t len;
  while (getline(&line, &len, stdin) != -1)
  {
    line[strlen(line) - 1] = 0;
    if (usb_send(handle, line, strlen(line) + 1) == 0)
    {
      fprintf(stderr, "Communication failed\n");
      usb_close(handle);
      exit(EXIT_FAILURE);
    }
    /* Now we must receive the answer */
    char buf[1024];
    int len = usb_recv(handle, buf, sizeof(buf) - 1);
    if (len > 0)
    {
      buf[len] = '\0';
      printf("%s\n", buf);

    }
    else if (len < 0)
    {
      fprintf(stderr, "Communication failed\n");
      usb_close(handle);
      exit(EXIT_FAILURE);
    }
  }
  usb_close(handle);
}
