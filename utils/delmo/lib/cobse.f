      SUBROUTINE OBSNT ( KOUNT, KEND)
      IMPLICIT None
!
! 1.    OBSNT
!
! 1.1   OBSNT PROGRAM SPECIFICATION
!
! 1.1.1 OBSNT calls the database handler to fetch an observation. If the
!       end-of-data is detected, a flag is set and CALC finishes processing the
!       experiment. Before each new observation is processed, OBSNT writes the
!       observation number, time tag, the baseline identification, and the
!       source name. OBSNT is called once for each observation.
!
! 1.2   OBSNT PROGRAM INTERFACE
!
! 1.2.1 CALLING SEQUENCE -
!            INPUT VARIABLES:
!              1.  KOUNT  -  THE VARIABLE WHICH INITIALIZES THE COUNTER OF THE
!                            OBSERVATION ITEMS TO ZERO IN SUBROUTINE INITL.
!            OUTPUT VARIABLES:
!              1.  KEND   -  THE VARIABLE WHICH FLAGS WHEN AN END OF FILE HAS
!                            BEEN REACHED. (NOTE: KEND = 0 MEANS PROCESS
!                            ANOTHER OBSERVATION ITEM, KEND = 1 MEANS THAT THE
!                            END OF FILE HAS BEEN REACHED.)
!
! 1.2.2 COMMON BLOCKS USED -
      INCLUDE 'ccon.i'
!
! 1.2.3 PROGRAM SPECIFICATIONS -
      Integer*2 ITAG(5), KERR(5), LNBASE(4,2), LSTRNM(4), NDO(3), NN
      Integer*4 KOUNT, KEND, N
      Real*8  TAGSEC
!
! 1.2.4 DATA BASE ACCESS - CALL SUBROUTINE MVREC TO OBTAIN THE NEXT OBSERVATION
!                          FROM THE DATA BASE. CALL SUBROUTINE FINIS TO CLOSE
!                          THE DATA BASE.
!            'GET' VARIABLES:
!              1. ITAG(5)     -  AN ARRAY USED TO STORE THE YEAR, MONTH, DAY,
!                                HOUR, AND MINUTE OF THE OBSERVATION TIME TAG.
!              2. LNBASE(4,2) -  AN ARRAY USED TO STORE THE EIGHT CHARACTER
!                                BASELINE IDENTIFICATIONS OF THE CURRENT
!                                OBSERVATION. (ALPHAMERIC)
!              3. LSTRNM(4)   -  AN ARRAY USED TO STORE THE EIGHT CHARACTER
!                                STAR NAME OF THE CURRENT OBSERVATION.
!                                (ALPHAMERIC)
!              4. TAGSEC      -  AN ARRAY USED TO STORE THE SECONDS PORTION OF
!                                THE OBSERVATION TIME TAG. (SEC)
!            ACCESS CODES:
!              1. 'UTC TAG ' -  THE DATA BASE ACCESS CODE FOR THE YEAR, MONTH,
!                               DAY, HOUR, AND MINUTE PORTION OF THE UTC
!                               OBSERVATION TIME TAG ARRAY. (Year is 2-digit
!                               in the Mark III analysis system.)
!              2. 'UTC TAG4' -  THE DATA BASE ACCESS CODE FOR THE YEAR, MONTH,
!                               DAY, HOUR, AND MINUTE PORTION OF THE UTC
!                               OBSERVATION TIME TAG ARRAY. (New proposed
!                               L-code. Year will be 2-digits in the Mark
!                               III analysis system.)
!              3. 'BASELINE' -  THE DATA BASE ACCESS CODE FOR THE BASELINE
!                               IDENTIFICATION ARRAY.
!              4. 'STAR ID ' -  THE DATA BASE ACCESS CODE FOR THE SOURCE
!                               IDENTIFICATION ARRAY.
!              5. 'SEC TAG ' -  THE DATA BASE ACCESS CODE FOR THE SECONDS
!                               PORTION OF THE UTC OBSERVATION TIME TAG.
!
! 1.2.5 EXTERNAL INPUT/OUTPUT -
!            OUTPUT VARIABLES:
!              1. THE OBSERVATION NUMBER, TIME TAG, BASELINE IDENTIFICATION,
!                 AND SOURCE IDENTIFICATION. (NOTE: THESE VARIABLES ARE
!                 WRITTEN OUT ONCE FOR EACH OBSERVATION ITEM PROCESSED.)
!              2. POSSIBLE ERROR OUTPUT
!
! 1.2.6 SUBROUTINE INTERFACE -
!             CALLER SUBROUTINES: MAIN
!             CALLED SUBROUTINES: GETA, GETI, GET4, TERMINATE_CALC, MVREC, FINIS
!
! 1.2.7 PROGRAM VARIABLES -
!           1. KEND    - THE VARIABLE WHICH FLAGS WHEN AN END OF FILE HAS BEEN
!                        REACHED.  (NOTE: KEND = 0 MEANS PROCESS ANOTHER
!                        OBSERVATION ITEM, KEND = 1 MEANS THAT THE END OF FILE
!                        HAS BEEN REACHED.)
!           2. KERR(5) - THE DATA BASE ERROR RETURN FLAGS.
!           3. NDO(3)  -
!
! 1.2.9 PROGRAMMER - DALE MARKHAM  01/12/77
!                    PETER DENATALE 07/06/77
!                    BRUCE SCHUPLER 05/11/78
!                    BRUCE SCHUPLER 12/05/78
!                    SAVITA GOEL    06/03/87 (CDS FOR A900)
!                    Jim Ryan 89.07.25 Documentation simplified.
!                    D. Gordon 94.05.23 Changed $Include to Include.
!                    D. Gordon 94.06.08 Corrected format statement.
!                    D. Gordon 98.10.13 Changed year field to I4 in output
!                       format for 4-digit years.
!                    D. Gordon 98.11.04 Added GETI of 'UTC TAG4' (4-digit
!                       year). If not there (will not be there in Mark III
!                       system for a while) will get 'UTC TAG ' (2-digit
!                       year) as before.
!                    Jim Ryan Sept 2002 Integer*2/4 mods.
!                    Jim Ryan 03.03.10 Kill replaced with terminate_calc.
!
!   OBSNT Program Structure
!
!   Try to move to the next observation.
      CALL MVREC (int2(2), int2(1), int2(1), KERR(1))
!
!   Check the error return to determine course of action.
!   KERR = 0 says observation found. KERR = 1 says end-of-file.
!   First check for database failure.
      IF ( KERR(1) .NE. 0 ) THEN
        IF ( KERR(1) .EQ. 1 )  GO TO 810
          CALL TERMINATE_CALC ('OBSNT ', int2(1), KERR(1))
      ENDIF
!
!   GET the time tag, the baseline id, and source name.
!    First try to get new time tag with 4-digit year. If not there, get old
!     time tag with 2-digit year. Doesn't really matter though.
      CALL GETI ('UTC TAG4      ',ITAG,int2(5),int2(1),int2(1),NDO,
     .           KERR(2))
       IF (KERR(2) .ne. 0) CALL GETI ('UTC TAG       ',ITAG,int2(5),
     .           int2(1),int2(1),NDO,KERR(2))
      CALL GET4 ('SEC TAG       ',TAGSEC,int2(1),int2(1),int2(1),NDO,
     .           KERR(3))
      CALL GETA ('BASELINE      ',LNBASE,int2(4),int2(2),int2(1),NDO,
     .           KERR(4))
      CALL GETA ('STAR ID       ',LSTRNM,int2(4),int2(1),int2(1),NDO,
     .           KERR(5))
!
!   Check for database errors and TERMINATE_CALC if found.
      DO  N = 1,5
        NN = N
        IF (KERR(N) .NE. 0) CALL TERMINATE_CALC ('OBSNT ',NN,KERR(NN))
      ENDDO
!
!   Kick the counter for another observation and set end-of-data flag to zero.
      KEND = 0
      KOUNT = KOUNT + 1
!
!   Write the observation line to the screen.
      IF (ILUOUT .NE. -1)
     .WRITE ( 6, 9000 )  KOUNT, ITAG, TAGSEC, LNBASE, LSTRNM
 9000 FORMAT (1X,I6,1X,I4,'/',I2,'/',I2,1X,I2,':',I2,F6.2,
     . 1X,4A2,'-',4A2, 1X, 4A2 )
      GO TO 820
!
!  End-of-file encountered, so set KEND and close the database.
  810 KEND = 1
      CALL FINIS( int2(0))
!
  820 RETURN
      END
