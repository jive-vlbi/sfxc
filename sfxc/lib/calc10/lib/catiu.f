      SUBROUTINE ATIMA()
      IMPLICIT None
!
! 1.    ATIMA
! 1.1   ATIMA PROGRAM SPECIFICATION
! 1.1.1 ATIMA ADDS ENTRIES TO THE TABLE OF CONTENTS FOR THE ATIME UTILITY
!       ROUTINE TEXT MESSAGE. IT ALSO ADDS ENTRIES TO THE TABLE OF CONTENTS
!       FOR THE FLOW CONTROL MESSAGE.
!
! 1.2   ATIMA PROGRAM INTERFACE
! 1.2.4 DATA BASE ACCESS -
!            ACCESS CODES:
!              1.  'ATI MESS'  -  THE DATA BASE ACCESS CODE FOR THE
!                                 ATIME UTILITY ROUTINE TEXT MESSAGE.
!              2.  'ATI CFLG'  -  THE DATA BASE ACCESS CODE FOR THE ATIME
!                                 UTILITY ROUTINE FLOW CONTROL MESSAGE.
! 1.2.6 SUBROUTINE INTERFACE -
!             CALLER SUBROUTINES: TOCUP
!             CALLED SUBROUTINES: ADDA
! 1.2.9 PROGRAMMER - DALE MARKHAM  02/04/77
!                    PETER DENATALE 07/18/77
!                    BRUCE SCHUPLER 09/16/77
!                    SAVITA GOEL    06/04/87 (CDS FOR A900)
!                    Jim Ryan 89.07.25 Documentaton simplified.
!                    Jim Ryan 02Sept Integer*4 mods.
!
!     ATIMA Program Structure
!
!     ADD for ATIME utility text message.
      CALL ADDA (int2(1),'ATI MESS','ATIME Message Definition        ',
     . int2(40), int2(1), int2(1))
!
!     ADD for ATIME utility flow control message.
      CALL ADDA (int2(1),'ATI CFLG','ATIME Flow Control Message Def. ',
     . int2(40), int2(1), int2(1))
!
!     Normal Program Conclusion.
      RETURN
      END
!***********************************************************************
      SUBROUTINE ATIMI()
      IMPLICIT None
!
! 2.    ATIMI
! 2.1   ATIMI PROGRAM SPECIFICATION
! 2.1.1 ATIMI IS THE ATIME UTILITY ROUTINE INPUT AND INITIALIZATION SECTION.
! 2.2   ATIMI PROGRAM INTERFACE
! 2.2.2 COMMON BLOCKS USED -
!
      Real*8  ATMUTC(3), ROTEPH(2,20), A1UTC(3), A1DIFF(3)
      COMMON / EOPCM / ATMUTC, ROTEPH, A1UTC, A1DIFF
!           VARIABLES 'TO/FROM':
!            1. ATMUTC(3)   - THE 'TAI MINUS UTC' INFORMATION ARRAY. THIS ARRAY
!                             CONTAINS RESPECTIVELY THE EPOCH, VALUE, AND TIME
!                             RATE OF CHANGE OF 'TAI MINUS UTC'.
!                             (DAYS, SEC, SEC/SEC)
!
      INCLUDE 'ccon.i'
!            VARIABLES 'FROM':
!              1. KATIC - THE ATIME UTILITY ROUTINE FLOW CONTROL FLAG.
!              2. KATID - THE ATIME UTILITY ROUTINE DEBUG OUTPUT FLAG.
!
      INCLUDE 'inputs.i'
!            Variables from:
!              1. Input_EOP - T/F logical flag telling whether to use external
!                             EOP file input
!
! 2.2.3 PROGRAM SPECIFICATIONS -
!
      INTEGER*2 NDO(3), Kerr
      INTEGER*2      LATIU(40),      LON(40),    LOFF(40)
      CHARACTER*40 C_LATIU(2),     C_LON(2),   C_LOFF(2)
      EQUIVALENCE (C_LATIU,LATIU),(C_LON,LON),(C_LOFF,LOFF)
!
      DATA C_LATIU /
     .'ATIME Utility routine - VERSION # 3, Las',
     .'t modification - 89:12:12 Jim Ryan.     '/
!
      DATA C_LON /
     .'ATIME Utility routine is turned on.     ',
     .'                                        '/
!
      DATA C_LOFF /
     .'ATIME Utility routine is turned off.    ',
     .'                                        '/
!
! 2.2.4 DATA BASE ACCESS -
!            'GET' VARIABLES:
!              1. ATMUTC(3) - THE 'TAI MINUS UTC' INFORMATION ARRAY. THIS ARRAY
!                             CONTAINS RESPECTIVELY THE EPOCH, VALUE, AND TIME
!                             RATE OF CHANGE OF 'TAI MINUS UTC'.
!                             (DAYS, SEC, SEC/SEC)
!            'PUT' VARIABLES:
!              1. LATIU(40) - THE ATIME UTILITY ROUTINE TEXT MESSAGE.
!              2. LON(40)   - THE ATIME UTILITY 'TURNED ON' MESSAGE.
!              3. LOFF(40)  - THE ATIME UTILITY 'TURNED OFF' MESSAGE.
!            ACCESS CODES:
!              1. 'ATI MESS'  -  THE DATA BASE ACCESS CODE FOR THE
!                                 ATIME UTILITY ROUTINE TEXT MESSAGE.
!              2. 'TAI- UTC'  -  THE DATA BASE ACCESS CODE FOR THE
!                                 'TAI MINUS UTC' INFORMATION ARRAY.
!              3. 'ATI CFLG'  -  THE DATA BASE ACCESS CODE FOR THE
!                                 ATIME UTILITY FLOW CONTROL MESSAGE.
!
! 2.2.6 SUBROUTINE INTERFACE -
!             CALLER SUBROUTINES: INITL
!             CALLED SUBROUTINES: GET4, TERMINATE_CALC, PUTA
!
! 2.2.8 PROGRAM VARIABLES -
!           1. KERR   - THE DATA BASE ERROR RETURN FLAG.
!           2. NDO(3) - THE DATA BASE RETURN ARRAY INDICES.
!
! 2.2.9 PROGRAMMER - DALE MARKHAM  02/04/77
!                    PETER DENATALE 07/18/77
!                    BRUCE SCHUPLER 02/07/78
!                    Jim Ryan 89.07.25 Documentaton simplified.
!                    Jim Ryan 89.12.12 UNIX-like database interface
!                             implimented.
!                    D. Gordon 96.02.27 Double dimensioning of ATMUTC(3)
!                             removed, found by Warwick Wilson, ATNF.
!                    D. Gordon 98.05.01 Expanded and renamed common block
!                             ATICM to EOPCM. Pass in ATMUTC if using external
!                             EOP input.
!                    Jim Ryan 02Sept Integer*4 mods.
!                    Jim Ryan 03.03.10 Kill replaced with terminate_calc.
!
!     ATIMI Program Structure
!
!     PUT the ATIME utility text message.
      CALL PUTA ('ATI MESS      ', LATIU, int2(40), int2(1), int2(1))
!
!     PUT the flow control message based on the value of KATIC.
      IF (KATIC .NE. 1) CALL PUTA ('ATI CFLG      ', LON, int2(40),
     . int2(1), int2(1))
      IF (KATIC .EQ. 1) CALL PUTA ('ATI CFLG      ', LOFF, int2(40),
     . int2(1), int2(1))
!
      IF (.not. Input_EOP) THEN         ! Already have ATMUTC?
!      GET the 'TAI MINUS UTC' array from the database and check for errors.
        CALL GET4 ('TAI- UTC      ',ATMUTC,int2(3),int2(1),int2(1),NDO,
     .   KERR)
        IF ( KERR .EQ. 0 )  GO TO 300
         CALL TERMINATE_CALC ( 'ATIMI ', int2(1), KERR)
      ENDIF                             ! Already have ATMUTC?
!
!     Check KATID for debug output.
  300 IF ( KATID .EQ. 0 )  GO TO 500
      WRITE ( 6, 9)
    9 FORMAT (1X, "Debug output for subroutine ATIMI." )
!
      WRITE(6,8)' ATMUTC  ',ATMUTC
    8 FORMAT(A,4D25.16/(7X,5D25.16))
!
!     Normal conclusion.
  500 RETURN
      END
!***********************************************************************
      SUBROUTINE ATIME ( UTC, XJD, AT, DUTCAT )
      IMPLICIT None
!
! 3.    ATIME
! 3.1   ATIME PROGRAM SPECIFICATION
! 3.1.1 ATIME IS THE UTILITY ROUTINE WHICH COMPUTES THE ATOMIC TIME
!       FROM THE UTC TIME AND ALSO CALCULATES THE PARTIAL DERIVATIVE
!       OF THE UTC TIME WITH RESPECT TO THE ATOMIC TIME.
!
! 3.2   ATIME PROGRAM INTERFACE
      Real*8 UTC, XJD, AT, DUTCAT
!
! 3.2.1 CALLING SEQUENCE -
!          INPUT VARIABLES:
!            1. UTC - THE UTC FRACTION OF THE UTC DAY. (SEC/SEC)
!            2. XJD - THE JULIAN DATE AT 0:00 UTC OF THE DATE IN QUESTION.
!          OUTPUT VARIABLES:
!            1. AT     - THE ATOMIC TIME FRACTION OF THE ATOMIC TIME DAY. (DAYS)
!            2. DUTCAT - THE PARTIAL DERIVATIVE OF THE UTC TIME WITH RESPECT TO
!                        THE ATOMIC TIME. (SEC/SEC)
!
! 3.2.2 COMMON BLOCKS USED -
!
      Real*8           PI, TWOPI, HALFPI, CONVD, CONVDS, CONVHS, SECDAY
      COMMON / CMATH / PI, TWOPI, HALFPI, CONVD, CONVDS, CONVHS, SECDAY
!           VARIABLES 'FROM':
!             1. SECDAY - THE CONVERSION FACTOR OF COORDINATE TIME SECONDS PER
!                         COORDINATE TIME DAY. (SEC/DAY)
!
      Real*8  ATMUTC(3), ROTEPH(2,20), A1UTC(3), A1DIFF(3)
      COMMON / EOPCM / ATMUTC, ROTEPH, A1UTC, A1DIFF
!           VARIABLES 'TO/FROM':
!             1. ATMUTC(3) - THE 'TAI MINUS UTC' INFORMATION ARRAY. CONTAINS
!                            RESPECTIVELY THE EPOCH, VALUE, AND TIME RATE OF
!                            CHANGE OF 'TAI MINUS UTC'. (DAYS, SEC, SEC/SEC)
!
      INCLUDE 'ccon.i'
!           VARIABLES 'FROM':
!             1. KATIC - THE ATIME UTILITY ROUTINE FLOW CONTROL FLAG.
!             2. KATID - THE ATIME UTILITY ROUTINE DEBUG OUTPUT FLAG.
!
! 3.2.3 PROGRAM SPECIFICATIONS -
! 3.2.6 SUBROUTINE INTERFACE -
!             CALLER SUBROUTINES: DRIVG
!             CALLED SUBROUTINES: NONE
! 3.2.7 CONSTANTS USED - SECDAY
! 3.2.8 PROGRAM VARIABLES - NONE
! 3.2.9 PROGRAMMER - DALE MARKHAM  02/04/77
!                    PETER DENATALE 07/18/77
!                    BRUCE SCHUPLER 02/07/78
!                    Jim Ryan 89.07.25 Documentaton simplified.
!                    Jim Ryan 02Sept Integer*4 mods.
!
!   ATIME Program Structure
!
!     Compute the atomic time fraction of the atomic time day.
      AT = + ATMUTC(2) / SECDAY
     .     + ATMUTC(3) * ( XJD  -  ATMUTC(1) )
     .     + UTC
!
!     Compute the partial derivative of the UTC time with respect
!     to atomic time.
      DUTCAT = 1.D0 / ( 1.D0  +  ATMUTC(3) )
!
!     Check KATIC to see if the utility is to be turned off.
      IF ( KATIC .NE. 1 )  GO TO 400
        AT     = UTC
        DUTCAT = 1.D0
!
!     Check KATID for debug output.
  400 IF ( KATID .EQ. 0 )  GO TO 500
      WRITE ( 6, 9)
    9 FORMAT (1X, "Debug output for utility ATIME." )
!
      WRITE(6,8)' ATMUTC  ',ATMUTC
    8 FORMAT(A,4D25.16/(7X,5D25.16))
      WRITE(6,8)' SECDAY  ',SECDAY
      WRITE ( 6, 9200 )  UTC, XJD, AT, DUTCAT
 9200 FORMAT (1X, "UTC    = ", D30.16, /, 1X,
     .            "XJD    = ", D30.16, /, 1X,
     .            "AT     = ", D30.16, /, 1X,
     .            "DUTCAT = ", D30.16 )
!
  500 RETURN
      END
