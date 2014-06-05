/*
mq135 0x01

copyright (c) Davide Gironi, 2012

Released under GPLv3.
Please refer to LICENSE file for licensing information.
*/
#include <stdint.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include "config.h"

#ifndef _MQ135_H_
#define _MQ135_H_

#ifdef MQ135_SUPPORT
extern long mq135_defaultro;
double mq135_ppm;
uint16_t mq135_adc;

//define sensor resistance at 100ppm of NH3 in the clean air
//calibrate your sensor to obtain precise value
//Ro = Rs * sqrt(MQ135_SCALINGFACTOR/ppm, MQ135_EXPONENT)
//   = Rs * exp( ln(MQ135_SCALINGFACTOR/ppm) / MQ135_EXPONENT )
//#define MQ135_DEFAULTPPM 100 //default ppm of NH3 for calibration
//#define MQ135_DEFAULTRO 108251 //default Ro for MQ135_DEFAULTPPM ppm of NH3

//the graphic at fig.2 of datasheet (sensitivity characteristics of the MQ-135)
//seems a power function y = a*x^b
//so ppm = a*(Rs/Ro)^b
//using power regression, you can obtain scaling factor (a), and exponent (b) !for a specific GAS!
//points: (1.5 , 10) (0.75 , 100) (0.59 , 200)
//#define MQ135_SCALINGFACTOR 37.58805473 //for NH3
//#define MQ135_EXPONENT -3.235365807 //for NH3

//define the Rs/Ro valid interval, MQ135 detect from 10ppm to 300ppm
//look at the datasheet and use the helper to define those values
//#define MQ135_MAXRSRO 1.505 //for NH3
//#define MQ135_MINRSRO 0.56 //for NH3

//#define MQ135_DEFAULTPPM 392 //default ppm of CO2 for calibration
//#define MQ135_DEFAULTRO 41763 //default Ro for MQ135_DEFAULTPPM ppm of CO2
//#define MQ135_DEFAULTRO 31930 //default Ro for MQ135_DEFAULTPPM ppm of CO2
#define MQ135_SCALINGFACTOR 116.6020682 //CO2 gas value
#define MQ135_EXPONENT -2.769034857 //CO2 gas value
#define MQ135_MAXRSRO 2.428 //for CO2
#define MQ135_MINRSRO 0.358 //for CO2
//#define MQ135_BALANCERESISTOR 22000 //for CO2


//functions
extern long mq135_getres(uint16_t adc);
extern long mq135_getro(long resvalue, double ppm);
extern double mq135_getppm(long resvalue, long ro);
void mq135_main(void);

#endif
#endif
