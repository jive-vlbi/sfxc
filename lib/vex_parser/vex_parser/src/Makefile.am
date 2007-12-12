AM_CXXFLAGS=-I@top_srcdir@/include -I@top_srcdir@/lib -I.

noinst_PROGRAMS = cpp_parser parser

cpp_parser_SOURCES = cpp_parser.cc
cpp_parser_LDADD = -L@top_srcdir@/lib -lvex_parser

parser_SOURCES = parser.cc
parser_LDADD = -L@top_srcdir@/lib -lvex_parser
