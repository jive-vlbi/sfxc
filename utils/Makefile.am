EXTRA_DIST = 

AM_CPPFLAGS = -L/usr/lib
AM_CXXFLAGS = -I../include

bin_PROGRAMS = vex_parser/libvex.a \
               vex_parser/sfxc_vex2ccf \
               vex_parser/sfxc_vex2dcf \
               delmo/src/delmo \
               create_delay_function \
               produce_fringe \
               analyse_fringes \
               analyse_mark4_file \
               print_headers \
               produce_html_plotpage \
               make_sfxc_experiment_makefile

vex_parser/libvex.a$(EXEEXT):
	cd vex_parser; make

vex_parser/sfxc_vex2ccf$(EXEEXT): vex_parser/libvex.a
	cd vex_parser; make -f Makefile.v2c sfxc_vex2ccf

vex_parser/sfxc_vex2dcf$(EXEEXT): vex_parser/libvex.a
	cd vex_parser; make -f Makefile.v2c sfxc_vex2dcf

delmo/src/delmo$(EXEEXT):
	cd delmo/lib; make
	cd delmo/src; make delmo

create_delay_function_SOURCES = create_delay_function.cc
create_delay_function_LDFLAGS = -lgsl -lgslcblas

produce_fringe_SOURCES = produce_fringe.cc

print_headers_SOURCES = print_headers.cc \
  ../src/staPrms.cc ../src/genPrms.cc \
  ../src/runPrms.cc ../src/genFunctions.cc \
  ../src/Log_writer.cc ../src/Log_writer_cout.cc \
  ../src/Data_reader.cc ../src/Data_reader_file.cc \
  ../src/Channel_extractor_mark4.cc \
  ../src/utils.cc

analyse_fringes_SOURCES = analyse_fringes.cc

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

make_sfxc_experiment_makefile_SOURCES = make_sfxc_experiment_makefile.cc \
	vex_parser/vexplus.cc
make_sfxc_experiment_makefile_LDFLAGS = -Lvex_parser -lvex -lfl -ljson

