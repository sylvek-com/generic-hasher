#include <stdio.h>
#include <stdlib.h>

#define STEP 0
#define RIPE 0
#define BYTE unsigned char
#define WORD unsigned int
#define SIZE unsigned long long
#define SW sizeof(WORD)
#define SS sizeof(SIZE)
#define NI 16
#if RIPE==0 || RIPE==128
#define NO 4
#elif RIPE==160
#define NO 5
#else
#error unimplemented RIPE variant
#endif
#define NS 16

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

static void next(const BYTE ba[NI*SW])
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
#define F5(x, y, z)   ((x) ^ ((y) | ~(z)))

#if RIPE == 0
/*
 * RIPEMD (original hash, deprecated).
 */

#define FF1(A, B, C, D, X, s)   do { \
		WORD tmp = (A) + F(B, C, D) + (X); \
		(A) = ROTL(tmp, (s)); \
	} while (0)

#define GG1(A, B, C, D, X, s)   do { \
		WORD tmp = (A) + G(B, C, D) \
			+ (X) + (WORD)0x5A827999; \
		(A) = ROTL(tmp, (s)); \
	} while (0)

#define HH1(A, B, C, D, X, s)   do { \
		WORD tmp = (A) + H(B, C, D) \
			+ (X) + (WORD)0x6ED9EBA1; \
		(A) = ROTL(tmp, (s)); \
	} while (0)

#define FF2(A, B, C, D, X, s)   do { \
		WORD tmp = (A) + F(B, C, D) \
			+ (X) + (WORD)(0x50A28BE6); \
		(A) = ROTL(tmp, (s)); \
	} while (0)

#define GG2(A, B, C, D, X, s)   do { \
		WORD tmp = (A) + G(B, C, D) + (X); \
		(A) = ROTL(tmp, (s)); \
	} while (0)

#define HH2(A, B, C, D, X, s)   do { \
		WORD tmp = (A) + H(B, C, D) \
			+ (X) + (WORD)(0x5C4DD124); \
		(A) = ROTL(tmp, (s)); \
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
	
#elif RIPE == 128
#elif RIPE == 160
#endif

	size += NI*SW;
}

static void last(const BYTE ba[NI*SW],size_t nb)
{
	int i,j,k,l;
	BYTE temp[2*NI*SW];

	size += nb;
	size *= 8;
	
	i = 0;
	while (i < nb)
		temp[i++] = *ba++;
	temp[i++] = 0x80u;
	l = 1 + ((i + SS) > NI*SW);
	k = l*NI*SW - SS;
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
	if (i != NI*SW*l)
		abort();
	next(temp);
	if (l > 1)
		next(temp+NI*SW);
}

static void pok(char fn[])
{
	print(accu,NO,fn);
}

int main(int ac,char *av[])
{
	int an;
	FILE *ap;
	size_t rv;
	BYTE ar[NI*SW];

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
		while ((rv = fread(ar,1,sizeof ar,ap)) == sizeof ar)
			next(ar);
		last(ar,rv);

		if (ferror(ap))
			perror("read()");
		else
			pok(av[an]);
		if (fclose(ap))
			perror("close()");
	}
	return EXIT_SUCCESS;
}
