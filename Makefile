#
# Generic Merkle-Damgard hashing codes
#
# hasher: SHA-{0,1}
# basher: SHA-256
# washer: SHA-512
# masher: MD4
# lasher: MD5
# dasher: RIPEMD-{0,128,160}
#
CC=gcc
CFLAGS=-Wall -Wextra -Werror -g -Og

TGT=hasher basher washer masher lasher dasher

all:	$(TGT)

clean:
	rm -f $(TGT)
	rm -f *.o
