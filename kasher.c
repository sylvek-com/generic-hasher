#include <stdio.h>
#include <stdlib.h>

#define STEP 1
#define BYTE unsigned char
#define WORD unsigned int
#define SIZE unsigned long long
#define SW (int)sizeof(WORD)
#define SS (int)sizeof(SIZE)
#define NI 16
#define NO 4
#define NB NI*SW
#define NR 3*NI+NO

static WORD accu[NO];

static SIZE size;

static const WORD iv[NO] = {
#define B2W( a, b, c, d ) 0x ## d ## c ## b ## a ## u
B2W( 01, 23, 45, 67 ),
B2W( 89, AB, CD, EF ),
B2W( FE, DC, BA, 98 ),
B2W( 76, 54, 32, 10 ),
#undef B2W
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

#define KF 0x00000000u
#define KG 0x5A827999u
#define KH 0x6ED9EBA1u

#define PF(i) (i)
#define PG(i) (((i&12) >> 2) | ((i&03) << 2))
#define PH(i) (((i&8) >> 3) | ((i&4) >> 1) | ((i&2) << 1) | ((i&1) << 3))

#define RF(i) (((2*i-6)*i+16)*i+9)/3
#define RG(i) (((-i+6)*i+1)*i+9)/3
#define RH(i) (((i-5)*i+10)*i+3)

#define F(x,y,z) OR(AND(x,y),ANDNOT(x,z)) // Mux
#define G(x,y,z) OR(AND(x,y),OR(AND(y,z),AND(x,z))) // Maj
#define H(x,y,z) XOR(x,XOR(y,z)) // Par

#define OR(l,r) ((l) | (r))
#define XOR(l,r) ((l) ^ (r))
#define AND(l,r) ((l) & (r))
#define ANDNOT(l,r) ((l) | ~(r))
#define ADD(l,r) ((l) + (r))

#define ROTL(w,s) ((w) << (s) | (w) >> (32-s))

static void next(const BYTE ba[NB])
{
	int i,j;
	WORD W[NI];
	WORD H[NO];
	WORD R[NR];

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
		H[i] = accu[i],
		R[i] = H[i];
	for (i = 0*NI; i < 1*NI; ++i) {
		R[i+4] = ADD(R[i],ADD(F(R[i+3],R[i+2],R[i+1]),ADD(W[PF(i)],KF)));
		R[i+4] = ROTL(R[i+4],RF(i%4));
#if STEP
		print(&R[i+4],NO,"F-round");
#endif
	}
	for (i = 1*NI; i < 2*NI; ++i) {
		R[i+4] = ADD(R[i],ADD(G(R[i+3],R[i+2],R[i+1]),ADD(W[PG(i)],KG)));
		R[i+4] = ROTL(R[i+4],RG(i%4));
#if STEP
		print(&R[i+4],NO,"G-round");
#endif
	}
	for (i = 2*NI; i < 3*NI; ++i) {
		R[i+4] = ADD(R[i],ADD(H(R[i+3],R[i+2],R[i+1]),ADD(W[PH(i)],KH)));
		R[i+4] = ROTL(R[i+4],RH(i%4));
#if STEP
		print(&R[i+4],NO,"H-round");
#endif
	}
	for (i = 0; i < NO; ++i)
		H[i] = R[3*NI+i],
		accu[i] += H[i];
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

static void init(void)
{
	int i;

	for (i = 0; i < NO; ++i)
		accu[i] = iv[i];
	size = 0ull;
#if STEP
	for (i = 0; i < NI; ++i)
		printf("%2d,",PF(i));
	puts("");
	for (i = 0; i < NI; ++i)
		printf("%2d,",PG(i));
	puts("");
	for (i = 0; i < NI; ++i)
		printf("%2d,",PH(i));
	puts("");
	for (i = 0; i < 4; ++i)
		printf("%2d,",RF(i));
	puts("");
	for (i = 0; i < 4; ++i)
		printf("%2d,",RG(i));
	puts("");
	for (i = 0; i < 4; ++i)
		printf("%2d,",RH(i));
	puts("");
#endif
}

static void pok(const char fn[])
{
	print(accu,NO,fn);
}

int main(int ac,char *av[])
{
	int an;
	FILE *ap;
	size_t rv;
	BYTE ar[NB];

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
