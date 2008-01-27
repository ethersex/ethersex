/*! @file
	\brief Skipjack declarations.

	$Id: skipjack.c,v 1.1 2003/03/30 12:42:21 m Exp $
*/

#include <avr/pgmspace.h>

#include "../config.h"
#include "skipjack.h"

#ifdef SKIPJACK_SUPPORT

#if CONF_SMALL_MEMORY==1
#define INLINE
#endif

#if defined(RFM12_SUPPORT) || defined(ZBUS_SUPPORT)
#  ifndef CONF_WITH_DECRYPT
#    define CONF_WITH_DECRYPT 1
#  endif
#endif

#define W1_L	0
#define W1_R	1
#define W2_L	2
#define W2_R	3
#define W3_L	4
#define W3_R	5
#define W4_L	6
#define W4_R	7

static const PROGMEM iu8 fTable[256] = { 
	0xa3,0xd7,0x09,0x83,0xf8,0x48,0xf6,0xf4,0xb3,0x21,0x15,0x78,0x99,0xb1,0xaf,0xf9,
	0xe7,0x2d,0x4d,0x8a,0xce,0x4c,0xca,0x2e,0x52,0x95,0xd9,0x1e,0x4e,0x38,0x44,0x28,
	0x0a,0xdf,0x02,0xa0,0x17,0xf1,0x60,0x68,0x12,0xb7,0x7a,0xc3,0xe9,0xfa,0x3d,0x53,
	0x96,0x84,0x6b,0xba,0xf2,0x63,0x9a,0x19,0x7c,0xae,0xe5,0xf5,0xf7,0x16,0x6a,0xa2,
	0x39,0xb6,0x7b,0x0f,0xc1,0x93,0x81,0x1b,0xee,0xb4,0x1a,0xea,0xd0,0x91,0x2f,0xb8,
	0x55,0xb9,0xda,0x85,0x3f,0x41,0xbf,0xe0,0x5a,0x58,0x80,0x5f,0x66,0x0b,0xd8,0x90,
	0x35,0xd5,0xc0,0xa7,0x33,0x06,0x65,0x69,0x45,0x00,0x94,0x56,0x6d,0x98,0x9b,0x76,
	0x97,0xfc,0xb2,0xc2,0xb0,0xfe,0xdb,0x20,0xe1,0xeb,0xd6,0xe4,0xdd,0x47,0x4a,0x1d,
	0x42,0xed,0x9e,0x6e,0x49,0x3c,0xcd,0x43,0x27,0xd2,0x07,0xd4,0xde,0xc7,0x67,0x18,
	0x89,0xcb,0x30,0x1f,0x8d,0xc6,0x8f,0xaa,0xc8,0x74,0xdc,0xc9,0x5d,0x5c,0x31,0xa4,
	0x70,0x88,0x61,0x2c,0x9f,0x0d,0x2b,0x87,0x50,0x82,0x54,0x64,0x26,0x7d,0x03,0x40,
	0x34,0x4b,0x1c,0x73,0xd1,0xc4,0xfd,0x3b,0xcc,0xfb,0x7f,0xab,0xe6,0x3e,0x5b,0xa5,
	0xad,0x04,0x23,0x9c,0x14,0x51,0x22,0xf0,0x29,0x79,0x71,0x7e,0xff,0x8c,0x0e,0xe2,
	0x0c,0xef,0xbc,0x72,0x75,0x6f,0x37,0xa1,0xec,0xd3,0x8e,0x62,0x8b,0x86,0x10,0xe8,
	0x08,0x77,0x11,0xbe,0x92,0x4f,0x24,0xc5,0x32,0x36,0x9d,0xcf,0xf3,0xa6,0xbb,0xac,
	0x5e,0x6c,0xa9,0x13,0x57,0x25,0xb5,0xe3,0xbd,0xa8,0x3a,0x01,0x05,0x59,0x2a,0x46
};

#ifdef INLINE
#define g(v,k,idx) \
		for( tmp=0; tmp<2; tmp++ ) { \
			(v)[W1_L] ^= fTable[k[kidx++]^(v)[W1_R]]; \
			(v)[W1_R] ^= fTable[k[kidx++]^(v)[W1_L]]; \
			if( kidx>=10 ) kidx-=10; \
		}
#else
void g( iu8* v, const iu8* k, iu8 kidx )
{
#if 0
	v[W1_L] ^= fTable[k[kidx++]^v[W1_R]];
	v[W1_R] ^= fTable[k[kidx++]^v[W1_L]];
	if( kidx>=10 ) kidx-=10;
	v[W1_L] ^= fTable[k[kidx++]^v[W1_R]];
	v[W1_R] ^= fTable[k[kidx++]^v[W1_L]];
#endif
	v[W1_L] ^= pgm_read_byte(fTable+(k[kidx++]^v[W1_R]));
	v[W1_R] ^= pgm_read_byte(fTable+(k[kidx++]^v[W1_L]));
	if( kidx>=10 ) kidx-=10;
	v[W1_L] ^= pgm_read_byte(fTable+(k[kidx++]^v[W1_R]));
	v[W1_R] ^= pgm_read_byte(fTable+(k[kidx++]^v[W1_L]));
}
#endif

#ifdef INLINE
#define rule_a(v,counter) \
	{ \
		tmp=(v)[W4_L]; (v)[W4_L]=(v)[W3_L]; (v)[W3_L]=(v)[W2_L]; \
			(v)[W2_L]=(v)[W1_L]; (v)[W1_L]^=tmp; \
		tmp=(v)[W4_R]; (v)[W4_R]=(v)[W3_R]; (v)[W3_R]=(v)[W2_R]; \
			(v)[W2_R]=(v)[W1_R]; (v)[W1_R]^=tmp^counter; \
	}
#else
void rule_a( iu8* v, iu8 counter )
{
	iu8 tmp;

	/* Rotate right High byte */
	tmp=v[W4_L]; v[W4_L]=v[W3_L]; v[W3_L]=v[W2_L]; v[W2_L]=v[W1_L];
		v[W1_L]^=tmp;
	/* Rotate right Low byte */
	tmp=v[W4_R]; v[W4_R]=v[W3_R]; v[W3_R]=v[W2_R]; v[W2_R]=v[W1_R];
		v[W1_R]^=tmp^counter;
}
#endif

#ifdef INLINE
#define rule_b(v,counter) \
	{ \
		tmp=(v)[W4_L]; (v)[W4_L]=(v)[W3_L]; (v)[W3_L]=(v)[W1_L]^(v)[W2_L]; \
			(v)[W2_L]=(v)[W1_L]; (v)[W1_L]=tmp; \
		tmp=(v)[W4_R]; (v)[W4_R]=(v)[W3_R]; (v)[W3_R]=(v)[W1_R]^(v)[W2_R]^counter; \
			(v)[W2_R]=(v)[W1_R]; (v)[W1_R]=tmp; \
	}
#else
void rule_b( iu8* v, iu8 counter )
{
	iu8 tmp;

	/* Rotate right High byte */
	tmp=v[W4_L]; v[W4_L]=v[W3_L]; v[W3_L]=v[W1_L]^v[W2_L]; v[W2_L]=v[W1_L];
		v[W1_L]=tmp;
	/* Rotate right Low byte */
	tmp=v[W4_R]; v[W4_R]=v[W3_R]; v[W3_R]=v[W1_R]^v[W2_R]^counter; v[W2_R]=v[W1_R];
		v[W1_R]=tmp;
}
#endif

void skipjack_enc( iu8* v, iu8* k )
{
	iu8 kidx=0, counter=1;
#ifdef INLINE
	iu8 tmp;
#endif

	while( counter<9 ) {
		g( v, k, kidx );
		rule_a( v, counter );
#ifndef INLINE
		kidx += 4;
		if( kidx>=10 ) kidx-=10;
#endif
		counter++;
	}

	while( counter<17 ) {
		rule_b( v, counter );
		g( v+2, k, kidx );
#ifndef INLINE
		kidx += 4;
		if( kidx>=10 ) kidx-=10;
#endif
		counter++;
	}

	while( counter<25 ) {
		g( v, k, kidx );
		rule_a( v, counter );
#ifndef INLINE
		kidx += 4;
		if( kidx>=10 ) kidx-=10;
#endif
		counter++;
	}

	while( counter<33 ) {
		rule_b( v, counter );
		g( v+2, k, kidx );
#ifndef INLINE
		kidx += 4;
		if( kidx>=10 ) kidx-=10;
#endif
		counter++;
	}
}

#if CONF_WITH_DECRYPT==1

#ifdef INLINE
#define ginv(v,k,kidx) \
	for( tmp=0; tmp<2; tmp++ ) { \
		(v)[W1_R] ^= fTable[k[--kidx]^(v)[W1_L]]; \
		(v)[W1_L] ^= fTable[k[--kidx]^(v)[W1_R]]; \
		if( kidx==0 ) kidx+=10; \
	}
#else
void ginv( iu8* v, const iu8* k, iu8 kidx )
{
#if 0
	v[W1_R] ^= fTable[k[(kidx+3)%10]^v[W1_L]];
	v[W1_L] ^= fTable[k[(kidx+2)%10]^v[W1_R]];
	v[W1_R] ^= fTable[k[kidx+1]^v[W1_L]];
	v[W1_L] ^= fTable[k[kidx]^v[W1_R]];
#endif
	v[W1_R] ^= pgm_read_byte(fTable+(k[(kidx+3)%10]^v[W1_L]));
	v[W1_L] ^= pgm_read_byte(fTable+(k[(kidx+2)%10]^v[W1_R]));
	v[W1_R] ^= pgm_read_byte(fTable+(k[kidx+1]^v[W1_L]));
	v[W1_L] ^= pgm_read_byte(fTable+(k[kidx]^v[W1_R]));
}
#endif

#ifdef INLINE
#define rule_ainv(v,counter) \
	{ \
		tmp=(v)[W1_L]; (v)[W1_L]=(v)[W2_L]; (v)[W2_L]=(v)[W3_L]; \
			(v)[W3_L]=(v)[W4_L]; (v)[W4_L]=tmp^(v)[W1_L]; \
		tmp=(v)[W1_R]; (v)[W1_R]=(v)[W2_R]; (v)[W2_R]=(v)[W3_R]; \
			(v)[W3_R]=(v)[W4_R]; (v)[W4_R]=tmp^(v)[W1_R]^counter; \
	}
#else
void rule_ainv( iu8* v, iu8 counter )
{
	iu8 tmp;

	/* Rotate left High byte */
	tmp=v[W1_L]; v[W1_L]=v[W2_L]; v[W2_L]=v[W3_L]; v[W3_L]=v[W4_L];
		v[W4_L]=tmp^v[W1_L];
	/* Rotate left Low byte */
	tmp=v[W1_R]; v[W1_R]=v[W2_R]; v[W2_R]=v[W3_R]; v[W3_R]=v[W4_R];
		v[W4_R]=tmp^v[W1_R]^counter;
}
#endif

#ifdef INLINE
#define rule_binv(v,counter) \
	{ \
		tmp=(v)[W1_L]; (v)[W1_L]=(v)[W2_L]; (v)[W2_L]^=(v)[W3_L]; \
			(v)[W3_L]=(v)[W4_L]; (v)[W4_L]=tmp; \
		tmp=(v)[W1_R]; (v)[W1_R]=(v)[W2_R]; (v)[W2_R]^=(v)[W3_R]^counter; \
			(v)[W3_R]=(v)[W4_R]; (v)[W4_R]=tmp; \
	}
#else
void rule_binv( iu8* v, iu8 counter )
{
	iu8 tmp;

	/* Rotate left High byte */
	tmp=v[W1_L]; v[W1_L]=v[W2_L]; v[W2_L]^=v[W3_L]; v[W3_L]=v[W4_L];
		v[W4_L]=tmp;
	/* Rotate left Low byte */
	tmp=v[W1_R]; v[W1_R]=v[W2_R]; v[W2_R]^=v[W3_R]^counter; v[W3_R]=v[W4_R];
		v[W4_R]=tmp;
}
#endif

void skipjack_dec( iu8* v, iu8* k )
{
#ifdef INLINE
	iu8 kidx=8, counter=32;
	iu8 tmp;
#else
	iu8 kidx=4, counter=32;
#endif

	while( counter>24 ) {
		ginv( v+2, k, kidx );
		rule_binv( v, counter );
#ifndef INLINE
		kidx += 6;
		if( kidx>=10 ) kidx-=10;
#endif
		counter--;
	}

	while( counter>16 ) {
		rule_ainv( v, counter );
		ginv( v, k, kidx );
#ifndef INLINE
		kidx += 6;
		if( kidx>=10 ) kidx-=10;
#endif
		counter--;
	}

	while( counter>8 ) {
		ginv( v+2, k, kidx );
		rule_binv( v, counter );
#ifndef INLINE
		kidx += 6;
		if( kidx>=10 ) kidx-=10;
#endif
		counter--;
	}

	while( counter>0 ) {
		rule_ainv( v, counter );
		ginv( v, k, kidx );
#ifndef INLINE
		kidx += 6;
		if( kidx>=10 ) kidx-=10;
#endif
		counter--;
	}
}
#endif /* CONF_WITH_DECRYPT==1 */

#ifdef TEST
#include <stdio.h>
#include <string.h>
#include <time.h>

int main() {
	iu8 inp[8]	= { 0x33, 0x22, 0x11, 0x00, 0xdd, 0xcc, 0xbb, 0xaa };
	iu8 key[10]	= { 0x00, 0x99, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11 };
	iu8 enc[8], dec[8];
	iu8 chk[8]	= { 0x25, 0x87, 0xca, 0xe2, 0x7a, 0x12, 0xd3, 0x00 };
	iu8 tab[10][256];
	long i;
	clock_t elapsed;

	memcpy( enc, inp, 8 );
	skipjack_enc( enc, key );
	printf((memcmp(enc, chk, 8) == 0) ? "encryption OK!\n" : "encryption failure!\n");
	memcpy( dec, enc, 8 );
	skipjack_dec( dec, key );
	printf((memcmp(dec, inp, 8) == 0) ? "decryption OK!\n" : "decryption failure!\n");

	elapsed = -clock();
	for (i = 0; i < 1000000L; i++) {
		skipjack_enc( enc, key );
	}
	elapsed += clock();
	printf ("elapsed time: %.1f s.\n", (float)elapsed/CLOCKS_PER_SEC);
	return 0;
}
#endif /* TEST */

#endif /* SKIPJACK_SUPPORT */

