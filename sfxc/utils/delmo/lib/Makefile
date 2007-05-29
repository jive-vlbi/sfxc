FC =		g77 -m32
FFLAGS = 	-I../include -ffixed-line-length-80 -fno-underscoring

OBJS =	 	cwobm.o 	\
		cutcu.o 	\
		cuvm.o 		\
		cvecu.o 	\
		ctheu.o 	\
		cstrt.o 	\
		cstrm.o 	\
		csitm.o 	\
		csitu.o 	\
		crosu.o 	\
		cprem.o 	\
		cptdm.o 	\
		cquit.o 	\
		cplxm.o 	\
		cpepu.o 	\
		cobse.o 	\
		cocem.o 	\
		cnutu.o 	\
		cnutm.o 	\
		cmatu.o 	\
		cm20u.o 	\
		cmabd.o 	\
		cmain.o 	\
		cetdm.o 	\
		chead.o 	\
		cinit.o 	\
		ckill.o 	\
		cdrvr.o 	\
		cctiu.o 	\
		cdiuu.o 	\
		caxom.o 	\
		catmm.o 	\
		cut1m.o 	\
		catiu.o 	

OBJS +=		getunit.o

all: libfcalc.a

libfcalc.a: $(OBJS)
	ar -r libfcalc.a $(OBJS)
	ranlib libfcalc.a

clean:
	-rm $(OBJS)
	-rm libfcalc.a
