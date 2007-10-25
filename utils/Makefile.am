SUBDIRS = vex_parser

# disable all libraries for delmo
LIBS =

AM_CXXFLAGS = -I@top_srcdir@/lib/vex_parser/ -I@top_srcdir@/include

bin_PROGRAMS = generate_delay_model \
               create_delay_function \
               produce_fringe \
               analyse_fringes \
               print_headers \
               produce_html_plotpage \
               make_sfxc_experiment_makefile \
               vex2ccf \
	       generate_uvw_coordinates \
	       plot_delay_table

bin_SCRIPTS= run_sfxc.py

create_delay_function_SOURCES = create_delay_function.cc
create_delay_function_LDADD = -lgsl -lgslcblas

produce_fringe_SOURCES = produce_fringe.cc
produce_fringe_LDADD = -lfftw3 -lm

print_headers_SOURCES = print_headers.cc \
  ../src/Log_writer.cc ../src/Log_writer_cout.cc \
  ../src/Data_reader.cc ../src/Data_reader_file.cc \
  ../src/Channel_extractor_mark4.cc \
  ../src/utils.cc ../src/Control_parameters.cc
print_headers_LDADD = \
  -lmpich -lvex_parser -lantlr3c -ljson

analyse_fringes_SOURCES = analyse_fringes.cc
analyse_fringes_LDADD = -lfftw3 -lm

produce_html_plotpage_SOURCES = produce_html_plotpage.cc \
	../src/Control_parameters.cc \
	../src/Log_writer.cc ../src/Log_writer_cout.cc ../src/utils.cc \
	gnuplot_i.cc
produce_html_plotpage_LDADD= -lfftw3 -lm -lvex_parser -lantlr3c -ljson -lmpich
produce_html_plotpage_CFLAGS =-I@top_srcdir@/lib
produce_html_plotpage_CXXFLAGS =-I@top_srcdir@/lib -I@top_srcdir@/lib/vex_parser -I@top_srcdir@/include 

make_sfxc_experiment_makefile_SOURCES = make_sfxc_experiment_makefile.cc \
	@top_srcdir@/lib/vex_parser/vexplus.cc
make_sfxc_experiment_makefile_CXXFLAGS = -I@top_srcdir@/lib/vex_parser
make_sfxc_experiment_makefile_LDADD = \
  @top_srcdir@/lib/vex_parser/libvex.a @LEXLIB@ -ljson


generate_delay_model_SOURCES=\
  generate_delay_model.cc \
  generate_delay_model_calc.c
generate_delay_model_CFLAGS  = -I@top_srcdir@/lib -m32 -std=c99
generate_delay_model_CXXFLAGS=-m32 -I@top_srcdir@/lib -I@top_srcdir@/lib/vex_parser -I@top_srcdir@/include 
generate_delay_model_LDFLAGS=-m32
generate_delay_model_LDADD = -L@top_srcdir@/lib/calc10/lib/ -lfcalc -lg2c

vex2ccf_SOURCES=vex2ccf.cc
vex2ccf_LDADD=-lvex_parser -lantlr3c -ljson

generate_uvw_coordinates_SOURCES=generate_uvw_coordinates.cc \
	../src/Uvw_model.cc
generate_uvw_coordinates_LDADD=-lgsl -lgslcblas

plot_delay_table_SOURCES=plot_delay_table.cc ../src/Delay_table_akima.cc ../src/utils.cc
plot_delay_table_CXXFLAGS =-I@top_srcdir@/include 
plot_delay_table_LDADD = -lgsl -lgslcblas -lmpich
