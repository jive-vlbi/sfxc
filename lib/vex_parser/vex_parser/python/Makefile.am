if BUILD_PYTHONLIB

instdir = $(libdir)

pkgpyexec_LTLIBRARIES = vex_parser.la
pkgpyexec_SCRIPTS     = __init__.py MultiDict.py

vex_parser_la_SOURCES = \
  python_interface.cc \
  ../src/vex_parser.yy ../src/vex_scanner.ll \
  ../src/Vex++.cc ../src/Vexpp_node.cc
vex_parser_la_CXXFLAGS = \
  -I@top_srcdir@/include -I@top_srcdir@/src -I. \
  $(BOOST_PYTHON_LIB_CFLAGS)
vex_parser_la_LDFLAGS  = -module
vex_parser_la_LIBADD   = $(BOOST_PYTHON_LIB_LDFLAGS)

../src/generic.h: vex_parser.h

endif