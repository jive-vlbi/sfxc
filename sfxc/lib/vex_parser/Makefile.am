AM_YFLAGS = -d
#AM_CFLAGS=-fPIC

noinst_LIBRARIES = libvex.a

libvex_a_SOURCES = vex.c vex.yy.c vex_util.c print_vex.c vex_get.c

y.tab.c: vex.y
	$(YACC) $(YFLAGS) $(AM_YFLAGS) vex.y
y.tab.h: y.tab.c

# Make sure y.tab.h does not overwrite vex.h
vex.c: y.tab.c
	cp -f y.tab.c vex.c

vex.yy.c: vex.yy.l
	$(LEX) -t vex.yy.l > vex.yy.c

vex_util.o: vex_util.c y.tab.h
print_vex.o: print_vex.c y.tab.h
vex_get.o: vex_get.c y.tab.h
vex.yy.o: vex.yy.c y.tab.h

