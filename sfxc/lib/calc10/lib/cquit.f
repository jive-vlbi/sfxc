      SUBROUTINE QUIT_CALC(IPAR)
      IMPLICIT NONE
!
!     QUIT_CALC send a message to the scheduling program and quits.
!
!     Calling sequence -
!
!        Input variables:
!
!        1.  IPAR(5)   -  The array to be sent to the scheduler.
!
      INTEGER*2 IPAR(5)
!
!     CALL PRTN(IPAR)
      STOP
      END
