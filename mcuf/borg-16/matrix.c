#ifdef MCUF_MODUL_BORG16_MATRIX_SUPPORT

#include "../mcuf_modul.h"

typedef struct{
	pixel start;
	unsigned char len;
	unsigned char decay;
	unsigned char index;
	unsigned char speed;
} streamer;

typedef uint8_t pixel_matrix_t[NUM_COLS][NUM_ROWS/4];

inline static uint8_t get_bright(pixel_matrix_t *matrix, uint8_t x, uint8_t y){
	uint8_t ret;
	ret = (*matrix)[x][y/4];
	return 0x3&(ret>>(2*(y%4)));
}

inline static void set_bright(pixel_matrix_t *matrix, uint8_t x, uint8_t y, uint8_t value){
	uint8_t t;
	t = (*matrix)[x][y/4];
	t &= ~(0x3<<(2*(y%4)));
	t |= value<<(2*(y%4));
	(*matrix)[x][y/4] = t;
}

void matrix() {
	unsigned int counter = 500; /* run 500 cycles */
	streamer streamers[STREAMER_NUM];
	pixel_matrix_t matrix_bright;
	unsigned char x, y;
	unsigned char index = 0;
	unsigned char draw;
	unsigned char streamer_num = 0;

	while(counter--){
		unsigned char i, j;
		/* initialise matrix-buffer */
		for(x=0;x<NUM_COLS;x++)
			for(y=0;y<NUM_ROWS/4;y++)
				matrix_bright[x][y]=0;
		
		for(i=0;i<streamer_num;i++){
			streamer str = streamers[i];
			
			unsigned char bright = 0xFF; draw = 0;
			for(j=(str.len/8);j!=0xFF;j--){ /* Draw streamer */
				if(j+str.start.y<NUM_ROWS){
					if(bright>>6) /* bright>>6 */
						draw = 1;
					if(bright > (get_bright(&matrix_bright, str.start.x, str.start.y+j)<<6) ){
						set_bright(&matrix_bright, str.start.x, str.start.y+j, bright>>6);
					
					}
				}
				bright-=((bright>>5)*str.decay);
			}
			
			str.len+=str.speed/2;
			streamers[i] = str;
			if(!draw){
				for(j=i;j<streamer_num-1;j++){
					streamers[j] = streamers[j+1];
				}
				streamer_num--;
				i--;
			}						
		}
		
		for(y=0;y<NUM_ROWS;y++)
			for(x=0;x<NUM_COLS;x++){
				setpixel((pixel){x,y}, get_bright(&matrix_bright,x,y));
			}
					
		unsigned char nsc;
		for(nsc=0;nsc<6;nsc++){
			if(streamer_num<STREAMER_NUM){
				unsigned char sy = random8()%(2*NUM_ROWS);
				if (sy>NUM_ROWS-1) sy=0;
				streamers[streamer_num] = (streamer){{random8()%NUM_COLS, sy}, 0, (random8()%8)+12, index++,(random8()%16)+3};
				streamer_num++;	
			}
		}
		wait(60);	
		
	}
}


#endif //MCUF_MODUL_BORG16_MATRIX_SUPPORT
