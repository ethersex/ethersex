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
usb_recv(usb_dev_handle *handle, const char *buf, int len, int timeout)
{
    int ret = usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE |  USB_ENDPOINT_IN, 
                            USB_REQUEST_ECMD, 0, 0, (char*)buf, len, timeout);
    if (ret <= 0) 
      return 0;
    return ret; /* > 0 = ok */
}



int
usb_send(usb_dev_handle * handle, const char *data, int len, int timeout)
{
  int ret = usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | 
                            USB_ENDPOINT_OUT, USB_REQUEST_ECMD, 1, 0, (char*) data, len, timeout);
    if (ret <= 0) 
    return 0;
  return ret; /* > 0 = ok */
}

//####################################### UDP UDP ################################################

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

//####################################### TCP TCP ################################################

int
tcp_open(const char *ip, int port) //ip, port
{
	// create an tcp socket
	struct sockaddr_in remote;
	int fd;

	if ((fd=socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))==-1) return 0;

	memset((char *) &remote, 0, sizeof(remote));
	remote.sin_family = AF_INET;
	remote.sin_port = htons(port);
	remote.sin_addr.s_addr = inet_addr(ip);

	int flags = fcntl(fd, F_GETFL);
	if(fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) return 0;

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
tcp_send(int fd, const char *data, int len)
{
	int ret = write(fd, data, len);
    if (ret <= 0) 
		return 0;
	return ret; /* > 0 = ok */
}

//####################################### RS232 RS232 ################################################

int
rs232_open(const char *device, int baudrate) //"/dev/ttyS0"
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
rs232_send(int fd, const char *data, int len)
{
	int ret = write(fd, data, len);
    if (ret <= 0) 
		return 0;
	return ret; /* > 0 = ok */
}

//###################################### HELPER ##############################

int64_t timespecDiff(struct timespec *timeA_p, struct timespec *timeB_p)
{
  return ((timeA_p->tv_sec * 1000000000) + timeA_p->tv_nsec) -
           ((timeB_p->tv_sec * 1000000000) + timeB_p->tv_nsec);
}

int  splitStr(std::vector<std::string>& L, const std::string& seq, const std::string& delims)
{
    typedef std::string::size_type ST;
    std::string STR;

    ST pos=0, LEN = seq.size();
    while(pos < LEN ){
        STR=""; // Init/clear the STR token buffer
        // remove any delimiters including optional (white)spaces
        while( (delims.find(seq[pos]) != std::string::npos) && (pos < LEN) ) ++pos;
        // leave if @eos
        if(pos==LEN) return L.size();
        // Save token data
        while( (delims.find(seq[pos]) == std::string::npos) && (pos < LEN) ) STR += seq[pos++];
        // put valid STR buffer into the supplied list
        //std::cout << "[" << STR << "]";
        if( ! STR.empty() ) L.push_back(STR);
    }
    return L.size();
}
//####################################### Connection ################################################

ecmd_connection::ecmd_connection() {}

ecmd_connection::~ecmd_connection() {

for(std::list<usb_esex_device*>::iterator iter = usb.begin(); iter != usb.end(); iter++)
    delete *iter;

for(std::list<rs232_esex_device*>::iterator iter = rs232.begin(); iter != rs232.end(); iter++)
    delete *iter;

for(std::list<udp_esex_device*>::iterator iter = udp.begin(); iter != udp.end(); iter++)
    delete *iter;

for(std::list<tcp_esex_device*>::iterator iter = tcp.begin(); iter != tcp.end(); iter++)
    delete *iter;
}

int ecmd_connection::add(std::string conn_string) {
    // split
    std::vector<std::string> pre;
    if (splitStr(pre,conn_string, ":")<2) return 0;
    pre[1] = pre[1].erase(0,2); // remove heading slashes

    //std::cout << pre[0] << " " << pre[1] << " " << pre[2];

    if (pre[0] == "rs232")
	{
        if (pre.size()==3)
            return add_rs232_device(pre[1].c_str(), atoi(pre[2].c_str()));
		else
            return add_rs232_device(pre[1].c_str(), RS232_DEFAULT_DATA);
	}
    else if (pre[0] == "usb")
	{
        add_usb_device(pre[1].c_str());
	}
    else if (pre[0] == "udp")
	{
        if (pre.size()==3)
            return add_udp_device(pre[1].c_str(), atoi(pre[2].c_str()));
		else
            return add_udp_device(pre[1].c_str(), UDP_DEFAULT_DATA);
	}
    else if (pre[0] == "tcp")
	{
        if (pre.size()==3)
            return add_tcp_device(pre[1].c_str(), atoi(pre[2].c_str()));
		else
            return add_tcp_device(pre[1].c_str(), TCP_DEFAULT_DATA);
	}
	return 0;
}


int ecmd_connection::add_usb_device(const char* vendor_product_id) {
	usb_dev_handle * fd = usb_opendev(vendor_product_id);
	if (!fd) return 0;

    usb_esex_device* dev = new usb_esex_device();
	dev->fd = fd;
	dev->next = 0;
    usb.push_back(dev);
    return 1;
}

int ecmd_connection::add_rs232_device(const char* device_file, int baudrate) {
	int fd = rs232_open(device_file, baudrate);
	if (!fd) return 0;

    rs232_esex_device* dev = new rs232_esex_device();
    dev->fd = fd;
    dev->next = 0;
    rs232.push_back(dev);
    return 1;
}

int ecmd_connection::add_udp_device(const char* ip, int port) {
	int fd = udp_open(ip, port);
	if (!fd) return 0;

    udp_esex_device* dev = new udp_esex_device();
    dev->fd = fd;
    dev->next = 0;
    udp.push_back(dev);
    return 1;
}

int ecmd_connection::add_tcp_device(const char* ip, int port) {
	int fd = tcp_open(ip, port);
	if (!fd) return 0;

    tcp_esex_device* dev = new tcp_esex_device();
    dev->fd = fd;
    dev->next = 0;
    tcp.push_back(dev);
    return 1;
}

std::string ecmd_connection::execute(std::string ecmd,int timeout) {
    if (!ecmd.size()) return std::string();
    responses.clear();
	int pos = 0;
    if (ecmd.at(ecmd.size()-1) != '\n')
        ecmd.push_back('\n');
    int len = ecmd.size();
    struct timespec start, end;
    char response_buffer[response_buffer_len];

    for(std::list<usb_esex_device*>::iterator iter = usb.begin(); iter != usb.end(); iter++)
    {
        memset(response_buffer, 0, response_buffer_len);
        if (usb_send((*iter)->fd, ecmd.c_str(), len, timeout) == 0) printf("usb send failed\n");
		clock_gettime(CLOCK_MONOTONIC, &start);
		do
		{
            if (usb_recv((*iter)->fd, response_buffer, response_buffer_len, timeout) != 0) clock_gettime(CLOCK_MONOTONIC, &start);
            pos = strlen(response_buffer);
			usleep(30000);
			clock_gettime(CLOCK_MONOTONIC, &end);
		} while (timespecDiff(&end, &start)/100000 < timeout);
        responses.push_back(std::string(response_buffer));
	}
    for(std::list<rs232_esex_device*>::iterator iter = rs232.begin(); iter != rs232.end(); iter++)
    {
        memset(response_buffer, 0, response_buffer_len);
        if (rs232_send((*iter)->fd, ecmd.c_str(), len) == 0) printf("rs232 send failed\n");
		clock_gettime(CLOCK_MONOTONIC, &start);
		do
		{
            if (rs232_recv((*iter)->fd, response_buffer+pos, response_buffer_len-pos) != 0) clock_gettime(CLOCK_MONOTONIC, &start);
            pos = strlen(response_buffer);
			usleep(30000);
			clock_gettime(CLOCK_MONOTONIC, &end);
        } while (timespecDiff(&end, &start)/100000 < timeout*2);
        std::string temp(response_buffer);
        // omit first received line (is an echo of the command)
        responses.push_back(temp.substr(temp.find('\n')+1));
	}
    for(std::list<udp_esex_device*>::iterator iter = udp.begin(); iter != udp.end(); iter++)
    {
        memset(response_buffer, 0, response_buffer_len);
        if (udp_send((*iter)->fd, ecmd.c_str(), len) == 0) printf("udp send failed\n");
		clock_gettime(CLOCK_MONOTONIC, &start);
		do
		{
            if (udp_recv((*iter)->fd, response_buffer, response_buffer_len) != 0) clock_gettime(CLOCK_MONOTONIC, &start);
            pos = strlen(response_buffer);
			usleep(30000);
			clock_gettime(CLOCK_MONOTONIC, &end);
		} while (timespecDiff(&end, &start)/100000 < timeout);
        responses.push_back(std::string(response_buffer));
	}
    for(std::list<tcp_esex_device*>::iterator iter = tcp.begin(); iter != tcp.end(); iter++)
    {
        memset(response_buffer, 0, response_buffer_len);
        if (tcp_send((*iter)->fd, ecmd.c_str(), len) == 0) printf("tcp send failed\n");
		clock_gettime(CLOCK_MONOTONIC, &start);
		do
		{
            if (tcp_recv((*iter)->fd, response_buffer, response_buffer_len) != 0) clock_gettime(CLOCK_MONOTONIC, &start);
            pos = strlen(response_buffer);
			usleep(30000);
			clock_gettime(CLOCK_MONOTONIC, &end);
		} while (timespecDiff(&end, &start)/100000 < timeout);
        responses.push_back(std::string(response_buffer));
	}

    if (responses.size())
        return responses.front();
    else
        return std::string();
}

