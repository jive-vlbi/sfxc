EXTRA_DIST = 

AM_CPPFLAGS = -I../include
AM_CXXFLAGS = 

bin_PROGRAMS = sfxc sfxc_SC sfxc_adam

OBJ=runPrms.cc genPrms.cc staPrms.cc \
	genFunctions.cc \
	utils.cc timer.cc \
	InData.cc ProcessData.cc delayTable.cc \
  Data_reader_file.cc Data_writer_file.cc \
	Log_writer.cc Log_writer_cout.cc Log_writer_void.cc \
	Log_writer_file.cc \
  TScorrelator.cc CorrelationCore.cc DelayCorrection.cc


sfxc_SOURCES = $(OBJ) sfxc.cc 

sfxc_adam_SOURCES = $(OBJ) sfxc_adam.cc \
  Node.cc Manager_node.cc Log_node.cc \
  Correlator_node.cc Input_node.cc Output_node.cc \
	Controller.cc Input_controller.cc Output_controller.cc \
    Correlator_controller.cc Manager_controller.cc Log_controller.cc \
  MPI_Transfer.cc TCP_Connection.cc \
  Log_writer_mpi.cc Data_reader_tcp.cc  Data_writer_tcp.cc

sfxc_SC_SOURCES = $(OBJ) sfxc_SC.cc

