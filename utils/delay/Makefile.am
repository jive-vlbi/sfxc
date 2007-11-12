AM_CXXFLAGS = -I@top_srcdir@/include

# disable all libraries for delmo
LIBS =

bin_PROGRAMS = generate_delay_model

generate_delay_model_SOURCES=\
  generate_delay_model.cc \
  generate_delay_model_calc.c
generate_delay_model_CFLAGS  = -I@top_srcdir@/lib -m32 -std=c99
generate_delay_model_CXXFLAGS= -m32 -I@top_srcdir@/lib -I@top_srcdir@/include
generate_delay_model_LDFLAGS = -m32
generate_delay_model_LDADD   = -L@top_srcdir@/lib/calc10/lib/ -lfcalc -lg2c -lvex_parser32
