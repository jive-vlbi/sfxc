SUBDIRS = delay

AM_CXXFLAGS = $(SFXC_CXXFLAGS)
LDADD = $(SFXC_LDADD)

bin_PROGRAMS = create_delay_function \
               produce_fringe \
               analyse_fringes \
               print_headers \
               produce_html_plotpage \
               vex2ccf \
               generate_uvw_coordinates \
               plot_delay_table \
               produce_output_graphs \
               print_new_output_format \
               dnfp_fileserver \
               dnfp_fileclient

bin_SCRIPTS  = run_sfxc.py vumeter.py

create_delay_function_SOURCES = create_delay_function.cc

produce_fringe_SOURCES = produce_fringe.cc

dnfp_fileserver_SOURCES = \
  dnfp_fileserver.cc \
   ../src/data_reader.cc \
   ../src/data_writer.cc \
   ../src/data_reader_file.cc \
   ../src/data_writer_socket.cc \
   ../src/data_reader_socket.cc \
   ../src/data_reader_blocking.cc \
   ../src/utils.cc   
   
dnfp_fileclient_SOURCES = \
  dnfp_fileclient.cc \
   ../src/data_reader.cc \
   ../src/data_writer.cc \
   ../src/data_reader_file.cc \
   ../src/data_writer_socket.cc \
   ../src/data_reader_socket.cc \
   ../src/data_reader_blocking.cc  \
   ../src/utils.cc

print_headers_SOURCES = \
  print_headers.cc \
  ../src/log_writer.cc \
  ../src/log_writer_cout.cc \
  ../src/data_reader.cc \
  ../src/data_reader_file.cc \
  ../src/channel_extractor_mark4.cc \
  ../src/channel_extractor_brute_force.cc \
  ../src/utils.cc \
  ../src/control_parameters.cc

analyse_fringes_SOURCES = analyse_fringes.cc

produce_html_plotpage_SOURCES = \
  produce_html_plotpage.cc \
  ../src/control_parameters.cc \
  ../src/log_writer.cc \
  ../src/log_writer_cout.cc \
  ../src/utils.cc \
  ../src/output_header.cc \
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

produce_output_graphs_SOURCES = \
  produce_output_graphs.cc

pkginclude_HEADERS = *.h *.py

print_new_output_format_SOURCES = \
  print_new_output_format.cc \
  ../src/output_header.cc
