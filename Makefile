CC=gcc
CFLAGS=-Wall -g

TGT=hasher basher washer masher lasher dasher

all:	$(TGT)

clean:
	rm -f $(TGT)
	rm -f *.o
