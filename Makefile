CC=gcc
CFLAGS=-Wall -g

TGT=hasher basher washer masher

all:	$(TGT)

clean:
	rm -f $(TGT)
	rm -f *.o
