!@This is the start of file &SAREQ
!
!     Some important equivalence + dimension statements:
!     Note that DABF is REAL*8 to insure double precision even in
!          programs which use extended precision.
!     JABF is equivalenced to IABF to prevent doubleword boundary error. (??)
!                                             ( no such thing! )
!
      CHARACTER*8 QABF(5)
      INTEGER*4 N4BF(5)
      INTEGER*2  IABF(21), JABF(24), NABF(20), ITPR
      REAL*4     RABF(8)
      REAL*8     DABF(4)
!
      COMMON/TURF/JABF
      EQUIVALENCE (JABF(4), IABF(1), ITPR )
      EQUIVALENCE (JABF(5), NABF(1), QABF(1),N4BF(1))
      EQUIVALENCE (JABF(9), DABF(1), RABF(1))
!
