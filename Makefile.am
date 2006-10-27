SUBDIRS = src

dist_noinst_DATA = 

dist-test: dist
	tar -z -x -f dsr-pdb*.tar.gz -C /tmp
	cd /tmp/dsr-pdb*; ./configure; make
