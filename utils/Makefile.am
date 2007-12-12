SUBDIRS = delay

AM_CXXFLAGS = \
  -I@top_srcdir@/include \
  -I@top_srcdir@/lib/common/src \
  -I@top_srcdir@/lib/containers/src \
  -I@top_srcdir@/lib/netlib/src \
  -I@top_srcdir@/lib/testunit/src \
  -I@top_srcdir@/lib/jsoncpp/include \
  -I@top_srcdir@/lib/gsl/install/include \
  -I@top_srcdir@/lib/vex_parser/install/include
LDADD = \
  -L@top_srcdir@/lib/testunit -ltestunit \
  -L@top_srcdir@/lib/netlib -lnetlib \
  -L@top_srcdir@/lib/containers -lcontainers \
  -L@top_srcdir@/lib/common -lcommon \
  -L@top_srcdir@/lib/jsoncpp -ljsoncpp \
  -L@top_srcdir@/lib/gsl/install/lib -lgsl -lgslcblas \
  -L@top_srcdir@/lib/vex_parser/install/lib -lvex_parser

bin_PROGRAMS = create_delay_function \
               produce_fringe \
               analyse_fringes \
               print_headers \
               produce_html_plotpage \
               vex2ccf \
	       generate_uvw_coordinates \
	       plot_delay_table

bin_SCRIPTS  = run_sfxc.py

create_delay_function_SOURCES = create_delay_function.cc

produce_fringe_SOURCES = produce_fringe.cc

print_headers_SOURCES = \
  print_headers.cc \
  ../src/log_writer.cc \
  ../src/log_writer_cout.cc \
  ../src/data_reader.cc \
  ../src/data_reader_file.cc \
  ../src/channel_extractor_mark4.cc \
  ../src/utils.cc \
  ../src/control_parameters.cc

analyse_fringes_SOURCES = analyse_fringes.cc

produce_html_plotpage_SOURCES = \
  produce_html_plotpage.cc \
  ../src/control_parameters.cc \
  ../src/log_writer.cc \
  ../src/log_writer_cout.cc \
  ../src/utils.cc \
  gnuplot_i.cc

vex2ccf_SOURCES = \
  vex2ccf.cc \
  ../src/utils.cc

generate_uvw_coordinates_SOURCES = \
  generate_uvw_coordinates.cc \
  ../src/uvw_model.cc \
  ../src/utils.cc

plot_delay_table_SOURCES = \
  plot_delay_table.cc \
  ../src/delay_table_akima.cc \
  ../src/utils.cc
