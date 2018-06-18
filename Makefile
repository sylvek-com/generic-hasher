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
CFLAGS=-Wall -Wextra -Werror -g

# general recipe for generating benchmarking
# that preserves the generated assembly file
# using the same suffix as the target
define BENCH.c
	$(LINK.c) $^ $(LOADLIBES) $(LDLIBS) -o $@
	$(RM) $(^:.c=.i) $(^:.c=.o)
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
%-5m:	TARGET_ARCH=-m32 -march=core2 -mmmx -msse2
%-5v:	TARGET_ARCH=-m32 -march=core2 -mno-mmx -mno-sse2
%-2m:	TARGET_ARCH=-m64 -march=core2 -mmmx -msse2
%-2v:	TARGET_ARCH=-m64 -march=core2 -mno-mmx -mno-sse2

#
# end of generic settings
#

#
# begin of specific rules
#

ALL=hasher basher washer masher lasher dasher

DB=dasher-5m dasher-5v dasher-2m dasher-2v
WB=washer-5m washer-5v washer-2m washer-2v

all:	$(ALL)

db:	$(DB)

wb:	$(WB)

$(DB):	CPPFLAGS+=-DZERO -DVECT
$(WB):	CPPFLAGS+=-DZERO
$(DB) $(WB):	CFLAGS+=-fverbose-asm -save-temps -g0 -Ofast

clean:
	$(RM) $(ALL) $(DB) $(WB)
	$(RM) *.i *.o $(DB:=.s) $(WB:=.s)

#
# end of specific rules
#
