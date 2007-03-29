EXTRA_DIST = 

AM_CPPFLAGS = -I../../include
AM_CXXFLAGS = 

bin_PROGRAMS = \
  manager_node input_node correlator_node dummy_correlator_node output_node log_node

OBJ= \
  ../Node.cc ../Controller.cc \
  ../Multiple_data_readers_controller.cc \
  ../Multiple_data_writers_controller.cc \
  ../Single_data_writer_controller.cc \
  ../Single_data_reader_controller.cc \
	../Data_reader_file.cc ../Data_reader_tcp.cc ../Data_reader_buffer.cc \
	../Data_writer.cc ../Data_writer_file.cc ../Data_writer_tcp.cc \
    ../Data_writer_buffer.cc \
	../Log_writer.cc ../Log_writer_mpi.cc \
  ../runPrms.cc ../genPrms.cc ../staPrms.cc ../genFunctions.cc \
	../utils.cc ../timer.cc \
  ../MPI_Transfer.cc ../delayTable.cc ../TCP_Connection.cc

manager_node_SOURCES = $(OBJ) manager_node.cc \
  ../Manager_node.cc ../Manager_node_controller.cc

log_node_SOURCES = $(OBJ) log_node.cc \
  ../Log_node.cc ../Log_node_controller.cc \
  ../Log_writer_cout.cc ../Log_writer_file.cc

input_node_SOURCES = $(OBJ) input_node.cc \
  ../Input_node.cc ../Input_node_controller.cc

correlator_node_SOURCES = $(OBJ) correlator_node.cc \
  ../Correlator_node.cc ../Correlator_node_controller.cc \
  ../InData.cc \
  ../Integration_slice.cc ../CorrelationCore.cc ../DelayCorrection.cc
	
output_node_SOURCES = $(OBJ) output_node.cc \
  ../Output_node.cc ../Output_node_controller.cc 
  
dummy_correlator_node_SOURCES = $(OBJ) correlator_node.cc \
  ../Correlator_node.cc ../Correlator_node_controller.cc \
	../InData.cc \
  ../Dummy_Integration_slice.cc ../CorrelationCore.cc ../DelayCorrection.cc
