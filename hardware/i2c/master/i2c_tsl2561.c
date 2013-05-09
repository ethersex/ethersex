#include <avr/io.h>
#include <util/twi.h>
#include <util/delay.h>

#include "autoconf.h"
#include "config.h"
#include "core/debug.h"
#include "i2c_master.h"
#include "i2c_tsl2561.h"

#ifdef I2C_TSL2561_SUPPORT

#define DEVID(n) (n==0?TSL2561_ADDR_LOW:(n==1?TSL2561_ADDR_FLOAT:TSL2561_ADDR_HIGH))

static uint8_t devstatus[3]; // maximum of 3 devices

#define DEVSTATUS_ON 128
#define DEVSTATUS_PACKAGE_CS 64

static uint8_t write8(uint8_t n,uint8_t reg,uint8_t val)
{
	uint8_t ret=0xff;
	if(!i2c_master_select(DEVID(n),TW_WRITE))
		goto end;
	TWDR=reg;
	if(i2c_master_transmit_with_ack()!=TW_MT_DATA_ACK)
		goto end;
	TWDR=val;
	if(i2c_master_transmit_with_ack()!=TW_MT_DATA_ACK)
		goto end;
	ret=0;
end:
	i2c_master_stop();
	return ret;
}

static int32_t read16(uint8_t n,uint8_t reg)
{
	if(!i2c_master_select(DEVID(n),TW_WRITE))
		goto end;
	TWDR=reg;
	if(i2c_master_transmit_with_ack() != TW_MT_DATA_ACK) goto end;
	i2c_master_stop();
	if(!i2c_master_select(DEVID(n),TW_READ))
		goto end;
	if(i2c_master_transmit_with_ack() != TW_MR_DATA_ACK) goto end;
	uint16_t t=TWDR;
	if(i2c_master_transmit() != TW_MR_DATA_NACK) goto end;
	t=t|(TWDR<<8);
	i2c_master_stop();
	return t;
end:
	i2c_master_stop();
	return -1;
	
}

/*
 * Read both channels of the specified device in the given mode
 */
uint8_t i2c_tsl2561_getluminosity(uint8_t devnum,uint16_t *ch0,uint16_t *ch1)
{
	if(!(devstatus[devnum]&DEVSTATUS_ON))
		return 1; // Device not active
	
	int32_t x;
	x=read16(devnum,TSL2561_COMMAND_BIT | TSL2561_WORD_BIT | TSL2561_REGISTER_CHAN1_LOW);
	if(x<0)
		return 1; // Ch1 failed
	*ch1=x;
	x=read16(devnum,TSL2561_COMMAND_BIT | TSL2561_WORD_BIT | TSL2561_REGISTER_CHAN0_LOW);
	if(x<0)
		return 2; // Ch0 failed
	*ch0=x;
	return 0;
}

//
// Calculation code is taken directly from datasheet
//
#define LUX_SCALE 14 // scale by 2^14
#define RATIO_SCALE 9 // scale ratio by 2^9
// Integration time scaling factors
#define CH_SCALE 10 // scale channel values by 2^10
#define CHSCALE_TINT0 0x7517 // 322/11 * 2^CH_SCALE
#define CHSCALE_TINT1 0x0fe7 // 322/81 * 2^CH_SCALE
// T Package coefficients
// For Ch1/Ch0=0.00 to 0.50
// Lux/Ch0=0.0304-0.062*((Ch1/Ch0)^1.4)
// piecewise approximation
// For Ch1/Ch0=0.00 to 0.125:
// Lux/Ch0=0.0304-0.0272*(Ch1/Ch0)
//
// For Ch1/Ch0=0.125 to 0.250:
// Lux/Ch0=0.0325-0.0440*(Ch1/Ch0)
//
// For Ch1/Ch0=0.250 to 0.375:
// Lux/Ch0=0.0351-0.0544*(Ch1/Ch0)
//
// For Ch1/Ch0=0.375 to 0.50:
// Lux/Ch0=0.0381-0.0624*(Ch1/Ch0)
//
// For Ch1/Ch0=0.50 to 0.61:
// Lux/Ch0=0.0224-0.031*(Ch1/Ch0)
//
// For Ch1/Ch0=0.61 to 0.80:
// Lux/Ch0=0.0128-0.0153*(Ch1/Ch0)
//
// For Ch1/Ch0=0.80 to 1.30:
// Lux/Ch0=0.00146-0.00112*(Ch1/Ch0)
//
// For Ch1/Ch0>1.3:
// Lux/Ch0=0
//---------------------------------------------------
#define K1T 0x0040 // 0.125 * 2^RATIO_SCALE
#define B1T 0x01f2 // 0.0304 * 2^LUX_SCALE
#define M1T 0x01be // 0.0272 * 2^LUX_SCALE
#define K2T 0x0080 // 0.250 * 2^RATIO_SCALE
#define B2T 0x0214 // 0.0325 * 2^LUX_SCALE
#define M2T 0x02d1 // 0.0440 * 2^LUX_SCALE
#define K3T 0x00c0 // 0.375 * 2^RATIO_SCALE
#define B3T 0x023f // 0.0351 * 2^LUX_SCALE
#define M3T 0x037b // 0.0544 * 2^LUX_SCALE
#define K4T 0x0100 // 0.50 * 2^RATIO_SCALE
#define B4T 0x0270 // 0.0381 * 2^LUX_SCALE
#define M4T 0x03fe // 0.0624 * 2^LUX_SCALE
#define K5T 0x0138 // 0.61 * 2^RATIO_SCALE
#define B5T 0x016f // 0.0224 * 2^LUX_SCALE
#define M5T 0x01fc // 0.0310 * 2^LUX_SCALE
#define K6T 0x019a // 0.80 * 2^RATIO_SCALE
#define B6T 0x00d2 // 0.0128 * 2^LUX_SCALE
#define M6T 0x00fb // 0.0153 * 2^LUX_SCALE
#define K7T 0x029a // 1.3 * 2^RATIO_SCALE
#define B7T 0x0018 // 0.00146 * 2^LUX_SCALE
#define M7T 0x0012 // 0.00112 * 2^LUX_SCALE
#define K8T 0x029a // 1.3 * 2^RATIO_SCALE
#define B8T 0x0000 // 0.000 * 2^LUX_SCALE
#define M8T 0x0000 // 0.000 * 2^LUX_SCALE
//---------------------------------------------------
// CS package coefficients
//---------------------------------------------------
// For 0 <= Ch1/Ch0 <= 0.52
// Lux/Ch0 = 0.0315-0.0593*((Ch1/Ch0)^1.4)
// piecewise approximation
// For 0 <= Ch1/Ch0 <= 0.13
// Lux/Ch0 = 0.0315-0.0262*(Ch1/Ch0)
// For 0.13 <= Ch1/Ch0 <= 0.26
// Lux/Ch0 = 0.0337-0.0430*(Ch1/Ch0)
// For 0.26 <= Ch1/Ch0 <= 0.39
// Lux/Ch0 = 0.0363-0.0529*(Ch1/Ch0)
// For 0.39 <= Ch1/Ch0 <= 0.52
// Lux/Ch0 = 0.0392-0.0605*(Ch1/Ch0)
// For 0.52 < Ch1/Ch0 <= 0.65
// Lux/Ch0 = 0.0229-0.0291*(Ch1/Ch0)
// For 0.65 < Ch1/Ch0 <= 0.80
// Lux/Ch0 = 0.00157-0.00180*(Ch1/Ch0)
// For 0.80 < Ch1/Ch0 <= 1.30
// Lux/Ch0 = 0.00338-0.00260*(Ch1/Ch0)
// For Ch1/Ch0 > 1.30
// Lux = 0
//---------------------------------------------------
#define K1C 0x0043 // 0.130 * 2^RATIO_SCALE
#define B1C 0x0204 // 0.0315 * 2^LUX_SCALE
#define M1C 0x01ad // 0.0262 * 2^LUX_SCALE
#define K2C 0x0085 // 0.260 * 2^RATIO_SCALE
#define B2C 0x0228 // 0.0337 * 2^LUX_SCALE
#define M2C 0x02c1 // 0.0430 * 2^LUX_SCALE
#define K3C 0x00c8 // 0.390 * 2^RATIO_SCALE
#define B3C 0x0253 // 0.0363 * 2^LUX_SCALE
#define M3C 0x0363 // 0.0529 * 2^LUX_SCALE
#define K4C 0x010a // 0.520 * 2^RATIO_SCALE
#define B4C 0x0282 // 0.0392 * 2^LUX_SCALE
#define M4C 0x03df // 0.0605 * 2^LUX_SCALE
#define K5C 0x014d // 0.65 * 2^RATIO_SCALE
#define B5C 0x0177 // 0.0229 * 2^LUX_SCALE
#define M5C 0x01dd // 0.0291 * 2^LUX_SCALE
#define K6C 0x019a // 0.80 * 2^RATIO_SCALE
#define B6C 0x0101 // 0.0157 * 2^LUX_SCALE
#define M6C 0x0127 // 0.0180 * 2^LUX_SCALE
#define K7C 0x029a // 1.3 * 2^RATIO_SCALE
#define B7C 0x0037 // 0.00338 * 2^LUX_SCALE
#define M7C 0x002b // 0.00260 * 2^LUX_SCALE
#define K8C 0x029a // 1.3 * 2^RATIO_SCALE
#define B8C 0x0000 // 0.000 * 2^LUX_SCALE
#define M8C 0x0000 // 0.000 * 2^LUX_SCALE
// lux equation approximation without floating point calculations
//////////////////////////////////////////////////////////////////////////////
// Routine: unsigned int CalculateLux(unsigned int ch0, unsigned int ch0, int iType)
//
// Description: Calculate the approximate illuminance (lux) given the raw
// channel values of the TSL2560. The equation is implemented
// as a piece-wise linear approximation.
//
// Arguments: unsigned int iGain - gain, where 0:1X, 1:16X
// unsigned int tInt - integration time, where 0:13.7mS, 1:100mS, 2:402mS,
// 3:Manual
// unsigned int ch0 - raw channel value from channel 0 of TSL2560
// unsigned int ch1 - raw channel value from channel 1 of TSL2560
// unsigned int iType - package type (T or CS)
//
// Return: unsigned int - the approximate illuminance (lux)
//
//////////////////////////////////////////////////////////////////////////////
static unsigned long CalculateLux(unsigned int iGain, unsigned int tInt, unsigned int ch0,unsigned int ch1, int iType)
{
	//------------------------------------------------------------------------
	// first, scale the channel values depending on the gain and integration time
	// 16X, 402mS is nominal.
	// scale if integration time is NOT 402 msec
	unsigned long chScale;
	unsigned long channel1;
	unsigned long channel0;
	switch(tInt)
	{
		case 0: // 13.7 msec
			chScale = CHSCALE_TINT0;
			break;
		case 1: // 101 msec
			chScale = CHSCALE_TINT1;
			break;
		default: // assume no scaling
			chScale = (1 << CH_SCALE);
			break;
	}
	// scale if gain is NOT 16X
	if(!iGain) chScale = chScale << 4; // scale 1X to 16X
	// scale the channel values
	channel0 = (ch0 * chScale) >> CH_SCALE;
	channel1 = (ch1 * chScale) >> CH_SCALE;
	//------------------------------------------------------------------------
	// find the ratio of the channel values (Channel1/Channel0)
	// protect against divide by zero
	unsigned long ratio1 = 0;
	if(channel0) 
		ratio1 = (channel1 << (RATIO_SCALE+1)) / channel0;
	// round the ratio value
	unsigned long ratio = (ratio1 + 1) >> 1;
	// is ratio <= eachBreak ?
	unsigned int b, m;
	switch (iType)
	{
		case 0: // T package
		if ((ratio >= 0) && (ratio <= K1T))
			{b=B1T; m=M1T;}
		else if (ratio <= K2T)
			{b=B2T; m=M2T;}
		else if (ratio <= K3T)
			{b=B3T; m=M3T;}
		else if (ratio <= K4T)
			{b=B4T; m=M4T;}
		else if (ratio <= K5T)
			{b=B5T; m=M5T;}
		else if (ratio <= K6T)
			{b=B6T; m=M6T;}
		else if (ratio <= K7T)
			{b=B7T; m=M7T;}
		else if (ratio > K8T)
			{b=B8T; m=M8T;}
		break;
		case 1:// CS package
		if ((ratio >= 0) && (ratio <= K1C))
			{b=B1C; m=M1C;}
		else if (ratio <= K2C)
			{b=B2C; m=M2C;}
		else if (ratio <= K3C)
			{b=B3C; m=M3C;}
		else if (ratio <= K4C)
			{b=B4C; m=M4C;}
		else if (ratio <= K5C)
			{b=B5C; m=M5C;}
		else if (ratio <= K6C)
			{b=B6C; m=M6C;}
		else if (ratio <= K7C)
			{b=B7C; m=M7C;}
		else if (ratio > K8C)
			{b=B8C; m=M8C;}
		break;
	}
	unsigned long temp;
	temp = ((channel0 * b) - (channel1 * m));
	// do not allow negative lux value
	//if (temp < 0) temp = 0;
	// round lsb (2^(LUX_SCALE-1))
	temp += (1 << (LUX_SCALE-1));
	// strip off fractional portion
	unsigned long lux = temp >> LUX_SCALE;
	return(lux);
}

int32_t i2c_tsl2561_getlux(uint8_t devnum)
{
  uint16_t ch0, ch1;

  if(i2c_tsl2561_getluminosity(devnum,&ch0,&ch1))
  	  return -1;
  if(ch1>ch0 || ch0==0xffff || ch1==0xffff)
  {
  	// Clipping
  	return -2;
  }
  return CalculateLux(
  	  	devstatus[devnum]&TSL2561_GAIN_16X,
  	  	devstatus[devnum]&TSL2561_INTEGRATIONTIME_MASK,
  	  	ch0,ch1,
  	  	(devstatus[devnum]&DEVSTATUS_PACKAGE_CS)?1:0
  	  );
}

uint8_t i2c_tsl2561_setmode(uint8_t devnum,uint8_t integration_time,uint8_t gain,uint8_t packagetype)
{
	uint8_t mode=integration_time&TSL2561_INTEGRATIONTIME_MASK;
	if(gain)
		mode|=TSL2561_GAIN_16X;
	// Set parameters
	if(write8(devnum,TSL2561_COMMAND_BIT | TSL2561_REGISTER_TIMING, mode))
		return 1; // Error
	// Enable device
	if(write8(devnum,TSL2561_COMMAND_BIT | TSL2561_REGISTER_CONTROL, TSL2561_CONTROL_POWERON))
		return 2; // Error
	// Remember device mode for calculations
	devstatus[devnum]=mode | DEVSTATUS_ON | (packagetype?DEVSTATUS_PACKAGE_CS:0);
	return 0;
}

uint8_t i2c_tsl2561_setpower(uint8_t devnum,uint8_t power)
{
	if(write8(devnum,TSL2561_COMMAND_BIT | TSL2561_REGISTER_CONTROL, power ? TSL2561_CONTROL_POWERON : TSL2561_CONTROL_POWEROFF))
		return 2; // Error
	if(power)
		devstatus[devnum]|=DEVSTATUS_ON;
	else
		devstatus[devnum]&=~DEVSTATUS_ON;
	return 0;
}

/*
 -- Ethersex META --
 header(hardware/i2c/master/i2c_tsl2561.h)
 */

#endif /* I2C_TSL2561_SUPPORT */
