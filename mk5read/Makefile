SS=/usr/local/src/streamstor
SS_LIBS=-lssapi -lwdapi1150 -lrt
CPPFLAGS=-I${SS}/linux/include
LDFLAGS=-L${SS}/linux/lib/gcc_v4 -L${SS}/linux/driver/lib
CFLAGS=-Wall -g -O2

all: mk5read

mk5read: mk5read.c
	$(CXX) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -o $@ $< $(SS_LIBS)

vsnread: vsnread.c
	$(CXX) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -o $@ $< $(SS_LIBS)

