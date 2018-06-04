#include <stdio.h>
#include <stdlib.h>

#define STEP 0
#define BYTE unsigned char
#define WORD unsigned int
#define SIZE unsigned long long
#define SW sizeof(WORD)
#define SS sizeof(SIZE)
#define NI 16
#define NO 4
#define NS 16

static WORD accu[NO];

static SIZE size;

static WORD iv[NO] = {
0x67452301,
0xefcdab89,
0x98badcfe,
0x10325476,
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
	WORD H[NO];

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
	for (i = 0; i < NO; ++i)
		H[i] = accu[i];
#define a H[0]
#define b H[1]
#define c H[2]
#define d H[3]
#define x W
#if 0

   This step uses a 64-element table T[1 ... 64] constructed from the
   sine function. Let T[i] denote the i-th element of the table, which
   is equal to the integer part of 4294967296 times abs(sin(i)), where i
   is in radians. The elements of the table are given in the appendix.

     /* Round 1. */
     /* Let [abcd k s i] denote the operation
          a = b + ((a + F(b,c,d) + X[k] + T[i]) <<< s). */
     /* Do the following 16 operations. */
     [ABCD  0  7  1]  [DABC  1 12  2]  [CDAB  2 17  3]  [BCDA  3 22  4]
     [ABCD  4  7  5]  [DABC  5 12  6]  [CDAB  6 17  7]  [BCDA  7 22  8]
     [ABCD  8  7  9]  [DABC  9 12 10]  [CDAB 10 17 11]  [BCDA 11 22 12]
     [ABCD 12  7 13]  [DABC 13 12 14]  [CDAB 14 17 15]  [BCDA 15 22 16]

     /* Round 2. */
     /* Let [abcd k s i] denote the operation
          a = b + ((a + G(b,c,d) + X[k] + T[i]) <<< s). */
     /* Do the following 16 operations. */
     [ABCD  1  5 17]  [DABC  6  9 18]  [CDAB 11 14 19]  [BCDA  0 20 20]
     [ABCD  5  5 21]  [DABC 10  9 22]  [CDAB 15 14 23]  [BCDA  4 20 24]
     [ABCD  9  5 25]  [DABC 14  9 26]  [CDAB  3 14 27]  [BCDA  8 20 28]
     [ABCD 13  5 29]  [DABC  2  9 30]  [CDAB  7 14 31]  [BCDA 12 20 32]

     /* Round 3. */
     /* Let [abcd k s t] denote the operation
          a = b + ((a + H(b,c,d) + X[k] + T[i]) <<< s). */
     /* Do the following 16 operations. */
     [ABCD  5  4 33]  [DABC  8 11 34]  [CDAB 11 16 35]  [BCDA 14 23 36]
     [ABCD  1  4 37]  [DABC  4 11 38]  [CDAB  7 16 39]  [BCDA 10 23 40]
     [ABCD 13  4 41]  [DABC  0 11 42]  [CDAB  3 16 43]  [BCDA  6 23 44]
     [ABCD  9  4 45]  [DABC 12 11 46]  [CDAB 15 16 47]  [BCDA  2 23 48]

     /* Round 4. */
     /* Let [abcd k s t] denote the operation
          a = b + ((a + I(b,c,d) + X[k] + T[i]) <<< s). */
     /* Do the following 16 operations. */
     [ABCD  0  6 49]  [DABC  7 10 50]  [CDAB 14 15 51]  [BCDA  5 21 52]
     [ABCD 12  6 53]  [DABC  3 10 54]  [CDAB 10 15 55]  [BCDA  1 21 56]
     [ABCD  8  6 57]  [DABC 15 10 58]  [CDAB  6 15 59]  [BCDA 13 21 60]
     [ABCD  4  6 61]  [DABC 11 10 62]  [CDAB  2 15 63]  [BCDA  9 21 64]

#else

/*
 */ 
#define S11 7
#define S12 12
#define S13 17
#define S14 22
#define S21 5
#define S22 9
#define S23 14
#define S24 20
#define S31 4
#define S32 11
#define S33 16
#define S34 23
#define S41 6
#define S42 10
#define S43 15
#define S44 21

/* F, G, H and I are basic MD5 functions.
 */
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))

/* ROTATE_LEFT rotates x left n bits.
 */
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))

/* FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4.
Rotation is separate from addition to prevent recomputation.
 */
#define FF(a, b, c, d, x, s, ac) { \
 (a) += F ((b), (c), (d)) + (x) + (ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
  }
#define GG(a, b, c, d, x, s, ac) { \
 (a) += G ((b), (c), (d)) + (x) + (ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
  }
#define HH(a, b, c, d, x, s, ac) { \
 (a) += H ((b), (c), (d)) + (x) + (ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
  }
#define II(a, b, c, d, x, s, ac) { \
 (a) += I ((b), (c), (d)) + (x) + (ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
  }

  /* Round 1 */
  FF (a, b, c, d, x[ 0], S11, 0xd76aa478u); /* 1 */
  FF (d, a, b, c, x[ 1], S12, 0xe8c7b756u); /* 2 */
  FF (c, d, a, b, x[ 2], S13, 0x242070dbu); /* 3 */
  FF (b, c, d, a, x[ 3], S14, 0xc1bdceeeu); /* 4 */
  FF (a, b, c, d, x[ 4], S11, 0xf57c0fafu); /* 5 */
  FF (d, a, b, c, x[ 5], S12, 0x4787c62au); /* 6 */
  FF (c, d, a, b, x[ 6], S13, 0xa8304613u); /* 7 */
  FF (b, c, d, a, x[ 7], S14, 0xfd469501u); /* 8 */
  FF (a, b, c, d, x[ 8], S11, 0x698098d8u); /* 9 */
  FF (d, a, b, c, x[ 9], S12, 0x8b44f7afu); /* 10 */
  FF (c, d, a, b, x[10], S13, 0xffff5bb1u); /* 11 */
  FF (b, c, d, a, x[11], S14, 0x895cd7beu); /* 12 */
  FF (a, b, c, d, x[12], S11, 0x6b901122u); /* 13 */
  FF (d, a, b, c, x[13], S12, 0xfd987193u); /* 14 */
  FF (c, d, a, b, x[14], S13, 0xa679438eu); /* 15 */
  FF (b, c, d, a, x[15], S14, 0x49b40821u); /* 16 */

  /* Round 2 */
  GG (a, b, c, d, x[ 1], S21, 0xf61e2562u); /* 17 */
  GG (d, a, b, c, x[ 6], S22, 0xc040b340u); /* 18 */
  GG (c, d, a, b, x[11], S23, 0x265e5a51u); /* 19 */
  GG (b, c, d, a, x[ 0], S24, 0xe9b6c7aau); /* 20 */
  GG (a, b, c, d, x[ 5], S21, 0xd62f105du); /* 21 */
  GG (d, a, b, c, x[10], S22,  0x2441453u); /* 22 */
  GG (c, d, a, b, x[15], S23, 0xd8a1e681u); /* 23 */
  GG (b, c, d, a, x[ 4], S24, 0xe7d3fbc8u); /* 24 */
  GG (a, b, c, d, x[ 9], S21, 0x21e1cde6u); /* 25 */
  GG (d, a, b, c, x[14], S22, 0xc33707d6u); /* 26 */
  GG (c, d, a, b, x[ 3], S23, 0xf4d50d87u); /* 27 */
  GG (b, c, d, a, x[ 8], S24, 0x455a14edu); /* 28 */
  GG (a, b, c, d, x[13], S21, 0xa9e3e905u); /* 29 */
  GG (d, a, b, c, x[ 2], S22, 0xfcefa3f8u); /* 30 */
  GG (c, d, a, b, x[ 7], S23, 0x676f02d9u); /* 31 */
  GG (b, c, d, a, x[12], S24, 0x8d2a4c8au); /* 32 */

  /* Round 3 */
  HH (a, b, c, d, x[ 5], S31, 0xfffa3942u); /* 33 */
  HH (d, a, b, c, x[ 8], S32, 0x8771f681u); /* 34 */
  HH (c, d, a, b, x[11], S33, 0x6d9d6122u); /* 35 */
  HH (b, c, d, a, x[14], S34, 0xfde5380cu); /* 36 */
  HH (a, b, c, d, x[ 1], S31, 0xa4beea44u); /* 37 */
  HH (d, a, b, c, x[ 4], S32, 0x4bdecfa9u); /* 38 */
  HH (c, d, a, b, x[ 7], S33, 0xf6bb4b60u); /* 39 */
  HH (b, c, d, a, x[10], S34, 0xbebfbc70u); /* 40 */
  HH (a, b, c, d, x[13], S31, 0x289b7ec6u); /* 41 */
  HH (d, a, b, c, x[ 0], S32, 0xeaa127fau); /* 42 */
  HH (c, d, a, b, x[ 3], S33, 0xd4ef3085u); /* 43 */
  HH (b, c, d, a, x[ 6], S34,  0x4881d05u); /* 44 */
  HH (a, b, c, d, x[ 9], S31, 0xd9d4d039u); /* 45 */
  HH (d, a, b, c, x[12], S32, 0xe6db99e5u); /* 46 */
  HH (c, d, a, b, x[15], S33, 0x1fa27cf8u); /* 47 */
  HH (b, c, d, a, x[ 2], S34, 0xc4ac5665u); /* 48 */

  /* Round 4 */
  II (a, b, c, d, x[ 0], S41, 0xf4292244u); /* 49 */
  II (d, a, b, c, x[ 7], S42, 0x432aff97u); /* 50 */
  II (c, d, a, b, x[14], S43, 0xab9423a7u); /* 51 */
  II (b, c, d, a, x[ 5], S44, 0xfc93a039u); /* 52 */
  II (a, b, c, d, x[12], S41, 0x655b59c3u); /* 53 */
  II (d, a, b, c, x[ 3], S42, 0x8f0ccc92u); /* 54 */
  II (c, d, a, b, x[10], S43, 0xffeff47du); /* 55 */
  II (b, c, d, a, x[ 1], S44, 0x85845dd1u); /* 56 */
  II (a, b, c, d, x[ 8], S41, 0x6fa87e4fu); /* 57 */
  II (d, a, b, c, x[15], S42, 0xfe2ce6e0u); /* 58 */
  II (c, d, a, b, x[ 6], S43, 0xa3014314u); /* 59 */
  II (b, c, d, a, x[13], S44, 0x4e0811a1u); /* 60 */
  II (a, b, c, d, x[ 4], S41, 0xf7537e82u); /* 61 */
  II (d, a, b, c, x[11], S42, 0xbd3af235u); /* 62 */
  II (c, d, a, b, x[ 2], S43, 0x2ad7d2bbu); /* 63 */
  II (b, c, d, a, x[ 9], S44, 0xeb86d391u); /* 64 */

#endif

#undef a
#undef b
#undef c
#undef d
#undef x

#if STEP
		print(H,NO,"round");
#endif
	for (i = 0; i < NO; ++i)
		accu[i] += H[i];
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
