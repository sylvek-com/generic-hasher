#include <stdio.h>
#ifdef __x86_64__
#undef __USE_EXTERN_INLINES // fix for error in gcc's stdlib-float.h
#endif
#include <stdlib.h>

#ifdef __MMX__
#include <mmintrin.h>

#ifndef MADD
#define MADD 0
#endif
// various ways of avoiding the dependency on SSE2 paddq instruction
#define _mm_add_si64 madd
extern __inline __m64 __attribute__((__gnu_inline__,__always_inline__))
_mm_add_si64 (__m64 m1,__m64 m2)
{
#if MADD==1 // fastest, but cheating
	asm("paddq\t%1,%0"
	: "+y" (m1) // outputs
	: "ym" (m2) // inputs
	: // clobbers
	);
	return m1;
#elif MADD==2 // moderately fast
	__asm__("movd\t%0, %%eax\n\t"
		"movd\t%1, %%ebx\n\t"
		"punpckhdq %0, %0\n\t"
		"punpckhdq %1, %1\n\t"
		"movd\t%0, %%edx\n\t"
		"movd\t%1, %%ecx\n\t"
		"addl\t%%ebx, %%eax\n\t"
		"adcl\t%%ecx, %%edx\n\t"
		"movd\t%%eax, %0\n\t"
		"movd\t%%edx, %1\n\t"
		"punpckldq %1, %0"
	: "+y" (m1), "+y" (m2) // outputs
	: // inputs
	: "eax","ebx","ecx","edx" // clobbers
	);
	return m1;
#elif MADD==3 // slowest
	__m64 m0;
	__asm__("movl\t%1, %%eax\n\t"
		"addl\t%2, %%eax\n\t"
		"movl\t4+%1, %%edx\n\t"
		"adcl\t4+%2, %%edx\n\t"
		"movl\t%%eax, %0\n\t"
		"movl\t%%edx, 4+%0"
	: "=m" (m0) // outputs
	: "m" (m1), "m" (m2) // inputs
	: "eax","edx" // clobbers
	);
	return m0;
#elif MADD==4 // moderately slow
	__asm__("movl\t%1, %%eax\n\t"
		"addl\t%%eax, %0\n\t"
		"movl\t4+%1, %%edx\n\t"
		"adcl\t%%edx, 4+%0"
	: "+m" (m1) // outputs
	: "m" (m2) // inputs
	: "eax","edx" // clobbers
	);
	return m1;
#else // fastest without cheating
	return (__m64) ((long long)m1 + (long long)m2);
#endif
}
#ifndef __x86_64__
/* fix for errors & omissions in mmintrin.h */
/* Intel intrinsic.  */
extern __inline __m64  __attribute__((__gnu_inline__, __always_inline__, __artificial__))
_m_from_int64 (long long __i)
{
  return (__m64) __i;
}
/* Intel intrinsic.  */
extern __inline long long __attribute__((__gnu_inline__, __always_inline__, __artificial__))
_m_to_int64 (__m64 __i)
{
  return (long long)__i;
}
#endif
#endif

#ifndef SWAP /* endian-swap builtins */
#define SWAP 1
#endif

#ifndef STEP /* step-by-step tracing */
#define STEP 0
#endif

#ifndef ZERO /* benchmark with zeros */
#define ZERO 0
#endif

#define BYTE unsigned char
#define WORD unsigned long long
#define SIZE unsigned long long
#define SW (int)sizeof(WORD)
#define SS (int)sizeof(SIZE)
#define NI 16
#define NO 8
#define NS 80
#define NB NI*SW

static WORD accu[NO];

static SIZE size;

static WORD iv[NO] = {
0x6a09e667f3bcc908,
0xbb67ae8584caa73b,
0x3c6ef372fe94f82b,
0xa54ff53a5f1d36f1,
0x510e527fade682d1,
0x9b05688c2b3e6c1f,
0x1f83d9abfb41bd6b,
0x5be0cd19137e2179,
};

static WORD K[NS] = {
0x428a2f98d728ae22,0x7137449123ef65cd,0xb5c0fbcfec4d3b2f,0xe9b5dba58189dbbc,
0x3956c25bf348b538,0x59f111f1b605d019,0x923f82a4af194f9b,0xab1c5ed5da6d8118,
0xd807aa98a3030242,0x12835b0145706fbe,0x243185be4ee4b28c,0x550c7dc3d5ffb4e2,
0x72be5d74f27b896f,0x80deb1fe3b1696b1,0x9bdc06a725c71235,0xc19bf174cf692694,
0xe49b69c19ef14ad2,0xefbe4786384f25e3,0x0fc19dc68b8cd5b5,0x240ca1cc77ac9c65,
0x2de92c6f592b0275,0x4a7484aa6ea6e483,0x5cb0a9dcbd41fbd4,0x76f988da831153b5,
0x983e5152ee66dfab,0xa831c66d2db43210,0xb00327c898fb213f,0xbf597fc7beef0ee4,
0xc6e00bf33da88fc2,0xd5a79147930aa725,0x06ca6351e003826f,0x142929670a0e6e70,
0x27b70a8546d22ffc,0x2e1b21385c26c926,0x4d2c6dfc5ac42aed,0x53380d139d95b3df,
0x650a73548baf63de,0x766a0abb3c77b2a8,0x81c2c92e47edaee6,0x92722c851482353b,
0xa2bfe8a14cf10364,0xa81a664bbc423001,0xc24b8b70d0f89791,0xc76c51a30654be30,
0xd192e819d6ef5218,0xd69906245565a910,0xf40e35855771202a,0x106aa07032bbd1b8,
0x19a4c116b8d2d0c8,0x1e376c085141ab53,0x2748774cdf8eeb99,0x34b0bcb5e19b48a8,
0x391c0cb3c5c95a63,0x4ed8aa4ae3418acb,0x5b9cca4f7763e373,0x682e6ff3d6b2b8a3,
0x748f82ee5defb2fc,0x78a5636f43172f60,0x84c87814a1f0ab72,0x8cc702081a6439ec,
0x90befffa23631e28,0xa4506cebde82bde9,0xbef9a3f7b2c67915,0xc67178f2e372532b,
0xca273eceea26619c,0xd186b8c721c0c207,0xeada7dd6cde0eb1e,0xf57d4f7fee6ed178,
0x06f067aa72176fba,0x0a637dc5a2c898a6,0x113f9804bef90dae,0x1b710b35131c471b,
0x28db77f523047d84,0x32caab7b40c72493,0x3c9ebe0a15c9bebc,0x431d67c49c100d4c,
0x4cc5d4becb3e42b6,0x597f299cfc657e2a,0x5fcb6fab3ad6faec,0x6c44198c4a475817,
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

#if !__MMX__
#define ROTL(w,s) ((w) << (s) | (w) >> (64-s))
#define ROTR(w,s) ((w) >> (s) | (w) << (64-s))
#define SHR(w,s) ((w) >> (s))
#define CHO(x,y,z) ((x & y) | (~x & z))
#define MAJ(x,y,z) ((x & y) | (x & z) | (y & z))
#define BSIG0(x) (ROTR(x,28) ^ ROTR(x,34) ^ ROTR(x,39))
#define BSIG1(x) (ROTR(x,14) ^ ROTR(x,18) ^ ROTR(x,41))
#define SSIG0(x) (ROTR(x,1) ^ ROTR(x,8) ^ SHR(x,7))
#define SSIG1(x) (ROTR(x,19) ^ ROTR(x,61) ^ SHR(x,6))
#else
#define ROTR(w,s) _m_por(_m_psrlqi(w,s),_m_psllqi(w,64-s))
#define SHR(w,s) _m_psrlqi(w,s)
#define CHO(x,y,z) _m_por(_m_pand(x,y),_m_pandn(x,z))
#define MAJ(x,y,z) _m_por(_m_pand(x,y),_m_por(_m_pand(x,z),_m_pand(y,z)))
#define BSIG0(x) _m_pxor(ROTR(x,28),_m_pxor(ROTR(x,34),ROTR(x,39)))
#define BSIG1(x) _m_pxor(ROTR(x,14),_m_pxor(ROTR(x,18),ROTR(x,41)))
#define SSIG0(x) _m_pxor(ROTR(x,1),_m_pxor(ROTR(x,8),SHR(x,7)))
#define SSIG1(x) _m_pxor(ROTR(x,19),_m_pxor(ROTR(x,61),SHR(x,6)))
#endif

static void next(const BYTE ba[NB])
{
	int i,j;
#if !__MMX__
	WORD W[NS];
	WORD H[NO];
	WORD t1,t2;
#else
	__m64 W[NS];
	__m64 H[NO];
	__m64 t1,t2;
#endif

	for (i = 0; i < NI; ++i)
#if SWAP
		((WORD*)W)[i] = __builtin_bswap64(((WORD*)ba)[i]);
	(void)j;
#else
#if BYTE_ORDER == BIG_ENDIAN
		for (j = 0; j <= SW-1; ++j)
#elif BYTE_ORDER == LITTLE_ENDIAN
		for (j = SW-1; j >= 0; --j)
#else
		for (;;)
#endif
			BA(W)[i*SW+j] = *ba++;
#endif
#if STEP
	print(W,NI,"input");
#endif
#if !__MMX__
	for (i = NI; i < NS; ++i)
		W[i] = SSIG1(W[i-2]) + W[i-7] + SSIG0(W[i-15]) + W[i-16];
	for (i = 0; i < NO; ++i)
		H[i] = accu[i];
#else
	for (i = NI; i < NS; ++i)
		W[i] = _mm_add_si64(_mm_add_si64(SSIG1(W[i-2]),W[i-7]),_mm_add_si64(SSIG0(W[i-15]),W[i-16]));
	for (i = 0; i < NO; ++i)
		H[i] = _m_from_int64(accu[i]);
#endif
	for (i = 0; i < NS; ++i) {
#define a H[0]
#define b H[1]
#define c H[2]
#define d H[3]
#define e H[4]
#define f H[5]
#define g H[6]
#define h H[7]
#if !__MMX__
		t1 = h + BSIG1(e) + CHO(e,f,g) + K[i] + W[i];
		t2 = BSIG0(a) + MAJ(a,b,c);
#else
		t1 = _mm_add_si64(_mm_add_si64(h,BSIG1(e)),_mm_add_si64(CHO(e,f,g),_mm_add_si64(_m_from_int64(K[i]),W[i])));
		t2 = _mm_add_si64(BSIG0(a),MAJ(a,b,c));
#endif
		h = g;
		g = f;
		f = e;
#if !__MMX__
		e = d + t1;
#else
		e = _mm_add_si64(d,t1);
#endif
		d = c;
		c = b;
		b = a;
#if !__MMX__
		a = t1 + t2;
#else
		a = _mm_add_si64(t1,t2);
#endif
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
#if !__MMX__
	for (i = 0; i < NO; ++i)
		accu[i] += H[i];
#else
	for (i = 0; i < NO; ++i)
		accu[i] += _m_to_int64(H[i]);
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
