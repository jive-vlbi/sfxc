SUBDIRS = delay

AM_CXXFLAGS = -I@top_srcdir@/include

bin_PROGRAMS = create_delay_function \
               produce_fringe \
               analyse_fringes \
               print_headers \
               produce_html_plotpage \
               vex2ccf \
	       generate_uvw_coordinates \
	       plot_delay_table \
               produce_output_graphs

bin_SCRIPTS  = run_sfxc.py

create_delay_function_SOURCES = create_delay_function.cc

produce_fringe_SOURCES = produce_fringe.cc

print_headers_SOURCES = \
  print_headers.cc \
  ../src/Log_writer.cc \
  ../src/Log_writer_cout.cc \
  ../src/Data_reader.cc \
  ../src/Data_reader_file.cc \
  ../src/Channel_extractor_mark4.cc \
  ../src/utils.cc \
  ../src/Control_parameters.cc

analyse_fringes_SOURCES = analyse_fringes.cc

produce_html_plotpage_SOURCES = \
  produce_html_plotpage.cc \
  ../src/Control_parameters.cc \
  ../src/Log_writer.cc \
  ../src/Log_writer_cout.cc \
  ../src/utils.cc \
  gnuplot_i.cc

vex2ccf_SOURCES = \
  vex2ccf.cc \
  ../src/utils.cc

generate_uvw_coordinates_SOURCES = \
  generate_uvw_coordinates.cc \
  ../src/Uvw_model.cc \
  ../src/utils.cc

plot_delay_table_SOURCES = \
  plot_delay_table.cc \
  ../src/Delay_table_akima.cc \
  ../src/utils.cc

produce_output_graphs_SOURCES = \
  produce_output_graphs.cc
