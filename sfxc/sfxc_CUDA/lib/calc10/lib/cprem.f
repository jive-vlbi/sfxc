      SUBROUTINE PREA()
      IMPLICIT None
!
! 1.1.1 PREA ADDS ENTRIES TO THE TABLE OF CONTENTS FOR THE PRECESSION MODULE
!       TEXT MESSAGE AND PARTIAL DERIVATIVES ARRAY. IT ALSO ADDS ENTRIES TO THE
!       TABLE OF CONTENTS FOR THE PRECESSION MODULE FLOW CONTROL MESSAGE.
!
! 1.2   PREA PROGRAM INTERFACE
!
! 1.2.4 DATA BASE ACCESS:
!           ACCESS CODES:
!             1. 'PRE MESS' - THE DATA BASE ACCESS CODE FOR THE PRECESSION
!                             MODULE TEXT MESSAGE.
!             2. 'PRE PART' - THE DATA BASE ACCESS CODE FOR THE PRECESSION
!                             MODULE PARTIAL DERIVATIVES ARRAY.
!             3. 'PRE CFLG' - THE DATA BASE ACCESS CODE FOR THE PRECESSION
!                             MODULE FLOW CONTROL MESSAGE.
!
! 1.2.6 SUBROUTINE INTERFACE -
!             CALLER SUBROUTINES: TOCUP
!             CALLED SUBROUTINES: ADDA, ADDR
!
! 1.2.9 PROGRAMMER - DALE MARKHAM  01/13/77
!                    PETER DENATALE 07/12/77
!                    BRUCE SCHUPLER 10/21/77
!                    SAVITA GOEL    06/03/87 (CDS FOR A900)
!                    Jim Ryan       89.07.07 Documentation simplified.
!                    Jim Ryan 89.12.12 UNIX-like database interface
!                             implimented.
!                    David Gordon 94.04.14 Converted to Implicit None
!                    Jim Ryan 02SEP Integer*4 upgrades.
!
!     PREA PROGRAM STRUCTURE
!
!   ADD for precession module text message.
      CALL ADDA (int2(1),'PRE MESS','Precession message definition   ',
     . int2(40), int2(1), int2(1))
!
!   ADDA_S for precession module flow control message.
      CALL ADDA (int2(1),'PRE CFLG','Precession flow contril mess def',
     . int2(40), int2(1), int2(1))
!
!   ADD for precession module partial derivatives.
      CALL ADDR (int2(2),'PRE PART','Precession partial deriv. def. ',
     . int2(2), int2(1), int2(1))
!
      RETURN
      END
!******************************************************************
      SUBROUTINE PREI()
      IMPLICIT None
!
! 3.    PREI
!
! 3.1.1 PREI IS THE PRECESSION MODULE INPUT AND INITIALIZATION SECTION.
!
! 3.1.2 RESTRICTIONS - NONE
! 3.1.3 REFERENCES - NONE
!
! 3.2   PREI PROGRAM INTERFACE
!
! 3.2.1 CALLING SEQUENCE - NONE
!
! 3.2.2 COMMON BLOCKS USED -
      INCLUDE 'ccon.i'
!        VARIABLES 'FROM':
!          1. KPREC - THE PRECESSION MODULE FLOW CONTROL FLAG.
!          2. KPRED - THE PRECESSION MODULE DEBUG OUTPUT FLAG.
!
      Real*8 CENTJ, CTHETA(3), CZEE(3), CZETA(3), DJ2000, HEPOCH,
     .    PRECON, RP1(3,3), RP2(3,3), RP3(3,3), THETA, ZEE, ZETA,CENTR,
     .    THET2(6), ZEE2(6), ZETA2(6), ZEE2K, DZEE2K, THET2K, DTHE2K,
     .         ZETA2K, DZET2K
      COMMON / PRECM / CENTJ, CTHETA, CZEE, CZETA, DJ2000, HEPOCH,
     .         PRECON, RP1, RP2, RP3, THETA, ZEE, ZETA, CENTR,
     .         THET2, ZEE2, ZETA2, ZEE2K, DZEE2K, THET2K, DTHE2K,
     .         ZETA2K, DZET2K
!        VARIABLES 'TO':
!           1. PRECON - THE PRECESSION CONSTANT. (ARCSEC/JCENTURY)
!
! 3.2.3 PROGRAM SPECIFICATIONS -
      INTEGER*2  NDO(3), KERR
      INTEGER*2      LPREM(40),      LON(40),    LOFF(40)
      CHARACTER*40 C_LPREM(2) ,    C_LON(2) ,  C_LOFF(2)
      EQUIVALENCE (C_LPREM,LPREM),(C_LON,LON),(C_LOFF,LOFF)
!
!     DATA C_LPREM /
!    .'Precession Module - Version # 2, last mo',
!    .'dification - 08/04/81, Chopo Ma         '/
      DATA C_LPREM /
     .'Precession Module - Last Modified 2004.0',
     .'3.19 - D. Gordon/GSFC.                  '/
!
      DATA C_LON  /
     .'Precession Module is turned ON.         ',
     .'                                        '/
!
      DATA C_LOFF /
     .'Precession module is turned OFF.        ',
     .'                                        '/
!
! 3.2.4 DATA BASE ACCESS -
!        'GET' VARIABLES:
!             1. PRECON  -  THE PRECESSION CONSTANT.  (ARCSEC/JCENTURY)
!        'PUT' VARIABLES:
!             1. LPREM(40)  -  THE PRECESSION MODULE TEXT MESSAGE.
!             2. LON(40)    -  THE PRECESSION MODULE TURNED ON MESSAGE.
!             3. LOFF(40)   -  THE PRECESSION MODULE TURNED OFF MESSAGE.
!        ACCESS CODES:
!             1. 'PRE MESS' -  THE DATABASE ACCESS CODE FOR THE PRECESSION
!                              MODULE TEXT MESSAGE.
!             2. 'PRE DATA' -  THE DATABASE ACCESS CODE FOR THE PRECESSION DATA.
!             3. 'PRE CFLG' -  THE DATABASE ACCESS CODE FOR THE PRECESSION
!                              MODULE FLOW CONTROL MESSAGE.
!
! 3.2.5 EXTERNAL INPUT/OUTPUT -
!             1. POSSIBLE DEBUG OUTPUT
!             2. POSSIBLE ERROR OUTPUT
!
! 3.2.6 SUBROUTINE INTERFACE -
!             CALLER SUBROUTINES: INITL
!             CALLED SUBROUTINES: GET4, TERMINATE_CALC, PUTA
!
! 3.2.8 PROGRAM VARIABLES -
!             1. KERR   -  THE DATA BASE ERROR RETURN FLAG.
!             2. NDO(3) -  THE DATA BASE RETURN ARRAY INDICES.
!
! 3.2.9 PROGRAMMER - DALE MARKHAM   01/13/77
!                    PETER DENATALE 07/12/77
!                    BRUCE SCHUPLER 10/21/77
!                    BRUCE SCHUPLER 06/05/78
!                    CHOPO MA       08/04/81
!                    Jim Ryan       89.07.07 Documentation simplified.
!                    Jim Ryan 89.12.12 UNIX-like database interface
!                             implimented.
!                    David Gordon 94.04.14 Converted to Implicit None
!                    Jim Ryan 02SEP Integer*4 upgrades.
!                    David Gordon 2003/2004 Updated for IERS Conventions 2003.
!                    Jim Ryan 03.03.10 Kill replaced with Terminate_Calc.
!
!     PREI PROGRAM STRUCTURE
!
!   PUT the precession module text message.
      CALL PUTA ('PRE MESS      ',LPREM,int2(40),int2(1),int2(1))
!
!   PUT the Precession Module control flag message.
      IF (KPREC .NE. 1) CALL PUTA('PRE CFLG      ',LON,int2(40),
     .    int2(1), int2(1))
      IF (KPREC .EQ. 1) CALL PUTA('PRE CFLG      ',LOFF,int2(40),
     .    int2(1), int2(1))
!
!   GET the precession constant from the database
      CALL GET4 ('PRE DATA      ',PRECON,int2(1),int2(1),int2(1),NDO,
     .     KERR)
      IF ( KERR .EQ. 0 ) GO TO 300
           CALL TERMINATE_CALC ('PREI  ', int2(1), KERR)
!
!   Check KPRED for debug output.
  300 IF ( KPRED .EQ. 0 ) GO TO 500
      WRITE ( 6, 9)
    9 FORMAT (1X, "Debug output for Subroutine PREI." )
      WRITE(6,8)' PRECON   ',PRECON
    8 FORMAT(A,4D25.16/(7X,5D25.16))
!
  500 RETURN
      END
!********************************************************************
      SUBROUTINE PREG (CT, CENT, EPSMNR, XJD, RPC2K)
      IMPLICIT None
!
! 4.    PREG
!
! 4.1.1 PREG CALCULATES THE PRECESSION PORTION OF THE COMPLETE CRUST FIXED TO
!       J2000.0 ROTATION MATRIX AND THE CT TIME DERIVATIVE OF THAT MATRIX.
!
! 4.1.2 RESTRICTIONS - NONE
!
! 4.1.3 REFERENCES -  1) LIESKE, J.H., PRECESSION MATRIX BASED ON IAU
!                        (1976) SYSTEM OF ASTRONOMICAL CONSTANTS,
!                        ASTRON. ASTROPHYS. 73, 282-284, 1979.
!                     2) ASH, M.E., "DETERMINATION OF EARTH SATELLITE
!                        ORBITS", LINCOLN LABORATORY TECHNICAL REPORT
!                        1972-5, 04/19/76, P. 57-59,
!                     3) MUELLER, I.V., "SPHERICAL AND PRACTICAL
!                        ASTRONOMY AS APPLIED TO GEODESY", 1969, P. 62-65.
!                        (NOTE: THE REFERENCE IN MUELLER REFERS TO THE
!                        COMPUTATION OF THE PRECESSION PORTION OF THE
!                        COMPLETE J2000.0 TO CRUST FIXED ROTATION MATRIX.
!                        HOWEVER, THE CALC PROGRAM REQUIRES THE TRANSPOSE OF
!                        THIS MATRIX. CARE MUST BE TAKEN WHEN COMPARING THE
!                        REFERENCE TO THE FOLLOWING PROGRAM.)
!
! 4.2   PREG PROGRAM INTERFACE
!
! 4.2.1 CALLING SEQUENCE -
!         INPUT VARIABLES:
!             1. CT      -  THE COORDINATE TIME FRACTION OF THE COORDINATE
!                           TIME DAY. (SEC/SEC)
!             2. EPSMNR  -  MEAN OBLIQUITY OF REFERENCE EPOCH J2000.0. (RAD)
!             3. XJD     -  THE JULIAN DATE AT ZERO HOURS UTC OF THE DATE IN
!                           QUESTION. (DAYS)
!             4. CENT    -  CT fraction of century from 2000.0.
!         OUTPUT VARIABLES:
!             1. RPC2K(3,3,2)-THE PRECESSION PORTION OF THE COMPLETE CRUST
!                            FIXED TO J2000.0 ROTATION MATRIX AND ITS CT TIME
!                            DERIVATIVE. (UNITLESS, 1/SEC)
!
      Real*8 CT, EPSMNR, XJD,            RPC2K(3,3,2)
!
! 4.2.2 COMMON BLOCKS USED -
!
      Real*8           PI, TWOPI, HALFPI, CONVD, CONVDS, CONVHS, SECDAY
      COMMON / CMATH / PI, TWOPI, HALFPI, CONVD, CONVDS, CONVHS, SECDAY
!           VARIABLES 'FROM':
!             1. CONVDS  -  THE CONVERSION FACTOR OF RADIANS PER ARC SECOND.
!             2. HALFPI  -  THE VALUE OF PI DIVIDED BY TWO. (RAD)
!             3. SECDAY  -  THE CONVERSION FACTOR OF COORDINATE TIME SECONDS PER
!                           COORDINATE TIME DAY. (SEC/DAY)
!
      Real*8 CENTJ, CTHETA(3), CZEE(3), CZETA(3), DJ2000, HEPOCH,
     .    PRECON, RP1(3,3), RP2(3,3), RP3(3,3), THETA, ZEE, ZETA,CENTR,
     .    THET2(6), ZEE2(6), ZETA2(6), ZEE2K, DZEE2K, THET2K, DTHE2K,
     .    ZETA2K, DZET2K
      COMMON / PRECM / CENTJ, CTHETA, CZEE, CZETA, DJ2000, HEPOCH,
     .         PRECON, RP1, RP2, RP3, THETA, ZEE, ZETA, CENTR,
     .         THET2, ZEE2, ZETA2, ZEE2K, DZEE2K, THET2K, DTHE2K,
     .         ZETA2K, DZET2K
!
!        VARIABLES 'FROM':
!            1. CENTJ     -  THE NUMBER OF COORDINATE TIME DAYS PER JULIAN
!                            CENTURY. (DAYS/CENTURY)
!            2. CTHETA(3) -  THE CONSTANTS APPEARING IN TERMS 1-3 IN THE
!                            CALCULATION OF THE PRECESSIONAL ELEMENT THETA.
!                            (ARCSEC)
!            3. CZEE(3)   -  THE CONSTANTS APPEARING IN TERMS 1-3 IN THE
!                            CALCULATION OF THE PRECESSIONAL ELEMENT ZEE.
!                            (ARCSEC)
!            4. CZETA(3)  -  THE CONSTANTS APPEARING IN TERMS 1-3 IN THE
!                            CALCULATION OF THE PRECESSIONAL ELEMENT ZETA.
!                            (ARCSEC)
!            5. DJ2000    -  THE JULIAN DATE OF THE EPOCH J2000.0. (DAYS)
!            6. HEPOCH    -  THE NOMINAL VALUE OF THE PRECESSION CONSTANT IN
!                            THE EPOCH J2000.0. (ARCSEC/JCENTURY)
!            7. PRECON    -  THE PRECESSION CONSTANT. (ARCSEC/JCENTURY)
!
!        VARIABLES 'TO':
!            1. RP1(3,3)  - THE ROTATION MATRIX WHICH PERFORMS A ROTATION ABOUT
!                           THE NORTH CELESTIAL POLE OF THE EPOCH 2000.0 BY THE
!                           ANGLE ZEE+HALFPI. (UNITLESS)
!            2. RP2(3,3) -  THE ROTATION MATRIX WHICH PERFORMS A ROTATION ABOUT
!                           THE ASCENDING NODE OF THE EQUATOR OF THE CURRENT
!                           EPOCH ON THE EQUATOR OF THE J2000.0 EPOCH BY THE
!                           ANGLE -THETA. (UNITLESS)
!            3. RP3(3,3) -  THE ROTATION MATRIX WHICH PERFORMS A ROTATION ABOUT
!                           THE NORTH CELESTIAL POLE OF THE CURRENT EPOCH BY
!                           THE ANGLE ZETA-HALFPI. (UNITLESS)
!            4. THETA    -  THE PRECESSIONAL ELEMENT THETA. (RAD)
!            5. ZEE      -  THE PRECESSIONAL ELEMENT ZEE. (RAD)
!            6. ZETA     -  THE PRECESSIONAL ELEMENT ZETA. (RAD)
!            7. CENTR    -  THE NUMBER OF JULIAN CENTURIES FROM J2000.0 TO THE
!                           EPOCH OF THE OBSERVATION. (CENTURIES)
!
      INCLUDE 'ccon.i'
!          VARIABLES 'FROM':
!            1. KPREC - THE PRECESSION MODULE FLOW CONTROL FLAG.
!            2. KPRED - THE PRECESSION MODULE DEBUG OUTPUT FLAG.
!
! 4.2.3 PROGRAM SPECIFICATIONS -
      Real*8 DRP1DT(3,3), DRP2DT(3,3), DRP3DT(3,3), DRPDT1(3,3),
     .       DRPDT2(3,3), DRPDT3(3,3)
      Real*8 dysj, dcentr, hc, hs, tzeta, tzee, ttheta
!     Real*8 ZETA2K, DZET2K, ZEE2K, DZEE2K, THET2K, DTHE2K,
      Real*8 CENT, CENTR2, CENTR3, CENTR4, CENTR5
      Integer*4 N, M
!****************************************************************
!** Temporary Test:
!      Real*8 Psi_A, Omega_A, Epsilon_A, Chi_A, TP1(3,3),
!    *        TP2(3,3), TP3(3,3), TP4(3,3), TP5(3,3), ABC(3,3)
!****************************************************************
!
! 4.2.6 SUBROUTINE INTERFACE -
!             CALLER SUBROUTINES: DRIVG
!             CALLED SUBROUTINES: DABS, DROTT, MADD3, MMUL3, ROTAT
!
! 4.2.7 CONSTANTS USED - CENTJ, CONVDS, DJ2000, SECDAY,
!                        CTHETA(3), CZEE(3), CZETA(3)
!
! 4.2.8 PROGRAM VARIABLES -
!             1. DCENTR       -  THE CT TIME DERIVATIVE OF CENTR. (CENTURIES/S)
!             2. DRP1DT(3,3)  -  THE CT TIME DERIVATIVE OF THE ROTATION MATRIX
!                                RP1. (1/SEC)
!             3. DRP2DT(3,3)  -  THE CT TIME DERIVATIVE OF THE ROTATION MATRIX
!                                RP2. (1/SEC)
!             4. DRP3DT(3,3)  -  THE CT TIME DERIVATIVE OF THE ROTATION MATRIX
!                                RP3. (1/SEC)
!             5. DRPDT1(3,3)  -  THE FIRST TERM OF THE CT TIME DERIVATIVE OF THE
!                                PRECESSION MATRIX. (1/SEC)
!             6. DRPDT2(3,3)  -  THE SECOND TERM OF THE CT TIME DERIVATIVE OF
!                                THE PRECESSION MATRIX. (1/SEC)
!             7. DRPDT3(3,3)  -  THE THIRD TERM OF THE CT TIME DERIVATIVE OF THE
!                                PRECESSION MATRIX. (1/SEC)
!             8. DYSJ         -  THE NUMBER OF JULIAN DAYS ELAPSED SINCE THE
!                                EPOCH J2000.0. (DAYS)
!             9. HC           -  THE TERM WHICH SHOWS THE DEPENDENCE OF SMALL
!                                CHANGES IN THE PRECESSION CONSTANT FROM ITS
!                                NOMINAL VALUE IN THE EPOCH J2000.0 ON THE
!                                PRECESSIONAL ELEMENTS  ZEE AND ZETA. (RAD)
!            10. HS           -  THE TERM WHICH SHOWS THE DEPENDENCE OF SMALL
!                                CHANGES IN THE PRECESSION CONSTANT FROM ITS
!                                NOMINAL VALUE IN THE EPOCH J2000.0 ON THE
!                                PRECESSIONAL ELEMENT THETA. (RAD)
!            11. TTHETA       -  THE CT TIME DERIVATIVE OF THE PRECESSIONAL
!                                ELEMENT THETA. (RAD/SEC)
!            12. TZEE         -  THE CT TIME DERIVATIVE OF THE PRECESSIONAL
!                                ELEMENT ZEE. (RAD/SEC)
!            13. TZETA        -  THE CT TIME DERIVATIVE OF THE PRECESSIONAL
!                                ELEMENT ZETA. (RAD/SEC)
!
! 4.2.9 PROGRAMMER - DALE MARKHAM   01/13/77
!                    PETER DENATALE 07/12/77
!                    BRUCE SCHUPLER 11/10/77
!                    BRUCE SCHUPLER 06/06/78
!                    CHOPO MA       08/04/81
!                    Jim Ryan       89.07.07 Documentation simplified.
!                    David Gordon   94.04.14 Converted to Implicit None
!                    Jim Ryan Sept 2002  Integer*2/4 upgrades.
!                    D. Gordon      2003/4 Updated for IERS 2003.
!
!     PREG PROGRAM STRUCTURE
!
!   Calculate the number of Julian centuries elapsed since epoch J2000.0.
      CENTR = CENT
!   Calculate the CT time derivative of CENTR.
      DCENTR = 1.D0 / ( CENTJ * SECDAY )
!         print *,' PREG: CENTR,DCENTR ', CENTR,DCENTR
!
!   Calculate the precessional elements ZETA, ZEE, and THETA and
!   their CT time derivatives.
!
!   Compute the terms which show the dependence of small changes of the
!   precession constant from its nominal value in the epoch J2000.0 on the
!   precessional elements.
!!!   HC = 0.D0
!!!   HS = 0.D0
!!!   IF ( DABS ( PRECON - HEPOCH ) .LT. 1.D-4 )  GO TO 220
!!!   HC =( PRECON - HEPOCH ) * DCOS ( EPSMNR ) * CONVDS *
!!!  1      CENTR / 2.D0
!!!   HS =( PRECON - HEPOCH ) * DSIN ( EPSMNR ) * CONVDS *
!!!  1      CENTR
!
!   Compute ZETA and its CT time derivative.
!!!20 Continue
!!!   ZETA = ( CZETA(1) * CENTR
!!!  1       + CZETA(2) * CENTR**2
!!!  2       + CZETA(3) * CENTR**3 ) * CONVDS
!!!  3       + HC
!!!   TZETA = ( CZETA(1)
!!!  1        + CZETA(2) * ( 2.D0 * CENTR )
!!!  2        + CZETA(3) * ( 3.D0 * CENTR**2 ) ) * CONVDS * DCENTR
!!!  3        + ( HC / CENTR ) * DCENTR
!
!   Compute ZEE and its CT time derivative.
!!!   ZEE = ( CZEE(1) * CENTR
!!!  1      + CZEE(2) * CENTR**2
!!!  2      + CZEE(3) * CENTR**3 ) * CONVDS
!!!  3      + HC
!!!   TZEE = ( CZEE(1)
!!!  1       + CZEE(2) * ( 2.D0 * CENTR )
!!!  2       + CZEE(3) * ( 3.D0 * CENTR**2 ) ) * CONVDS * DCENTR
!!!  3       + ( HC / CENTR ) * DCENTR
!
!   Compute THETA and its CT time derivative.
!!!   THETA = ( CTHETA(1) * CENTR
!!!  1        + CTHETA(2) * CENTR**2
!!!  2        + CTHETA(3) * CENTR**3 ) * CONVDS
!!!  3        + HS
!!!   TTHETA = ( CTHETA(1)
!!!  1         + CTHETA(2) * ( 2.D0 * CENTR )
!!!  2         + CTHETA(3) * ( 3.D0 * CENTR**2 ) ) * CONVDS * DCENTR
!!!  3         + ( HS / CENTR ) * DCENTR
!
!   Construct the precession matrix.
!
!   Construct the rotation matrix which rotates about the current north
!   celestial pole by an angle equal to ZEE+HALFPI.
!!!   CALL ROTAT ( ZEE+HALFPI, int2(3), RP1)
!
!   Construct the rotation matrix which rotates about the ascending node
!   of the equator by an angle equal to TO -THETA.
!!!   CALL ROTAT ( -THETA, int2(1), RP2)
!
!   Construct the rotation matrix which rotates about the J2000.0 north
!   celestial pole by an angle equal to ZETA - HALFPI.
!!!   CALL ROTAT ( ZETA-HALFPI, int2(3), RP3)
!
!   Complete the construction of the precession matrix.
!!!   CALL MMUL3 ( RP3, RP2, RP1, RP(1,1,1) )
!
!   Construct the CT time derivative of the precession matrix.
!
!   Construct the CT time derivative of the rotation matrix RP1.
!!!   CALL DROTT ( ZEE+HALFPI, TZEE, int2(3), DRP1DT)
!
!   Construct the CT time derivative of the rotation matrix RP2.
!!!   CALL DROTT ( -THETA, -TTHETA, int2(1), DRP2DT)
!
!   Construct the CT time derivative of the rotation mariix RP3.
!!!   CALL DROTT ( ZETA-HALFPI, TZETA, int2(3), DRP3DT)
!
!   Compute the three terms which make up the CT time derivative of the
!   precession matrix.
!!!   CALL MMUL3 ( DRP3DT, RP2, RP1, DRPDT1 )
!!!   CALL MMUL3 ( RP3, DRP2DT, RP1, DRPDT2 )
!!!   CALL MMUL3 ( RP3, RP2, DRP1DT, DRPDT3 )
!
!   Complete the construction of the CT time derivative of the precession
!   matrix.
!!!   CALL MADD3 ( DRPDT1, DRPDT2, DRPDT3, RP(1,1,2) )
!
!***************************************************************
       CENTR2 = CENTR**2
       CENTR3 = CENTR**3
       CENTR4 = CENTR**4
       CENTR5 = CENTR**5
!   Same terms using IAU2000 model:
      ZETA2K = (ZETA2(1)        + ZETA2(2)*CENTR  + ZETA2(3)*CENTR2 +
     .          ZETA2(4)*CENTR3 + ZETA2(5)*CENTR4 + ZETA2(6)*CENTR5)
     .           * CONVDS
      DZET2K = (ZETA2(2) + 2.D0*ZETA2(3)*CENTR + 3.D0*ZETA2(4)*CENTR2
     .           + 4.D0*ZETA2(5)*CENTR3 + 5.D0*ZETA2(6)*CENTR4)
     .           * CONVDS*DCENTR
!      print *, 'PREG: ZETA2K, DZET2K: ', ZETA2K, DZET2K
!
      ZEE2K  = (ZEE2(1)        + ZEE2(2)*CENTR  + ZEE2(3)*CENTR2 +
     .          ZEE2(4)*CENTR3 + ZEE2(5)*CENTR4 + ZEE2(6)*CENTR5)
     .           * CONVDS
      DZEE2K  = (ZEE2(2) + 2.D0*ZEE2(3)*CENTR + 3.D0*ZEE2(4)*CENTR2
     .           + 4.D0*ZEE2(5)*CENTR3 + 5.D0*ZEE2(6)*CENTR4)
     .           * CONVDS*DCENTR
!      print *, 'PREG: ZEE2K, DZEE2K: ', ZEE2K, DZEE2K
!
      THET2K = (THET2(1)        + THET2(2)*CENTR  + THET2(3)*CENTR2 +
     .          THET2(4)*CENTR3 + THET2(5)*CENTR4 + THET2(6)*CENTR5)
     .           * CONVDS
      DTHE2K = (THET2(2) + 2.D0*THET2(3)*CENTR + 3.D0*THET2(4)*CENTR2
     .          + 4.D0*THET2(5)*CENTR3 + 5.D0*THET2(6)*CENTR4)
     .           * CONVDS*DCENTR
!      print *, 'PREG: THET2K, DTHE2K: ', THET2K, DTHE2K
!
!   Construct the IAU2000 precession matrix.
!
!   Construct the rotation matrix which rotates about the current north
!   celestial pole by an angle equal to ZEE+HALFPI.
      CALL ROTAT ( ZEE2K+HALFPI, int2(3), RP1)
!
!   Construct the rotation matrix which rotates about the ascending node
!   of the equator by an angle equal to TO -THETA.
      CALL ROTAT ( -THET2K, int2(1), RP2)
!
!   Construct the rotation matrix which rotates about the J2000.0 north
!   celestial pole by an angle equal to ZETA - HALFPI.
      CALL ROTAT ( ZETA2K-HALFPI, int2(3), RP3)
!
!   Complete the construction of the precession matrix.
      CALL MMUL3 ( RP3, RP2, RP1, RPC2K(1,1,1) )
!
!   Construct the CT time derivative of the precession matrix.
!
!   Construct the CT time derivative of the rotation matrix RP1.
      CALL DROTT ( ZEE2K+HALFPI, DZEE2K, int2(3), DRP1DT)
!
!   Construct the CT time derivative of the rotation matrix RP2.
      CALL DROTT ( -THET2K, -DTHE2K, int2(1), DRP2DT)
!
!   Construct the CT time derivative of the rotation mariix RP3.
      CALL DROTT ( ZETA2K-HALFPI, DZET2K, int2(3), DRP3DT)
!
!   Compute the three terms which make up the CT time derivative of the
!   precession matrix.
      CALL MMUL3 ( DRP3DT, RP2, RP1, DRPDT1 )
      CALL MMUL3 ( RP3, DRP2DT, RP1, DRPDT2 )
      CALL MMUL3 ( RP3, RP2, DRP1DT, DRPDT3 )
!
!   Complete the construction of the CT time derivative of the precession
!   matrix.
      CALL MADD3 ( DRPDT1, DRPDT2, DRPDT3, RPC2K(1,1,2) )
!
!***************************************************************
! Temporary Test:
!  Expressions of Lieske, modified for IAU2000A nutation/precession
!
!      Psi_A     = (5038.47875D0*CENTR - 1.07259D0*CENTR2 -
!    *             .001147D0*CENTR3) * CONVDS
!      Omega_A   = (84381.448D0 - .02524D0*CENTR +
!    *             .05127D0*CENTR2 - .007726D0*CENTR3) * CONVDS
!      Epsilon_A = (84381.448D0 - 46.84024D0*CENTR -
!    *             .00059D0*CENTR2 + .001813D0*CENTR3) * CONVDS
!      Chi_A     = (10.5526D0*CENTR - 2.38064D0*CENTR2 -
!    *             .001125D0*CENTR3) * CONVDS
!
!     CALL ROTAT ((-84381.448D0*CONVDS), int2(1), TP1)
!     CALL ROTAT (Psi_A, int2(3), TP2)
!     CALL ROTAT (Omega_A, int2(1), TP3)
!     CALL ROTAT (-Chi_A, int2(3), TP4)
!     CALL MMUL3 (TP1, TP2, TP3, ABC)
!     CALL MMUL2 (ABC, TP4, TP5)
!
!      Write(6,1014) RPC2K
 1014  Format(1x,'PREG/Precession Matrix:',(6(/,3E25.15)))
!      Write(6,1016) TP5
!1016  Format(1x,'PREG/Alternate Precession Matrix:',
!    *  (3(/,3E25.15)))
!
!     DO 57 N=1,3
!       DO 56 M=1,3
!        RPC2K(M,N,1) = TP5(M,N)
!56     Continue
!57   Continue
!
!***************************************************************
!
!   Check KPREC to determine if the precession module is to be turned off.
      IF (KPREC .NE. 1)  GO TO 600
!
!   It's turned off, so set the position portion of the precession matrix to
!   the identity matrix and zero out the velocity matrix.
      CALL ROTAT (0.D0, int2(3), RPC2K(1,1,1))
      DO 500 N=1,3
        DO 500 M=1,3
          RPC2K(M,N,2) = 0.0D0
  500 CONTINUE
!
!  Check KPRED for debug.
  600 IF ( KPRED .EQ. 0 ) GO TO 700
      WRITE ( 6, 9)
    9 FORMAT (1X, "Debug output for subroutine PREG." )
!
      WRITE(6,8)' CENTR    ',CENTR
    8 FORMAT(A,4D25.16/(7X,5D25.16))
      WRITE(6,8)' CENTJ    ',CENTJ
      WRITE(6,8)' CONVDS   ',CONVDS
!     WRITE(6,8)' CTHETA   ',CTHETA
!     WRITE(6,8)' CZEE     ',CZEE
!     WRITE(6,8)' CZETA    ',CZETA
      WRITE(6,8)' DCENTR   ',DCENTR
      WRITE(6,8)' DJ2000   ',DJ2000
!     WRITE(6,8)' DRP1DT   ',DRP1DT
!     WRITE(6,8)' DRP2DT   ',DRP2DT
!     WRITE(6,8)' DRP3DT   ',DRP3DT
!     WRITE(6,8)' DRPDT1   ',DRPDT1
!     WRITE(6,8)' DRPDT2   ',DRPDT2
!     WRITE(6,8)' DRPDT3   ',DRPDT3
!     WRITE(6,8)' DYSJ     ',DYSJ
      WRITE(6,8)' HALFPI   ',HALFPI
!     WRITE(6,8)' HC       ',HC
!     WRITE(6,8)' HEPOCH   ',HEPOCH
!     WRITE(6,8)' HS       ',HS
      WRITE(6,8)' PRECON   ',PRECON
!     WRITE(6,8)' RP1      ',RP1
!     WRITE(6,8)' RP2      ',RP2
!     WRITE(6,8)' RP3      ',RP3
      WRITE(6,8)' SECDAY   ',SECDAY
!     WRITE(6,8)' THETA    ',THETA
!     WRITE(6,8)' TTHETA   ',TTHETA
!     WRITE(6,8)' TZEE     ',TZEE
!     WRITE(6,8)' TZETA    ',TZETA
!     WRITE(6,8)' ZEE      ',ZEE
!     WRITE(6,8)' ZETA     ',ZETA
!
      WRITE ( 6, * ) ' ZETA/ZEE/THETA = ', ZETA2K, ZEE2K, THET2K 
      WRITE ( 6, 9200 )  CT, EPSMNR, XJD, RPC2K
 9200 FORMAT (1X, "CT    = ", D30.16, /, 1X,
     .            "EPSMNR= ", D30.16, /, 1X,
     .            "XJD   = ", D30.16, /, 1X,
     .            "RPC2K = ", 6 ( 3 ( D30.16, 10X ), /, 1X ) )
!
!   7.    NORMAL PROGRAM CONCLUSION.
!
  700 RETURN
      END
!**********************************************************************
      SUBROUTINE PREP (CENT, CFBASE, EPSMNR, RFR2K, RNC2K, RSC2K, RW2K,
     .                 STAR)
      IMPLICIT None
!
! 5.1.1 PREP IS THE PRECESSION MODULE PARTIAL DERIVATIVES SECTION.
!       PREP COMPUTES THE PARTIAL DERIVATIVES OF THE DELAY AND THE
!       DELAY RATE W.R.T. THE PRECESSION CONSTANT.
!
! 5.1.3 REFERENCES - 'DETERMINATION OF EARTH SATELLITE ORBITS', M.ASH, M.I.T.
!                     LINCOLN LAB TECHNICAL REPORT, APRIL 19, 1972,  P.57-59.
!
! 5.2   PREP PROGRAM INTERFACE
! 5.2.1 CALLING SEQUENCE -
!          INPUT VARIABLES:
!            1. CFBASE(3)   - THE CRUST FIXED BASELINE VECTOR. (M)
!            2. EPSMNR      - MEAN OBLIQUITY OF REFERENCE EPOCH J2000.0 (RAD)
!            4. RNC2K(3,3,2)- THE NUTATION PORTION OF THE COMPLETE CRUST FIXED
!                             TO 2000.0 ROTATION MATRIX  AND ITS CT TIME
!                             DERIVATIVE. (UNITLESS, 1/SEC)
!            5. RSC2k(3,3,3)- THE DIURNAL SPIN PORTION OF THE COMPLETE CRUST
!                             FIXED TO 2000.0 ROTATION MATRIX AND ITS FIRST TWO
!                             CT TIME DERIVATIVES. (UNITLESS, 1/SEC, 1/SEC**2)
!            6. RW2k(3,3,2) - THE WOBBLE PORTION OF THE COMPLETE CRUST FIXED TO
!                             J2000.0 ROTATION MATRIX and its first time
!                             derivative. (unitless, 1/sec)
!            7. STAR(3)     - THE 2000.0 SOURCE UNIT VECTOR. (UNITLESS)
!          OUTPUT VARIABLES: NONE
!
! 5.2.2 COMMON BLOCKS USED -
!
      Real*8           PI, TWOPI, HALFPI, CONVD, CONVDS, CONVHS, SECDAY
      COMMON / CMATH / PI, TWOPI, HALFPI, CONVD, CONVDS, CONVHS, SECDAY
!           VARIABLES 'FROM':
!             1. CONVDS - THE CONVERSION FACTOR OF RADIANS PER ARC SECOND.
!                         (RAD/ARCSEC)
!             2. HALFPI - THE VALUE OF PI DIVIDED BY TWO. (RAD)
!
      INCLUDE 'cphys.i'
!           VARIABLES 'FROM':
!             1. VLIGHT - THE VELOCITY OF LIGHT IN VACUUM. (M/SEC)
!
      Real*8 CENTJ, CTHETA(3), CZEE(3), CZETA(3), DJ2000, HEPOCH,
     .    PRECON, RP1(3,3), RP2(3,3), RP3(3,3), THETA, ZEE, ZETA,CENTR,
     .    THET2(6), ZEE2(6), ZETA2(6), ZEE2K, DZEE2K, THET2K, DTHE2K,
     .         ZETA2K, DZET2K
      COMMON / PRECM / CENTJ, CTHETA, CZEE, CZETA, DJ2000, HEPOCH,
     .         PRECON, RP1, RP2, RP3, THETA, ZEE, ZETA, CENTR,
     .         THET2, ZEE2, ZETA2, ZEE2K, DZEE2K, THET2K, DTHE2K,
     .         ZETA2K, DZET2K
!
!           VARIABLES 'FROM':
!             1. RP1(3,3) -  THE ROTATION MATRIX WHICH PERFORMS A ROTATION ABOUT
!                            THE NORTH CELESTIAL POLE OF THE EPOCH 2000.0 BY THE
!                            ANGLE ZEE+HALFPI. (UNITLESS)
!             2. RP2(3,3) -  THE ROTATION MATRIX WHICH PERFORMS A ROTATION ABOUT
!                            THE ASCENDING NODE OF THE EQUATOR OF THE CURRENT
!                            EPOCH ON THE EQUATOR OF THE EPOCH 2000.0 BY THE
!                            ANGLE -THETA. (UNITLESS)
!             3. RP3(3,3) -  THE ROTATION MATRIX WHICH PERFORMS A ROTATION ABOUT
!                            THE NORTH CELESTIAL POLE OF THE CURRENT EPOCH BY
!                            THE ANGLE ZETA-HALFPI. (UNITLESS)
!             4. THETA    -  THE PRECESSIONAL ELEMENT THETA. (RAD)
!             5. ZEE      -  THE PRECESSIONAL ELEMENT ZEE. (RAD)
!             6. ZETA     -  THE PRECESSIONAL ELEMENT ZETA. (RAD)
!             7. CENTR    -  THE NUMBER OF JULIAN CENTURIES FROM J2000.0 TO THE
!                            EPOCH OF THE OBSERVATION. (CENTURIES)
!
      INCLUDE 'ccon.i'
!           VARIABLES 'FROM':
!             1. KPREC - THE PRECESSION MODULE FLOW CONTROL FLAG.
!             2. KPRED - THE PRECESSION MODULE DEBUG OUTPUT FLAG.
!
! 5.2.3 PROGRAM SPECIFICATIONS -
!     Real*8  CFBASE(3), DPREP(2), DRP1DC(3,3), DRP2DC(3,3),
!    1        DRP3DC(3,3), DRPDC1(3,3), DRPDC2(3,3), DRPDC3(3,3),
!    2        PBASE(3,2), PR2000(3,3,2), RDNP(3,3), RN(3,3,2),
!    3        RPC(3,3), RS(3,3,3), RW(3,3,2), STAR(3), CENT,
!    4        RFR2K(3,3),RNC2K(3,3,2),RSC2K(3,3,3),RW2K(3,3,2)
      Real*8  CFBASE(3), DPREP(2), DRP1DC(3,3), DRP2DC(3,3),
     .        DRP3DC(3,3), DRPDC1(3,3), DRPDC2(3,3), DRPDC3(3,3),
     .        PBASE(3,2), PR2000(3,3,2),
     .        RPC(3,3),                       STAR(3), CENT,
     .        RFR2K(3,3),RNC2K(3,3,2),RSC2K(3,3,3),RW2K(3,3,2)
      Real*8  Epsmnr, Pzeta, Pzee, Ptheta, DOTP
      Integer*4 K
!
! 5.2.4 DATA BASE ACCESS -
!           'GET' VARIABLES: NONE
!           'PUT' VARIABLES:
!             1. DPREP(2)  -  THE PARTIAL DERIVATIVES OF THE DELAY AND OF THE
!                             DELAY RATE W.R.T. THE PRECESSION CONSTANT.
!                             ((SEC/(RAD/CENTURY), (SEC/SEC)/(RAD/CENTURY))
!           ACCESS CODES:
!             1. 'PRE PART'  -  THE DATA BASE ACCESS CODE FOR THE PRECESSION
!                               MODULE PARTIAL DERIVATIVES ARRAY.
!
! 5.2.5 EXTERNAL INPUT/OUTPUT - POSSIBLE ERROR OUTPUT
!
! 5.2.6 SUBROUTINE INTERFACE -
!             CALLER SUBROUTINES: DRIVP
!             CALLED SUBROUTINES: DCOS, DOTP, DROTT, DSIN, MADD3,
!                                 MMUL3, MMUL5, PUT4_S, VECRT
!
! 5.2.7 CONSTANTS USED - VLIGHT
!
! 5.2.8 PROGRAM VARIABLES -
!            1. DRP1DC(3,3)   -  THE PARTIAL DERIVATIVE OF THE ROTATION MATRIX
!                                RP1 WITH RESPECT TO THE PRECESSION CONSTANT.
!                                (1/(RAD/CENTURY))
!            2. DRP2DC(3,3)   -  THE PARTIAL DERIVATIVE OF THE ROTATION MATRIX
!                                RP2 WITH RESPECT TO THE PRECESSION CONSTANT.
!                                (1/(RAD/CENTURY))
!            3. DRP3DC(3,3)   -  THE PARTIAL DERIVATIVE OF THE ROTATION MATRIX
!                                RP3 WITH RESPECT TO THE PRECESSION CONSTANT.
!                                (1/(RAD/CENTURY))
!            4. DRPDC1(3,3)   -  THE FIRST TERM OF THE THREE TERMS NECESSARY
!                                FOR THE CALCULATION OF THE PARTIAL DERIVATIVE
!                                OF THE PRECESSION MATRIX WITH RESPECT TO THE
!                                PRECESSION  CONSTANT.  (1/(RAD/CENTURY))
!            5. DRPDC2(3,3)   -  THE SECOND TERM OF THE THREE TERMS NECESSARY
!                                FOR THE CALCULATION OF THE PARTIAL DERIVATIVE
!                                OF THE PRECESSION MATRIX WITH RESPECT TO THE
!                                PRECESSION CONSTANT. (1/(RAD/CENTURY))
!            6. DRPDC3(3,3)   -  THE THIRD TERM OF THE THREE TERMS NECESSARY
!                                FOR THE CALCULATION OF THE PARTIAL DERIVATIVE
!                                OF THE PRECESSION MATRIX WITH RESPECT TO THE
!                                PRECESSION CONSTANT. (1/(RAD/CENTURY))
!            7. PBASE(3,2)    -  THE PARTIAL DERIVATIVES OF THE 2000.0
!                                BASELINE POSITION AND VELOCITY VECTORS WITH
!                                RESPECT TO THE PRECESSION CONSTANT.
!                                (M/(RAD/CENTURY))
!            8. PR2000(3,3,2) -  THE PARTIAL DERIVATIVE OF THE COMPLETE CRUST
!                                FIXED TO 2000.0 ROTATION MATRIX AND OF ITS CT
!                                TIME DERIVATIVE WITH RESPECT TO THE PRECESSION
!                                CONSTANT. (1/(RAD/SEC), 1/(RAD/CENTURY)*SEC))
!            9. PTHETA        -  THE PARTIAL DERIVATIVE OF THE PRECESSIONAL
!                                ELEMENT THETA WITH RESPECT TO THE PRECESSION
!                                CONSTANT. ((1/CENTURY))
!           10. PZEE          -  THE PARTIAL DERIVATIVE OF THE PRECESSIONAL
!                                ELEMENT ZEE WITH RESPECT TO THE PRECESSION
!                                CONSTANT. ((1/CENTURY))
!           11. PZETA         -  THE PARTIAL DERIVATIVE OF THE PRECESSIONAL
!                                ELEMENT ZETA WITH RESPECT TO THE PRECESSION
!                                CONSTANT. ((1/CENTURY))
!           12. RPC(3,3)      -  THE PARTIAL DERIVATIVE OF THE PRECESSION
!                                PORTION OF THE COMPLETE CRUST FIXED TO 2000.0
!                                ROTATION MATRIX WITH RESPECT TO THE PRECESSION
!                                CONSTANT. ((1/(RAD/CENTURY))
!
! 5.2.9 PROGRAMMER - DALE MARKHAM   01/13/77
!                    PETER DENATALE 07/12/77
!                    BRUCE SCHUPLER 06/05/78
!                    CHOPO MA       08/04/81
!                    JIM RYAN       09/14/81 FIXED DOCUMENTATION ONLY
!                    Jim Ryan       89.07.07 Documentation simplified.
!                    Jim Ryan       89:10:05 CPHYS common made an include file
!                    Jim Ryan 89.12.12 UNIX-like database interface
!                             implimented.
!                    David Gordon 94.04.14 Converted to Implicit None
!                    David Gordon 95.12.11 Changed RW(3,3) to RW(3,3,2).
!                    Jim Ryan Sept 2002  Integer*2/4 upgrades.
!                    D. Gordon      2003/4 Minpor updates for IERS 2003.
!
!     PREP PROGRAM STRUCTURE
!
!   Compute the partial derivative of the precession matrix with
!   respect to the precession constrat.
!
!     Compute the partial derivatives of the precessional elements with
!     respect to the precession constant.
!     PZETA  = DCOS ( EPSMNR ) * CENTR / 2.D0
!     PZEE   = DCOS ( EPSMNR ) * CENTR / 2.D0
!     PTHETA = DSIN ( EPSMNR ) * CENTR
!
      PZETA  = DCOS ( EPSMNR ) * CENT / 2.D0
      PZEE   = DCOS ( EPSMNR ) * CENT / 2.D0
      PTHETA = DSIN ( EPSMNR ) * CENT
!
!   Construct the partial derivative of the rotation matrices
!   RP1, RP2, and RP3 with respect to the precession constant.
!!!   CALL DROTT ( ZEE+HALFPI, PZEE, int2(3), DRP1DC)
!!!   CALL DROTT ( -THETA, -PTHETA, int2(1), DRP2DC)
!!!   CALL DROTT ( ZETA-HALFPI, PZETA, int2(3), DRP3DC)
!
      CALL DROTT ( ZEE2K+HALFPI, PZEE, int2(3), DRP1DC)
      CALL DROTT ( -THET2K, -PTHETA, int2(1), DRP2DC)
      CALL DROTT ( ZETA2K-HALFPI, PZETA, int2(3), DRP3DC)
!
!   Construct the partial derivative of the precession matrix
!   with respect to the precession constant.
!
!   Compute the three terms neccesary for the calculation.
      CALL MMUL3 ( DRP3DC, RP2, RP1, DRPDC1 )
      CALL MMUL3 ( RP3, DRP2DC, RP1, DRPDC2 )
      CALL MMUL3 ( RP3, RP2, DRP1DC, DRPDC3 )
!
!   Complete the construction by adding the three terms.
      CALL MADD3 ( DRPDC1, DRPDC2, DRPDC3, RPC )
!
!       Write(6,1043) RPC
!1043    Format(1x,'PREP/RPC: ',(3(/,3E25.15)))
!
!
!   Compute the partial derivative of the complete crust fixed to J2000.0
!   rotation matix and its first CT time derivative with respect to the
!   precession constant.
!    (NOTE: Of the three terms which are used to compute the CT
!    time derivative of the complete crust fixed to J2000.0 rotation
!    matrix only the term which contains the CT time
!    derivative of the diurnal spin matrix is considered significant
!    enough to include in this partial derivatives section.)
!
      CALL MMUL5 (RFR2K, RPC, RNC2K(1,1,1), RSC2K(1,1,1),
     .            RW2K(1,1,1), PR2000(1,1,1))
!
      CALL MMUL5 (RFR2K, RPC, RNC2K(1,1,1), RSC2K(1,1,2),
     .            RW2K(1,1,1), PR2000(1,1,2))
!
!   Compute the partial derivatives of the J200.0 baseline position and
!   velocity vectors with respect to the precession constant.
      DO 300  K = 1,2
           CALL VECRT (PR2000(1,1,K), CFBASE, PBASE(1,K))
  300 CONTINUE
!
!   Compute the partial derivatives of the delay and rate with respect
!   to the precession constant.
      DO 400  K = 1,2
        DPREP(K) = DOTP (PBASE(1,K), STAR) / VLIGHT
  400 CONTINUE
!      print *,' PREP/DPREP: ', DPREP
!
!   PUT precession partials.
      CALL PUT4 ('PRE PART      ',DPREP,int2(2),int2(1),int2(1))
!
!   Check KPRED for debug output.
      IF ( KPRED .EQ. 0 )  GO TO 800
      WRITE ( 6, 9)
    9 FORMAT (1X, "Debug output for subroutine PREP." )
!
      WRITE(6,8)' DPREP   ',DPREP
    8 FORMAT(A,4D25.16/(7X,5D25.16))
      WRITE(6,8)' DRP1DC  ',DRP1DC
      WRITE(6,8)' DRP2DC  ',DRP2DC
      WRITE(6,8)' DRP3DC  ',DRP3DC
      WRITE(6,8)' DRPDC1  ',DRPDC1
      WRITE(6,8)' DRPDC2  ',DRPDC2
      WRITE(6,8)' DRPDC3  ',DRPDC3
      WRITE(6,8)' HALFPI  ',HALFPI
      WRITE(6,8)' PBASE   ',PBASE
      WRITE(6,8)' PR2000  ',PR2000
      WRITE(6,8)' PTHETA  ',PTHETA
      WRITE(6,8)' PZEE    ',PZEE
      WRITE(6,8)' PZETA   ',PZETA
      WRITE(6,8)' RP1     ',RP1
      WRITE(6,8)' RP2     ',RP2
      WRITE(6,8)' RP3     ',RP3
      WRITE(6,8)' RPC     ',RPC
      WRITE(6,8)' THETA   ',THETA
      WRITE(6,8)' VLIGHT  ',VLIGHT
      WRITE(6,8)' ZEE     ',ZEE
      WRITE(6,8)' ZETA    ',ZETA
      WRITE(6,8)' CENT    ',CENT
!
      WRITE ( 6, 9200 )  CFBASE, EPSMNR, RNC2K, RSC2K, RW2K, STAR
 9200 FORMAT (1X, "CFBASE = ", 3 ( D30.16, 10X ), /, 1X,
     .            "EPSMNR = ",  ( D30.16, 10X ), /, 1X,
     .            "RNC2K  = ", 6 ( 3 ( D30.16, 10X ), /, 1X ),
     .            "RSC2K  = ", 9 ( 3 ( D30.16, 10X ), /, 1X ),
     .            "RW2K   = ", 6 ( 3 ( D30.16, 10X ), /, 1X ),
     .            "STAR   = ", 3 ( D30.16, 10X ) )
!
!     Normal conclusion.
  800 RETURN
      END
!****************************************************************
      SUBROUTINE PREC()
      Implicit None
!
       Integer*4 I
!
      I = 1
!
      RETURN
      END
!****************************************************************
      BLOCK DATA PRECMB
      IMPLICIT None
!
! 7.    PREBD
! 7.1   PREBD PROGRAM SPECIFICATION
!
! 7.1.1 PREBD IS THE PRECESSION MODULE BLOCK DATA INPUT AND INITIALIZATION
!       SECTION.
!
! 7.1.3 REFERENCES - 1) LIESKE, J.H., PRECESSION MATRIX BASED ON IAU
!                       (1976) SYSTEM OF ASTRONOMICAL CONSTANTS,
!                       ASTRON. ASTROPHYS. 73, 282-284, 1979.
!                    2) LIESKE, J.H., ET AL., EXPRESSIONS FOR THE
!                       PRECESSION QUANTITIES BASED UPON THE IAU (1976)
!                       SYSTEM OF ASTRONOMICAL CONSTANTS, ASTRON. ASTROPHYS.
!                       58, 1-16, 1977.
!                    3) ASH, M.E., "DETERMINATION OF EARTH SATELLITE
!                       ORBITS", LINCOLN LABORATORY TECHNICAL REPORT
!                       1972-5, 04/19/72, P. 55-57.
!                    4) IERS Technical Note 32, IERS Converntions (2003).
!
! 7.2   PREBD PROGRAM INTERFACE
!
! 7.2.1 CALLING SEQUENCE - NONE
!
! 7.2.2 COMMON BLOCK -
!
      Real*8 CENTJ, CTHETA(3), CZEE(3), CZETA(3), DJ2000, HEPOCH,
     .    PRECON, RP1(3,3), RP2(3,3), RP3(3,3), THETA, ZEE, ZETA,CENTR,
     .    THET2(6), ZEE2(6), ZETA2(6), ZEE2K, DZEE2K, THET2K, DTHE2K,
     .         ZETA2K, DZET2K
      COMMON / PRECM / CENTJ, CTHETA, CZEE, CZETA, DJ2000, HEPOCH,
     .         PRECON, RP1, RP2, RP3, THETA, ZEE, ZETA, CENTR,
     .         THET2, ZEE2, ZETA2, ZEE2K, DZEE2K, THET2K, DTHE2K,
     .         ZETA2K, DZET2K
!
!          VARIABLES 'TO':
!             1. CENTJ     -  THE NUMBER OF COORDINATE TIME DAYS PER JULIAN
!                             CENTURY.  (DAYS/CENTURY)  ( CENTJ = 36525.D0 )
!             2. CTHETA(3) -  THE CONSTANTS APPEARING IN TERMS 1-3 IN THE
!                             CALCULATION OF THE PRECESSIONAL ELEMENT THETA.
!                             (ARCSEC)  (SEE REFERENCES)
!                             ( CTHETA(1) = + 2004.3109D0,
!                               CTHETA(2) = - 0.42665D0,
!                               CTHETA(3) = - 0.041833D0 )
!             3. CZEE(3)   -  THE CONSTANTS APPEARING IN TERMS 1-3 IN THE
!                             CALCULATION OF THE PRECESSIONAL ELEMENT ZEE.
!                             (ARCSEC)  (SEE REFERENCES)
!                             ( CZEE(1) = + 2306.2181D0,
!                               CZEE(2) = + 1.09468D0,
!                               CZEE(3) = + 0.018203D0 )
!             4. CZETA(3)  -  THE CONSTANTS APPEARING IN TERMS 1-3 IN THE
!                             CALCULATION OF THE PRECESSIONAL ELEMENT ZETA.
!                             (SECOND-SEC)  (SEE REFERENCES)
!                             ( CZETA(1) = + 2306.2181D0,
!                               CZETA(2) = + 0.30188D0,
!                               CZETA(3) = + 0.017998D0 )
!             5. DJ2000    -  THE JULIAN DATE OF THE EPOCH J2000.0. (DAYS)
!                             (2000 JAN 01 12HR UT)
!                             ( DJ2000 = 2451545.0D0 )
!             6. HEPOCH    -  THE NOMINAL VALUE OF THE PRECESSION CONSTANT IN
!                             THE EPOCH J2000.0. (ARCSEC/JCENTURY)
!                             ( HEPOCH = 5029.0966D0 )
!
!          VARIABLES 'PASSED' FROM OTHER MODULE SECTIONS:
!             1. PRECON    -  THE PRECESSION CONSTANT. (ARCSEC/JCENTURY)
!             2. RP1(3,3)  -  THE ROTATION MATRIX WHICH PERFORMS A ROTATION
!                             ABOUT THE NORTH CELESTIAL POLE OF THE EPOCH 2000.0
!                             BY THE ANGLE ZEE+HALFPI. (UNITLESS)
!             3. RP2(3,3)  -  THE ROTATION MATRIX WHICH PERFORMS A ROTATION
!                             ABOUT THE ASCENDING NODE OF THE EQUATOR OF THE
!                             CURRENT EPOCH ON THE EQUATOR OF THE 2000.0 EPOCH
!                             BY THE ANGLE -THETA. (UNITLESS)
!             4. RP3(3,3)  -  THE ROTATION MATRIX WHICH PERFORMS A ROTATION
!                             ABOUT THE NORTH CELESTIAL POLE OF THE CURRENT
!                             EPOCH BY THE ANGLE ZETA-HALFPI. (UNITLESS)
!             5. THETA     -  THE PRECESSIONAL ELEMENT THETA. (RAD)
!             6. ZEE       -  THE PRECESSIONAL ELEMENT ZEE. (RAD)
!             7. ZETA      -  THE PRECESSIONAL ELEMENT ZETA. (RAD)
!
! 7.2.3 PROGRAM SPECIFICATIONS -
!
      DATA  CENTJ     / 36525.D0 /,
     .      CTHETA    / +2004.3109D0, -0.42665D0, -0.041833D0 /,
     .      CZEE      / +2306.2181D0, +1.09468D0, +0.018203D0 /,
     .      CZETA     / +2306.2181D0, +0.30188D0, +0.017998D0 /,
     .      DJ2000    / 2451545.D0 /,
     .      HEPOCH    / 5029.0966D0 /
      DATA
     .      THET2   / 0.D0,         +2004.1917476D0, -0.4269353D0,
     .                -0.0418251D0, -0.0000601D0,    -0.0000001D0 /,
     .      ZEE2    / -2.5976176D0, +2306.0803226D0, +1.0947790D0,
     .                +0.0182273D0, +0.0000470D0,    -0.0000003D0 /,
     .      ZETA2   / +2.5976176D0, +2306.0809506D0, +0.3019015D0,
     .                +0.0179663D0, -0.0000327D0,    -0.0000002D0 /
!
! 7.2.4 CONSTANTS USED - CENTJ, CTHETA(3), CZEE(3), CZETA(3), DJ2000,
!                        HEPOCH
!
! 7.2.5 PROGRAMMER - DALE MARKHAM   01/13/77
!                    PETER DENATALE 07/12/77
!                    BRUCE SCHUPLER 06/05/78
!                    CHOPO MA       08/04/81
!                    David Gordon 94.04.14 Converted to Implicit None
!                    David Gordon 2003.07.16  Added coefficients for
!                                 IAU2000 Precession model
!
! 7.3   PREBD PROGRAM STRUCTURE - NONE
      END
