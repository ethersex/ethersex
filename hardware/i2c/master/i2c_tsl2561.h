#ifndef _I2C_TSL2561_H
#define _I2C_TSL2561_H

#define TSL2561_ADDR_LOW 0x29
#define TSL2561_ADDR_FLOAT 0x39
#define TSL2561_ADDR_HIGH 0x49

#define TSL2561_READBIT (0x01)

#define TSL2561_COMMAND_BIT (0x80) // Must be 1
#define TSL2561_CLEAR_BIT (0x40) // Clears any pending interrupt (write 1 to clear)
#define TSL2561_WORD_BIT (0x20) // 1 = read/write word (rather than byte)
#define TSL2561_BLOCK_BIT (0x10) // 1 = using block read/write

#define TSL2561_CONTROL_POWERON (0x03)
#define TSL2561_CONTROL_POWEROFF (0x00)

enum
{
  TSL2561_REGISTER_CONTROL = 0x00,
  TSL2561_REGISTER_TIMING = 0x01,
  TSL2561_REGISTER_THRESHHOLDL_LOW = 0x02,
  TSL2561_REGISTER_THRESHHOLDL_HIGH = 0x03,
  TSL2561_REGISTER_THRESHHOLDH_LOW = 0x04,
  TSL2561_REGISTER_THRESHHOLDH_HIGH = 0x05,
  TSL2561_REGISTER_INTERRUPT = 0x06,
  TSL2561_REGISTER_CRC = 0x08,
  TSL2561_REGISTER_ID = 0x0A,
  TSL2561_REGISTER_CHAN0_LOW = 0x0C,
  TSL2561_REGISTER_CHAN0_HIGH = 0x0D,
  TSL2561_REGISTER_CHAN1_LOW = 0x0E,
  TSL2561_REGISTER_CHAN1_HIGH = 0x0F
};

enum
{
  TSL2561_INTEGRATIONTIME_13MS = 0x00, // 13.7ms
  TSL2561_INTEGRATIONTIME_101MS = 0x01, // 101ms
  TSL2561_INTEGRATIONTIME_402MS = 0x02, // 402ms
  
  TSL2561_INTEGRATIONTIME_MASK
};

enum
{
  TSL2561_PACKAGE_T,
  TSL2561_PACKAGE_CS
};
  
enum
{
  TSL2561_GAIN_1X = 0x00, // Low gain (1x)
  TSL2561_GAIN_16X = 0x10, // High gain (16x)
};

int32_t i2c_tsl2561_getlux(uint8_t devnum);
uint8_t i2c_tsl2561_getluminosity(uint8_t devnum,uint16_t *ch0,uint16_t *ch1);
uint8_t i2c_tsl2561_setmode(uint8_t devnum,uint8_t integration_time,uint8_t gain,uint8_t packagetype);
uint8_t i2c_tsl2561_setpower(uint8_t devnum,uint8_t power);

#endif /* _I2C_TSL2550_H */
