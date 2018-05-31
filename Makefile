CC=gcc
CFLAGS=-Wall -g

TGT=hasher

all:	$(TGT)

clean:
	rm -f $(TGT)
	rm -f *.o
