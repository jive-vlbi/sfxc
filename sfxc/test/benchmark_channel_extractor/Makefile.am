AM_CXXFLAGS = -I. $(SFXC_CXXFLAGS)
LDADD = $(SFXC_LDADD)

bin_PROGRAMS = benchmark_channel_extractor

benchmark_channel_extractor_SOURCES = \
  main.cc \
  benchmark.cc \
  utils_bench.cc \
  ../../src/channel_extractor_brute_force.cc
