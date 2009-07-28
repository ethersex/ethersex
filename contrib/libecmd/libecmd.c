#include "libecmd.h"

//####################################### USB USB ################################################

  usb_dev_handle *
usb_opendev(const char *device)
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
            return usb_bus_dev;
          }
        }
        if (usb_bus_dev)
          usb_close(usb_bus_dev);
      }
    }
  }
  return 0;
}

int
usb_recv(usb_dev_handle *handle, char *buf, int len, int timeout)
{
    int ret = usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE |  USB_ENDPOINT_IN, 
                            USB_REQUEST_ECMD, 0, 0, (char*)buf, len, timeout);
    if (ret <= 0) 
      return 0;
    return ret; /* > 0 = ok */
}



int
usb_send(usb_dev_handle * handle, char *data, int len, int timeout)
{
  int ret = usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | 
                            USB_ENDPOINT_OUT, USB_REQUEST_ECMD, 1, 0, (char*) data, len, timeout);
    if (ret <= 0) 
    return 0;
  return ret; /* > 0 = ok */
}

//####################################### UDP UDP ################################################

int
udp_open(char *ip, int port) //ip, port
{
	// create an udp socket
	struct sockaddr_in remote;
	int fd;

	if ((fd=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1) return 0;

	memset((char *) &remote, 0, sizeof(remote));
	remote.sin_family = AF_INET;
	remote.sin_port = htons(port);
	remote.sin_addr.s_addr = inet_addr(ip);

	// connect to the socket
	if (connect(fd, (struct sockaddr*)&remote, sizeof(remote)) < 0) return 0;

	return fd;
}

int
udp_recv(int fd, char *buf, int len)
{
	int ret = read(fd, buf, len);
    if (ret <= 0) 
      return 0;
    return ret; /* > 0 = ok */
}

int
udp_send(int fd, char *data, int len)
{
	int ret = write(fd, data, len);
    if (ret <= 0) 
		return 0;
	return ret; /* > 0 = ok */
}

//####################################### TCP TCP ################################################

int
tcp_open(char *ip, int port) //ip, port
{
	// create an tcp socket
	struct sockaddr_in remote;
	int fd;

	if ((fd=socket(AF_INET, SOCK_DGRAM, IPPROTO_TCP))==-1) return 0;

	memset((char *) &remote, 0, sizeof(remote));
	remote.sin_family = AF_INET;
	remote.sin_port = htons(port);
	remote.sin_addr.s_addr = inet_addr(ip);

	// connect to the socket
	if (connect(fd, (struct sockaddr*)&remote, sizeof(remote)) < 0) return 0;

	return fd;
}

int
tcp_recv(int fd, char *buf, int len)
{
	int ret = read(fd, buf, len);
    if (ret <= 0) 
      return 0;
    return ret; /* > 0 = ok */
}

int
tcp_send(int fd, char *data, int len)
{
	int ret = write(fd, data, len);
    if (ret <= 0) 
		return 0;
	return ret; /* > 0 = ok */
}

//####################################### RS232 RS232 ################################################

int
rs232_open(char *device, int baudrate) //"/dev/ttyS0"
{
	// open
	int fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY);
	if (fd == -1) return 0;

    struct termios options;

    /*
     * Get the current options for the port...
     */

    tcgetattr(fd, &options);

    /*
     * Set the baud rates to 19200...
     */
	switch (baudrate) {
		case 2400:
			cfsetispeed(&options, B2400);
			cfsetospeed(&options, B2400);
			break;
		case 4800: 
			cfsetispeed(&options, B4800);
			cfsetospeed(&options, B4800);
			break;
		case 9600: 
			cfsetispeed(&options, B9600);
			cfsetospeed(&options, B9600);
			break;
		case 19200: 
			cfsetispeed(&options, B19200);
			cfsetospeed(&options, B19200);
			break;
		case 57600: 
			cfsetispeed(&options, B57600);
			cfsetospeed(&options, B57600);
			break;
		case 115200: 
			cfsetispeed(&options, B115200);
			cfsetospeed(&options, B115200);
			break;
		default:
			break;
	}

    /*
     * Enable the receiver and set local mode...
     */
	options.c_cflag &= ~PARENB;    // set no parity, stop bits, data bits
	options.c_cflag &= ~CSTOPB;
	options.c_cflag &= ~CSIZE;
	options.c_cflag |= CS8;

    options.c_cflag |= (CLOCAL | CREAD);

    /*
     * Set the new options for the port...
     */

    tcsetattr(fd, TCSANOW, &options);
   	fcntl(fd, F_SETFL, O_NONBLOCK);
	//fcntl(fd, F_SETFL, 0);

	return fd;
}


int
rs232_recv(int fd, char *buf, int len)
{
	errno = 0;
	int ret = read(fd, buf, len);
    if (ret <= 0) 
      return 0;
    return ret; /* > 0 = ok */
}



int
rs232_send(int fd, char *data, int len)
{
	int ret = write(fd, data, len);
    if (ret <= 0) 
		return 0;
	return ret; /* > 0 = ok */
}

//####################################### Connection ################################################

int ecmd_add(struct connection* c, char* conn_string) {
	char* arg1;
	char* arg2;
	int data = 0;
	// find slash
	arg1 = strchr(conn_string, '/') + 2;
	if (arg1 == 0) return 0;
	// replace colon with 0 to terminate string 1
	arg2 = strchr(arg1, ':');
	if (arg2!=0) {
		*arg2 = 0;
		arg2++;
		data = atoi(arg2);
	}
	//printf("arg1: %s\n", arg1);
	//printf("arg2: %s\n", arg2);

	if (strncmp(conn_string, "rs232", 5)==0)
	{
		if (arg2)
			return ecmd_add_rs232_device(c, arg1, data);
		else
			return ecmd_add_rs232_device(c, arg1, RS232_DEFAULT_DATA);
	}
	else if (strncmp(conn_string, "usb", 3)==0)
	{
		ecmd_add_usb_device(c, arg1);
	}
	else if (strncmp(conn_string, "udp", 3)==0)
	{
		if (arg2)
			return ecmd_add_udp_device(c, arg1, data);
		else
			return ecmd_add_udp_device(c, arg1, UDP_DEFAULT_DATA);
	}
	else if (strncmp(conn_string, "tcp", 3)==0)
	{
		if (arg2)
			return ecmd_add_tcp_device(c, arg1, data);
		else
			return ecmd_add_tcp_device(c, arg1, TCP_DEFAULT_DATA);
	}
	return 0;
}

struct connection* ecmd_init() {
	struct connection* c = malloc(sizeof(struct connection));
	if (!c) return 0;
	c->usb = 0;
	c->rs232 = 0;
	c->udp = 0;
	c->tcp = 0;
	return c;
}

int ecmd_add_usb_device(struct connection* c, char* vendor_product_id) {
	usb_dev_handle * fd = usb_opendev(vendor_product_id);
	if (!fd) return 0;

	struct usb_esex_device* dev = malloc(sizeof(struct usb_esex_device));
	if (!dev) { // malloc failed
		usb_close(fd);
		return 0;
	}

	dev->fd = fd;
	dev->next = 0;

	if (c->usb == 0) {
		// first device
		c->usb = dev;
		return 1;
	} else {
		// einhängen
		struct usb_esex_device* d = c->usb;
		while (1) {
			if (d->next) d = d->next;
			else break;
		}
		d->next = dev;
		return 1;
	}
}

int ecmd_add_rs232_device(struct connection* c, char* device_file, int baudrate) {
	int fd = rs232_open(device_file, baudrate);
	if (!fd) return 0;

	struct rs232_esex_device* dev = malloc(sizeof(struct rs232_esex_device));
	if (!dev) { // malloc failed
		close(fd);
		return 0;
	}

	dev->fd = fd;
	dev->next = 0;

	if (c->rs232 == 0) {
		// first device
		c->rs232 = dev;
		return 1;
	} else {
		// einhängen
		struct rs232_esex_device* d = c->rs232;
		while (1) {
			if (d->next) d = d->next;
			else break;
		}
		d->next = dev;
		return 1;
	}
}

int ecmd_add_udp_device(struct connection* c, char* ip, int port) {
	int fd = udp_open(ip, port);
	if (!fd) return 0;

	struct udp_esex_device* dev = malloc(sizeof(struct udp_esex_device));
	if (!dev) { // malloc failed
		close(fd);
		return 0;
	}

	dev->fd = fd;
	dev->next = 0;

	if (c->udp == 0) {
		// first device
		c->udp = dev;
		return 1;
	} else {
		// einhängen
		struct udp_esex_device* d = c->udp;
		while (1) {
			if (d->next) d = d->next;
			else break;
		}
		d->next = dev;
		return 1;
	}
}

int ecmd_add_tcp_device(struct connection* c, char* ip, int port) {
	int fd = tcp_open(ip, port);
	if (!fd) return 0;

	struct tcp_esex_device* dev = malloc(sizeof(struct tcp_esex_device));
	if (!dev) { // malloc failed
		close(fd);
		return 0;
	}

	dev->fd = fd;
	dev->next = 0;

	if (c->tcp == 0) {
		// first device
		c->tcp = dev;
		return 1;
	} else {
		// einhängen
		struct tcp_esex_device* d = c->tcp;
		while (1) {
			if (d->next) d = d->next;
			else break;
		}
		d->next = dev;
		return 1;
	}
}

void ecmd_close(struct connection* c) {
	// get head of linked lists
	struct usb_esex_device* usb = c->usb;
	struct rs232_esex_device* rs232 = c->rs232;
	struct udp_esex_device* udp = c->udp;
	struct tcp_esex_device* tcp = c->tcp;
	// temp
	struct usb_esex_device* tusb = c->usb;
	struct rs232_esex_device* trs232 = c->rs232;
	struct udp_esex_device* tudp = c->udp;
	struct tcp_esex_device* ttcp = c->tcp;

	// free
	while (usb) {
		tusb = usb;
		usb = usb->next;
		usb_close(tusb->fd);
		free (tusb);
	}
	while (rs232) {
		trs232 = rs232;
		rs232 = rs232->next;
		close(trs232->fd);
		free (trs232);
	}
	while (udp) {
		tudp = udp;
		udp = udp->next;
		close(tudp->fd);
		free (tudp);
	}
	while (tcp) {
		ttcp = tcp;
		tcp = tcp->next;
		close(ttcp->fd);
		free (ttcp);
	}

	// free connection
	free (c);
}

int64_t timespecDiff(struct timespec *timeA_p, struct timespec *timeB_p)
{
  return ((timeA_p->tv_sec * 1000000000) + timeA_p->tv_nsec) -
           ((timeB_p->tv_sec * 1000000000) + timeB_p->tv_nsec);
}

char* ecmd_execute(struct connection* c,char* ecmd,int len,int timeout) {
	// get head of linked lists
	struct usb_esex_device* usb = c->usb;
	struct rs232_esex_device* rs232 = c->rs232;
	struct udp_esex_device* udp = c->udp;
	struct tcp_esex_device* tcp = c->tcp;

	memset(response_buffer, 0, response_buffer_len);
	int pos = 0;
	struct timespec start, end;

	while (usb) {
		if (usb_send(usb->fd, ecmd, len, timeout) == 0) printf("usb send failed\n");
		clock_gettime(CLOCK_MONOTONIC, &start);
		do
		{
			if (usb_recv(usb->fd, response_buffer, response_buffer_len, timeout) == 0) clock_gettime(CLOCK_MONOTONIC, &start);
			pos = strlen(response_buffer);
			usleep(30000);
			clock_gettime(CLOCK_MONOTONIC, &end);
		} while (timespecDiff(&end, &start)/100000 < timeout);
		usb = usb->next;
	}
	while (rs232) {
		if (rs232_send(rs232->fd, ecmd, len) == 0) printf("rs232 send failed\n");
		clock_gettime(CLOCK_MONOTONIC, &start);
		do
		{
			if (rs232_recv(rs232->fd, response_buffer+pos, response_buffer_len-pos) != 0) clock_gettime(CLOCK_MONOTONIC, &start);
			pos = strlen(response_buffer);
			usleep(30000);
			clock_gettime(CLOCK_MONOTONIC, &end);
		} while (timespecDiff(&end, &start)/100000 < timeout);
		// omit first received line (is an echo of the command)
		if (pos) strcpy(response_buffer,strchr((char*)response_buffer,'\n')+1);
		rs232 = rs232->next;
	}
	while (udp) {
		if (udp_send(udp->fd, ecmd, len) == 0) printf("udp send failed\n");
		clock_gettime(CLOCK_MONOTONIC, &start);
		do
		{
			if (udp_recv(udp->fd, response_buffer, response_buffer_len) == 0) clock_gettime(CLOCK_MONOTONIC, &start);
			pos = strlen(response_buffer);
			usleep(30000);
			clock_gettime(CLOCK_MONOTONIC, &end);
		} while (timespecDiff(&end, &start)/100000 < timeout);
		udp = udp->next;
	}
	while (tcp) {
		if (tcp_send(tcp->fd, ecmd, len) == 0) printf("tcp send failed\n");
		clock_gettime(CLOCK_MONOTONIC, &start);
		do
		{
			if (tcp_recv(udp->fd, response_buffer, response_buffer_len) == 0) clock_gettime(CLOCK_MONOTONIC, &start);
			pos = strlen(response_buffer);
			usleep(30000);
			clock_gettime(CLOCK_MONOTONIC, &end);
		} while (timespecDiff(&end, &start)/100000 < timeout);
		tcp = tcp->next;
	}

	return response_buffer;
}

