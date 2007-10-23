EXTRA_DIST = 

AM_CPPFLAGS = -Iinclude -I../include -g
AM_CXXFLAGS = 


check_PROGRAMS = \
  test_Control_parameters \
  test_mpi \
  test_buffer \
  test_Time_slicer \
  test_Transfer_control_parameters \
  test_Data_reader_file \
  test_Data_reader_mark5 \
  test_Data_reader_TCP \
  test_Input_node \
  test_Output_node \
  test_Log_writer_mpi \
  test_TCP_connection \
  test_Channel_extractor

TESTS           = \
  test_Control_parameters \
  test_mpi.py \
  test_Transfer_control_parameters.py \
  test_buffer \
  test_Time_slicer \
  test_Data_reader_file \
  test_Data_reader_mark5.py \
  test_Data_reader_TCP.py \
  test_Output_node.py \
  test_Input_node.py \
  test_Log_writer_mpi.py \
  test_Channel_extractor.py

OBJ = ../src/utils.cc \
      ../src/TCP_Connection.cc \
      ../src/MPI_Transfer.cc \
      ../src/Control_parameters.cc \
      ../src/Delay_table_akima.cc \
      ../src/Channel_extractor_mark4.cc \
      ../src/Bits_to_float_converter.cc \
      \
      ../src/Data_reader.cc \
      ../src/Data_reader_file.cc \
      ../src/Data_reader_tcp.cc \
      ../src/Data_reader_buffer.cc \
      ../src/Data_writer.cc \
      ../src/Data_writer_file.cc \
      ../src/Data_writer_tcp.cc \
      ../src/Data_writer_buffer.cc \
      ../src/Data_writer_void.cc \
      ../src/Log_writer.cc \
      ../src/Log_writer_file.cc \
      ../src/Log_writer_cout.cc \
      ../src/Log_writer_void.cc \
      ../src/Log_writer_mpi.cc\
      \
      ../src/Node.cc \
      ../src/Correlator_node.cc \
      ../src/Abstract_manager_node.cc \
      ../src/Input_node.cc \
      ../src/Output_node.cc \
      ../src/Log_node.cc \
      \
      ../src/Controller.cc \
      ../src/Correlator_node_controller.cc \
      ../src/Output_node_controller.cc \
      ../src/Input_node_controller.cc \
      ../src/Log_node_controller.cc \
      ../src/Single_data_reader_controller.cc \
      ../src/Multiple_data_readers_controller.cc \
      ../src/Single_data_writer_controller.cc \
      ../src/Multiple_data_writers_controller.cc \
      ../src/sfxc_mpi.cc \
      ../src/Integration_slice.cc \
      ../src/DelayCorrection.cc \
      ../src/CorrelationCore.cc

test_mpi_SOURCES = \
  test_mpi.cc ../src/utils.cc

test_buffer_SOURCES = \
  test_buffer.cc ../src/utils.cc

test_Time_slicer_SOURCES = \
  test_Time_slicer.cc \
  ../src/Data_writer.cc ../src/Data_writer_file.cc ../src/utils.cc

test_Data_reader_file_SOURCES = \
  test_Data_reader_file.cc ../src/Data_reader.cc \
  ../src/Data_reader_file.cc ../src/Data_writer.cc \
  ../src/Data_writer_file.cc \
  ../src/Log_writer.cc ../src/Log_writer_cout.cc ../src/utils.cc

test_Data_reader_TCP_SOURCES = \
  test_Data_reader_TCP.cc ../src/Data_reader_tcp.cc ../src/Data_writer.cc \
  ../src/Data_writer_tcp.cc ../src/Data_reader.cc ../src/TCP_Connection.cc ../src/Data_reader_file.cc \
  ../src/Log_writer.cc ../src/Log_writer_cout.cc ../src/Data_reader_buffer.cc ../src/utils.cc

test_Transfer_control_parameters_SOURCES = \
  $(OBJ) test_Transfer_control_parameters.cc

test_Data_reader_mark5_SOURCES = \
  $(OBJ) test_Data_reader_mark5.cc ../src/Data_reader_mark5.cc

test_Channel_extractor_SOURCES = \
  $(OBJ) test_Channel_extractor.cc

test_Input_node_SOURCES = \
  $(OBJ) test_Input_node.cc

test_Output_node_SOURCES = \
  $(OBJ) test_Output_node.cc

test_Log_writer_mpi_SOURCES = \
  $(OBJ) test_Log_writer_mpi.cc

test_TCP_connection_SOURCES = \
  test_TCP_connection.cc \
  ../src/TCP_Connection.cc \
  ../src/utils.cc

test_Control_parameters_SOURCES = \
  test_Control_parameters.cc \
  ../src/utils.cc  \
  ../src/Log_writer_cout.cc ../src/Log_writer.cc \
  ../src/Control_parameters.cc

