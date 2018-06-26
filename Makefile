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

#
# begin gmake defaults
#
# default
LINK.c = $(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) $(TARGET_ARCH)
# recipe to execute (built-in):
%: %.c
	$(LINK.c) $^ $(LOADLIBES) $(LDLIBS) -o $@
#
# end gmake defaults
#

#
# begin generic settings
#

CC=gcc
RM=rm -f
MV=mv -f
CFLAGS=-Wall -Wextra -Wno-error -g

# general recipe for generating benchmarking
# that preserves the generated assembly file
# using the same suffix as the target
define BENCH.c
	$(LINK.c) $^ $(LOADLIBES) $(LDLIBS) -o $@
	$(RM) $(^:.c=.i) $(^:.c=.o) $(^:.c=.bc)
	$(MV) $(<:.c=.s) $@.s
endef
# bench-specific recipes
%-5v: %.c
	$(BENCH.c)
%-5m: %.c
	$(BENCH.c)
%-2m: %.c
	$(BENCH.c)
%-2v: %.c
	$(BENCH.c)
# bench-specific variables
%-5m:	TARGET_ARCH=-m32 -march=pentium2 -mmmx
%-5v:	TARGET_ARCH=-m32 -march=pentium2 -mno-mmx
%-2m:	TARGET_ARCH=-m64 -march=core2 -mmmx -msse -msse2
%-2v:	TARGET_ARCH=-m64 -march=core2 -mno-mmx -mno-sse -mno-sse2

#
# end of generic settings
#

#
# begin of specific rules
#

ALL=hasher basher washer masher lasher dasher

DB=dasher-5m dasher-5v dasher-2m dasher-2v
WB=washer-5m washer-5v washer-2m washer-2v
HB=hasher-5m hasher-5v hasher-2m hasher-2v
AH=ahasher ahasher-2

all:	$(ALL)

db:	$(DB)
wb:	$(WB)
hb:	$(HB)
ah:	$(AH)

$(DB):	CPPFLAGS+=-DZERO -DVECT
$(WB) $(HB):	CPPFLAGS+=-DZERO
$(DB) $(WB) $(HB):	CFLAGS+=-fverbose-asm -save-temps -g0 -Ofast

ahasher:	ahasher.c intel_sha1.o
ahasher-2:	CPPFLAGS+=-DZERO
ahasher-2:	CFLAGS+=-g0 -Ofast
ahasher-2:	ahasher.c intel_sha1.o
	$(LINK.c) $^ $(LOADLIBES) $(LDLIBS) -o $@
intel_sha1.o:	intel_sha1.asm
	nasm -felf64 -g -Fdwarf -o $@ -l ${@:.o=.lst} $<

clean:
	$(RM) $(ALL)
	$(RM) $(DB) $(WB)
	$(RM) $(HB) $(AH)
	$(RM) *.i *.bc *.s *.o *.lst

#
# end of specific rules
#
