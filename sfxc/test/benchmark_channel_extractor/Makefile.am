AM_CXXFLAGS = -I. $(SFXC_CXXFLAGS) -O3 
LDADD = $(SFXC_LDADD)

bin_PROGRAMS = benchmark_channel_extractor

addclean:
	rm -f *.orig *~

benchmark_channel_extractor_SOURCES = \
  main.cc \
  benchmark.cc \
  utils_bench.cc \
  ../../src/channel_extractor_5.cc \
  ../../src/channel_extractor_fast.cc \
  ../../src/channel_extractor_brute_force.cc \
  ../../src/utils.cc
