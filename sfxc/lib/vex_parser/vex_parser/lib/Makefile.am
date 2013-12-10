AM_CXXFLAGS=-I@top_srcdir@/include -I@top_srcdir@/src -I.

instdir = $(libdir)

inst_LIBRARIES = libvex_parser32.a libvex_parser.a
noinst_HEADERS = vex_parser.h

libvex_parser_a_SOURCES = \
  ../src/vex_parser.yy ../src/vex_scanner.ll \
  ../src/Vex++.cc ../src/Vexpp_node.cc

if DO_32BIT
libvex_parser32_a_SOURCES  = \
  ../src/vex_parser.yy ../src/vex_scanner.ll \
  ../src/Vex++.cc ../src/Vexpp_node.cc
libvex_parser32_a_CXXFLAGS = $(AM_CXXFLAGS) -m32
endif

../src/Vex++.cc: vex_parser.cc
