      SUBROUTINE UTCTM ( UTC, XJD )
      IMPLICIT None
!
! 1.1.1 UTCTM is the utility which fetches the UTC time tag of the observation
!       and computes the Julian date at 0 hours UTC of the date of the obs.
!
! 1.1.2 RESTRICTIONS - Computation of Julian date correct for 1901 to 2099 only.
!
! 1.1.3 REFERENCES - ALMANAC FOR COMPUTERS - 1981, Nautical Almanac Office,
!                    United States Naval Observatory, Washington, D.C., 20390
!
! 1.2   UTCTM PROGRAM INTERFACE
!
! 1.2.1 CALLING SEQUENCE -
!           INPUT VARIABLES: None
!           OUTPUT VARIABLES:
!             1. UTC  -  THE UTC TIME AS A FRACTION OF THE UTC DAY. (SEC/SEC)
!             2. XJD  -  THE JULIAN DATE AT ZERO HOURS UTC OF THE DATE IN
!                        QUESTION. (DAYS)
!
! 1.2.2 COMMON BLOCKS USED -
!
      Real*8           PI, TWOPI, HALFPI, CONVD, CONVDS, CONVHS, SECDAY
      COMMON / CMATH / PI, TWOPI, HALFPI, CONVD, CONVDS, CONVHS, SECDAY
!           VARIABLES 'FROM':
!             1. SECDAY  -  THE CONVERSION FACTOR OF COORDINATE TIME SECONDS
!                           PER COORDINATE TIME DAY. (SEC/DAY)
!           VARIABLES 'TO': NONE
!
      INCLUDE 'ccon.i'
!           VARIABLES 'FROM':
!             1. KUTCC  -  THE UTCTM UTILITY ROUTINE FLOW CONTROL FLAG.
!             2. KUTCD  -  THE UTCTM UTILITY ROUTINE FLOW CONTROL FLAG.
!           VARIABLES 'TO': NONE
!
! 1.2.3 PROGRAM SPECIFICATIONS -
      Integer*2 ITAG(5), KERR(3), NDO(3), ndum
      Real*8    TAGSEC, UTC, XJD, JDY2K
      Integer*4 N, NN, IYY, IM, ID
!
! 1.2.4 DATA BASE ACCESS -
!          'GET' VARIABLES:
!             1. ITAG(5) - THE ARRAY CONTAINING THE YEAR/MONTH/DAY/HOUR/MINUTE
!                          PORTION OF THE OBSERVATION TIME TAG.
!             2. TAGSEC  - THE SECONDS PORTION OF THE OBSERVATION TIME TAG.
!          'PUT' VARIABLES: NONE
!          ACCESS CODES:
!             1. 'UTC TAG '  - THE DATA BASE ACCESS CODE FOR THE
!                              YEAR/MONTH/DAY/HOUR/MINUTE PORTION OF THE
!                              OBSERVATION TIME TAG ARRAY. (Year is 2-digit
!                              in the Mark III analysis system.)
!             l.5 'UTC TAG4' - THE DATA BASE ACCESS CODE FOR THE YEAR, MONTH,
!                              DAY, HOUR, AND MINUTE PORTION OF THE UTC
!                              OBSERVATION TIME TAG ARRAY. (New proposed
!                              L-code. Year will be 2-digits in the Mark
!                              III analysis system.)
!             2. 'SEC TAG '  - THE DATA BASE ACCESS CODE FOR THE SECONDS
!                              PORTION OF THE OBSERVATION TIME TAG.
!
! 1.2.6 SUBROUTINE INTERFACE -
!          CALLER SUBROUTINES: DRIVG
!          CALLED SUBROUTINES: DFLOTI, GETI, GET4, TERMINATE_CALC, JDY2K
!
! 1.2.7 CONSTANTS USED - SECDAY
!
! 1.2.8 PROGRAM VARIABLES -
!             1. KERR(3)  -  THE DATA BASE ERROR RETURN FLAGS.
!             2. NDO(3)   -  THE DATA BASE RETURN ARRAY INDICES.
!             3. IYY, IM, ID - TEMPORARY VARIABLES USED IN COMPUTING XJD.
!
! 1.2.9 PROGRAMMER - DALE MARKHAM  01/17/77
!                    PETER DENATALE 07/18/77
!                    JIM RYAN 09/14/81
!                         CHANGED TO COMPUTE XJD, RATHER THAN 'GET' IT.
!                    SAVITA GOEL 06/04/87 (CDS FOR A900)
!                    Jim Ryan 89.07.26 Documentation simplified.
!                    Jim Ryan 89.12.12 UNIX-like database interface
!                    implimented.
!                    D. Gordon 94.04.13 - Implicit none instituted. DFLOT
!                              changed to Fortran 77 DFLOTI.
!                    D. Gordon 98.07.29 - Convert to use JDY2K to convert year,
!                              month, day to Julian date. Year can be 2-digit
!                              or 4-digit.
!                    D. Gordon 98.11.04 Added GETI of 'UTC TAG4' (4-digit
!                              year). If not there (will not be there in Mark
!                              III system for a while) will get 'UTC TAG '
!                              (2-digit year) as before.
!                    Jim Ryan Sept 2002 Integer*2/4 mods.
!                    Jim Ryan 03.03.10 Kill replaced with terminate_solve
!
!     UTCTM Program Structure
!
!     GET the time tag information from the database. Check for db errors.
!    First try to get new time tag with 4-digit year. If not there, get old
!     time tag with 2-digit year. Doesn't really matter though.
      CALL GETI ('UTC TAG4      ', ITAG, int2(5), int2(1), int2(1),
     .     NDO, KERR(1))
       If (KERR(1) .ne. 0) CALL GETI ('UTC TAG       ', ITAG, int2(5),
     .     int2(1), int2(1), NDO, KERR(1))
      CALL GET4 ('SEC TAG       ', TAGSEC, int2(1), int2(1), int2(1),
     .     NDO, KERR(2))
      DO 200  N = 1,2
        NN = N
        IF ( KERR(N) .EQ. 0 )  GO TO 200
        CALL TERMINATE_CALC ('UTCTM ', NN, KERR(NN) )
  200 CONTINUE
!
!  Compute the Julian date at 0 hours UTC for the year, month, day.
!  Use function JDY2K to convert year, month, day to Julian date.
!  Year can be either 2-digit or 4-digit.
      IYY = ITAG(1)
      IM  = ITAG(2)
      ID  = ITAG(3)
      XJD = JDY2K(IYY,IM,ID)
!     write(6,'("UTCTM: ITAG(1), New XJD = ",i5,f12.2)') IYY,XJD
!
!     Compute the UTC time as a fraction of the UTC day.
      UTC = ( DFLOAT ( ITAG(4) ) * 3600.D0
     .      + DFLOAT ( ITAG(5) ) * 60.D0
     .      + TAGSEC ) / SECDAY
!
!     Check for debug.
      IF ( KUTCD .EQ. 0 )  GO TO 500
      WRITE ( 6, 9)
    9 FORMAT (1X, "Debug output for subroutine UTCTM." )
      WRITE(6,7)' ITAG    ',ITAG
    7 FORMAT(A,15I8/(9X,15I8))
      WRITE(6,8) 'SECDAY',SECDAY
    8 FORMAT(A,4D25.16/(7X,5D25.16))
      WRITE(6,8)' TAGSEC  ',TAGSEC
      WRITE ( 6, 9200 )  UTC, XJD
 9200 FORMAT (1X, "UTC  = ", D30.16, /, 1X,
     .            "XJD  = ", D30.16 )
!
  500 RETURN
      END
