AUTOMAKE_OPTIONS = subdir-objects

AM_CXXFLAGS=-Iinclude

noinst_LIBRARIES = libjsoncpp.a
libjsoncpp_a_SOURCES = \
  src/json_reader.cpp \
  src/json_value.cpp \
  src/json_writer.cpp

pkginclude_HEADERS = include/json/*.h src/*.h src/*.inl
