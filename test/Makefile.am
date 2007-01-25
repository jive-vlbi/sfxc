EXTRA_DIST = 

AM_CPPFLAGS = -Iinclude -I../include
AM_CXXFLAGS = 

check_PROGRAMS = \
  test_buffer \
  test_Transfer_control_parameters \
  test_Data_reader_file \
  test_Data_reader_mark5 \
  test_Data_node \
  test_Correlator_node \
  test_Log_writer_mpi \
  test_TCP_connection
TESTS           = \
  test_Transfer_control_parameters.py \
  test_Correlator_node.py \
  test_buffer \
  test_Data_reader_file \
  test_Data_reader_mark5.py \
  test_Data_node.py \
  test_Log_writer_mpi.py


OBJ = ../src/MPI_Transfer.cc \
      ../src/Controller.cc \
      ../src/Input_controller.cc \
      ../src/Output_controller.cc \
      ../src/Node.cc ../src/Data_node.cc \
      ../src/Data_reader_file.cc ../src/Data_reader_tcp.cc \
      ../src/Data_writer_file.cc \
      ../src/utils.cc \
      ../src/timer.cc \
      ../src/Log_writer.cc \
      ../src/Log_writer_cout.cc \
      ../src/Log_writer_void.cc

test_buffer_SOURCES = \
  test_buffer.cc

test_Transfer_control_parameters_SOURCES = \
  $(OBJ) test_Transfer_control_parameters.cc ../src/delayTable.cc \
  ../src/runPrms.cc ../src/genPrms.cc ../src/staPrms.cc \
  ../src/genFunctions.cc

test_Data_reader_file_SOURCES = \
  test_Data_reader_file.cc ../src/Data_reader_file.cc

test_Data_reader_mark5_SOURCES = \
  test_Data_reader_mark5.cc ../src/Data_reader_mark5.cc

test_Data_node_SOURCES = \
  $(OBJ) test_Data_node.cc \
  ../src/runPrms.cc ../src/genPrms.cc ../src/staPrms.cc \
  ../src/genFunctions.cc ../src/delayTable.cc

test_Correlator_node_SOURCES = \
  $(OBJ) test_Correlator_node.cc \
  ../src/Correlator_node.cc ../src/Correlator_controller.cc \
  ../src/runPrms.cc ../src/genPrms.cc ../src/staPrms.cc \
  ../src/genFunctions.cc \
  ../src/InData.cc ../src/delayTable.cc \
  ../src/ProcessData.cc

test_Log_writer_mpi_SOURCES = \
  test_Log_writer_mpi.cc \
  ../src/Node.cc \
  ../src/Log_writer.cc ../src/Log_writer_cout.cc ../src/Log_writer_mpi.cc \
  ../src/Controller.cc ../src/Log_controller.cc
  
test_TCP_connection_SOURCES = \
  test_TCP_connection.cc \
  ../src/TCP_Connection.cc