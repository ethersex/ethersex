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


 block(Stella commands) dnl ==========================
ecmd_ifdef(STELLA_SUPPORT)
  ecmd_feature(stella_eeprom_store, "stella store",, Store values in eeprom)
  ecmd_feature(stella_eeprom_load, "stella load",, Load values from eeprom)
  ecmd_ifdef(CRON_SUPPORT)
  ecmd_ifdef(ECMD_SPEED_SUPPORT)
  ecmd_feature(stella_cron, "stella cron", CHANNEL VALUE MINUTE HOUR DAY MONTH DAYOFWEEK, Add a cronjob for stella to set the CHANNEL to VALUE. You may use fade channel constants.)
  ecmd_endif()
  ecmd_endif()
  ecmd_feature(stella_channels, "channels",, Return stella channel size)
  ecmd_ifdef(STELLA_MOODLIGHT)
  ecmd_feature(stella_moodlight_get, "moodlight get",CHANNEL, Return on if channel CHANNEL is moodlighted)
  ecmd_feature(stella_moodlight_set, "moodlight set",CHANNEL ONOFF, Set CHANNEL moodlight on=1 or off=0)
  ecmd_endif()
  ecmd_feature(stella_fadestep_set, "stella fadestep set", FADESTEP, Set stella fade step)
  ecmd_feature(stella_fadestep_get, "stella fadestep get",, Get stella fade step)
  ecmd_feature(stella_channel_set, "channel set", CHANNEL VALUE FUNCTION,Set stella channel to value. The third parameter is optional. You may use 's' for instant set, 'f' for fade and 'y' for flashy fade. )
  ecmd_feature(stella_channel_get, "channel get", CHANNEL, Get stella channel value)
ecmd_endif()

block(Cron commands (dynamic variant)) dnl ==========================
ecmd_ifdef(CRON_SUPPORT)
  ecmd_feature(cron_list, "cron_list",, Show all cron entries)
  ecmd_feature(cron_rm, "cron_rm", POSITION, Remove one cron entry)
  ecmd_feature(cron_add, "cron_add", MIN HOUR DAY MONTH DOW ECMD, Add ECMD to cron to be executed at given time)
ecmd_endif()

block(Control 6) dnl ==========================
ecmd_ifdef(CONTROL6_SUPPORT)
  ecmd_ifdef(C6_ECMD_USED)
    ecmd_feature(c6_get, "c6 get ", VARNAME, Display the current value of the ECMD_GLOBAL Variable)
    ecmd_feature(c6_set, "c6 set ", VARNAME VALUE, Set an new value on the ECMD_GLOBAL Variable)
  ecmd_endif()
ecmd_endif()

block(Dallas 1-wire) dnl ==========================
ecmd_ifdef(ONEWIRE_SUPPORT)
  ecmd_ifdef(ONEWIRE_DETECT_SUPPORT)
    ecmd_feature(onewire_list, "1w list",,Return a list of the connected onewire devices)
  ecmd_endif()
  ecmd_feature(onewire_get, "1w get", DEVICE, Return temperature value of onewire DEVICE (provide 64-bit ID as 16-hex-digits))
  ecmd_feature(onewire_convert, "1w convert", [DEVICE], Trigger temperature conversion of either DEVICE or all connected devices)
ecmd_endif()

block(KTY) dnl ==========================
ecmd_ifdef(KTY_SUPPORT)
  ecmd_feature(kty_get, "kty get", [CHANNEL], Get the temperature in xxx.x °C of CHANNEL or if no channel set of all channels.)
  ecmd_feature(kty_cal_get, "kty cal get",, Return the calibration difference to 2k2 Resistor.)
  ecmd_feature(kty_calibration, "kty autocalibrate", CHANNEL, Calibrate to 1000 Ohm precision Resistor.)
ecmd_endif()

block(DataFlash) dnl ==========================
ecmd_ifdef(DATAFLASH_SUPPORT)
  ecmd_feature(df_status, "df status",, Display internal status.)

  ecmd_feature(fs_format, "fs format",, Format the filesystem.)
  ecmd_feature(fs_list, "fs list",, List the directory.)
  ecmd_feature(fs_mkfile, "fs mkfile ", NAME, Create a new file NAME.)
  ecmd_feature(fs_remove, "fs remove ", NAME, Delete the file NAME.)
  ecmd_feature(fs_truncate, "fs truncate ", NAME LEN, Truncate the file NAME to LEN bytes.)
  ecmd_ifdef(DEBUG_FS)
    ecmd_feature(fs_inspect_node, "fs inspect node ", NODE, Inspect NODE and dump to serial.)
    ecmd_feature(fs_inspect_inode, "fs inspect inode ", INODE, Inspect INODE (and associated page).)
  ecmd_endif()
ecmd_endif()

block(SD/MMC Card Reader) dnl ==========================
ecmd_ifdef(SD_READER_SUPPORT)
  ecmd_feature(sd_dir, "sd dir",, List contents of current SD directory.)
  ecmd_feature(sd_mkdir, "sd mkdir",PATH, Create directory hierarchy PATH.)
ecmd_endif

block(RFM12) dnl ==========================
ecmd_ifndef(TEENSY_SUPPORT)
  ecmd_ifdef(RFM12_SUPPORT)
    ecmd_feature(rfm12_status, "rfm12 status",, Display internal status.)
  ecmd_endif()
  ecmd_ifdef(RFM12_IP_SUPPORT)
    ecmd_feature(rfm12_setbaud, "rfm12 setbaud", BAUD, Set baudrate to BAUD.)
    ecmd_feature(rfm12_setbandwidth, "rfm12 setbandwidth", BW, Set RX bandwidth to BW.)
    ecmd_feature(rfm12_setmod, "rfm12 setmod", MOD, Set modulation to MOD.)
    ecmd_feature(rfm12_setgain, "rfm12 setgain", GAIN, Set preamplifier gain to GAIN.)
    ecmd_feature(rfm12_setdrssi, "rfm12 setdrssi", DRSSI, Set the drssi to DRSSI.)
  ecmd_endif()
ecmd_endif()

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

block(DC3840 Handycam support) dnl ==========================
ecmd_ifdef(DC3840_SUPPORT)
  ecmd_feature(dc3840_capture, "dc3840 capture",, Take a picture.  Access 'dc3840' via VFS afterwards.  See [[DC3840 Camera]] for details.)
  ecmd_feature(dc3840_send, "dc3840 send ", A B C D E, Send provided command bytes to the camera.)
  ecmd_feature(dc3840_sync, "dc3840 sync",, Re-sync to the camera)
ecmd_endif

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

dnl block(Infrared) dnl ==========================
ecmd_ifdef(RC5_SUPPORT)
  ecmd_feature(ir_send, "ir send")
  ecmd_feature(ir_receive, "ir receive")
ecmd_endif()
