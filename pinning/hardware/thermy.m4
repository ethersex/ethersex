
dnl Block pins used by LCD ...
define(`port_mask_A', eval(port_mask_A | 0xF7))dnl
define(`port_mask_C', eval(port_mask_C | 0xFF))dnl
define(`port_mask_D', eval(port_mask_D | 0xFE))dnl
define(`port_mask_G', eval(port_mask_G | 0x07))dnl



dnl Motor
pin(MOTOR_POS, PB7, OUTPUT)
pin(MOTOR_NEG, PB6, OUTPUT)
pin(MOTOR_DIRP, PG3, OUTPUT)
pin(MOTOR_DIRN, PG4, OUTPUT)

dnl Stellrad
pin(STELLRAD_A, PB5)
pin(STELLRAD_B, PB4)

dnl Taster
pin(TASTE_AUTO, PB0)
pin(TASTE_PROG, PB1)
pin(TASTE_GRAD, PB2)

dnl Lichtschranke (not declared OUTPUT to not touch USART0 pins)
pin(LICHTSCHRANKE_ENABLE1, PE3)
pin(LICHTSCHRANKE_ENABLE2, PE2)
pin(LICHTSCHRANKE_SENSE, PE1)

dnl Temperaturfuehler
pin(TEMP_ENABLE, PF2, OUTPUT)
pin(TEMP_SENSE, PF0)


#define ADC_MUX_TEMP_SENSE	0

dnl *******************************************************************************
dnl *  LCD Layout:
dnl *
dnl *        B|B|B|B|B|B|B|B|B|B|B |B |B |B |B |B |B |B |B |B |B |B |B |B |    
dnl *        0|1|2|3|4|5|6|7|8|9|10|11|12|13|14|15|16|17|18|19|20|21|22|23|    
dnl *                                                                          
dnl *        ---------------------------BAR24------------------------------    
dnl *                                                                          
dnl *      Auto      3A         2A                  1A         0A       SUN    
dnl *             3F    3B   2F    2B            1F    1B   0F    0B           
dnl *      Manu      3g         2g       Col2       1g         0g       MOON   
dnl *             3E    3C   2E    2C            1E    1C   0E    0C           
dnl *      Prog      3D         2D       Col1       1D         0D       STAR   
dnl *                                                                          
dnl *******************************************************************************

#define LCD_SEG_BAR24		0	 // 0, 0
#define LCD_SEG_B0		1	 // 0, 1
#define LCD_SEG_B1		2	 // 0, 2
#define LCD_SEG_B2		3	 // 0, 3
#define LCD_SEG_B3		4	 // 0, 4
#define LCD_SEG_B4		5	 // 0, 5
#define LCD_SEG_B5		6	 // 0, 6
#define LCD_SEG_B6		7	 // 0, 7
#define LCD_SEG_B7		8	 // 0, 8
#define LCD_SEG_B8		9	 // 0, 9
#define LCD_SEG_B9		10	 // 0, 10
#define LCD_SEG_B14		11	 // 0, 11
#define LCD_SEG_B15		12	 // 0, 12
#define LCD_SEG_B16		13	 // 0, 13
#define LCD_SEG_B17		14	 // 0, 14
#define LCD_SEG_B18		15	 // 0, 15
#define LCD_SEG_B19		16	 // 0, 16
#define LCD_SEG_B20		17	 // 0, 17
#define LCD_SEG_B21		18	 // 0, 18
#define LCD_SEG_B22		19	 // 0, 19
#define LCD_SEG_B23		20	 // 0, 20
//*****************************************************************
#define LCD_SEG_COL2		40	 // 1, 0
#define LCD_SEG_AUTO		41	 // 1, 1
#define LCD_SEG_PROG		42	 // 1, 2
#define LCD_SEG_3F		43	 // 1, 3
#define LCD_SEG_3G		44	 // 1, 4
#define LCD_SEG_3A		45	 // 1, 5
#define LCD_SEG_3B		46	 // 1, 6
#define LCD_SEG_2F		47	 // 1, 7
#define LCD_SEG_2G		48	 // 1, 8
#define LCD_SEG_2A		49	 // 1, 9
#define LCD_SEG_2B		50	 // 1, 10
#define LCD_SEG_1F		51	 // 1, 11
#define LCD_SEG_1G		52	 // 1, 12
#define LCD_SEG_1A		53	 // 1, 13
#define LCD_SEG_1B		54	 // 1, 14
#define LCD_SEG_0F		55	 // 1, 15
#define LCD_SEG_0G		56	 // 1, 16
#define LCD_SEG_0A		57	 // 1, 17
#define LCD_SEG_0B		58	 // 1, 18
#define LCD_SEG_SNOW		59	 // 1, 19
#define LCD_SEG_SUN		60	 // 1, 20
//*****************************************************************
#define LCD_SEG_COL1		80	 // 2, 0
#define LCD_SEG_MANU		82	 // 2, 2
#define LCD_SEG_3E		83	 // 2, 3
#define LCD_SEG_3D		84	 // 2, 4
#define LCD_SEG_B11		85	 // 2, 5
#define LCD_SEG_3C		86	 // 2, 6
#define LCD_SEG_2E		87	 // 2, 7
#define LCD_SEG_2D		88	 // 2, 8
#define LCD_SEG_B10		89	 // 2, 9
#define LCD_SEG_2C		90	 // 2, 10
#define LCD_SEG_1E		91	 // 2, 11
#define LCD_SEG_1D		92	 // 2, 12
#define LCD_SEG_B13		93	 // 2, 13
#define LCD_SEG_1C		94	 // 2, 14
#define LCD_SEG_0E		95	 // 2, 15
#define LCD_SEG_0D		96	 // 2, 16
#define LCD_SEG_B12		97	 // 2, 17
#define LCD_SEG_0C		98	 // 2, 18
#define LCD_SEG_MOON		100	 // 2, 20

