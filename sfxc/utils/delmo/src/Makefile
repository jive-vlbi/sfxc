CC =		gcc -m32 -std=c99
CPPFLAGS = 	-I../include
CFLAGS = 	-Wall

OBJS =		calc.o delmo.o

LIBS =		../lib/libfcalc.a -lg2c

all: calc

calc: calc.o
	$(CC) -o calc calc.o $(LIBS)

delmo: delmo.o
	$(CC) -o delmo delmo.o $(LIBS)


clean:
	-rm $(OBJS)
	-rm calc
	-rm delmo
  
