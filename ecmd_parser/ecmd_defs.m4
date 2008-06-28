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
    ecmd_feature(show_mac, "show mac")
    ecmd_feature(mac, "mac ")
  ecmd_endif()
  ecmd_feature(show_ip, "show ip")
  ecmd_ifndef(IPV6_SUPPORT)
    ecmd_feature(show_netmask, "show netmask")
  ecmd_endif()
  ecmd_feature(show_gw, "show gw")
  ecmd_feature(show_version, "show version")

  ecmd_ifndef(IPV6_SUPPORT)
    ecmd_ifndef(BOOTP_SUPPORT)
      ecmd_feature(ip, "ip ")
      ecmd_feature(netmask, "netmask ")
      ecmd_feature(gw, "gw ")
    ecmd_else()
      ecmd_ifdef(OPENVPN_SUPPORT)
        dnl OpenVPN-capable Ethersex with IPv4 enabled, configuring OpenVPN
	dnl outer-IP using BOOTP
        ecmd_feature(ip, "ip ")
      ecmd_endif()
    ecmd_endif()
  ecmd_endif()

  ecmd_ifdef(IPV6_STATIC_SUPPORT)
    ecmd_feature(ip, "ip ")
  ecmd_endif()

  ecmd_feature(reset, "reset")
  ecmd_feature(wdreset, "wdreset")
ecmd_endif()

ecmd_feature(bootloader, "bootloader")

ecmd_ifdef(PORTIO_SUPPORT)
  ecmd_feature(io_set_ddr, "io set ddr")
  ecmd_feature(io_get_ddr, "io get ddr")
  ecmd_feature(io_set_port, "io set port")
  ecmd_feature(io_get_port, "io get port")
  ecmd_feature(io_get_pin, "io get pin")
ecmd_endif()

ecmd_ifdef(PORTIO_SIMPLE_SUPPORT)
  ecmd_feature(io, "io ")
ecmd_endif()

ecmd_ifdef(NAMED_PIN_SUPPORT)
  ecmd_ifdef(PORTIO_SUPPORT)
    ecmd_feature(pin_get, "pin get")
    ecmd_feature(pin_set, "pin set")
    ecmd_feature(pin_toggle, "pin toggle")
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
  ecmd_feature(nslookup, "nslookup ")
  ecmd_feature(show_dns, "show dns")
  ecmd_ifndef(BOOTP_SUPPORT)
    ecmd_feature(dns, "dns ")
  ecmd_endif()
ecmd_endif()

ecmd_ifdef(CLOCK_SUPPORT)
  ecmd_feature(time, "time")
  ecmd_feature(date, "date")
ecmd_endif()

ecmd_ifdef(ADC_SUPPORT)
  ecmd_feature(adc_get, "adc get")
ecmd_endif()

ecmd_ifdef(DATAFLASH_SUPPORT)
  ecmd_feature(df_status, "df status")

  ecmd_feature(fs_format, "fs format")
  ecmd_feature(fs_list, "fs list")
  ecmd_feature(fs_mkfile, "fs mkfile ")
  ecmd_feature(fs_remove, "fs remove ")
ecmd_endif()

ecmd_ifndef(TEENSY_SUPPORT)
  ecmd_ifdef(RFM12_SUPPORT)
    ecmd_feature(rfm12_status, "rfm12 status")
    ecmd_feature(rfm12_setbaud, "rfm12 setbaud")
  ecmd_endif()
ecmd_endif()

ecmd_ifndef(TEENSY_SUPPORT)
  ecmd_feature(d, "d ")

  ecmd_ifdef(USART_SUPPORT)
    ecmd_feature(usart_baud, "usart baud")
  ecmd_endif()

  ecmd_ifdef(MODBUS_SUPPORT)
    ecmd_feature(modbus_recv, "mb recv ")
  ecmd_endif()
ecmd_endif()

ecmd_ifdef(HTTPD_AUTH_SUPPORT)
  ecmd_feature(http_passwd, "http passwd")
ecmd_endif()
