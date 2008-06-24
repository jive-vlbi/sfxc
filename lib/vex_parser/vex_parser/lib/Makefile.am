AM_CXXFLAGS=-I@top_srcdir@/include -I@top_srcdir@/src -I.

instdir = $(libdir)

inst_LIBRARIES = libvex_parser32.a libvex_parser.a

libvex_parser_a_SOURCES = \
  ../src/vex_parser.yy ../src/vex_scanner.ll \
  ../src/Vex++.cc ../src/Vexpp_node.cc

libvex_parser32_a_SOURCES  = \
  ../src/vex_parser.yy ../src/vex_scanner.ll \
  ../src/Vex++.cc ../src/Vexpp_node.cc
libvex_parser32_a_CXXFLAGS = $(AM_CXXFLAGS) -m32

../src/Vex++.cc: vex_parser.cc