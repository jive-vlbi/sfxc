CC=gcc
CFLAGS=-fPIC
YACC=bison
YFLAGS=-d -y
LEX=flex

OBJECTS=vex.o vex.yy.o vex_util.o print_vex.o vex_get.o

libvex.a: $(OBJECTS)
	touch $@
	rm $@
	ar -qc $@ $(OBJECTS)
	ranlib $@
	rm -f vex.c vex.yy.c

vex.yy.o:	vex.yy.l y.tab.h

y.tab.h:	vex.y
	$(YACC) $(YFLAGS) vex.y

vex_util.o:	y.tab.h

print_vex.o:	y.tab.h

vex_get.o:	y.tab.h

vex_put.o:	y.tab.h

vexf.o:		y.tab.h
