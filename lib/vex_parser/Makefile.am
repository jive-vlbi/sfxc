AM_YFLAGS = -d
AM_CFLAGS=-fPIC

noinst_LIBRARIES = libvex.a

libvex_a_SOURCES = vex.c vex.yy.c vex_util.c print_vex.c vex_get.c

# Make sure y.tab.h does not overwrite vex.h
vex.c: vex.y
	$(YACC) $(YFLAGS) $(AM_YFLAGS) vex.y
	cp -f y.tab.c vex.c
	