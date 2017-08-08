AUTOMAKE_OPTIONS = subdir-objects

AM_CXXFLAGS=-I../testunit/src -I./src $(SFXC_CXXFLAGS) 

noinst_LIBRARIES = libcommon.a
libcommon_a_SOURCES = \
  src/backtrace.cc \
  src/demangler.cc\
  src/mutex.cc \
  src/thread.cc \
  src/common.cc \
  src/exception_common.cc \
  src/raiimutex.cc \
  src/condition.cc \
  src/exception_indexoutofbound.cc \
  src/signal_handler.cc \
  src/monitor.cc \
  src/align_malloc.cc 

pkginclude_HEADERS = src/*.h
