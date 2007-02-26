EXTRA_DIST = 

AM_CPPFLAGS = -I../include
AM_CXXFLAGS = 

bin_PROGRAMS = sfxc sfxc_singlecore sfxc_adam

OBJ=runPrms.cc genPrms.cc staPrms.cc \
	genFunctions.cc \
	utils.cc timer.cc \
	InData.cc ProcessData.cc delayTable.cc \
  Data_reader_file.cc Data_writer_file.cc \
	Log_writer.cc Log_writer_cout.cc Log_writer_void.cc Log_writer_file.cc


sfxc_SOURCES = $(OBJ) sfxc.cc 

sfxc_adam_SOURCES = $(OBJ) sfxc_adam.cc \
  Node.cc Controller.cc \
  Manager_node.cc Manager_node_controller.cc \
  Log_node.cc Log_node_controller.cc \
  Correlator_node.cc Correlator_node_controller.cc \
  Input_node.cc Input_node_controller.cc \
  Output_node.cc Output_node_controller.cc \
  MPI_Transfer.cc TCP_Connection.cc \
  Log_writer_mpi.cc Data_reader_tcp.cc  Data_writer_tcp.cc \
  Single_data_writer_controller.cc  Multiple_data_writers_controller.cc \
  Multiple_data_readers_controller.cc Single_data_reader_controller.cc \
  Data_reader_buffer.cc

sfxc_singlecore_SOURCES = $(OBJ) sfxc_singlecore.cc
