AM_CXXFLAGS=-I@top_srcdir@/include -I@top_srcdir@/src -I.

# We write our own rules for yacc and lex below. We'd rather use automake,
# but automake makes it especially difficult for a number of reasons:
#
#  * < automake-1.12 generates .h files from .yy and .ypp files, but
#    >=automake-1.12 generates .hh and .hpp files respectively. There's no
#    good way of making a project that uses C++ yacc files compatible with
#    both versions of automake. Strong work automake developers.

vex_parser.cc vex_parser.h: ../src/vex_parser.yy
	$(YACC) $(YFLAGS) -o $@ $<

instdir = $(libdir)

inst_LIBRARIES = libvex_parser.a
noinst_HEADERS = vex_parser.h
BUILT_SOURCES = vex_parser.cc vex_parser.h

libvex_parser_a_SOURCES = \
  vex_parser.cc ../src/vex_scanner.ll \
  ../src/Vex++.cc ../src/Vexpp_node.cc

if DO_32BIT
inst_LIBRARIES += libvex_parser32.a

libvex_parser32_a_SOURCES  = \
  vex_parser.cc ../src/vex_scanner.ll \
  ../src/Vex++.cc ../src/Vexpp_node.cc
libvex_parser32_a_CXXFLAGS = $(AM_CXXFLAGS) -m32
endif
