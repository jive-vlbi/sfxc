# remove the shared libraries

all-local: installed.txt

install-exec-local: installed.txt

clean-local:
	$(MAKE) -C vex_parser clean

distclean-local:
	rm -f installed.txt
	$(MAKE) -C vex_parser distclean
	rm -rf install

installed.txt:
	cd vex_parser; ./compile.sh
	cd vex_parser; \
	  ./configure --prefix=@abs_top_srcdir@/lib/vex_parser/install
	$(MAKE) -C vex_parser all
	$(MAKE) -C vex_parser install
	rm -f install/lib/*.so* install/lib/*.la*
	touch installed.txt