AM_YFLAGS = -d -y

noinst_LIBRARIES = libvex.a

libvex_a_SOURCES = y.tab.h vex.y vex.yy.c vex_util.c print_vex.c vex_get.c

y.tab.h: vex.y
	$(YACC) $(AM_YFLAGS) $(YFLAGS) vex.y
