!@This is the start of file &ADDCM
!
!  92.01.13  MWH  Expanded IBUFF to handle 1536 parameters
!
      LOGICAL*2
     .             STDALN
      CHARACTER*64
     .             CGMINN,THINGN,IONAM
      CHARACTER*4
     .             ADORSB,ARORCG
      INTEGER*2
     .             NPARMT,NPARMC,NPARMF,IXTTF(MAX_PAR),IXCTF(MAX_PAR)
      INTEGER*2    IBUFF(106+2*MAX_PAR)
      COMMON/BUFF/
!     LOGICAL*2
     .             STDALN,
!     CHARACTER*64
     .             CGMINN,THINGN,IONAM,
!     CHARACTER*4
     .             ADORSB,ARORCG,
!     INTEGER*2
     .             NPARMT,NPARMC,NPARMF,IXTTF,IXCTF
!
      EQUIVALENCE (IBUFF(1),STDALN)
