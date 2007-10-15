# Don't link with the fftw3 library
LIBS = 


bin_PROGRAMS = calc

calc_SOURCES = calc.c
calc_CFLAGS  = -I../include -m32 -std=c99
calc_LDFLAGS = -m32
calc_LDADD = -lg2c -L../lib -lfcalc
