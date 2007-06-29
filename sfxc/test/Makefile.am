EXTRA_DIST = 

AM_CPPFLAGS = -Iinclude -I../include -g
AM_CXXFLAGS = 


check_PROGRAMS = \
  test_buffer \
  test_Transfer_control_parameters \
  test_Data_reader_file \
  test_Data_reader_mark5 \
  test_Data_reader_TCP \
  test_Input_node \
  test_Correlator_node \
  test_Log_writer_mpi \
  test_TCP_connection

TESTS           = \
  test_Transfer_control_parameters.py \
  test_Correlator_node.py \
  test_buffer \
  test_Data_reader_file \
  test_Data_reader_mark5.py \
  test_Data_reader_TCP.py \
  test_Input_node.py \
  test_Log_writer_mpi.py


OBJ = ../src/utils.cc \
      ../src/Timer.cc \
      ../src/TCP_Connection.cc \
      ../src/MPI_Transfer.cc \
      ../src/runPrms.cc \
      ../src/genPrms.cc \
      ../src/staPrms.cc \
      ../src/genFunctions.cc \
      ../src/delayTable.cc \
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
      ../src/Log_writer.cc \
      ../src/Log_writer_file.cc \
      ../src/Log_writer_cout.cc \
      ../src/Log_writer_void.cc \
      ../src/Log_writer_mpi.cc\
      \
      ../src/Node.cc \
      ../src/Input_node.cc \
      ../src/Output_node.cc \
      ../src/Log_node.cc \
      \
      ../src/Controller.cc \
      ../src/Output_node_controller.cc \
      ../src/Input_node_controller.cc \
      ../src/Log_node_controller.cc \
      ../src/Single_data_reader_controller.cc \
      ../src/Multiple_data_readers_controller.cc \
      ../src/Single_data_writer_controller.cc \
      ../src/Multiple_data_writers_controller.cc

test_buffer_SOURCES = \
  test_buffer.cc

test_Transfer_control_parameters_SOURCES = \
  $(OBJ) test_Transfer_control_parameters.cc 

test_Data_reader_file_SOURCES = \
  test_Data_reader_file.cc ../src/Data_reader.cc \
  ../src/Data_reader_file.cc ../src/Data_writer.cc \
  ../src/Data_writer_file.cc \
  ../src/Log_writer.cc ../src/Log_writer_cout.cc

test_Data_reader_TCP_SOURCES = \
  test_Data_reader_TCP.cc ../src/Data_reader_tcp.cc ../src/Data_writer.cc \
  ../src/Data_writer_tcp.cc ../src/Data_reader.cc ../src/TCP_Connection.cc ../src/Data_reader_file.cc \
  ../src/Log_writer.cc ../src/Log_writer_cout.cc ../src/Data_reader_buffer.cc

test_Data_reader_mark5_SOURCES = \
  test_Data_reader_mark5.cc ../src/Data_reader.cc ../src/Data_reader_mark5.cc

test_Input_node_SOURCES = \
  $(OBJ) test_Input_node.cc

test_Correlator_node_SOURCES = \
  $(OBJ) test_Correlator_node.cc \
  ../src/Correlator_node.cc ../src/Correlator_node_controller.cc \
  ../src/Integration_slice.cc \
  ../src/CorrelationCore.cc \
  ../src/DelayCorrection.cc \
  ../src/InData.cc

test_Log_writer_mpi_SOURCES = \
  test_Log_writer_mpi.cc \
  ../src/Node.cc \
  ../src/Log_writer.cc ../src/Log_writer_cout.cc \
  ../src/Log_writer_mpi.cc  ../src/Log_writer_file.cc \
  ../src/Controller.cc ../src/Log_node_controller.cc  ../src/Log_node.cc

test_TCP_connection_SOURCES = \
  test_TCP_connection.cc \
  ../src/TCP_Connection.cc
