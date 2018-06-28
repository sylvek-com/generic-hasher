/*
 * MD4
 */
// permutations
{ // identity
 0, 1, 2, 3,
 4, 5, 6, 7,
 8, 9,10,11,
12,13,14,15,
}, // [i%16]
{ // transpose
 0, 4, 8,12,
 1, 5, 9,13,
 2, 6,10,14,
 3, 7,11,15,
}, // [((4*i)+(i/4))%16] or [rot(i,2)%16] or [(4.25*i)%16]
{ // swap rows 2&3 and columns 2&3
 0, 8, 4,12,
 2,10, 6,14,
 1, 9, 5,13,
 3,11, 7,15,
}, // [bitreverse(i)]
// rotations
{
 3, 7,11,19, // 0 4 4 8 = (2*i^3-6*i^2+16*i+9)/3
 3, 5, 9,13, // 0 2 4 4 = (-i^3+6*i^2+1*i+9)/3
 3, 9,11,15, // 0 6 2 4 = (i^3-5*i^2+10*i+3)
},
// constants
{
sqrt(0)*pow(2,30),
sqrt(2)*pow(2,30),
sqrt(3)*pow(2,30),
}
// initializers
{
#define B2W( a, b, c, d ) 0x ## d ## c ## b ## a ## u
B2W( 01, 23, 45, 67 ),
B2W( 89, AB, CD, EF ),
B2W( FE, DC, BA, 98 ),
B2W( 76, 54, 32, 10 ),
}
// functions
F = Mux(sel=x,one=y,zero=z)
G = Maj(x,y,z)
H = Par(x,y,z)
/*
 * MD5
 */
// permutations
{ // identity
 0, 1, 2, 3,
 4, 5, 6, 7,
 8, 9,10,11,
12,13,14,15,
}, // [i%16]
{ // rotate columms left, rotate 2nd column up 1 time, rotate 3rd column up 2 times
 1, 6,11, 0,
 5,10,15, 4,
 9,14, 3, 8,
13, 2, 7,12,
}, // [(5*i+1)%16]
{ // swap rows 1&2 and 3&4, swap columns 2&4, rotate 2&4 column down 1 time each
 5, 8,11,14,
 1, 4, 7,10,
13, 0, 3, 6,
 9,12,15, 2,
}, // [(3*i+5)%16]
{ // rotate columns left, rotate 1st column up 2 times, rotate 2nd column up 1 time
 0, 7,14, 5,
12, 3,10, 1,
 8,15, 6,13,
 4,11, 2, 9,
}, // [(7*i)%16]
// rotations
{
 7,12,17,22, // 0 5 5 5 = 5*i + 7
 5, 9,14,20, // 0 4 5 6 = 0.5*i*i + 3.5*i + 5 = 0.5*(i+7)*i + 5
 4,11,16,23, // 0 7 5 7 = (((2*i-9)*i+28)*i+12)/3 = (2*i^3-9*i^2+28*i+12)/3
 6,10,15,21, // 0 4 5 6 = 0.5*i*i + 3.5*i + 6 = i*(i+7)/2 + 6
},
// constants
for (i=1;i<=64;++i)
  (unsigned)abs(sin(i))*2^32;
// initializers
// same as MD4
// functions
F = Mux(sel=x,one=y,zero=z)
G = Mux(one=x,zero=y,sel=z)
H = Par(x,y,z)
I = y ^ (x | ~z)
000 1
001 0
010 0
011 1
100 1
101 1
110 0
111 0
