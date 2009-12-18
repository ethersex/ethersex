#ifndef _VS1053_H_
#define _VS1053_H_

// SCI_MODE defines for the VS1053
#define SM_DIFF 0 // Differential
#define SM_LAYER12 1 // Allow MPEG layers I&II
#define SM_RESET 2 // Soft reset
#define SM_CANCEL 3 // Cancel decoding current file
#define SM_EARSPEAKER_LO 4 // EarSpeaker low setting
#define SM_TESTS 5 // Allow SDI tests
#define SM_STREAM 6 // Stream mode
#define SM_EARSPEAKER_HI 7 // EarSpeaker high setting
#define SM_DACT 8 // DCLK active edge
#define SM_SDIORD 9 // SDI bit order
#define SM_SDISHARE 10 // Share SDI chip select
#define SM_SDINEW 11 // VS1002 native SDI modes
#define SM_ADPCM 12 // ADPCM recording active
#define SM_LINE1 14 // MIC/LINE1 selector
#define SM_CLK_RANGE 15 // Input clock range

int sci_read(char addr);		// Read
void sci_write(char addr, int data);	// Write
void vs1053_sinetest(char pitch);	// Sinewave

void cs_high(void);			// Set CS high
void cs_low(void);			// Set CS low
void cs_init(void);			// Initialize CS

#endif // _VS1053_H_
