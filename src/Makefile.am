AM_CXXFLAGS = $(SFXC_CXXFLAGS)
LDADD = $(SFXC_LDADD)

bin_PROGRAMS = sfxc

TESTS           = \
	test_sfxc.py

OBJ=\
  abstract_manager_node.cc \
  control_parameters.cc \
  sfxc_mpi.cc \
  utils.cc \
  delay_table_akima.cc \
  mark4_reader_tasklet.cc \
  mark4_reader.cc \
  mark4_header.cc \
  data_writer.cc data_reader.cc \
  data_reader_factory.cc \
  data_reader_dnfp.cc \
  data_reader_blocking.cc \
  data_reader_socket.cc \
  data_reader_mark5.cc \
  data_writer_socket.cc \
  data_reader_file.cc data_writer_file.cc \
  data_writer_void.cc \
  log_writer.cc log_writer_cout.cc log_writer_void.cc \
  log_writer_file.cc \
  correlation_core.cc \
  delay_correction.cc \
  integer_delay_correction_per_channel.cc \
  channel_extractor_tasklet.cc \
  channel_extractor_brute_force.cc \
  channel_extractor_5.cc \
  channel_extractor_fast.cc \
  channel_extractor_utils.cc \
  tasklet/mutex.cc \
  tasklet/tasklet.cc \
  tasklet/tasklet_manager.cc \
  tasklet/tasklet_pool.cc \
  tasklet/tasklet_worker.cc \
  tasklet/thread.cc

sfxc_SOURCES = $(OBJ) sfxc.cc \
  node.cc manager_node.cc log_node.cc \
  correlator_node.cc input_node.cc output_node.cc \
  controller.cc input_node_controller.cc output_node_controller.cc \
  correlator_node_controller.cc manager_node_controller.cc \
  log_node_controller.cc \
  correlator_node_data_reader_tasklet.cc \
  mpi_transfer.cc tcp_connection.cc \
  log_writer_mpi.cc data_reader_tcp.cc  data_writer_tcp.cc \
  data_writer_buffer.cc \
  multiple_data_readers_controller.cc \
  multiple_data_writers_controller.cc \
  single_data_writer_controller.cc \
  single_data_reader_controller.cc \
  input_node_tasklet.cc \
  input_node_tasklet_impl.cc \
  input_node_data_writer_tasklet.cc \
  output_header.cc

sfxc.cc: svn_version.h
svn_version.h:
	echo "#define SVN_VERSION \"`svnversion -n ..`\"" > svn_version.h
