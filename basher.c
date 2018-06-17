#include <stdio.h>
#include <stdlib.h>

#define STEP 0
#define BYTE unsigned char
#define WORD unsigned int
#define SIZE unsigned long long
#define SW (int)sizeof(WORD)
#define SS (int)sizeof(SIZE)
#define NI 16
#define NO 8
#define NS 64

static WORD accu[NO];

static SIZE size;

static WORD iv[NO] = {
0x6a09e667,
0xbb67ae85,
0x3c6ef372,
0xa54ff53a,
0x510e527f,
0x9b05688c,
0x1f83d9ab,
0x5be0cd19,
};

static WORD K[NS] = {
0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,
0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,
0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,
0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,
0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,
0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,
0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,
0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,
0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2,
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
#define SHR(w,s) ((w) >> (s))
#define CHO(x,y,z) ((x & y) | (~x & z))
#define MAJ(x,y,z) ((x & y) | (x & z) | (y & z))
#define BSIG0(x) (ROTR(x,2) ^ ROTR(x,13) ^ ROTR(x,22))
#define BSIG1(x) (ROTR(x,6) ^ ROTR(x,11) ^ ROTR(x,25))
#define SSIG0(x) (ROTR(x,7) ^ ROTR(x,18) ^ SHR(x,3))
#define SSIG1(x) (ROTR(x,17) ^ ROTR(x,19) ^ SHR(x,10))

static void next(const BYTE ba[NI*SW])
{
	int i,j;
	WORD W[NS];
	WORD H[NO];
	WORD t1,t2;

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
		W[i] = SSIG1(W[i-2]) + W[i-7] + SSIG0(W[i-15]) + W[i-16];
	for (i = 0; i < NO; ++i)
		H[i] = accu[i];
	for (i = 0; i < NS; ++i) {
#define a H[0]
#define b H[1]
#define c H[2]
#define d H[3]
#define e H[4]
#define f H[5]
#define g H[6]
#define h H[7]
		t1 = h + BSIG1(e) + CHO(e,f,g) + K[i] + W[i];
		t2 = BSIG0(a) + MAJ(a,b,c);
		h = g;
		g = f;
		f = e;
		e = d + t1;
		d = c;
		c = b;
		b = a;
		a = t1 + t2;
#undef a
#undef b
#undef c
#undef d
#undef e
#undef f
#undef g
#undef h
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
