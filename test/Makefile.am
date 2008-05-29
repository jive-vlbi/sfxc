SUBDIRS = benchmark_channel_extractor

AM_CXXFLAGS = $(SFXC_CXXFLAGS)
LDADD = $(SFXC_LDADD)

check_PROGRAMS = \
  test_control_parameters \
  test_mpi \
  test_buffer \
  test_transfer_control_parameters \
  test_data_reader_file \
  test_data_reader_mark5 \
  test_mark5b_reader \
  test_tracks_mark5b \
  test_data_reader_tcp \
  test_input_node \
  test_output_node \
  test_log_writer_mpi \
  test_tcp_connection \
  test_channel_extractor \
  test_input_node_with_delay_correction \
  test_correlator_node_data_reader \
  do_channel_extractor

TESTS           = \
  test_control_parameters \
  test_mpi.py \
  test_transfer_control_parameters.py \
  test_buffer \
  test_data_reader_file \
  test_data_reader_mark5.py \
  test_mark5b_reader.py \
  test_data_reader_tcp.py \
  test_correlator_node_data_reader \
  test_output_node.py \
  test_input_node.py \
  test_log_writer_mpi.py \
  test_channel_extractor.py \
  test_input_node_with_delay_correction.py

OBJ = ../src/utils.cc \
      ../src/tcp_connection.cc \
      ../src/mpi_transfer.cc \
      ../src/control_parameters.cc \
      ../src/delay_table_akima.cc \
      ../src/channel_extractor_brute_force.cc \
      ../src/channel_extractor_fast.cc \
      ../src/channel_extractor_utils.cc \
      ../src/mark5a_reader.cc \
      ../src/mark5a_header.cc \
      \
      ../src/data_reader.cc \
      ../src/data_reader_tcp.cc \
      ../src/data_reader_file.cc \
      ../src/data_reader_factory.cc \
      ../src/data_reader_dnfp.cc \
      ../src/data_reader_blocking.cc \
      ../src/data_reader_socket.cc \
      ../src/data_reader_mark5.cc \
      ../src/data_writer.cc \
      ../src/data_writer_file.cc \
      ../src/data_writer_tcp.cc \
      ../src/data_writer_buffer.cc \
      ../src/data_writer_void.cc \
      ../src/data_writer_socket.cc \
      ../src/log_writer.cc \
      ../src/log_writer_file.cc \
      ../src/log_writer_cout.cc \
      ../src/log_writer_void.cc \
      ../src/log_writer_mpi.cc\
      \
      ../src/node.cc \
      ../src/correlator_node.cc \
      ../src/abstract_manager_node.cc \
      ../src/input_node.cc \
      ../src/output_node.cc \
      ../src/log_node.cc \
      \
      ../src/controller.cc \
      ../src/correlator_node_controller.cc \
      ../src/output_node_controller.cc \
      ../src/input_node_controller.cc \
      ../src/log_node_controller.cc \
      ../src/single_data_reader_controller.cc \
      ../src/multiple_data_readers_controller.cc \
      ../src/single_data_writer_controller.cc \
      ../src/multiple_data_writers_controller.cc \
      ../src/sfxc_mpi.cc \
      ../src/delay_correction.cc \
      ../src/correlation_core.cc \
      \
      ../src/tasklet/tasklet.cc \
      ../src/input_node_tasklet.cc

test_mpi_SOURCES = \
  test_mpi.cc ../src/utils.cc

test_buffer_SOURCES = \
  test_buffer.cc ../src/utils.cc

test_data_reader_file_SOURCES = \
  test_data_reader_file.cc ../src/data_reader.cc \
  ../src/data_reader_file.cc ../src/data_writer.cc \
  ../src/data_writer_file.cc \
  ../src/log_writer.cc ../src/log_writer_cout.cc ../src/utils.cc

test_data_reader_tcp_SOURCES = \
  test_data_reader_tcp.cc ../src/data_reader_tcp.cc ../src/data_writer.cc \
  ../src/data_writer_tcp.cc ../src/data_reader.cc ../src/tcp_connection.cc ../src/data_reader_file.cc \
  ../src/log_writer.cc ../src/log_writer_cout.cc ../src/utils.cc

test_transfer_control_parameters_SOURCES = \
  $(OBJ) test_transfer_control_parameters.cc

test_data_reader_mark5_SOURCES = \
  $(OBJ) test_data_reader_mark5.cc

test_mark5b_reader_SOURCES = \
  test_mark5b_reader.cc \
  ../src/mark5b_reader.cc \
  ../src/data_reader_file.cc \
  ../src/data_reader.cc \
  ../src/utils.cc

test_tracks_mark5b_SOURCES = \
  test_tracks_mark5b.cc \
  ../src/control_parameters.cc \
  ../src/utils.cc

test_channel_extractor_SOURCES = \
  $(OBJ) test_channel_extractor.cc

test_input_node_SOURCES = \
  $(OBJ) test_input_node.cc

test_output_node_SOURCES = \
  $(OBJ) test_output_node.cc

test_log_writer_mpi_SOURCES = \
  $(OBJ) test_log_writer_mpi.cc

test_tcp_connection_SOURCES = \
  test_tcp_connection.cc \
  ../src/tcp_connection.cc \
  ../src/utils.cc

test_control_parameters_SOURCES = \
  test_control_parameters.cc \
  ../src/utils.cc  \
  ../src/log_writer_cout.cc ../src/log_writer.cc \
  ../src/control_parameters.cc

test_input_node_with_delay_correction_SOURCES = \
  $(OBJ) \
  test_input_node_with_delay_correction.cc

do_channel_extractor_SOURCES = \
  do_channel_extractor.cc \
  ../src/utils.cc \
  ../src/mark5a_reader.cc \
  ../src/data_reader.cc \
  ../src/data_reader_file.cc \
  ../src/control_parameters.cc

test_correlator_node_data_reader_SOURCES = \
  $(OBJ) \
  test_correlator_node_data_reader.cc \
  ../src/correlator_node_data_reader_tasklet.cc
