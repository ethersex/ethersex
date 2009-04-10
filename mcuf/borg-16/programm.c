#include "mcuf/mcuf_modul.h"

#define RANDOM8() (random8())

#ifdef MCUF_MODUL_BORG16_TEST1_SUPPORT
void test1(){
	unsigned char x,y;
	for (y=0;y<NUM_ROWS;y++){
		for (x=0;x<NUM_COLS;x++){
			setpixel((pixel){x,y}, 3);
			wait(100);
		}
	}
}
#endif //MCUF_MODUL_BORG16_TEST1_SUPPORT

#ifdef MCUF_MODUL_BORG16_TESTL1_SUPPORT
void test_level1(){
	unsigned char x,y;
	for (y=0;y<NUM_ROWS;y++){
		for (x=0;x<NUM_COLS;x++){
			setpixel((pixel){x,y}, 1);
			wait(5);
		}
	}
	for(;;) wait(100);
}
#endif //MCUF_MODUL_BORG16_TESTL1_SUPPORT
#ifdef MCUF_MODUL_BORG16_TESTL2_SUPPORT
void test_level2(){
	unsigned char x,y;
	for (y=0;y<NUM_ROWS;y++){
		for (x=0;x<NUM_COLS;x++){
			setpixel((pixel){x,y}, 2);
			wait(5);
		}
	}
	for(;;) wait(100);
}
#endif //MCUF_MODUL_BORG16_TESTL2_SUPPORT
#ifdef MCUF_MODUL_BORG16_TESTL3_SUPPORT
void test_level3(){
	unsigned char x,y;
	for (y=0;y<NUM_ROWS;y++){
		for (x=0;x<NUM_COLS;x++){
			setpixel((pixel){x,y}, 3);
			wait(5);
		}
	}
	for(;;) wait(100);
}
#endif //MCUF_MODUL_BORG16_TESTL3_SUPPORT
#ifdef MCUF_MODUL_BORG16_TESTLX_SUPPORT

void test_levels(){
	unsigned char x,y,b;
	for(b=1;b<4;b++){
		for (y=0;y<NUM_ROWS;y++){
			for (x=0;x<NUM_COLS;x++){
				setpixel((pixel){x,y}, b);
				wait(5);
			}
		}
	}
}
#endif //MCUF_MODUL_BORG16_TESTLX_SUPPORT
#ifdef MCUF_MODUL_BORG16_PALETTE_SUPPORT
void test_palette(){
	unsigned char x,y,b;
	for (y=0;y<NUM_ROWS;y++){
		b=y%4;
		for (x=0;x<NUM_COLS;x++){
			setpixel((pixel){x,y}, b);
			wait(1);
		}
	}
	for(;;) wait(100);
}
#endif //MCUF_MODUL_BORG16_PALETTE_SUPPORT
#ifdef MCUF_MODUL_BORG16_SUPPORT
void off()
{
	clear_screen(0);

	while(1)
		wait(100);

}
#endif //MCUF_MODUL_BORG16_SUPPORT

#ifdef MCUF_MODUL_BORG16_SPIRAL_SUPPORT
void spirale(unsigned int delay){
	clear_screen(0);

	cursor cur;
	cur.dir = right;
	cur.mode = set;
	set_cursor (&cur, (pixel){NUM_COLS-1,0});
	
	unsigned char clearbit=0;
	while(clearbit == 0){
	
		clearbit = 1;
		while (!get_next_pixel(cur.pos, cur.dir)){
			clearbit = 0;
			walk(&cur, 1, delay);
		}
		cur.dir = direction_r(cur.dir);
	}

	cur.mode = clear;
	set_cursor(&cur, (pixel){(NUM_COLS/2)-1,(NUM_ROWS/2)-1});
	
	for(clearbit=0;clearbit==0;){
		if( get_next_pixel(cur.pos, direction_r(cur.dir)) ){
			cur.dir = direction_r(cur.dir);
		}
		if( get_next_pixel(cur.pos, cur.dir) == 1 ){
			walk(&cur , 1, delay);
		}else{
			clearbit = 1;
		}
	}

}
#endif //MCUF_MODUL_BORG16_SPIRAL_SUPPORT

#ifdef MCUF_MODUL_BORG16_JOERN1_SUPPORT
void joern1(){

unsigned char i, j, x;
	unsigned char rolr=0x01 , rol;
	clear_screen(3);
	for(i = 0; i< 80;i++){
		rol = rolr;
		for(j = 0 ;j < NUM_ROWS; j++){
			for(x=0;x<LINEBYTES;x++)
				setpixel((pixel){j,x}, rol);
			if((rol<<=1)==0)rol = 0x01;
		}
		if((rolr<<=1) == 0) rolr = 1;
		wait(100);
	}

}
#endif //MCUF_MODUL_BORG16_JOERN1_SUPPORT

#ifdef MCUF_MODUL_BORG16_SCHACHBRETT_SUPPORT
void schachbrett(unsigned char times){

	clear_screen(0);
	unsigned char j;
	for(j=0;j<times;j++){
		unsigned char i, x;
		for(i = 0; i<NUM_ROWS; i++){
			for(x=0;x<LINEBYTES;x++)
				setpixel((pixel){i,x},0x55<<(i&0x01));
		}
		wait(200);
		for(i = 0; i<NUM_ROWS; i++){
			for(x=0;x<LINEBYTES;x++)
				setpixel((pixel){i,x}, 0xAA>>(i&0x01));
		}
		wait(200);
	}

}

#endif //MCUF_MODUL_BORG16_SCHACHBRETT_SUPPORT
#ifdef MCUF_MODUL_BORG16_FADEIN_SUPPORT
void fadein()
{
	unsigned char value, x, y;

	for(value=1; value < 4; value++) {
	 	for(y=0; y<NUM_ROWS; y++)
			for(x=0; x<NUM_COLS; x++) {
				setpixel( (pixel){x,y}, value );
				wait(10);
			}
	}

}
#endif //MCUF_MODUL_BORG16_FADEIN_SUPPORT

#ifdef MCUF_MODUL_BORG16_FIRE_SUPPORT
#define FEUER_Y (NUM_ROWS + 3)

void feuer()
{
	unsigned char y, x;
	unsigned int  t;
	unsigned char world[NUM_COLS][FEUER_Y];   // double buffer
	

	for(t=0; t<800; t++) {
		// diffuse
		for(y=1; y<FEUER_Y; y++) {
			for(x=1; x<NUM_COLS-1; x++) {
				world[x][y-1] = (FEUER_N*world[x-1][y] + FEUER_S*world[x][y] + FEUER_N*world[x+1][y]) / FEUER_DIV;
			};

			world[0][y-1] = (FEUER_N*world[NUM_COLS-1][y] + FEUER_S*world[0][y] + FEUER_N*world[1][y]) / FEUER_DIV;
			world[NUM_COLS-1][y-1] = (FEUER_N*world[0][y] + FEUER_S*world[NUM_COLS-1][y] + FEUER_N*world[NUM_COLS-2][y]) / FEUER_DIV;
		};

		// update lowest line
		for(x=0; x<NUM_COLS; x++) {
			world[x][FEUER_Y-1] = RANDOM8();
		};
	
		// copy to screen
		for(y=0; y<NUM_ROWS; y++) {
			for(x=0; x<NUM_COLS; x++) {
				setpixel( (pixel){x,y}, (world[x][y] >> 5) );
			}		
		};

		wait(FEUER_DELAY);
	}
}

#endif //MCUF_MODUL_BORG16_FIRE_SUPPORT

#ifdef MCUF_MODUL_BORG16_TETRIS_SUPPORT
void tetris(){
//	PORTB = 0x0f;
	clear_screen(0);
	unsigned char i;
	for (i=0;i<NUM_ROWS;i++){
		setpixel((pixel){8,i},3);
	}
	struct block{
		pixel pix[4];
	};	
	struct block press (struct block foobar,unsigned char taste){
			unsigned char go;
			struct block aktuell=foobar;
			if(taste == 14){
			
			}
			if(taste == 7){
				for(i=0;i<4;i++){
					if((get_pixel((pixel){aktuell.pix[i].x+1,aktuell.pix[i].y}))){
						go=0;
						break;
					}
					else {
						go=1;
					}
				}
				if(go){
					for(i=0;i<4;i++){
						aktuell.pix[i].x++;
					}
				}
			}
			if(taste == 11){
				for(i=0;i<4;i++){
					if(aktuell.pix[i].x==0){
						go=0;
						break;
					}
					else {
						go=1;
					}
				}
				if(go){
					for(i=0;i<4;i++){
						aktuell.pix[i].x--;
					}
				}
			}
		
		return aktuell;
	}
	struct block line = {{{0,0},{0,1},{0,2},{0,3}}};
	struct block square={{{0,0},{0,1},{1,0},{1,1}}};	
	struct block left={{{1,0},{1,1},{0,1},{0,2}}};
	void dropin(struct block foobar){
		struct block aktuell = foobar;
			unsigned char i;
			unsigned char h=1;
			unsigned char taste = PINB & 0x0F;
			aktuell = press(aktuell,taste);
			struct block next=aktuell;
			for(i=0;i<4;i++){
				setpixel(aktuell.pix[i],3);
			}
			wait(450);
			for(i=0;i<4;i++){
				clearpixel(aktuell.pix[i]);
			}
			for(i=0;i<4;i++){
				next.pix[i].y++;
			}
			for(i=0;i<4;i++){
				h = get_pixel(next.pix[i]);
				if(h){break;}
			}
			if (!h){
	  				  dropin(next);
			}else{
				for(i=0;i<4;i++){
					setpixel(aktuell.pix[i],3);
				}
			}
	}
	dropin(square);	
	dropin(line);
	dropin(left);
	dropin(square);	
	dropin(square);	
	dropin(line);
	dropin(left);
	dropin(square);	
	dropin(line);
	dropin(left);
	dropin(line);
	dropin(left);
}

#endif //MCUF_MODUL_BORG16_TETRIS_SUPPORT
#ifdef MCUF_MODUL_BORG16_RANDOM_BRIGHT_SUPPORT
/**
 * void random_bright(void)
 *  by Daniel Otte
 * 
 * 
 */
void random_bright(unsigned cycles){
	uint8_t t,x,y;
	while(cycles--){
		for(y=0; y<NUM_ROWS; ++y)
			for(x=0; x<NUM_COLS/4; ++x){
				t=random8();
				setpixel((pixel){x*4+0, y}, 0x3&(t>>0));
				setpixel((pixel){x*4+1, y}, 0x3&(t>>2));
				setpixel((pixel){x*4+2, y}, 0x3&(t>>4));
				setpixel((pixel){x*4+3, y}, 0x3&(t>>6));
			}
		wait(200);
	}
}

#endif //MCUF_MODUL_BORG16_RANDOM_BRIGHT_SUPPORT
