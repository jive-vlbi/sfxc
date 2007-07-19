SUBDIRS = vex_parser

# disable all libraries for delmo
LIBS =

EXTRA_DIST = 

AM_CPPFLAGS = -L/usr/lib
AM_CXXFLAGS = -I../include

bin_PROGRAMS = delmo \
               create_delay_function \
               produce_fringe \
               analyse_fringes \
               analyse_mark4_file \
               print_headers \
               produce_html_plotpage \
               make_sfxc_experiment_makefile

create_delay_function_SOURCES = create_delay_function.cc
create_delay_function_LDFLAGS = -lgsl -lgslcblas

produce_fringe_SOURCES = produce_fringe.cc
produce_fringe_LDFLAGS = -lfftw3 -lm

print_headers_SOURCES = print_headers.cc \
  ../src/staPrms.cc ../src/genPrms.cc \
  ../src/runPrms.cc ../src/genFunctions.cc \
  ../src/Log_writer.cc ../src/Log_writer_cout.cc \
  ../src/Data_reader.cc ../src/Data_reader_file.cc \
  ../src/Channel_extractor_mark4.cc \
  ../src/utils.cc

analyse_fringes_SOURCES = analyse_fringes.cc
analyse_fringes_LDFLAGS = -lfftw3 -lm

analyse_mark4_file_SOURCES = \
  analyse_mark4_file.cc \
  ../src/staPrms.cc ../src/genPrms.cc ../src/runPrms.cc ../src/genFunctions.cc \
  ../src/Log_writer.cc ../src/Log_writer_cout.cc \
  ../src/Data_reader.cc ../src/Data_reader_file.cc \
  ../src/Channel_extractor_mark4.cc \
  ../src/utils.cc ../src/Bits_to_float_converter.cc

produce_html_plotpage_SOURCES = produce_html_plotpage.cc \
	../src/genPrms.cc ../src/staPrms.cc \
	../src/runPrms.cc ../src/genFunctions.cc \
	../src/Log_writer.cc ../src/Log_writer_cout.cc ../src/utils.cc \
	gnuplot_i.cc
produce_html_plotpage_LDFLAGS = -lfftw3 -lm

make_sfxc_experiment_makefile_SOURCES = make_sfxc_experiment_makefile.cc \
	@top_srcdir@/lib/vex_parser/vexplus.cc
make_sfxc_experiment_makefile_CXXFLAGS = -I@top_srcdir@/lib/vex_parser
make_sfxc_experiment_makefile_LDFLAGS = \
  @top_srcdir@/lib/vex_parser/libvex.a -lfl -ljson

delmo_SOURCES=delmo.c
delmo_CFLAGS =-I@top_srcdir@/lib/calc10/include -m32 -std=c99
delmo_LDFLAGS=-lg2c  -m32 -std=c99 @top_srcdir@/lib/calc10/lib/libfcalc.a
