EXTRA_DIST = 

AM_CPPFLAGS = -I../include
AM_CXXFLAGS = 

bin_PROGRAMS = sfxc sfxc_singlecore sfxc_adam

OBJ=runPrms.cc genPrms.cc staPrms.cc \
	genFunctions.cc \
	InData.cc delayTable.cc \
	ProcessData.cc Data_reader_file.cc Data_reader_tcp.cc \
	utils.cc timer.cc \
	Log_writer.cc Log_writer_cout.cc Log_writer_void.cc

sfxc_SOURCES = $(OBJ) sfxc.cc 

sfxc_adam_SOURCES = $(OBJ) sfxc_adam.cc \
  Controller_node.cc Correlator_node.cc Input_node.cc Node.cc

sfxc_singlecore_SOURCES = $(OBJ) sfxc_singlecore.cc
