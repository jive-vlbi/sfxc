SUBDIRS = multicore

AM_CPPFLAGS = -I../include

bin_PROGRAMS = sfxc

TESTS           = \
	test_sfxc.py

OBJ=\
  Abstract_manager_node.cc \
  Control_parameters.cc \
  sfxc_mpi.cc \
  utils.cc \
  Delay_table_akima.cc \
  Data_writer.cc Data_reader.cc \
  Data_reader_file.cc Data_writer_file.cc \
  Data_writer_void.cc \
  Log_writer.cc Log_writer_cout.cc Log_writer_void.cc \
  Log_writer_file.cc \
  Integration_slice.cc CorrelationCore.cc DelayCorrection.cc \
  Channel_extractor_mark4.cc \
  Bits_to_float_converter.cc


sfxc_SOURCES = $(OBJ) sfxc.cc \
  Node.cc Manager_node.cc Log_node.cc \
  Correlator_node.cc Input_node.cc Output_node.cc \
  Controller.cc Input_node_controller.cc Output_node_controller.cc \
  Correlator_node_controller.cc Manager_node_controller.cc Log_node_controller.cc \
  MPI_Transfer.cc TCP_Connection.cc \
  Log_writer_mpi.cc Data_reader_tcp.cc  Data_writer_tcp.cc \
  Data_writer_buffer.cc Data_reader_buffer.cc \
  Multiple_data_readers_controller.cc \
  Multiple_data_writers_controller.cc \
  Single_data_writer_controller.cc \
  Single_data_reader_controller.cc

