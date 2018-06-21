#include <stdio.h>
#ifdef __x86_64__ // 64-bit compile with SSE and above disabled
#undef __USE_EXTERN_INLINES // fix for error in gcc's stdlib-float.h
#endif
#include <stdlib.h>

#ifdef __MMX__
#include <mmintrin.h>
#endif

#ifndef VECT /* vectorisation */
#define VECT 0
#endif

#ifndef STEP /* step-by-step */
#define STEP 0
#endif

#ifndef RIPE /* variant */
#define RIPE 0
#endif

#ifndef ZERO /* benchmark */
#define ZERO 0
#endif

#define BYTE unsigned char
#define WORD unsigned int
#define SIZE unsigned long long
#define SW (int)sizeof(WORD)
#define SS (int)sizeof(SIZE)
#define NI 16
#if RIPE==0 || RIPE==128
#define NO 4
#elif RIPE==160
#define NO 5
#else
#error unimplemented RIPE variant
#endif
#define NS 16
#define NB NI*SW

static WORD accu[NO];

static SIZE size;

static WORD iv[NO] = {
0x67452301u,
0xefcdab89u,
0x98badcfeu,
0x10325476u,
#if RIPE > 128
0xc3d2e1f0u,
#endif
};

#define BA(w) ((BYTE*)(w))

static void print(const WORD wa[],int nw,const char sa[])
{
	int i,j;

	for (i = 0; i < nw; ++i) {
#if BYTE_ORDER == BIG_ENDIAN
		for (j = SW-1; j >= 0; --j)
#elif BYTE_ORDER == LITTLE_ENDIAN
		for (j = 0; j <= SW-1; ++j)
#else
		for (;;)
#endif
			printf("%02x",BA(wa)[i*SW+j]);
#if STEP
		putc(' ',stdout);
#endif
	}
	printf(" *%s\n",sa);
}

static void init(void)
{
	int i;

	for (i = 0; i < NO; ++i)
		accu[i] = iv[i];
	size = 0ull;
}

#define ROTL(w,s) ((w) << (s) | (w) >> (32-s))
#define ROTR(w,s) ((w) >> (s) | (w) << (32-s))

static void next(const BYTE ba[NB])
{
	int i,j;
	WORD W[NI];

	for (i = 0; i < NI; ++i)
#if BYTE_ORDER == BIG_ENDIAN
		for (j = SW-1; j >= 0; --j)
#elif BYTE_ORDER == LITTLE_ENDIAN
		for (j = 0; j <= SW-1; ++j)
#else
		for (;;)
#endif
			BA(W)[i*SW+j] = *ba++;
#if STEP
	print(W,NI,"input");
#endif

/*
 * Round functions for RIPEMD (original).
 */
#define F(x, y, z)    ((((y) ^ (z)) & (x)) ^ (z))
#define G(x, y, z)    (((x) & (y)) | (((x) | (y)) & (z)))
#define H(x, y, z)    ((x) ^ (y) ^ (z))

/*
 * Round functions for RIPEMD-128 and RIPEMD-160.
 */
#define F1(x, y, z)   ((x) ^ (y) ^ (z))
#define F2(x, y, z)   ((((y) ^ (z)) & (x)) ^ (z))
#define F3(x, y, z)   (((x) | ~(y)) ^ (z))
#define F4(x, y, z)   ((((x) ^ (y)) & (z)) ^ (y))
#if RIPE == 160
#define F5(x, y, z)   ((x) ^ ((y) | ~(z)))
#endif

/*
 * Round constants for RIPEMD (original).
 */
#define KF1    (WORD)0x00000000
#define KG1    (WORD)0x5A827999
#define KH1    (WORD)0x6ED9EBA1

#define KF2    (WORD)0x50A28BE6
#define KG2    (WORD)0x00000000
#define KH2    (WORD)0x5C4DD124

#if RIPE == 0 && VECT
	static WORD KF[2] = { KF1, KF2 };
	static WORD KG[2] = { KG1, KG2 };
	static WORD KH[2] = { KH1, KH2 };
#endif
/*
 * Round constants for RIPEMD-128 and RIPEMD-160.
 */
#define K11    (WORD)0x00000000
#define K12    (WORD)0x5A827999
#define K13    (WORD)0x6ED9EBA1
#define K14    (WORD)0x8F1BBCDC
#if RIPE > 128
#define K15    (WORD)0xA953FD4E
#endif

#define K21    (WORD)0x50A28BE6
#define K22    (WORD)0x5C4DD124
#define K23    (WORD)0x6D703EF3
#if RIPE == 128
#define K24    (WORD)0x00000000
#else
#define K24    (WORD)0x7A6D76E9
#define K25    (WORD)0x00000000
#endif

#if RIPE == 0
/*
 * RIPEMD (original hash, deprecated).
 */

#if !VECT /* non-vectorised */

#define FF1(A, B, C, D, X, s)   do { \
		A = ROTL((WORD)(A + F(B, C, D) + X + KF1), s); \
	} while (0)

#define GG1(A, B, C, D, X, s)   do { \
		A = ROTL((WORD)(A + G(B, C, D) + X + KG1), s); \
	} while (0)

#define HH1(A, B, C, D, X, s)   do { \
		A = ROTL((WORD)(A + H(B, C, D) + X + KH1), s); \
	} while (0)

#define FF2(A, B, C, D, X, s)   do { \
		A = ROTL((WORD)(A + F(B, C, D) + X + KF2), s); \
	} while (0)

#define GG2(A, B, C, D, X, s)   do { \
		A = ROTL((WORD)(A + G(B, C, D) + X + KG2), s); \
	} while (0)

#define HH2(A, B, C, D, X, s)   do { \
		A = ROTL((WORD)(A + H(B, C, D) + X + KH2), s); \
	} while (0)

	WORD A1, B1, C1, D1;
	WORD A2, B2, C2, D2;
	WORD tmp;

	A1 = A2 = accu[0];
	B1 = B2 = accu[1];
	C1 = C2 = accu[2];
	D1 = D2 = accu[3];

	FF1(A1, B1, C1, D1, W[ 0], 11);
	FF1(D1, A1, B1, C1, W[ 1], 14);
	FF1(C1, D1, A1, B1, W[ 2], 15);
	FF1(B1, C1, D1, A1, W[ 3], 12);
	FF1(A1, B1, C1, D1, W[ 4],  5);
	FF1(D1, A1, B1, C1, W[ 5],  8);
	FF1(C1, D1, A1, B1, W[ 6],  7);
	FF1(B1, C1, D1, A1, W[ 7],  9);
	FF1(A1, B1, C1, D1, W[ 8], 11);
	FF1(D1, A1, B1, C1, W[ 9], 13);
	FF1(C1, D1, A1, B1, W[10], 14);
	FF1(B1, C1, D1, A1, W[11], 15);
	FF1(A1, B1, C1, D1, W[12],  6);
	FF1(D1, A1, B1, C1, W[13],  7);
	FF1(C1, D1, A1, B1, W[14],  9);
	FF1(B1, C1, D1, A1, W[15],  8);

	GG1(A1, B1, C1, D1, W[ 7],  7);
	GG1(D1, A1, B1, C1, W[ 4],  6);
	GG1(C1, D1, A1, B1, W[13],  8);
	GG1(B1, C1, D1, A1, W[ 1], 13);
	GG1(A1, B1, C1, D1, W[10], 11);
	GG1(D1, A1, B1, C1, W[ 6],  9);
	GG1(C1, D1, A1, B1, W[15],  7);
	GG1(B1, C1, D1, A1, W[ 3], 15);
	GG1(A1, B1, C1, D1, W[12],  7);
	GG1(D1, A1, B1, C1, W[ 0], 12);
	GG1(C1, D1, A1, B1, W[ 9], 15);
	GG1(B1, C1, D1, A1, W[ 5],  9);
	GG1(A1, B1, C1, D1, W[14],  7);
	GG1(D1, A1, B1, C1, W[ 2], 11);
	GG1(C1, D1, A1, B1, W[11], 13);
	GG1(B1, C1, D1, A1, W[ 8], 12);

	HH1(A1, B1, C1, D1, W[ 3], 11);
	HH1(D1, A1, B1, C1, W[10], 13);
	HH1(C1, D1, A1, B1, W[ 2], 14);
	HH1(B1, C1, D1, A1, W[ 4],  7);
	HH1(A1, B1, C1, D1, W[ 9], 14);
	HH1(D1, A1, B1, C1, W[15],  9);
	HH1(C1, D1, A1, B1, W[ 8], 13);
	HH1(B1, C1, D1, A1, W[ 1], 15);
	HH1(A1, B1, C1, D1, W[14],  6);
	HH1(D1, A1, B1, C1, W[ 7],  8);
	HH1(C1, D1, A1, B1, W[ 0], 13);
	HH1(B1, C1, D1, A1, W[ 6],  6);
	HH1(A1, B1, C1, D1, W[11], 12);
	HH1(D1, A1, B1, C1, W[13],  5);
	HH1(C1, D1, A1, B1, W[ 5],  7);
	HH1(B1, C1, D1, A1, W[12],  5);

	FF2(A2, B2, C2, D2, W[ 0], 11);
	FF2(D2, A2, B2, C2, W[ 1], 14);
	FF2(C2, D2, A2, B2, W[ 2], 15);
	FF2(B2, C2, D2, A2, W[ 3], 12);
	FF2(A2, B2, C2, D2, W[ 4],  5);
	FF2(D2, A2, B2, C2, W[ 5],  8);
	FF2(C2, D2, A2, B2, W[ 6],  7);
	FF2(B2, C2, D2, A2, W[ 7],  9);
	FF2(A2, B2, C2, D2, W[ 8], 11);
	FF2(D2, A2, B2, C2, W[ 9], 13);
	FF2(C2, D2, A2, B2, W[10], 14);
	FF2(B2, C2, D2, A2, W[11], 15);
	FF2(A2, B2, C2, D2, W[12],  6);
	FF2(D2, A2, B2, C2, W[13],  7);
	FF2(C2, D2, A2, B2, W[14],  9);
	FF2(B2, C2, D2, A2, W[15],  8);

	GG2(A2, B2, C2, D2, W[ 7],  7);
	GG2(D2, A2, B2, C2, W[ 4],  6);
	GG2(C2, D2, A2, B2, W[13],  8);
	GG2(B2, C2, D2, A2, W[ 1], 13);
	GG2(A2, B2, C2, D2, W[10], 11);
	GG2(D2, A2, B2, C2, W[ 6],  9);
	GG2(C2, D2, A2, B2, W[15],  7);
	GG2(B2, C2, D2, A2, W[ 3], 15);
	GG2(A2, B2, C2, D2, W[12],  7);
	GG2(D2, A2, B2, C2, W[ 0], 12);
	GG2(C2, D2, A2, B2, W[ 9], 15);
	GG2(B2, C2, D2, A2, W[ 5],  9);
	GG2(A2, B2, C2, D2, W[14],  7);
	GG2(D2, A2, B2, C2, W[ 2], 11);
	GG2(C2, D2, A2, B2, W[11], 13);
	GG2(B2, C2, D2, A2, W[ 8], 12);

	HH2(A2, B2, C2, D2, W[ 3], 11);
	HH2(D2, A2, B2, C2, W[10], 13);
	HH2(C2, D2, A2, B2, W[ 2], 14);
	HH2(B2, C2, D2, A2, W[ 4],  7);
	HH2(A2, B2, C2, D2, W[ 9], 14);
	HH2(D2, A2, B2, C2, W[15],  9);
	HH2(C2, D2, A2, B2, W[ 8], 13);
	HH2(B2, C2, D2, A2, W[ 1], 15);
	HH2(A2, B2, C2, D2, W[14],  6);
	HH2(D2, A2, B2, C2, W[ 7],  8);
	HH2(C2, D2, A2, B2, W[ 0], 13);
	HH2(B2, C2, D2, A2, W[ 6],  6);
	HH2(A2, B2, C2, D2, W[11], 12);
	HH2(D2, A2, B2, C2, W[13],  5);
	HH2(C2, D2, A2, B2, W[ 5],  7);
	HH2(B2, C2, D2, A2, W[12],  5);

	tmp = accu[1] + C1 + D2;
	accu[1] = accu[2] + D1 + A2;
	accu[2] = accu[3] + A1 + B2;
	accu[3] = accu[0] + B1 + C2;
	accu[0] = tmp;
	
#else /* vectorised */

#if __MMX__ /* MMX vectors */

#if 0
	/* explicitly fixing registers doesn't change generated code much */
	register __v2si A asm("mm4"),B asm("mm5"),C asm("mm6"),D asm("mm7");
#else
	__v2si A,B,C,D;
#endif
	WORD tmp;

//efine ROTL(w,s) ((w) << (s) | (w) >> (32-s))
#undef ROTL
#define ROTL(x,s) _m_por(_m_pslldi(x,s),_m_psrldi(x,32-s))
#undef F
//efine f(x, y, z)    ((((y) ^ (z)) & (x)) ^ (z))
#define F(x, y, z)    _m_pxor(_m_pand(_m_pxor(y,z),x),z)
#undef G
//efine g(x, y, z)    (((x) & (y)) | (((x) | (y)) & (z)))
#define G(x, y, z)    _m_por(_m_pand(x,y),_m_pand(_m_por(x,y),z))
#undef H
//efine h(x, y, z)    ((x) ^ (y) ^ (z))
#define H(x, y, z)    _m_pxor(x,_m_pxor(y,z))
#define V(U, A, B, C, D, X, s) \
	A = ROTL(_m_paddd(_m_paddd(A,U(B, C, D)), \
		 _m_paddd(_mm_set1_pi32(X),_mm_set_pi32(K##U[1],K##U[0]))), \
		s)

	A = _mm_set1_pi32(accu[0]);
	B = _mm_set1_pi32(accu[1]);
	C = _mm_set1_pi32(accu[2]);
	D = _mm_set1_pi32(accu[3]);

#else /* plain vectors */

#if 0
	typedef __v2si WTWO; /* explicitly fixing register causes internal compiler error */
	register WTWO A asm("mm4"),B asm("mm5"),C asm("mm6"),D asm("mm7");
#else
	typedef WORD WTWO[2];
	WTWO A,B,C,D;
#endif
	WORD tmp;

#define V(U, A, B, C, D, X, s)  for (i = 0; i < 2; ++i) \
		A[i] = ROTL((WORD)(A[i] + U(B[i], C[i], D[i]) + X + K##U[i]), s)

	A[0] = A[1] = accu[0];
	B[0] = B[1] = accu[1];
	C[0] = C[1] = accu[2];
	D[0] = D[1] = accu[3];
#endif

	V(F, A, B, C, D, W[ 0], 11);
	V(F, D, A, B, C, W[ 1], 14);
	V(F, C, D, A, B, W[ 2], 15);
	V(F, B, C, D, A, W[ 3], 12);
	V(F, A, B, C, D, W[ 4],  5);
	V(F, D, A, B, C, W[ 5],  8);
	V(F, C, D, A, B, W[ 6],  7);
	V(F, B, C, D, A, W[ 7],  9);
	V(F, A, B, C, D, W[ 8], 11);
	V(F, D, A, B, C, W[ 9], 13);
	V(F, C, D, A, B, W[10], 14);
	V(F, B, C, D, A, W[11], 15);
	V(F, A, B, C, D, W[12],  6);
	V(F, D, A, B, C, W[13],  7);
	V(F, C, D, A, B, W[14],  9);
	V(F, B, C, D, A, W[15],  8);

	V(G, A, B, C, D, W[ 7],  7);
	V(G, D, A, B, C, W[ 4],  6);
	V(G, C, D, A, B, W[13],  8);
	V(G, B, C, D, A, W[ 1], 13);
	V(G, A, B, C, D, W[10], 11);
	V(G, D, A, B, C, W[ 6],  9);
	V(G, C, D, A, B, W[15],  7);
	V(G, B, C, D, A, W[ 3], 15);
	V(G, A, B, C, D, W[12],  7);
	V(G, D, A, B, C, W[ 0], 12);
	V(G, C, D, A, B, W[ 9], 15);
	V(G, B, C, D, A, W[ 5],  9);
	V(G, A, B, C, D, W[14],  7);
	V(G, D, A, B, C, W[ 2], 11);
	V(G, C, D, A, B, W[11], 13);
	V(G, B, C, D, A, W[ 8], 12);

	V(H, A, B, C, D, W[ 3], 11);
	V(H, D, A, B, C, W[10], 13);
	V(H, C, D, A, B, W[ 2], 14);
	V(H, B, C, D, A, W[ 4],  7);
	V(H, A, B, C, D, W[ 9], 14);
	V(H, D, A, B, C, W[15],  9);
	V(H, C, D, A, B, W[ 8], 13);
	V(H, B, C, D, A, W[ 1], 15);
	V(H, A, B, C, D, W[14],  6);
	V(H, D, A, B, C, W[ 7],  8);
	V(H, C, D, A, B, W[ 0], 13);
	V(H, B, C, D, A, W[ 6],  6);
	V(H, A, B, C, D, W[11], 12);
	V(H, D, A, B, C, W[13],  5);
	V(H, C, D, A, B, W[ 5],  7);
	V(H, B, C, D, A, W[12],  5);

	tmp = accu[1] + C[0] + D[1];
	accu[1] = accu[2] + D[0] + A[1];
	accu[2] = accu[3] + A[0] + B[1];
	accu[3] = accu[0] + B[0] + C[1];
	accu[0] = tmp;
#endif

#elif RIPE == 128
/*
 * RIPEMD-128
 */

#define RR(a, b, c, d, f, s, r, k)   do { \
		a = ROTL((WORD)(a + f(b, c, d) + r + k), s); \
	} while (0)

#define ROUND1(a, b, c, d, f, s, r, k)  \
	RR(a ## 1, b ## 1, c ## 1, d ## 1, f, s, r, K1 ## k)

#define ROUND2(a, b, c, d, f, s, r, k)  \
	RR(a ## 2, b ## 2, c ## 2, d ## 2, f, s, r, K2 ## k)

	WORD A1, B1, C1, D1;
	WORD A2, B2, C2, D2;
	WORD tmp;

	A1 = A2 = accu[0];
	B1 = B2 = accu[1];
	C1 = C2 = accu[2];
	D1 = D2 = accu[3];

	ROUND1(A, B, C, D, F1, 11, W[ 0],  1);
	ROUND1(D, A, B, C, F1, 14, W[ 1],  1);
	ROUND1(C, D, A, B, F1, 15, W[ 2],  1);
	ROUND1(B, C, D, A, F1, 12, W[ 3],  1);
	ROUND1(A, B, C, D, F1,  5, W[ 4],  1);
	ROUND1(D, A, B, C, F1,  8, W[ 5],  1);
	ROUND1(C, D, A, B, F1,  7, W[ 6],  1);
	ROUND1(B, C, D, A, F1,  9, W[ 7],  1);
	ROUND1(A, B, C, D, F1, 11, W[ 8],  1);
	ROUND1(D, A, B, C, F1, 13, W[ 9],  1);
	ROUND1(C, D, A, B, F1, 14, W[10],  1);
	ROUND1(B, C, D, A, F1, 15, W[11],  1);
	ROUND1(A, B, C, D, F1,  6, W[12],  1);
	ROUND1(D, A, B, C, F1,  7, W[13],  1);
	ROUND1(C, D, A, B, F1,  9, W[14],  1);
	ROUND1(B, C, D, A, F1,  8, W[15],  1);

	ROUND1(A, B, C, D, F2,  7, W[ 7],  2);
	ROUND1(D, A, B, C, F2,  6, W[ 4],  2);
	ROUND1(C, D, A, B, F2,  8, W[13],  2);
	ROUND1(B, C, D, A, F2, 13, W[ 1],  2);
	ROUND1(A, B, C, D, F2, 11, W[10],  2);
	ROUND1(D, A, B, C, F2,  9, W[ 6],  2);
	ROUND1(C, D, A, B, F2,  7, W[15],  2);
	ROUND1(B, C, D, A, F2, 15, W[ 3],  2);
	ROUND1(A, B, C, D, F2,  7, W[12],  2);
	ROUND1(D, A, B, C, F2, 12, W[ 0],  2);
	ROUND1(C, D, A, B, F2, 15, W[ 9],  2);
	ROUND1(B, C, D, A, F2,  9, W[ 5],  2);
	ROUND1(A, B, C, D, F2, 11, W[ 2],  2);
	ROUND1(D, A, B, C, F2,  7, W[14],  2);
	ROUND1(C, D, A, B, F2, 13, W[11],  2);
	ROUND1(B, C, D, A, F2, 12, W[ 8],  2);

	ROUND1(A, B, C, D, F3, 11, W[ 3],  3);
	ROUND1(D, A, B, C, F3, 13, W[10],  3);
	ROUND1(C, D, A, B, F3,  6, W[14],  3);
	ROUND1(B, C, D, A, F3,  7, W[ 4],  3);
	ROUND1(A, B, C, D, F3, 14, W[ 9],  3);
	ROUND1(D, A, B, C, F3,  9, W[15],  3);
	ROUND1(C, D, A, B, F3, 13, W[ 8],  3);
	ROUND1(B, C, D, A, F3, 15, W[ 1],  3);
	ROUND1(A, B, C, D, F3, 14, W[ 2],  3);
	ROUND1(D, A, B, C, F3,  8, W[ 7],  3);
	ROUND1(C, D, A, B, F3, 13, W[ 0],  3);
	ROUND1(B, C, D, A, F3,  6, W[ 6],  3);
	ROUND1(A, B, C, D, F3,  5, W[13],  3);
	ROUND1(D, A, B, C, F3, 12, W[11],  3);
	ROUND1(C, D, A, B, F3,  7, W[ 5],  3);
	ROUND1(B, C, D, A, F3,  5, W[12],  3);

	ROUND1(A, B, C, D, F4, 11, W[ 1],  4);
	ROUND1(D, A, B, C, F4, 12, W[ 9],  4);
	ROUND1(C, D, A, B, F4, 14, W[11],  4);
	ROUND1(B, C, D, A, F4, 15, W[10],  4);
	ROUND1(A, B, C, D, F4, 14, W[ 0],  4);
	ROUND1(D, A, B, C, F4, 15, W[ 8],  4);
	ROUND1(C, D, A, B, F4,  9, W[12],  4);
	ROUND1(B, C, D, A, F4,  8, W[ 4],  4);
	ROUND1(A, B, C, D, F4,  9, W[13],  4);
	ROUND1(D, A, B, C, F4, 14, W[ 3],  4);
	ROUND1(C, D, A, B, F4,  5, W[ 7],  4);
	ROUND1(B, C, D, A, F4,  6, W[15],  4);
	ROUND1(A, B, C, D, F4,  8, W[14],  4);
	ROUND1(D, A, B, C, F4,  6, W[ 5],  4);
	ROUND1(C, D, A, B, F4,  5, W[ 6],  4);
	ROUND1(B, C, D, A, F4, 12, W[ 2],  4);

	ROUND2(A, B, C, D, F4,  8, W[ 5],  1);
	ROUND2(D, A, B, C, F4,  9, W[14],  1);
	ROUND2(C, D, A, B, F4,  9, W[ 7],  1);
	ROUND2(B, C, D, A, F4, 11, W[ 0],  1);
	ROUND2(A, B, C, D, F4, 13, W[ 9],  1);
	ROUND2(D, A, B, C, F4, 15, W[ 2],  1);
	ROUND2(C, D, A, B, F4, 15, W[11],  1);
	ROUND2(B, C, D, A, F4,  5, W[ 4],  1);
	ROUND2(A, B, C, D, F4,  7, W[13],  1);
	ROUND2(D, A, B, C, F4,  7, W[ 6],  1);
	ROUND2(C, D, A, B, F4,  8, W[15],  1);
	ROUND2(B, C, D, A, F4, 11, W[ 8],  1);
	ROUND2(A, B, C, D, F4, 14, W[ 1],  1);
	ROUND2(D, A, B, C, F4, 14, W[10],  1);
	ROUND2(C, D, A, B, F4, 12, W[ 3],  1);
	ROUND2(B, C, D, A, F4,  6, W[12],  1);

	ROUND2(A, B, C, D, F3,  9, W[ 6],  2);
	ROUND2(D, A, B, C, F3, 13, W[11],  2);
	ROUND2(C, D, A, B, F3, 15, W[ 3],  2);
	ROUND2(B, C, D, A, F3,  7, W[ 7],  2);
	ROUND2(A, B, C, D, F3, 12, W[ 0],  2);
	ROUND2(D, A, B, C, F3,  8, W[13],  2);
	ROUND2(C, D, A, B, F3,  9, W[ 5],  2);
	ROUND2(B, C, D, A, F3, 11, W[10],  2);
	ROUND2(A, B, C, D, F3,  7, W[14],  2);
	ROUND2(D, A, B, C, F3,  7, W[15],  2);
	ROUND2(C, D, A, B, F3, 12, W[ 8],  2);
	ROUND2(B, C, D, A, F3,  7, W[12],  2);
	ROUND2(A, B, C, D, F3,  6, W[ 4],  2);
	ROUND2(D, A, B, C, F3, 15, W[ 9],  2);
	ROUND2(C, D, A, B, F3, 13, W[ 1],  2);
	ROUND2(B, C, D, A, F3, 11, W[ 2],  2);

	ROUND2(A, B, C, D, F2,  9, W[15],  3);
	ROUND2(D, A, B, C, F2,  7, W[ 5],  3);
	ROUND2(C, D, A, B, F2, 15, W[ 1],  3);
	ROUND2(B, C, D, A, F2, 11, W[ 3],  3);
	ROUND2(A, B, C, D, F2,  8, W[ 7],  3);
	ROUND2(D, A, B, C, F2,  6, W[14],  3);
	ROUND2(C, D, A, B, F2,  6, W[ 6],  3);
	ROUND2(B, C, D, A, F2, 14, W[ 9],  3);
	ROUND2(A, B, C, D, F2, 12, W[11],  3);
	ROUND2(D, A, B, C, F2, 13, W[ 8],  3);
	ROUND2(C, D, A, B, F2,  5, W[12],  3);
	ROUND2(B, C, D, A, F2, 14, W[ 2],  3);
	ROUND2(A, B, C, D, F2, 13, W[10],  3);
	ROUND2(D, A, B, C, F2, 13, W[ 0],  3);
	ROUND2(C, D, A, B, F2,  7, W[ 4],  3);
	ROUND2(B, C, D, A, F2,  5, W[13],  3);

	ROUND2(A, B, C, D, F1, 15, W[ 8],  4);
	ROUND2(D, A, B, C, F1,  5, W[ 6],  4);
	ROUND2(C, D, A, B, F1,  8, W[ 4],  4);
	ROUND2(B, C, D, A, F1, 11, W[ 1],  4);
	ROUND2(A, B, C, D, F1, 14, W[ 3],  4);
	ROUND2(D, A, B, C, F1, 14, W[11],  4);
	ROUND2(C, D, A, B, F1,  6, W[15],  4);
	ROUND2(B, C, D, A, F1, 14, W[ 0],  4);
	ROUND2(A, B, C, D, F1,  6, W[ 5],  4);
	ROUND2(D, A, B, C, F1,  9, W[12],  4);
	ROUND2(C, D, A, B, F1, 12, W[ 2],  4);
	ROUND2(B, C, D, A, F1,  9, W[13],  4);
	ROUND2(A, B, C, D, F1, 12, W[ 9],  4);
	ROUND2(D, A, B, C, F1,  5, W[ 7],  4);
	ROUND2(C, D, A, B, F1, 15, W[10],  4);
	ROUND2(B, C, D, A, F1,  8, W[14],  4);

	tmp = accu[1] + C1 + D2;
	accu[1] = accu[2] + D1 + A2;
	accu[2] = accu[3] + A1 + B2;
	accu[3] = accu[0] + B1 + C2;
	accu[0] = tmp;

#elif RIPE == 160
/*
 * RIPEMD-160
 */

#define RR(a, b, c, d, e, f, s, r, k)   do { \
		a = ROTL((WORD)(a + f(b, c, d) + r + k), s) + e; \
		c = ROTL(c, 10); \
	} while (0)

#define ROUND1(a, b, c, d, e, f, s, r, k)  \
	RR(a ## 1, b ## 1, c ## 1, d ## 1, e ## 1, f, s, r, K1 ## k)

#define ROUND2(a, b, c, d, e, f, s, r, k)  \
	RR(a ## 2, b ## 2, c ## 2, d ## 2, e ## 2, f, s, r, K2 ## k)

	WORD A1, B1, C1, D1, E1;
	WORD A2, B2, C2, D2, E2;
	WORD tmp;

	A1 = A2 = accu[0];
	B1 = B2 = accu[1];
	C1 = C2 = accu[2];
	D1 = D2 = accu[3];
	E1 = E2 = accu[4];

	ROUND1(A, B, C, D, E, F1, 11, W[ 0],  1);
	ROUND1(E, A, B, C, D, F1, 14, W[ 1],  1);
	ROUND1(D, E, A, B, C, F1, 15, W[ 2],  1);
	ROUND1(C, D, E, A, B, F1, 12, W[ 3],  1);
	ROUND1(B, C, D, E, A, F1,  5, W[ 4],  1);
	ROUND1(A, B, C, D, E, F1,  8, W[ 5],  1);
	ROUND1(E, A, B, C, D, F1,  7, W[ 6],  1);
	ROUND1(D, E, A, B, C, F1,  9, W[ 7],  1);
	ROUND1(C, D, E, A, B, F1, 11, W[ 8],  1);
	ROUND1(B, C, D, E, A, F1, 13, W[ 9],  1);
	ROUND1(A, B, C, D, E, F1, 14, W[10],  1);
	ROUND1(E, A, B, C, D, F1, 15, W[11],  1);
	ROUND1(D, E, A, B, C, F1,  6, W[12],  1);
	ROUND1(C, D, E, A, B, F1,  7, W[13],  1);
	ROUND1(B, C, D, E, A, F1,  9, W[14],  1);
	ROUND1(A, B, C, D, E, F1,  8, W[15],  1);

	ROUND1(E, A, B, C, D, F2,  7, W[ 7],  2);
	ROUND1(D, E, A, B, C, F2,  6, W[ 4],  2);
	ROUND1(C, D, E, A, B, F2,  8, W[13],  2);
	ROUND1(B, C, D, E, A, F2, 13, W[ 1],  2);
	ROUND1(A, B, C, D, E, F2, 11, W[10],  2);
	ROUND1(E, A, B, C, D, F2,  9, W[ 6],  2);
	ROUND1(D, E, A, B, C, F2,  7, W[15],  2);
	ROUND1(C, D, E, A, B, F2, 15, W[ 3],  2);
	ROUND1(B, C, D, E, A, F2,  7, W[12],  2);
	ROUND1(A, B, C, D, E, F2, 12, W[ 0],  2);
	ROUND1(E, A, B, C, D, F2, 15, W[ 9],  2);
	ROUND1(D, E, A, B, C, F2,  9, W[ 5],  2);
	ROUND1(C, D, E, A, B, F2, 11, W[ 2],  2);
	ROUND1(B, C, D, E, A, F2,  7, W[14],  2);
	ROUND1(A, B, C, D, E, F2, 13, W[11],  2);
	ROUND1(E, A, B, C, D, F2, 12, W[ 8],  2);

	ROUND1(D, E, A, B, C, F3, 11, W[ 3],  3);
	ROUND1(C, D, E, A, B, F3, 13, W[10],  3);
	ROUND1(B, C, D, E, A, F3,  6, W[14],  3);
	ROUND1(A, B, C, D, E, F3,  7, W[ 4],  3);
	ROUND1(E, A, B, C, D, F3, 14, W[ 9],  3);
	ROUND1(D, E, A, B, C, F3,  9, W[15],  3);
	ROUND1(C, D, E, A, B, F3, 13, W[ 8],  3);
	ROUND1(B, C, D, E, A, F3, 15, W[ 1],  3);
	ROUND1(A, B, C, D, E, F3, 14, W[ 2],  3);
	ROUND1(E, A, B, C, D, F3,  8, W[ 7],  3);
	ROUND1(D, E, A, B, C, F3, 13, W[ 0],  3);
	ROUND1(C, D, E, A, B, F3,  6, W[ 6],  3);
	ROUND1(B, C, D, E, A, F3,  5, W[13],  3);
	ROUND1(A, B, C, D, E, F3, 12, W[11],  3);
	ROUND1(E, A, B, C, D, F3,  7, W[ 5],  3);
	ROUND1(D, E, A, B, C, F3,  5, W[12],  3);

	ROUND1(C, D, E, A, B, F4, 11, W[ 1],  4);
	ROUND1(B, C, D, E, A, F4, 12, W[ 9],  4);
	ROUND1(A, B, C, D, E, F4, 14, W[11],  4);
	ROUND1(E, A, B, C, D, F4, 15, W[10],  4);
	ROUND1(D, E, A, B, C, F4, 14, W[ 0],  4);
	ROUND1(C, D, E, A, B, F4, 15, W[ 8],  4);
	ROUND1(B, C, D, E, A, F4,  9, W[12],  4);
	ROUND1(A, B, C, D, E, F4,  8, W[ 4],  4);
	ROUND1(E, A, B, C, D, F4,  9, W[13],  4);
	ROUND1(D, E, A, B, C, F4, 14, W[ 3],  4);
	ROUND1(C, D, E, A, B, F4,  5, W[ 7],  4);
	ROUND1(B, C, D, E, A, F4,  6, W[15],  4);
	ROUND1(A, B, C, D, E, F4,  8, W[14],  4);
	ROUND1(E, A, B, C, D, F4,  6, W[ 5],  4);
	ROUND1(D, E, A, B, C, F4,  5, W[ 6],  4);
	ROUND1(C, D, E, A, B, F4, 12, W[ 2],  4);

	ROUND1(B, C, D, E, A, F5,  9, W[ 4],  5);
	ROUND1(A, B, C, D, E, F5, 15, W[ 0],  5);
	ROUND1(E, A, B, C, D, F5,  5, W[ 5],  5);
	ROUND1(D, E, A, B, C, F5, 11, W[ 9],  5);
	ROUND1(C, D, E, A, B, F5,  6, W[ 7],  5);
	ROUND1(B, C, D, E, A, F5,  8, W[12],  5);
	ROUND1(A, B, C, D, E, F5, 13, W[ 2],  5);
	ROUND1(E, A, B, C, D, F5, 12, W[10],  5);
	ROUND1(D, E, A, B, C, F5,  5, W[14],  5);
	ROUND1(C, D, E, A, B, F5, 12, W[ 1],  5);
	ROUND1(B, C, D, E, A, F5, 13, W[ 3],  5);
	ROUND1(A, B, C, D, E, F5, 14, W[ 8],  5);
	ROUND1(E, A, B, C, D, F5, 11, W[11],  5);
	ROUND1(D, E, A, B, C, F5,  8, W[ 6],  5);
	ROUND1(C, D, E, A, B, F5,  5, W[15],  5);
	ROUND1(B, C, D, E, A, F5,  6, W[13],  5);

	ROUND2(A, B, C, D, E, F5,  8, W[ 5],  1);
	ROUND2(E, A, B, C, D, F5,  9, W[14],  1);
	ROUND2(D, E, A, B, C, F5,  9, W[ 7],  1);
	ROUND2(C, D, E, A, B, F5, 11, W[ 0],  1);
	ROUND2(B, C, D, E, A, F5, 13, W[ 9],  1);
	ROUND2(A, B, C, D, E, F5, 15, W[ 2],  1);
	ROUND2(E, A, B, C, D, F5, 15, W[11],  1);
	ROUND2(D, E, A, B, C, F5,  5, W[ 4],  1);
	ROUND2(C, D, E, A, B, F5,  7, W[13],  1);
	ROUND2(B, C, D, E, A, F5,  7, W[ 6],  1);
	ROUND2(A, B, C, D, E, F5,  8, W[15],  1);
	ROUND2(E, A, B, C, D, F5, 11, W[ 8],  1);
	ROUND2(D, E, A, B, C, F5, 14, W[ 1],  1);
	ROUND2(C, D, E, A, B, F5, 14, W[10],  1);
	ROUND2(B, C, D, E, A, F5, 12, W[ 3],  1);
	ROUND2(A, B, C, D, E, F5,  6, W[12],  1);

	ROUND2(E, A, B, C, D, F4,  9, W[ 6],  2);
	ROUND2(D, E, A, B, C, F4, 13, W[11],  2);
	ROUND2(C, D, E, A, B, F4, 15, W[ 3],  2);
	ROUND2(B, C, D, E, A, F4,  7, W[ 7],  2);
	ROUND2(A, B, C, D, E, F4, 12, W[ 0],  2);
	ROUND2(E, A, B, C, D, F4,  8, W[13],  2);
	ROUND2(D, E, A, B, C, F4,  9, W[ 5],  2);
	ROUND2(C, D, E, A, B, F4, 11, W[10],  2);
	ROUND2(B, C, D, E, A, F4,  7, W[14],  2);
	ROUND2(A, B, C, D, E, F4,  7, W[15],  2);
	ROUND2(E, A, B, C, D, F4, 12, W[ 8],  2);
	ROUND2(D, E, A, B, C, F4,  7, W[12],  2);
	ROUND2(C, D, E, A, B, F4,  6, W[ 4],  2);
	ROUND2(B, C, D, E, A, F4, 15, W[ 9],  2);
	ROUND2(A, B, C, D, E, F4, 13, W[ 1],  2);
	ROUND2(E, A, B, C, D, F4, 11, W[ 2],  2);

	ROUND2(D, E, A, B, C, F3,  9, W[15],  3);
	ROUND2(C, D, E, A, B, F3,  7, W[ 5],  3);
	ROUND2(B, C, D, E, A, F3, 15, W[ 1],  3);
	ROUND2(A, B, C, D, E, F3, 11, W[ 3],  3);
	ROUND2(E, A, B, C, D, F3,  8, W[ 7],  3);
	ROUND2(D, E, A, B, C, F3,  6, W[14],  3);
	ROUND2(C, D, E, A, B, F3,  6, W[ 6],  3);
	ROUND2(B, C, D, E, A, F3, 14, W[ 9],  3);
	ROUND2(A, B, C, D, E, F3, 12, W[11],  3);
	ROUND2(E, A, B, C, D, F3, 13, W[ 8],  3);
	ROUND2(D, E, A, B, C, F3,  5, W[12],  3);
	ROUND2(C, D, E, A, B, F3, 14, W[ 2],  3);
	ROUND2(B, C, D, E, A, F3, 13, W[10],  3);
	ROUND2(A, B, C, D, E, F3, 13, W[ 0],  3);
	ROUND2(E, A, B, C, D, F3,  7, W[ 4],  3);
	ROUND2(D, E, A, B, C, F3,  5, W[13],  3);

	ROUND2(C, D, E, A, B, F2, 15, W[ 8],  4);
	ROUND2(B, C, D, E, A, F2,  5, W[ 6],  4);
	ROUND2(A, B, C, D, E, F2,  8, W[ 4],  4);
	ROUND2(E, A, B, C, D, F2, 11, W[ 1],  4);
	ROUND2(D, E, A, B, C, F2, 14, W[ 3],  4);
	ROUND2(C, D, E, A, B, F2, 14, W[11],  4);
	ROUND2(B, C, D, E, A, F2,  6, W[15],  4);
	ROUND2(A, B, C, D, E, F2, 14, W[ 0],  4);
	ROUND2(E, A, B, C, D, F2,  6, W[ 5],  4);
	ROUND2(D, E, A, B, C, F2,  9, W[12],  4);
	ROUND2(C, D, E, A, B, F2, 12, W[ 2],  4);
	ROUND2(B, C, D, E, A, F2,  9, W[13],  4);
	ROUND2(A, B, C, D, E, F2, 12, W[ 9],  4);
	ROUND2(E, A, B, C, D, F2,  5, W[ 7],  4);
	ROUND2(D, E, A, B, C, F2, 15, W[10],  4);
	ROUND2(C, D, E, A, B, F2,  8, W[14],  4);

	ROUND2(B, C, D, E, A, F1,  8, W[12],  5);
	ROUND2(A, B, C, D, E, F1,  5, W[15],  5);
	ROUND2(E, A, B, C, D, F1, 12, W[10],  5);
	ROUND2(D, E, A, B, C, F1,  9, W[ 4],  5);
	ROUND2(C, D, E, A, B, F1, 12, W[ 1],  5);
	ROUND2(B, C, D, E, A, F1,  5, W[ 5],  5);
	ROUND2(A, B, C, D, E, F1, 14, W[ 8],  5);
	ROUND2(E, A, B, C, D, F1,  6, W[ 7],  5);
	ROUND2(D, E, A, B, C, F1,  8, W[ 6],  5);
	ROUND2(C, D, E, A, B, F1, 13, W[ 2],  5);
	ROUND2(B, C, D, E, A, F1,  6, W[13],  5);
	ROUND2(A, B, C, D, E, F1,  5, W[14],  5);
	ROUND2(E, A, B, C, D, F1, 15, W[ 0],  5);
	ROUND2(D, E, A, B, C, F1, 13, W[ 3],  5);
	ROUND2(C, D, E, A, B, F1, 11, W[ 9],  5);
	ROUND2(B, C, D, E, A, F1, 11, W[11],  5);

	tmp = accu[1] + C1 + D2;
	accu[1] = accu[2] + D1 + E2;
	accu[2] = accu[3] + E1 + A2;
	accu[3] = accu[4] + A1 + B2;
	accu[4] = accu[0] + B1 + C2;
	accu[0] = tmp;

#endif

	size += NB;
}

static void last(const BYTE ba[NB],int nb)
{
	int i,j,k,l;
	BYTE temp[2*NB];

	size += nb;
	size *= 8;
	
	i = 0;
	while (i < nb)
		temp[i++] = *ba++;
	temp[i++] = 0x80u;
	l = 1 + ((i + SS) > NB);
	k = l*NB - SS;
	while (i < k)
		temp[i++] = 0u;
#if BYTE_ORDER == BIG_ENDIAN
	for (j = SS-1; j >= 0; --j)
#elif BYTE_ORDER == LITTLE_ENDIAN
	for (j = 0; j <= SS-1; ++j)
#else
	for (;;)
#endif
		temp[i++] = BA(&size)[j];
	if (i != NB*l)
		abort();
	next(temp);
	if (l > 1)
		next(temp+NB);
}

static void pok(char fn[])
{
	print(accu,NO,fn);
}

int main(int ac,char *av[])
{
	int an;
	BYTE ar[NB];

#if !ZERO
	FILE *ap;
	size_t rv;

	if (ac <= 1)
		return fprintf(stderr,"usage: %s <filename> ...\n",av[0]),
			EXIT_FAILURE;
	for (an = 1; an < ac; ++an) {
		ap = fopen(av[an],"rb");
		if (!ap) {
			perror(av[an]);
			continue;
		}
		
		init();
		while ((rv = fread(ar,1,NB,ap)) == NB)
			next(ar);
		last(ar,rv);

		if (ferror(ap))
			perror("read()");
		else
			pok(av[an]);
		if (fclose(ap))
			perror("close()");
	}
#else
	long al;
	char *ae;

	if (ac <= 1)
		return fprintf(stderr,"usage: %s <length> ...\n",av[0]),
			EXIT_FAILURE;
	for (an = 0; an < NB; ++an)
		ar[an] = 0;
	for (an = 1; an < ac; ++an) {
		#include <errno.h>

		errno = 0;
		al = strtol(av[an],&ae,10);
		if (errno || al < 0 || *ae || ae == av[an]) {
			if (!errno)
				errno = EINVAL;
			perror(av[an]);
			continue;
		}

		init();
		while (al >= NB) {
			al -= NB;
			next(ar);
		}
		last(ar,al);

		pok(av[an]);
	}
#endif
	return EXIT_SUCCESS;
}
