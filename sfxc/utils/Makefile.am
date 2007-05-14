EXTRA_DIST = 

AM_CPPFLAGS = -L/usr/lib
AM_CXXFLAGS = -I../include

bin_PROGRAMS = create_delay_function \
               produce_fringe \
               analyse_fringes \
               analyse_mark4_file \
               print_headers

create_delay_function_SOURCES = create_delay_function.cc
create_delay_function_LDADD = -lgsl -lgslcblas

produce_fringe_SOURCES = produce_fringe.cc

print_headers_SOURCES = print_headers.cc \
  ../src/staPrms.cc ../src/genPrms.cc \
  ../src/runPrms.cc ../src/genFunctions.cc \
  ../src/Log_writer.cc ../src/Log_writer_cout.cc \
  ../src/Data_reader.cc ../src/Data_reader_file.cc \
  ../src/Channel_extractor_mark4.cc \
  ../src/utils.cc

analyse_fringes_SOURCES = analyse_fringes.cc

analyse_mark4_file_SOURCES = \
  analyse_mark4_file.cc \
  ../src/staPrms.cc ../src/genPrms.cc ../src/runPrms.cc ../src/genFunctions.cc \
  ../src/Log_writer.cc ../src/Log_writer_cout.cc \
  ../src/Data_reader.cc ../src/Data_reader_file.cc \
  ../src/Channel_extractor_mark4.cc \
  ../src/utils.cc ../src/Bits_to_float_converter.cc
