/*
	QR_NOISE
 
	Noise suitable for axial representation of hexagonal grid.
	http://www.redblobgames.com/grids/hexagons/

	Q and R coordinates defines as:

		  o---> Q (column)
		 /
		v R (row)
*/

#pragma once
#ifndef QR_NOISE_H
#define QR_NOISE_H

#if __cplusplus
extern "C" {
#endif

#include <stdint.h>


#define QR_RAND_MAX 0x7FFF

static int qr_randseed = 303;

// Returns a pseudo-random integer value in the range [0​,0x7FFF].
int qr_rand()
{
	qr_randseed = (qr_randseed * 1103515245 + 12345) & 0x7FFFFFFF;
	return (qr_randseed >> 16) & QR_RAND_MAX;
}

// Seeds the PRNG used by qr_rand() with the value seed.
void qr_srand(int seed)
{
	qr_randseed = seed;
}

// Macros from Bob Jenkins' lookup3.c
#define rot(x,k) (((x)<<(k)) | ((x)>>(32-(k))))
#define final(a,b,c) \
{ \
	c ^= b; c -= rot(b,14); \
	a ^= c; a -= rot(c,11); \
	b ^= a; b -= rot(a,25); \
	c ^= b; c -= rot(b,16); \
	a ^= c; a -= rot(c,4);  \
	b ^= a; b -= rot(a,14); \
	c ^= b; c -= rot(b,24); \
}

// Seeds the PRNG used by qr_rand() with the hashed mix of seed, q and r.
// A good seed value is necessary to avoid patterns for low q and r values.
void qr_srand2(int seed, int q, int r)
{
	final(q, r, seed);
	qr_randseed = seed;
}

#undef final
#undef rot

#define sqr(a) ((a)*(a))

#ifndef max
#define max(a,b) ((a)>(b)?(a):(b))
#endif
	
// Value noise
int qr_vnoise2(int seed, int q, int r, int freq)
{
	if (freq <= 0)
		return 0;

	if (freq == 1)
	{
		qr_srand2(seed, q, r);
		return qr_rand();
	}

	// FIXME: proper floor_div() for signed...
	unsigned qq = (unsigned) q - INT_MIN;
	unsigned rr = (unsigned) r - INT_MIN;

	int32_t q0 = qq / freq;
	int32_t r0 = rr / freq;

	int32_t dq0 = qq % freq;
	int32_t dr0 = rr % freq;

	int32_t dq1 = freq - dq0;
	int32_t dr1 = freq - dr0;

	if (dq0 >= dr0)
	{
		qr_srand2(seed, q0, r0);
		int n0 = qr_rand();
		int w0 = sqr(dq0) + sqr(dr0) - dq0*dr0;
		w0 = max(0, 255*sqr(freq) - 295*w0) / sqr(freq);
		w0 = sqr(w0) >> 8;

		qr_srand2(seed, q0+1, r0);
		int n1 = qr_rand();
		int w1 = sqr(dq1) + sqr(dr0) + dq1*dr0;
		w1 = max(0, 255*sqr(freq) - 295*w1) / sqr(freq);
		w1 = sqr(w1) >> 8;

		qr_srand2(seed, q0+1, r0+1);
		int n2 = qr_rand();
		int w2 = sqr(dq1) + sqr(dr1) - dq1*dr1;
		w2 = max(0, 255*sqr(freq) - 295*w2) / sqr(freq);
		w2 = sqr(w2) >> 8;

		return (n0*w0 + n1*w1 + n2*w2) / (w0+w1+w2);
	}
	else
	{
		qr_srand2(seed, q0, r0);
		int n0 = qr_rand();
		int w0 = sqr(dq0) + sqr(dr0) - dq0*dr0;
		w0 = max(0, 255*sqr(freq) - 295*w0) / sqr(freq);
		w0 = sqr(w0) >> 8;

		qr_srand2(seed, q0, r0+1);
		int n1 = qr_rand();
		int w1 = sqr(dq0) + sqr(dr1) + dq0*dr1;
		w1 = max(0, 255*sqr(freq) - 295*w1) / sqr(freq);
		w1 = sqr(w1) >> 8;

		qr_srand2(seed, q0+1, r0+1);
		int n2 = qr_rand();
		int w2 = sqr(dq1) + sqr(dr1) - dq1*dr1;
		w2 = max(0, 255*sqr(freq) - 295*w2) / sqr(freq);
		w2 = sqr(w2) >> 8;

		return (n0*w0 + n1*w1 + n2*w2) / (w0+w1+w2);
	}
}

#undef sqr
#undef max

// FBM value noise.
int qr_fbm_vnoise2(int seed, int q, int r, int freq, int oct)
{
	int n=0;
	for (int o=0; o<oct; o++)
	{
		n += qr_vnoise2(seed, q, r, freq) >> (o+1);
		freq = (freq >> 1);
		if (freq <= 0)
			break;
	}
	return n;
}

#if __cplusplus
}
#endif

#endif // QR_NOISE_H