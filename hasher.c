#include <stdio.h>
#include <stdlib.h>

#define STEP 0
#define BYTE unsigned char
#define WORD unsigned int
#define SIZE unsigned long long
#define SW (int)sizeof(WORD)
#define SS (int)sizeof(SIZE)
#define NI 16
#define NO 5
#define NS 80
#define SHA0 0

static WORD accu[NO];

static SIZE size;

static WORD iv[NO] = {
0x67452301,
0xEFCDAB89,
0x98BADCFE,
0x10325476,
0xC3D2E1F0,
};

static WORD K[NS/20] = {
0x5A827999,
0x6ED9EBA1,
0x8F1BBCDC,
0xCA62C1D6,
};
 
#define BA(w) ((BYTE*)(w))

static void print(const WORD wa[],int nw,const char sa[])
{
	int i,j;

	for (i = 0; i < nw; ++i) {
#if BYTE_ORDER == BIG_ENDIAN
		for (j = 0; j <= SW-1; ++j)
#elif BYTE_ORDER == LITTLE_ENDIAN
		for (j = SW-1; j >= 0; --j)
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
	WORD W[NS];
	WORD H[NO];
	WORD t;

	for (i = 0; i < NI; ++i)
#if BYTE_ORDER == BIG_ENDIAN
		for (j = 0; j <= SW-1; ++j)
#elif BYTE_ORDER == LITTLE_ENDIAN
		for (j = SW-1; j >= 0; --j)
#else
		for (;;)
#endif
			BA(W)[i*SW+j] = *ba++;
#if STEP
	print(W,NI,"input");
#endif
	for (i = NI; i < NS; ++i)
#if SHA0
		W[i] = W[i-3] ^ W[i-8] ^ W[i-14] ^ W[i-16];
#else
		W[i] = ROTL(W[i-3] ^ W[i-8] ^ W[i-14] ^ W[i-16],1);
#endif
	for (i = 0; i < NO; ++i)
		H[i] = accu[i];
	for (i = 0; i < NS; ++i) {
#define A H[0]
#define B H[1]
#define C H[2]
#define D H[3]
#define E H[4]
		j = i/20;
		switch (j) {
		case 0: t = (B & C) | (~B & D); break; // Cho(B,C,D)
		case 3:
		case 1: t = B ^ C ^ D; break; // Par(B,C,D)
		case 2: t = (B & C) | (B & D) | (C & D); break; // Maj(B,C,D)
		default: abort();
		}
		t += ROTL(A,5) + E + W[i] + K[j];
		E = D;
		D = C;
#if 0
		C = ROTL(B,30);
#else
		C = ROTR(B,2);
#endif
		B = A;
		A = t;
#undef A
#undef B
#undef C
#undef D
#undef E
#if STEP
		print(H,NO,"round");
#endif
	}
	for (i = 0; i < NO; ++i)
		accu[i] += H[i];
	size += NI*SW;
}

static void last(const BYTE ba[NI*SW],int nb)
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
	for (j = 0; j <= SS-1; ++j)
#elif BYTE_ORDER == LITTLE_ENDIAN
	for (j = SS-1; j >= 0; --j)
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
