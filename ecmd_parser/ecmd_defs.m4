dnl
dnl ecmd_defs.m4
dnl
dnl  You can declare your ecmd commands here.  The code will be expanded 
dnl  by the make system (which in turn uses GNU m4) to build the
dnl  ecmd_defs.c file.
dnl
dnl  Lines starting with `dnl' are comments.
dnl

ecmd_ifndef(TEENSY_SUPPORT)
  ecmd_ifdef(ENC28J60_SUPPORT)
    ecmd_feature(mac, "mac ",xx:xx:xx:xx:xx:xx,Set the MAC address.)
    ecmd_feature(show_mac, "show mac",,Display the MAC address.)
  ecmd_endif()
  ecmd_feature(show_ip, "show ip",,Display the IP address.)
  ecmd_ifndef(IPV6_SUPPORT)
    ecmd_feature(show_netmask, "show netmask",,Display the network mask.)
  ecmd_endif()
  ecmd_feature(show_gw, "show gw",,Display the IP address of the default router.)
  ecmd_feature(show_version, "show version",,Display the version number.)

  ecmd_ifndef(IPV6_SUPPORT)
    ecmd_ifndef(BOOTP_SUPPORT)
      ecmd_feature(ip, "ip ",IP,Set the IP address.)
      ecmd_feature(netmask, "netmask ",IP,Set the network mask.)
      ecmd_feature(gw, "gw ",IP,Set the address of the default router.)
    ecmd_endif()
  ecmd_endif()

  ecmd_ifdef(IPV6_STATIC_SUPPORT)
    ecmd_feature(ip, "ip ")
  ecmd_endif()

block(Resetting the controller)

  ecmd_feature(reset, "reset",,Reset the Ethersex.)
  ecmd_feature(wdreset, "wdreset",,Go into endless loop to trigger a watchdog timeout.)
ecmd_endif()

ecmd_feature(bootloader, "bootloader",,Call the bootloader.)

ecmd_ifdef(PORTIO_SUPPORT)
  block(Port I/O)
  ecmd_feature(io_set_ddr, "io set ddr", PORTNUM VALUE [MASK], Set the DDR of port PORTNUM to VALUE (possibly using the provided MASK).)
  ecmd_feature(io_get_ddr, "io get ddr", PORTNUM, Display the current value of the DDR PORTNUM.)
  ecmd_feature(io_set_port, "io set port", NUM VALUE [MASK], Set the PORT NUM to VALUE (possibly using the provided MASK).)
  ecmd_feature(io_get_port, "io get port", NUM, Display the current value of the PORT NUM.)
  ecmd_feature(io_get_pin, "io get pin", PORTNUM, Display the current value of the PIN-register of the port PORTNUM.)
  ecmd_feature(io_get_mask, "io get mask", PORTNUM, Display the mask of the port PORTNUM.)
ecmd_endif()

ecmd_ifdef(PORTIO_SIMPLE_SUPPORT)
  ecmd_feature(io, "io ")
ecmd_endif()

ecmd_ifdef(NAMED_PIN_SUPPORT)
  ecmd_ifdef(PORTIO_SUPPORT)
    block(Named Pin)
    ecmd_feature(pin_get, "pin get", NAME, Read and display the status of pin NAME.)
    ecmd_feature(pin_set, "pin set", NAME STATUS, Set the status of pin NAME to STATUS.)
    ecmd_feature(pin_toggle, "pin toggle", NAME, Toggle the status of pin NAME.)
  ecmd_endif()
ecmd_endif()

ecmd_ifdef(FS20_SUPPORT)
  ecmd_ifdef(FS20_SUPPORT_SEND)
    ecmd_feature(fs20_send, "fs20 send")
  ecmd_endif()

  ecmd_ifdef(FS20_SUPPORT_RECEIVE)
    ecmd_feature(fs20_receive, "fs20 receive")
  ecmd_endif()

  ecmd_ifdef(FS20_SUPPORT_RECEIVE_WS300)
    ecmd_feature(fs20_ws300, "fs20 ws300")
  ecmd_endif()
ecmd_endif()

ecmd_ifdef(HD44780_SUPPORT)
  ecmd_feature(lcd_clear, "lcd clear")
  ecmd_feature(lcd_write, "lcd write")
  ecmd_feature(lcd_goto, "lcd goto")
  ecmd_feature(lcd_char, "lcd char")
  ecmd_feature(lcd_init, "lcd init")
  ecmd_feature(lcd_shift, "lcd shift")
ecmd_endif()

ecmd_ifdef(ONEWIRE_SUPPORT)
  ecmd_feature(onewire_list, "1w list")
  ecmd_feature(onewire_get, "1w get")
  ecmd_feature(onewire_convert, "1w convert")
ecmd_endif()

ecmd_ifdef(RC5_SUPPORT)
  ecmd_feature(ir_send, "ir send")
  ecmd_feature(ir_receive, "ir receive")
ecmd_endif()

ecmd_ifdef(DNS_SUPPORT)
  ecmd_feature(nslookup, "nslookup ", HOSTNAME, Do DNS lookup for HOSTNAME (call twice).)
  ecmd_feature(show_dns, "show dns",, Display the IP address of the configured DNS server.)
  ecmd_ifndef(BOOTP_SUPPORT)
    ecmd_feature(dns, "dns ", IPADDR, Set the IP address of the DNS server to use to IPADDR.)
  ecmd_endif()
ecmd_endif()

ecmd_ifdef(CLOCK_SUPPORT)
  ecmd_feature(time, "time",, Display the current time.)
  ecmd_feature(date, "date",, Display the current date.)
ecmd_endif()

ecmd_ifdef(WHM_SUPPORT)
  ecmd_feature(whm, "whm",, Display the uptime.)
ecmd_endif()

ecmd_ifdef(ADC_SUPPORT)
  ecmd_feature(adc_get, "adc get")
ecmd_endif()

ecmd_ifdef(DATAFLASH_SUPPORT)
  block(Dataflash)
  ecmd_feature(df_status, "df status",, Display internal status.)

  ecmd_feature(fs_format, "fs format",, Format the filesystem.)
  ecmd_feature(fs_list, "fs list",, List the directory.)
  ecmd_feature(fs_mkfile, "fs mkfile ", NAME, Create a new file NAME.)
  ecmd_feature(fs_remove, "fs remove ", NAME, Delete the file NAME.)
  ecmd_feature(fs_truncate, "fs truncate ", NAME LEN, Truncate the file NAME to LEN bytes.)
ecmd_endif()

ecmd_ifndef(TEENSY_SUPPORT)
  ecmd_ifdef(RFM12_SUPPORT)
    block(RFM12)
    ecmd_feature(rfm12_status, "rfm12 status",, Display internal status.)
    ecmd_feature(rfm12_setbaud, "rfm12 setbaud", BAUD, Set baudrate to BAUD.)
    ecmd_feature(rfm12_setbandwidth, "rfm12 setbandwidth", BW, Set RX bandwidth to BW.)
    ecmd_feature(rfm12_setmod, "rfm12 setmod", MOD, Set modulation to MOD.)
    ecmd_feature(rfm12_setgain, "rfm12 setgain", GAIN, Set preamplifier gain to GAIN.)
    ecmd_feature(rfm12_setdrssi, "rfm12 setdrssi", DRSSI, Set the drssi to DRSSI.)
  ecmd_endif()
ecmd_endif()

block(Miscelleanous)

ecmd_ifndef(TEENSY_SUPPORT)
  ecmd_feature(d, "d ", ADDR, Dump the memory at ADDR (16 bytes).)

  ecmd_ifdef(USART_SUPPORT)
    ecmd_feature(usart_baud, "usart baud", BAUD, Set the USART baudrate to BAUD.)
  ecmd_endif()

ecmd_endif()

ecmd_ifdef(MODBUS_SUPPORT)
  ecmd_feature(modbus_recv, "mb recv ")
ecmd_endif()

ecmd_ifdef(HTTPD_AUTH_SUPPORT)
  ecmd_feature(http_passwd, "http passwd")
ecmd_endif()
