SUBDIRS = vex_parser

# disable all libraries for delmo
LIBS =

AM_CXXFLAGS = -I@top_srcdir@/lib/vex_parser/ -I@top_srcdir@/include

bin_PROGRAMS = generate_delay_model \
               create_delay_function \
               produce_fringe \
               analyse_fringes \
               analyse_mark4_file \
               print_headers \
               produce_html_plotpage \
               make_sfxc_experiment_makefile \
               vex2ccf

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
  @top_srcdir@/lib/vex_parser/libvex.a @LEXLIB@ -ljson


generate_delay_model_SOURCES=\
  generate_delay_model.cc \
  generate_delay_model_calc.c \
  @top_srcdir@/lib/vex_parser/vexplus.cc \
  @top_srcdir@/lib/vex_parser/vex.c \
  @top_srcdir@/lib/vex_parser/vex.yy.c \
  @top_srcdir@/lib/vex_parser/vex_util.c \
  @top_srcdir@/lib/vex_parser/vex_get.c
generate_delay_model_CFLAGS =-I@top_srcdir@/lib -m32 -std=c99
generate_delay_model_CXXFLAGS =-I@top_srcdir@/lib -m32 -I@top_srcdir@/lib/vex_parser -I@top_srcdir@/include 
generate_delay_model_LDFLAGS=-m32 @top_srcdir@/lib/calc10/lib/libfcalc.a -lg2c 

vex2ccf_SOURCES=vex2ccf.cc
vex2ccf_LDFLAGS=-lvex_parser -lantlr3c -ljson
