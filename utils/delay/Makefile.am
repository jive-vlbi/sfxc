CXX=g++

AM_CFLAGS   = -I@top_srcdir@/lib/vex_parser/install/include -I@top_srcdir@/lib -m32 -std=c99
AM_CXXFLAGS = -m32 -I@top_srcdir@/lib/vex_parser/install/include -I@top_srcdir@/lib -I@top_srcdir@/include
AM_LDFLAGS  = -m32
LDADD       = -L@top_srcdir@/lib/calc10/lib/ -L@top_srcdir@/lib/vex_parser/install/lib/ -lfcalc -lg2c -lvex_parser32

bin_PROGRAMS = generate_delay_model

generate_delay_model_SOURCES=\
  generate_delay_model.cc \
  generate_delay_model_calc.c

pkginclude_HEADERS = *.h
