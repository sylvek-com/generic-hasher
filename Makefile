CC=gcc
CFLAGS=-Wall -g

TGT=hasher basher

all:	$(TGT)

clean:
	rm -f $(TGT)
	rm -f *.o
