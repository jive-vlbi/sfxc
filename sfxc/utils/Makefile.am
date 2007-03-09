EXTRA_DIST = 

AM_CPPFLAGS = -L/usr/lib
AM_CXXFLAGS = -I../include

bin_PROGRAMS = create_delay_function \
               produce_fringe \
               analyse_fringes

create_delay_function_SOURCES = create_delay_function.cc
create_delay_function_LDADD = -lgsl -lgslcblas

produce_fringe_SOURCES = produce_fringe.cc

analyse_fringes_SOURCES = analyse_fringes.cc
