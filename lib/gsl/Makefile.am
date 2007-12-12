# remove the shared libraries

all-local: installed.txt

install-exec-local: installed.txt

clean-local:
	rm -rf gsl-1.9

distclean-local:
	rm -f installed.txt
	rm -rf gsl-1.9
	rm -rf install

installed.txt:
	tar xzvf gsl-1.9.tar.gz
	cd gsl-1.9; ./configure --prefix=@abs_top_srcdir@/lib/gsl/install
	cd gsl-1.9; $(MAKE) all install
	rm -f install/lib/*.so* install/lib/*.la*
	touch installed.txt