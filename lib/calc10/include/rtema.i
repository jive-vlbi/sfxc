!@This is the start of file &RTEMA
!
!  EMACOM declarations
!
      REAL*8 XMAT1(ELEMENTS_LONG), XMAT2(ELEMENTS_LONG)
      REAL*8 PROD(NUM_TRANSFORM,MAX_SRC_PAR),SCAL(NUM_TRANSFORM)
      REAL*8 B(NUM_TRANSFORM)
      REAL*8 MAPTRN(NUM_TRANSFORM,MAX_SRC_PAR)
      INTEGER*2 SRCXRF(MAX_PAR)
      REAL*8 MASPOS(2,MAX_SRC_RTSRC),TSTPOS(2,MAX_SRC_RTSRC)
      REAL*8 POSSIG(2,MAX_SRC_RTSRC),A(TRANSFORM_SIZE),DELTST(MAX_PAR)
!
      COMMON/EMACOM/XMAT1,XMAT2,PROD,B,MAPTRN,MASPOS,
     .              TSTPOS,A,SCAL,POSSIG,SRCXRF,DELTST
!