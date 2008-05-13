all:

clean:
	rm -f *~ *.pyc
	make -C site clean
	make -C site/cgi-bin clean
	make -C site/static clean
