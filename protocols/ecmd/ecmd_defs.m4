dnl
dnl ecmd_defs.m4
dnl
dnl  You can declare your ecmd commands here.  The code will be expanded
dnl  by the make system (which in turn uses GNU m4) to build the
dnl  ecmd_defs.c file.
dnl
dnl  Lines starting with `dnl' are comments.
dnl

dnl block(Network configuration) dnl ==========================
ecmd_ifndef(TEENSY_SUPPORT)
  ecmd_ifdef(UIP_SUPPORT)
    ecmd_ifndef(IPV6_SUPPORT)
      ecmd_feature(ip, "ip",[IP],Display/Set the IP address.)
      ecmd_feature(netmask, "netmask",[IP],Display/Set the network mask.)
      ecmd_feature(gw, "gw",[IP],Display/Set the address of the default router.)
    ecmd_endif()

    ecmd_ifdef(IPV6_STATIC_SUPPORT)
      ecmd_feature(ip, "ip")
      ecmd_feature(gw, "gw")
    ecmd_endif()
  ecmd_endif()
ecmd_endif()

block(Resetting the controller) dnl ==========================
ecmd_ifndef(TEENSY_SUPPORT)
  ecmd_ifndef(DISABLE_REBOOT_SUPPORT)
    ecmd_feature(reset, "reset",,Reset the Ethersex.)
    ecmd_feature(wdreset, "wdreset",,Go into endless loop to trigger a watchdog timeout.)
  ecmd_endif()
ecmd_endif()

ecmd_ifndef(DISABLE_REBOOT_SUPPORT)
  ecmd_feature(bootloader, "bootloader",,Call the bootloader.)
ecmd_endif()

block(RFM12 ASK) dnl ==========================
ecmd_ifdef(RFM12_ASK_SENDER_SUPPORT)
  ecmd_ifdef(RFM12_ASK_TEVION_SUPPORT)
  ecmd_feature(rfm12_ask_tevion_send, "rfm12 tevion", , housecode command delay cnt)
  ecmd_endif()
  ecmd_ifdef(RFM12_ASK_2272_SUPPORT)
    ecmd_feature(rfm12_ask_2272_send, "rfm12 2272", , housecodeCommand delay cnt)
  ecmd_endif()
ecmd_endif()
ecmd_ifdef(RFM12_ASK_EXTERNAL_FILTER_SUPPORT)
  ecmd_feature(rfm12_ask_external_filter, "rfm12 external filter",[1], Enable ext. filter pin if argument is present (disable otherwise))
ecmd_endif()
ecmd_ifdef(RFM12_ASK_SENSING_SUPPORT)
  ecmd_feature(rfm12_ask_sense, "rfm12 ask sense",, Trigger (Tevion) ASK sensing.  Enable ext. filter pin before!)
ecmd_endif()

block(Sound/Melody support) dnl ==========================
ecmd_ifdef(PWM_MELODY_SUPPORT)
  ecmd_feature(pwm_melody_play, "pwm melody", , Play melody)
ecmd_endif

ecmd_ifdef(PWM_WAV_SUPPORT)
  ecmd_feature(pwm_wav_play, "pwm wav", , Play wav)
  ecmd_feature(pwm_wav_stop, "pwm stop", , Stop wav)
ecmd_endif

block(I2C (TWI)) dnl ==========================
ecmd_ifdef(I2C_DETECT_SUPPORT)
  ecmd_feature(i2c_detect, "i2c detect",,list detected I2C Chips)
ecmd_endif

ecmd_ifdef(I2C_LM75_SUPPORT)
  ecmd_feature(i2c_lm75, "lm75",ADDR, Get temperature)
ecmd_endif
ecmd_ifdef(I2C_PCA9531_SUPPORT)
  ecmd_feature(i2c_pca9531, "pca9531",ADDR PERIODPWM1 DUTYPWM1 PERIODPWM2 DUTYPWM2 LED0..3 LED4..7, set PWM1 and PWM2 and LED states)
ecmd_endif
ecmd_ifdef(I2C_PCF8574X_SUPPORT)
  ecmd_feature(i2c_pcf8574x_read, "pcf8574x read",ADDR CHIP, Get bits)
  ecmd_feature(i2c_pcf8574x_set, "pcf8574x set",ADDR CHIP HEXVALUE, Set bits)
ecmd_endif

block(MCUF) dnl ============================
ecmd_ifdef(MCUF_SUPPORT)
  ecmd_ifdef(MCUF_CLOCK_SUPPORT)
    ecmd_feature(mcuf_show_clock, "mcuf showclock",, Show digital clock)
  ecmd_endif
  ecmd_ifdef(MCUF_SCROLLTEXT_SUPPORT)
   ecmd_feature(mcuf_show_string, "mcuf showstring",MESSAGE, Show scrolling MESSAGE on the display)
  ecmd_endif
  ecmd_ifdef(MCUF_MODUL_SUPPORT)
   ecmd_feature(mcuf_modul_list, "mcuf modul list",, List all modules)
   ecmd_feature(mcuf_modul, "mcuf modul",N, Select module N)
  ecmd_endif
ecmd_endif

block(FS20) dnl ==========================
ecmd_ifdef(FS20_SUPPORT)
  ecmd_ifdef(FS20_SEND_SUPPORT)
    ecmd_feature(fs20_send, "fs20 send",HOUSECODE ADDR CMD, Send FS20 command. See [[FS20]] for details.)
  ecmd_endif()

  ecmd_ifdef(FS20_RECEIVE_SUPPORT)
    ecmd_feature(fs20_receive, "fs20 receive",, Receive FS20 sequence and display it.)
  ecmd_endif()

  ecmd_ifdef(FS20_RECEIVE_WS300_SUPPORT)
    ecmd_feature(fs20_ws300, "fs20 ws300",, Receive FS20 sequence from WS300 weather station and decode it.)
  ecmd_endif()
ecmd_endif()




block(Miscelleanous) dnl ============================
dnl   Put stuff that doesn't fit elsewhere here, i.e. this is a good place
dnl   for commands that don't fit in any category and would thus require to add
dnl   a category just for one specific command (which we don't want)

ecmd_ifndef(TEENSY_SUPPORT)
  ecmd_feature(d, "d ", ADDR, Dump the memory at ADDR (16 bytes).)
  ecmd_feature(help, "help",, List which commands are available.)

  ecmd_feature(version, "version",,Display the version number.)

dnl  ecmd_ifdef(USART_SUPPORT)
dnl    ecmd_feature(usart_baud, "usart baud", BAUD, Set the USART baudrate to BAUD.)
dnl  ecmd_endif()
ecmd_endif()

ecmd_ifdef(FREE_SUPPORT)
  ecmd_feature(free, "free",, Display free space.)
ecmd_endif()

dnl block(EEPROM) dnl ==========================
ecmd_ifdef(EEPROM_SUPPORT)
  ecmd_feature(eeprom_reinit, "eeprom reinit",, Force reinitialization of the EEPROM config area)
ecmd_endif
