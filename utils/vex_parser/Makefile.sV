CC = g++
sV:     showVex.o vexplus.o libvex.a
	$(CC) -o sV vexplus.o  showVex.o -L. -lvex -lfl 
#-L lvex: link the local libvex.a
#-lfl: link the flex library, thjs one should come after lvex
showVex.o: showVex.cc
	$(CC) -c showVex.cc
vexplus.o: vexplus.C vexplus.h
	$(CC) -c vexplus.C
