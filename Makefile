CC=gcc
CFLAGS=-Wall -g

TGT=hasher basher washer

all:	$(TGT)

clean:
	rm -f $(TGT)
	rm -f *.o
