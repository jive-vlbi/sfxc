AM_CXXFLAGS = $(SFXC_CXXFLAGS)
LDADD = $(SFXC_LDADD)

SUBDIRS = delay

pkginclude_HEADERS = *.h *.py


bin_PROGRAMS = mark5a_print_headers \
               produce_html_plotpage \
               produce_html_diffpage \
               vex2ccf \
               generate_uvw_coordinates \
               plot_delay_table \
               print_new_output_format \
               dnfp_fileserver \
               dnfp_fileclient \
               phase_plot \
               make_anim \
               baseline_info \
							 extract_channelizer

bin_SCRIPTS  = run_sfxc.py vumeter.py

extract_channelizer_SOURCES = \
  extract_channelizer.cc \
  ../src/control_parameters.cc \
  ../src/log_writer_cout.cc \
  ../src/log_writer.cc \
  ../src/data_reader.cc \
  ../src/data_writer.cc \
  ../src/data_reader_file.cc \
  ../src/input_data_format_reader.cc \
  ../src/mark5a_reader.cc \
  ../src/mark5a_header.cc \
  ../src/data_reader_factory.cc \
  ../src/data_reader_dnfp.cc \
	../src/data_reader_socket.cc \
	../src/data_writer_socket.cc \
	../src/data_reader_blocking.cc \
	../src/channel_extractor_utils.cc \
	../src/channel_extractor_dynamic.cc \
	../src/channel_extractor_fast.cc \
  ../src/utils.cc

dnfp_fileserver_SOURCES = \
  dnfp_fileserver.cc \
   ../src/data_reader.cc \
   ../src/data_writer.cc \
   ../src/data_reader_file.cc \
   ../src/data_writer_socket.cc \
   ../src/data_reader_socket.cc \
   ../src/data_reader_blocking.cc \
   ../src/data_reader_dnfp.cc \
   ../src/utils.cc

dnfp_fileclient_SOURCES = \
  dnfp_fileclient.cc \
   ../src/data_reader.cc \
   ../src/data_writer.cc \
   ../src/data_reader_file.cc \
   ../src/data_writer_socket.cc \
   ../src/data_reader_socket.cc \
   ../src/data_reader_blocking.cc  \
   ../src/data_reader_dnfp.cc \
   ../src/utils.cc

mark5a_print_headers_SOURCES = \
  mark5a_print_headers.cc \
  ../src/log_writer.cc \
  ../src/log_writer_cout.cc \
  ../src/data_reader.cc \
  ../src/data_reader_file.cc \
  ../src/input_data_format_reader.cc \
  ../src/mark5a_reader.cc \
  ../src/mark5a_header.cc \
  ../src/channel_extractor_brute_force.cc \
  ../src/utils.cc \
  ../src/control_parameters.cc

produce_html_plotpage_SOURCES = \
  produce_html_plotpage.cc \
  fringe_info.cc \
  ../src/control_parameters.cc \
  ../src/log_writer.cc \
  ../src/log_writer_cout.cc \
  ../src/utils.cc \
  ../src/output_header.cc \
  gnuplot_i.cc

produce_html_diffpage_SOURCES = \
  produce_html_diffpage.cc \
  fringe_info.cc \
  ../src/control_parameters.cc \
  ../src/log_writer.cc \
  ../src/log_writer_cout.cc \
  ../src/utils.cc \
  ../src/output_header.cc \
  gnuplot_i.cc

baseline_info_SOURCES = \
  baseline_info.cc \
  fringe_info.cc \
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

print_new_output_format_SOURCES = \
  print_new_output_format.cc \
  ../src/output_header.cc \
   ../src/utils.cc

phase_plot_SOURCES = \
  phase_plot.cc \
  ../src/output_header.cc \
   ../src/utils.cc

make_anim_SOURCES = \
  make_anim.cc \
  gnuplot_i.cc
