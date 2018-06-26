#include <stdio.h>
#ifdef __x86_64__ // 64-bit compile with SSE and above disabled
#undef __USE_EXTERN_INLINES // fix for error in gcc's stdlib-float.h
#endif
#include <stdlib.h>

#ifndef ZERO /* benchmark */
#define ZERO 0
#endif

#define STEP 0
#define BYTE unsigned char
#define WORD unsigned int
#define SIZE unsigned long long
#define SW (int)sizeof(WORD)
#define SS (int)sizeof(SIZE)
#define NI 16
#define NO 5
#define NS 80
#define NB NI*SW
 
static WORD accu[NO];

static SIZE size;

static WORD iv[NO] = {
0x67452301,
0xEFCDAB89,
0x98BADCFE,
0x10325476,
0xC3D2E1F0,
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

#ifndef INTEL_SHA1_SINGLEBLOCK
// Updates 20-byte SHA-1 record in 'hash' for 'num_blocks' consequtive 64-byte blocks
extern void sha1_update_intel(WORD *hash, const BYTE* input, size_t num_blocks );
#else
// Updates 20-byte SHA-1 record in 'hash' for one 64-byte block pointed by 'input'
extern void sha1_update_intel(WORD *hash, const BYTE* input);
#endif

static void next(const BYTE ba[NB])
{
#ifndef INTEL_SHA1_SINGLEBLOCK
	sha1_update_intel(accu,ba,1);
#else
	sha1_update_intel(accu,ba);
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
	for (j = 0; j <= SS-1; ++j)
#elif BYTE_ORDER == LITTLE_ENDIAN
	for (j = SS-1; j >= 0; --j)
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
