#include "mcuf/mcuf_modul.h"
#ifdef MCUF_MODUL_BORG16_XONI_STUDY_SUPPORT


void draw_queue(uint8_t x, uint8_t y, uint8_t on);

void xoni_study1(void) {
	uint8_t i, x,y,z=1;
	for(i=0;i<6;i++) {
		for(y=0; y<16; y++) {
			for(x=0; x<16; x++){
				draw_queue((z?15-x:x),y, 1);
				WAIT(42);
				draw_queue((z?15-x:x),y, 0);
			}
			z ^= 1;
		}
		for(y=15; y; y--) {
			setpixel((pixel){y,y},3);
			WAIT(42);
			setpixel((pixel){y-1,y-1},3);
			WAIT(42);
			setpixel((pixel){y,y},0);			
		}
	}
}

void draw_queue(uint8_t x, uint8_t y, uint8_t on) {
	setpixel((pixel){x,y},on?3:0);
	
	if (y % 2) {
		// wir laufen nach links
		if (x>1) {
			//easy
			setpixel((pixel){x-1,y},on?2:0);
			setpixel((pixel){x-2,y},on?1:0);
		} else if (x == 1) {
			//Rand!!
			setpixel((pixel){0,y},on?2:0);
			setpixel((pixel){0,y-1},on?1:0);			
		} else /*if (x == 0)*/ {
			setpixel((pixel){0,y-1},on?2:0);
			setpixel((pixel){1,y-1},on?1:0);
		}
	} else {
		// wir laufen nach rechts
		if (x<14) {
			//easy
			setpixel((pixel){x+1,y},on?2:0);
			setpixel((pixel){x+2,y},on?1:0);
		} else if (x == 14) {
			//Rand!!
			setpixel((pixel){15,y},on?2:0);
			setpixel((pixel){15,y-1},on?1:0);			
		} else {
			setpixel((pixel){15,y-1},on?2:0);
			setpixel((pixel){14,y-1},on?1:0);
		}
	}
}
#endif  //MCUF_MODUL_BORG16_XONI_STUDY_SUPPORT
