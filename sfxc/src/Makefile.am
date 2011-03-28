AM_CXXFLAGS = $(SFXC_CXXFLAGS)
AM_CPPFLAGS = $(SFXC_CPPFLAGS)
LDADD = $(SFXC_LDADD)

if SFXC
bin_PROGRAMS = sfxc
endif

TESTS = test_sfxc.py

if DOUBLE_PRECISION
  FFT_SOURCES = sfxc_fft.cc
else
  FFT_SOURCES = sfxc_fft_float.cc
endif

OBJ=\
  abstract_manager_node.cc \
  control_parameters.cc \
  sfxc_mpi.cc \
  utils.cc \
  delay_table_akima.cc \
  input_data_format_reader.cc \
  input_data_format_reader_tasklet.cc \
  vdif_reader.cc \
  mark5a_reader.cc \
  mark5a_header.cc \
  vlba_reader.cc \
  vlba_header.cc \
  mark5b_reader.cc \
  data_writer.cc data_reader.cc \
  data_reader_factory.cc \
  data_reader_dnfp.cc \
  data_reader_mk5.cc \
  data_reader_blocking.cc \
  data_reader_socket.cc \
  data_reader_udp.cc \
  data_writer_socket.cc \
  data_reader_file.cc data_writer_file.cc \
  log_writer.cc log_writer_cout.cc \
  log_writer_file.cc \
  correlation_core.cc \
  correlation_core_pulsar.cc \
  delay_correction_base.cc \
  delay_correction_default.cc \
  delay_correction_swapped.cc \
  uvw_model.cc \
  channel_extractor_tasklet.cc \
  channel_extractor_tasklet_vdif.cc \
  channel_extractor_5.cc \
  channel_extractor_utils.cc \
  channel_extractor_dynamic.cc \
  tasklet/tasklet.cc \
  tasklet/tasklet_manager.cc \
  tasklet/tasklet_pool.cc \
  tasklet/tasklet_worker.cc

sfxc_SOURCES = $(OBJ) $(FFT_SOURCES) sfxc.cc \
  node.cc manager_node.cc log_node.cc \
  correlator_node.cc input_node.cc output_node.cc \
  controller.cc input_node_controller.cc output_node_controller.cc \
  correlator_node_controller.cc manager_node_controller.cc \
  log_node_controller.cc \
  correlator_node_data_reader_tasklet.cc \
  correlator_node_bit2float_tasklet.cc \
  bit2float_worker.cc \
  bit_statistics.cc\
  mpi_transfer.cc \
  log_writer_mpi.cc data_reader_tcp.cc  data_writer_tcp.cc \
  multiple_data_readers_controller.cc \
  multiple_data_writers_controller.cc \
  single_data_writer_controller.cc \
  single_data_reader_controller.cc \
  input_node_tasklet.cc \
  input_node_data_writer.cc \
  input_node_data_writer_tasklet.cc \
  output_header.cc \
  correlator_time.cc \
  svn_version.cc 

sfxc_DEPENDENCIES = update_svn_version svn_version.$(OBJEXT)

svn_version.cc: update_svn_version

update_svn_version:
	echo "char *SVN_VERSION=\"`svnversion -n ..`\";" > svn_version.cc
