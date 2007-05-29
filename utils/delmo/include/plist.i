!
!  modified:
!  JMG 960730  Made so that size of parm_fill is set automatically.
!  pet 990106  Renamed the common block since its old names coincided with
!              the name of subroutine PARMS
!
      INTEGER*2     PARM_NUM,
     .              PARM_NAMES(10,MAX_PAR),
     .              PARM_FILL(JPLIST_FILL+1)
      CHARACTER*20  CPARM_NAMES(MAX_PAR)
      EQUIVALENCE ( PARM_NAMES,CPARM_NAMES)
      COMMON / PARMS_COMMON / PARM_NUM, &    ! Total number of parameters
     .                        PARM_NAMES, &  ! Array of parameters name
     .                        PARM_FILL   ! Filler
