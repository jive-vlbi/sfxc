!@This is the start of file &RESFL
!
      INTEGER*2 IRSITE(2), IRSTAR, IRUNW, NAMB, IRPNTR, SUPSTAT_RES(2),
     .          UACSUP_RES
!
!           JMGipson added PDERR, PRERR =post_fit errors.
! 01-MAY-98 pet      added SUPSTAT
! 26-JUL-99 pet      added UACSUP
!
      INTEGER*2 IRESCM(JRESREC_WORDS)
      REAL*8 RDOC,RROC,RDERR,RRERR,PDERR,PRERR,RELEV(2)
      REAL*8 RFJD,RFRCT,RFAMB
!
      COMMON / RESFL / RDOC, RROC, RDERR, RRERR, PDERR, PRERR, RFJD,
     .                 RFRCT, RELEV, RFAMB,
     .                 IRSITE, IRSTAR, IRUNW, NAMB, IRPNTR, SUPSTAT_RES,
     .                 UACSUP_RES
!
      EQUIVALENCE (RDOC,IRESCM(1))
