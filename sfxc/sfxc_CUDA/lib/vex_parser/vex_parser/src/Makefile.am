AM_CXXFLAGS=-I@top_srcdir@/include -I@top_srcdir@/lib -I.
AM_LDFLAGS=-L@top_srcdir@/lib

noinst_PROGRAMS = cpp_parser parser
noinst_HEADERS = generic.h

cpp_parser_SOURCES = cpp_parser.cc \
  @top_srcdir@/lib/libvex_parser.a
cpp_parser_LDADD = -L@top_srcdir@/lib -lvex_parser

parser_SOURCES = parser.cc \
  @top_srcdir@/lib/libvex_parser.a
parser_LDADD = -L@top_srcdir@/lib -lvex_parser
