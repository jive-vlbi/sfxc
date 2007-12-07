all: library

library:
	scons prefix=${prefix} -j 2

install: library
	scons prefix=${prefix} install 

uninstall:
	scons uninstall -c

clean:
	scons -c
	rm -f *~
	rm -f src/*~
	rm -f tools/*.pyc
