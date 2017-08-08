AUTOMAKE_OPTIONS = subdir-objects

AM_CXXFLAGS=-I../common/src

noinst_LIBRARIES = libtestunit.a
libtestunit_a_SOURCES = \
  src/Testable.cc \
  src/Test_aninterface.cc \
  src/Test_manager.cc \
  src/Test_aclass.cc \
  src/Test_exception.cc \
  src/Test_unit.cc

pkginclude_HEADERS = src/*.h
