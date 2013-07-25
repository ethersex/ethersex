alias_ifdef(UIP_SUPPORT)
  alias("i", "ip")
  alias("if", "mac")
alias_endif()

alias_ifdef(RFM12_ASK_SENDER_SUPPORT)
  alias("off", `"rfm12 2272 0,5,80 76 10"')
  alias("on", `"rfm12 2272 0,5,85 76 10"')
alias_endif()

alias_ifdef(I2C_PCF8574X_SUPPORT)
  /* example for further parameter right behind '0 0 ' */
  alias("led", `"pcf8574x set 0 0 "')
alias_endif()

alias_ifdef(I2C_LM75_SUPPORT)
  alias("temp", 		"lm75 0")
alias_endif()

alias_ifdef(I2C_PCA9531_SUPPORT)
  alias("servo center","pca9531 0 20 80 2 ea ae ee")
  alias("servo left", 	"pca9531 0 20 80 2 e0 ae ee")
  alias("servo right",	"pca9531 0 20 80 2 f5 ae ee")
alias_endif()

alias_ifdef(DATAFLASH_SUPPORT)
  alias("ls df", 			"fs list")
alias_endif()

alias_ifdef(SD_READER_SUPPORT)
  alias("ls sd", 			"sd dir")
alias_endif()

alias_ifdef(MCUF_MODUL_SUPPORT)
  alias("mplay", 	"mcuf modul")
  alias("mlist",	"mcuf modul list")
alias_endif()
