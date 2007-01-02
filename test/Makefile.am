EXTRA_DIST = 

AM_CPPFLAGS = -Iinclude -I../include
AM_CXXFLAGS = 

check_PROGRAMS = \
  test_buffer \
  test_Data_reader_file \
  test_Data_reader_mark5 \
  test_Data_node \
  test_Correlate_node
TESTS           = \
  test_buffer \
  test_Data_reader_file \
  test_Data_reader_mark5.py \
  test_Data_node.py \
  test_Correlate_node


OBJ = ../src/Controller.cc \
      ../src/Input_controller.cc \
      ../src/Output_controller.cc \
      ../src/Node.cc ../src/Data_node.cc \
      ../src/Data_reader_file.cc ../src/Data_reader_tcp.cc \
      ../src/Data_writer_file.cc \
      ../src/utils.cc

test_buffer_SOURCES = \
  test_buffer.cc

test_Data_reader_file_SOURCES = \
  test_Data_reader_file.cc ../src/Data_reader_file.cc

test_Data_reader_mark5_SOURCES = \
  test_Data_reader_mark5.cc ../src/Data_reader_mark5.cc

test_Data_node_SOURCES = \
  $(OBJ) test_Data_node.cc 

test_Correlate_node_SOURCES = \
  $(OBJ) test_Correlate_node.cc \
  ../src/Correlate_node.cc ../src/Correlate_controller.cc \
  ../src/runPrms.cc ../src/genPrms.cc ../src/staPrms.cc \
	../src/genFunctions.cc \
	../src/InData.cc ../src/delayTable.cc \
	../src/ProcessData.cc
  