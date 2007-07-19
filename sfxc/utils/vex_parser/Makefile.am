AM_CXXFLAGS = -I@top_srcdir@/lib/vex_parser
LDADD = @top_srcdir@/lib/vex_parser/libvex.a
LIBS = -lfl -ljson

bin_PROGRAMS = sfxc_vex2ccf sfxc_vex2dcf

sfxc_vex2ccf_SOURCES = sfxc_vex2ccf.cc \
  @top_srcdir@/lib/vex_parser/vexplus.cc

sfxc_vex2dcf_SOURCES = sfxc_vex2dcf.cc \
  @top_srcdir@/lib/vex_parser/vexplus.cc