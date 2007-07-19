      SUBROUTINE NUTA()
      IMPLICIT None
!
! 1.    NUTA
!
! 1.1   NUTA PROGRAM SPECIFICATION
!
! 1.1.1 NUTA ADDS ENTRIES TO THE TABLE OF CONTENTS FOR THE NUTATION
!       MODULE TEXT MESSAGE, THE NUTATION MODULE FLOW CONTROL MESSAGE,
!       THE IAU2000 NUTATION VALUES, THE WAHR (OLD) NUTATION VALUES,
!       THE WAHR DIFFERENCE CONTRIBUTIONS, AND THE NUTATION PARTIAL
!       DERIVATIVES. ALSO REMOVES OLD LCODES.
!
! 1.2   NUTA PROGRAM INTERFACE
!
! 1.2.1 CALLING SEQUENCE - NONE
!
! 1.2.2 COMMON BLOCKS USED
      INCLUDE 'ccon.i'
!            VARIABLES 'FROM':
!              1. KNUTC - THE NUTATION MODULE CONTROL FLAG.
!                 0 - IAU200A Nutation
!                 1 - NO NUTATION
!
      Real*8 CENTJ, DJ2000, EC(4), ARGP(2,6)
      Integer*4 NOT, NOP, IDP(6)
      COMMON / NUTCM / CENTJ, DJ2000, EC, ARGP, NOT, NOP, IDP
!
! 1.2.3 PROGRAM SPECIFICATIONS - NONE
!
! 1.2.4 DATA BASE ACCESS -
!            ACCESS CODES:
!              1.  'NUT MESS'  -  THE DATA BASE ACCESS CODE FOR THE
!                                 NUTATION MODULE TEXT MESSAGE.
!              2.  'NUT CFLG'  -  THE DATA BASE ACCESS CODE FOR THE
!                                 NUTATION MODULE FLOW CONTROL MESSAGE.
!              3.  'NUT2000A'  -  THE DATA BASE ACCESS CODE FOR THE IAU
!                                 2000A NUTATION VALUES.
!              4.  'NUT WAHR'  -  THE DATA BASE ACCESS CODE FOR THE
!                                 WAHR NUTATION VALUES.
!              5.  'NUT2000P'  -  THE DATA BASE ACCESS CODE FOR THE NUTATION
!                                 MODULE PARTIAL DERIVATIVE ARRAY.
!              6.  'WAHRCONT'  -  The data base access code for the delay and
!                                 rate contribution to convert from IERS 1996
!                                 nutation to IAU 1980 (Wahr) nutation.
!              7.  'NUT2KXYS'  -  THE DATA BASE ACCESS CODE FOR THE
!                                 CEO-based X,Y,S NUTATION VALUES.
!              8.  'NUT2KXYP'  -  THE DATA BASE ACCESS CODE FOR THE NUTATION
!                                 MODULE X and Y PARTIAL DERIVATIVE ARRAY.
!
! 1.2.5 EXTERNAL INPUT/OUTPUT - NONE
!
! 1.2.6 SUBROUTINE INTERFACE -
!             CALLER SUBROUTINES: TOCUP
!             CALLED SUBROUTINES: ADDA, ADDR
!
! 1.2.7 CONSTANTS USED - NONE
!
! 1.2.8 PROGRAM VARIABLES - NONE
!
! 1.2.9 PROGRAMMER - DALE MARKHAM   01/13/77
!                    PETER DENATALE 07/12/77
!                    BRUCE SCHUPLER 12/16/77
!                    CHOPO MA      08/04/81
!                    JIM RYAN      08/26/81
!                    GEORGE KAPLAN 04/24/84
!                    DAVID GORDON  11/13/84  (REDIMENSIONED NT1 AMPS
!                                             NT1 PART, ETC)
!                    SAVITA GOEL  06/03/87  (CDS FOR A900)
!                    Jim Ryan     89.06.30 Documetation clean up.
!                    Jim Ryan     89.12.12 UNIX-like database interface
!                                 implimented.
!                    David Gordon 94.04.15 Converted to Implicit None.
!                    David Gordon 94.06.09 Rearranged COMMON /NUTCM/  (Real*8,
!                             Real*4, Integer*4, Integer*2). Changed a few
!                             Integer*2's to Integer*4.
!                    David Gordon 98.02.03 Default changed to 1996 IERS
!                             nutation model. 'NT1 AMPS', 'NT1 PART', etc.
!                             code removed, and DELR's added to remove old
!                             Lcodes. New Lcodes 'NUT 1996', 'WAHRCONT',
!                             and 'FUNDARGS' added. Nutation from database
!                             option removed.
!                    David Gordon 98.11.12 Add for 'GDNUTCON', geodesic
!                             nutation contribution.
!                    Jim Ryan 2002.09 Integer*4 conversion.
!                    David Gordon 2004.03.19
!
! 1.3   NUTA PROGRAM STRUCTURE
!
!   ADD for the module text message.
      CALL ADDA (int2(1),'NUT MESS','Nutation message definition     ',
     . int2(40), int2(1), int2(1))
!
!   ADD for  module flow control message.
      CALL ADDA (int2(1),'NUT CFLG','Nutation flow control mess def. ',
     . int2(40), int2(1), int2(1))
!
!   Nutation from data base no longer used, remove old Lcodes
      CALL DELR (int2(2), 'DPSI    ')
      CALL DELR (int2(2), 'DEPS    ')
!
      CALL DELR (int2(2), 'FUNDARGS')
      CALL DELR (int2(2), 'NUT PART')
      CALL DELR (int2(2), 'NUT 1996')
      CALL DELR (int2(2), 'GDNUTCON')
!  Lcodes for the nutation values
!
!   If using IAU2000 Nutation:
      IF ( KNUTC .eq. 0) then
       CALL ADDR (int2(2),'NUT2000A','IAU200A Nut. - Dpsi, Deps, Rates',
     .  int2(2), int2(2), int2(1))
       CALL ADDR (int2(2),'NUT2KXYS','CIP Coordinates X,Y,S, and Rates',
     .  int2(3), int2(2), int2(1))
       CALL ADDR (int2(2),'NUT WAHR','Wahr nut vals  - Dpsi,Deps&rates',
     .  int2(2), int2(2), int2(1))
       CALL ADDR (int2(2),'WAHRCONT','2000A Nut to Wahr Nut Contributn',
     .  int2(2), int2(1), int2(1))
!   ADD for IERS 2003 nutation partials w.r.t. X and Y.
       CALL ADDR (int2(2),'NUT2KXYP','IAU2000A Nutation X,Y Partials  ',
     .  int2(2), int2(2), int2(1))
!   ADD for IERS 2003 nutation partials w.r.t. Psi and Epsilon.
       CALL ADDR (int2(2),'NUT2000P','IAU2000A Nutation Psi,Eps Partls',
     .  int2(2), int2(2), int2(1))
      Endif
!
      RETURN
      END
!*************************************************************************
      SUBROUTINE NUTI()
      IMPLICIT None
!
! 3.    NUTI
!
! 3.1   NUTI PROGRAM SPECIFICATION
!
! 3.1.1 NUTI IS THE NUTATION MODULE INPUT AND INITIALIZATION SECTION.
!       MESSAGES ARE PUT IN THE HEADER TO INDICATE PROGRAM FLOW.
!       KNUTC = 0 - IERS 2003 NUTATION (IAU200A Precession/Nutation)
!                   TO BE USED (DEFAULT)
!               1 - NO NUTATION TO BE USED
!
! 3.2   NUTI PROGRAM INTERFACE
!
! 3.2.1 CALLING SEQUENCE - NONE
!
! 3.2.2 COMMON BLOCKS USED -
      INCLUDE 'ccon.i'
!      VARIABLES 'FROM':
!       1. KNUTC - THE NUTATION MODULE FLOW CONTROL FLAG
!       2. KNUTD - THE NUTATION MODULE DEBUG CONTROL FLAG
!
      Real*8           PI, TWOPI, HALFPI, CONVD, CONVDS, CONVHS, SECDAY
      COMMON / CMATH / PI, TWOPI, HALFPI, CONVD, CONVDS, CONVHS, SECDAY
!     VARIABLES 'FROM':
!       1.  CONVDS  -  THE CONVERSION FACTOR OF RADIANS PER
!                      ARCSECOND.  (RAD/ARCSEC)
!
! 3.2.3 PROGRAM SPECIFICATIONS -
      Real*8  APAMPS(2,2)
      Integer*4 N, I
      INTEGER*2      LNUTM(40),  LON(40),  LOFF(40)
      CHARACTER*40  C_LNUTM(2), C_LON(2), C_LOFF(2)
      EQUIVALENCE (LNUTM,C_LNUTM), (LON,C_LON), (LOFF,C_LOFF)
!
      DATA C_LNUTM /
     . 'Nutation Module - IAU2000A, 2004.03.19, ',
     . 'D. Gordon/GSFC.                         '/
!
      DATA C_LON  /
     . 'Nutation module is turned ON. IAU2000A m',
     . 'odel used.                              '/
!
      DATA C_LOFF /
     . 'Nutation module is turned OFF. This turn',
     . 's turns off the nut in obliq in DIURNAL.'/
!
! 3.2.4 DATA BASE ACCESS -
!           'PUT' VARIABLES:
!              1.  LNUTM(40)   - THE NUTATION MODULE TEXT MESSAGE.
!              2.  LON(40)     - THE NUTATION MODULE TURNED ON MESSAGE.
!              3.  LOFF(40)    - THE NUTATION MODULE TURNED OFF MESSAGE.
!            ACCESS CODES:
!              1.  'NUT MESS'  -  THE DATA BASE ACCESS CODE FOR THE
!                                 NUTATION MODULE TEXT MESSAGE.
!              2.  'NUT CFLG'  -  THE DATA BASE ACCESS CODE FOR THE
!                                 NUTATION MODULE FLOW CONTROL MESSAGE.
!
! 3.2.6 SUBROUTINE INTERFACE -
!             CALLER SUBROUTINES: INITL
!             CALLED SUBROUTINES: PUTA
!
! 3.2.9 PROGRAMMER - DALE MARKHAM   01/13/77
!                    PETER DENATALE 07/12/77
!                    BRUCE SCHUPLER 03/15/78
!                    BRUCE SCHUPLER 02/01/79
!                    BRUCE SCHUPLER 06/06/79
!                    CHOPO MA       08/04/81
!                    GEORGE KAPLAN  04/24/84
!                    DAVID GORDON   11/13/84  (REDIMENSIONED NT# AMPS)
!                    Jim Ryan 89.06.30 Documetation clean up.
!                    Jim Ryan 89.12.12 UNIX-like database interface
!                             implimented.
!                    David Gordon 94.04.15 Converted to Implicit None.
!                    David Gordon 95.09.27 Conversion of X(9,120) nutation table
!                                 coefficients to double precision removed - no
!                                 longer needed.
!                    David Gordon 98.02.03 Changed KNUTC definitions. Default
!                                 is now IERS 1996 Nutation. Removed
!                                 computation and PUT's of 'NT1 AMPS', etc.
!                    Jim Ryan 2002.09  Integer*4 conversion.
!                    David Gordon 2003/2004 Mods for IERS Conventions 2003.
!
!     NUTI PROGRAM STRUCTURE
!
!     PUT the nutation module text message.
      CALL PUTA ('NUT MESS      ', LNUTM, int2(40), int2(1), int2(1))
!
!     PUT the nutation module flow control message depending on KNUTC.
      IF (KNUTC .EQ. 0) CALL PUTA ('NUT CFLG      ',LON,int2(40),
     .                  int2(1), int2(1))
      IF (KNUTC .EQ. 1) CALL PUTA ('NUT CFLG      ',LOFF,int2(40),
     .                  int2(1), int2(1))
!
!     Normal conclusion.
      RETURN
      END
!**************************************************************************
      SUBROUTINE NUTG (CENT, FA2K, FAD2K, XJD, CT, TSKIP, EPS,
     .           EPSMNR, Xn, Yn, Sn, RPN2K,
     .           DEPS2K, DPSI2K, EPSA, RNC2K, NUTDIF)
      IMPLICIT None
!
! 4.    NUTG
!
! 4.1   NUTG PROGRAM SPECIFICATION
!
! 4.1.1 NUTG IS THE NUTATION MODULE GEOMETRY SECTION. IT CALCULATES THE NUTATION
!       PORTION OF THE COMPLETE CRUST FIXED TO J2000.0 ROTATION MATRIX AND ITS
!       CT TIME DERIVATIVE. THE TRUE OBLIQUITY OF THE ECLIPTIC AND ITS CT TIME
!       DERIVATIVE ARE ALSO CALCULATED.
!
! 4.1.3 REFERENCES -
!       1) 'THE EXPLANATORY SUPPLEMENT TO THE AMERICAN EPHEMERIS AND
!          NAUTICAL ALMANAC", P. 41-45, 98.
!       2) 'SPHERICAL AND PRACTICAL ASTRONOMY AS APPLIED TO GEODESY', I.
!          MUELLER, 1969, P. 68-75. (NOTE: THE REFERENCE IN MUELLER REFERS
!          TO THE COMPUTATION OF THE NUTATION PORTION OF THE COMPLETE
!          J2000.0 TO CRUST FIXED ROTATION MATRIX. HOWEVER, CALC REQUIRES
!          THE TRANSPOSE OF THIS MATRIX. CARE MUST BE TAKEN WHEN COMPARING
!          THIS REFERENCE TO THE FOLLOWING PROGRAM.)
!       3) LIESKE, J.H., ET AL., EXPRESSIONS FOR THE PRECESSIONAL QUANTITIES
!          BASED ON THE IAU (1976) SYSTEM OF ASTRONOMICAL CONSTANTS, ASTRON.
!          ASTROPHYS. 58, 1-16, 1977.
!       4) IERS Technical Note 32, IERS Conventions (2003).
!
! 4.2   NUTG PROGRAM INTERFACE
!
! 4.2.1 CALLING SEQUENCE -
!           INPUT VARIABLES:
!              1. CENT      -  The number of Julian centuries elapsed since the
!                              epoch January 1.5, 2000. (centuries)
!              2. FA2K(14)  -  The fundamental arguments (arcsec)
!              3. FAD2K(14) -  The CT time derivatives of the fundamental
!                              arguments. (arcsec/century)
!              4. XJD       -  The Julian Date at zero hours UTC of the
!                              observation.
!              5. CT        -  Coordinate time fraction of the day (~TDB).
!              6. TSKIP     -  Skip nutation recomputation if TSKIP=1.
!           OUTPUT VARIABLES:
!              1. EPS(2)    -  THE TRUE OBLIQUITY OF THE ECLIPTIC AND ITS CT
!                                (IAU1976/1980 Precession/Nutation)
!              2. EPSMNR    -  MEAN OBLIQUITY AT REFERENCE EPOCH J2000. (RAD)
!              3. Xn(2)     -  X-component of the CIP (Celestial
!                              Intermediate Pole) in the GCRS (Geocentric
!                              Celestial Reference System), and its time
!                              derivative. (Radians, Radians/sec)
!              4. Yn(2)     -  Y-component of the CIP (Celestial
!                              Intermediate Pole) in the GCRS (Geocentric
!                              Celestial Reference System), and its time
!                              derivative. (Radians, Radians/sec)
!              5. Sn(2)     -  Position of the CEO (Celestial Ephemeris
!                              Origin) on the equator of the CIP, and its
!                              time derivative. (Radians, Radians/sec)
!                              TIME DERIVATIVE. (RAD, RAD/SEC)
!              6. RPN2K(3,3,2)-The Bias Precession Nutation portion of
!                              the complete Fixed to J2000.0 rotation
!                              matrix and its CT time derivative,
!                              consistent with the IERS Conventions
!                              (2003). (unitless, 1/sec)
!              7. DEPS2K(2)  - THE NUTATION IN OBLIQUITY AND ITS CT TIME
!                              DERIVATIVE COMPUTED FROM THE IERS 1996 NUTATION
!                              MODEL OR FROM THE IAU 1980 (WAHR) NUTATION
!                              MODEL. (RAD, RAD/SEC)
!              8. DPSI2K(2)  - THE NUTATION IN LONGITUDE AND ITS CT TIME
!                              DERIVATIVE COMPUTED FROM THE IERS 1996 NUTATION
!                              MODEL OR FROM THE IAU 1980 (WAHR) NUTATION
!                              MODEL. (RAD, RAD/SEC)
!              9. EPSA(2)   -  THE MEAN OBLIQUITY OF THE ECLIPTIC at the
!                              reference epoch using IAU200A
!                              Precession/Nutation ???
!                              and its time derivative. ????
!             10. RNC2K(3,3,2)-THE NUTATION PORTION OF THE COMPLETE
!                              CRUST FIXED TO J2000.0 ROTATION MATRIX AND
!                              ITS CT TIME  DERIVATIVE. (UNITLESS, 1/SEC)
!             11. NUTDIF(2,2)- Differences between IAU2000A and 1980 nutation
!                              values. First variable runs over Dpsi and Deps,
!                              second runs over delay and rate. (rad, rad/sec)
!
! 4.2.2 COMMON BLOCKS USED -
!
      Real*8           PI, TWOPI, HALFPI, CONVD, CONVDS, CONVHS, SECDAY
      COMMON / CMATH / PI, TWOPI, HALFPI, CONVD, CONVDS, CONVHS, SECDAY
!           VARIABLES 'FROM':
!              1. CONVDS  -  THE CONVERSION FACTOR OF RADIANS PER ARCSECOND.
!                            (RAD/ARCSEC)
!              2. SECDAY  -  THE NUMBER OF COORDINATE TIME SECONDS PER
!                            COORDINATE TIME DAY.  (SEC/DAY)
!
      Real*8 CENTJ, DJ2000, EC(4), ARGP(2,6)
      Integer*4 NOT, NOP, IDP(6)
      COMMON / NUTCM / CENTJ, DJ2000, EC, ARGP, NOT, NOP, IDP
!            VARIABLES 'FROM':
!              1. CENTJ  -  THE NUMBER OF COORDINATE TIME DAYS PER JULIAN
!                           CENTURY. (DAYS/CENTURY)
!              2. EC(4)  -  THE CONSTANTS APPEARING IN TERMS 1-4 IN THE
!                           CALCULATION OF THE MEAN OBLIQUITY OF THE ECLIPTIC.
!                           (ARCSEC) (SEE REFERENCES)
!
      INCLUDE 'ccon.i'
!            VARIABLES 'FROM':
!              1. KNUTC - THE NUTATION MODULE FLOW CONTROL FLAG.
!                         KNUTC = 0 TO TURN ON THE IERS 1996 NUTATION MODEL
!                         KNUTC = 1 TO TURN OFF THE NUTATION MODULE
!              2. KNUTD - THE NUTATION MODULE DEBUG OUTPUT FLAG.
!
      Real*8 RN1(3,3), RN2(3,3), RN3(3,3), RN4(3,3), RN5(3,3), RN6(3,3)
      COMMON / RN1to6 / RN1, RN2, RN3, RN4, RN5, RN6
!
!************************************************************************
! 4.2.3 PROGRAM SPECIFICATIONS -
      REAL*8 CENT, FA(5), FAD(5), FUNDARG(28), FA2K(14), FAD2K(14)
      Real*8 DEPS(2), DPSI(2), EPS(2), EPSMN(2), R1N(3,3),
     .       R2N(3,3), R3N(3,3), NUTDIF(2,2),
     .       DTEMP(4), EPSMNR, DCENT, NUTWAHR(4), NUT1996(4),
     .       NUT2000(2,2), NUTXYS(3,2), DPSIdif, DEPSdif
      Real*8 XJD, CT, Xn(2), Yn(2), Sn(2), RPN2K(3,3,2), RBPN(3,3),
     .       dRBPN(3,3), DPSI2K(2), DEPS2K(2), RNC2K(3,3,2), EPSA(2)
      REAL*8 D1, RNCeps(3,3,2), RN1eps(3,3), RN4eps(3,3), R1Neps(3,3),
     .       R2Neps(3,3), R3Neps(3,3), RNCpsi(3,3,2), RN2psi(3,3),
     .       RN5psi(3,3), R1Npsi(3,3), R2Npsi(3,3), R3Npsi(3,3),
     .       RBPNX(3,3,2),RBPNY(3,3,2)
      Integer*4 TSKIP, M, N, II, JJ
!
! 4.2.4 DATA BASE ACCESS -
!       'PUT' VARIABLES -
!          1. NUTWAHR(4) - Variable used to hold the Wahr DPSI and DEPS and
!                          their time derivatives. (rad, rad/sec)
!          2. NUT2000(4) - Variable used to hold the IAU2000A DPSI and DEPS
!                          and their time derivatives. (rad, rad/sec)
!
      SAVE  NUTWAHR, NUT2000, NUTXYS
!
! 4.2.5 EXTERNAL INPUT/OUTPUT -
!           1. POSSIBLE DEBUG OUTPUT
!
! 4.2.6 SUBROUTINE INTERFACE -
!             CALLER SUBROUTINES: DRIVR
!             CALLED SUBROUTINES: DROTT, MADD3, MMUL3, ROTAT, PUT4, NUTW
!
! 4.2.7 CONSTANTS USED - CONVDS, CENTJ, DJ2000, EC(4), SECDAY
!
! 4.2.8 PROGRAM VARIABLES -
!         1. DCENT     -  THE CT TIME DERIVATIVE OF THE VARIABLE CENT.
!                         (CENTURIES/SEC)
!         2. R1N(3,3)  -  THE FIRST TERM OF THE CT TIME DERIVATIVE OF THE
!                         NUTATION PORTION OF THE COMPLETE CRUST FIXED TO
!                         J2000.0 ROTATION MATRIX. (1/SEC)
!         3. R2N(3,3)  -  THE SECOND TERM OF THE CT TIME DERIVATIVE OF THE
!                         NUTATION PORTION OF THE COMPLETE CRUST FIXED TO
!                         J2000.0 ROTATION MATRIX. (1/SEC)
!         4. R3N(3,3)  -  THE THIRD TERM OF THE CT TIME DERIVATIVE OF THE
!                         NUTATION PORTION OF THE COMPLETE CRUST FIXED TO
!                         J2000.0 ROTATION MATRIX. (1/SEC)
!         5. RN1(3,3)  -  THE ROTATION MATRIX WHICH PERFORMS A ROTATION ABOUT
!                         THE TRUE EQUINOCTIAL LINE BY AN ANGLE EQUAL TO
!                         EPSMN(1)+DEPS(1). (UNITLESS)
!         6. RN2(3,3)  -  THE ROTATION MATRIX WHICH PERFORMS A ROTATION ABOUT
!                         THE MEAN ECLIPTIC POLE BY AN ANGLE EQUAL TO DPSI(1).
!                         (UNITLESS)
!         7. RN3(3,3)  -  THE ROTATION MATRIX WHICH PERFORMS A ROTATION ABOUT
!                         THE MEAN EQUINOCTIAL LINE BY AN ANGLE EQUAL TO
!                         -EPSMN(1). (UNITLESS)
!         8. RN4(3,3)  -  THE CT TIME DERIVATIVE OF THE ROTATION MATRIX RN1.
!                         (1/SEC)
!         9. RN5(3,3)  -  THE CT TIME DERIVATIVE OF THE ROTATION MATRIX RN2.
!                         (1/SEC)
!        10. RN6(3,3)  -  THE CT TIME DERIVATIVE OF THE ROTATION  MATRIX RN3.
!                         (1/SEC)
!        11. EPSMN(2)  -  MEAN OBLIQUITY OF DATE AND CT TIME DERIVATIVE.
!                         (RAD, RAD/SEC)
!        12. NUTWAHR(4)-  Array to hold the IAU 1980 (Wahr) Dpsi, Deps, and
!                         their time derivatives (rad, rad/sec)
!        13. NUT2000(4)-  Array to hold the IAU2000A Dpsi, Deps, and
!                         their time derivatives (rad, rad/sec)
!
! 4.2.9 PROGRAMMER
!        77.01.13  DALE MARKHAM
!        77.07.12  PETER DENATALE
!        78.03.15  BRUCE SCHUPLER
!        79.02.01  BRUCE SCHUPLER
!        79.06.06  BRUCE SCHUPLER
!        81.08.04  CHOPO MA
!        81.08.24  JIM RYAN
!        89.06.30  Jim Ryan: Documetation clean up.
!        89.12.12  Jim Ryan: UNIX-like database interface implemented
!        93.09.01  Norbert Zacharias: put parts of NUTG,NUTW into NUTFA
!                  for equation of equinox update EQE
!        94.04.15  David Gordon: Converted to Implicit None.
!        98.02.03  D. Gordon: Default changed to IERS 1996 Nutation. Call to
!                  Subroutine KSV_1996 (from Tom Herring, modified at USNO)
!                  added. Added PUT for 'NUT 1996'. Other minor mods.
!        98.11.12  D. Gordon: Added code to compute the correction in longitude
!                  for the effect of geodesic nutation, according to Fukushima
!                  (IERS Conventions (1996), page 37).
!       2002.09    J. Ryan: Integer*4 conversion.
!       2003/2004  David Gordon: Mods for IERS Conventions 2003.
!
!  NUTG PROGRAM STRUCTURE
!
!  Compute the CT time derivative of CENT.
      DCENT = 1.D0 / ( CENTJ * SECDAY )
!
!  Compute the nutation in obliquity and longitude.
      IF (KNUTC .ne. 1) THEN
!   Compute Wahr nutation (skip if same time as previous observation)
       IF(TSKIP.EQ.1) GO TO 150
        CALL NUTW (CENT, fa, fad, DPSI, DEPS)
         DPSI (1) = DPSI(1) * CONVDS
         DEPS (1) = DEPS(1) * CONVDS
         DPSI (2) = DPSI(2) * CONVDS
         DEPS (2) = DEPS(2) * CONVDS
        NUTWAHR(1) = DPSI(1)
        NUTWAHR(2) = DEPS(1)
        NUTWAHR(3) = DPSI(2)
        NUTWAHR(4) = DEPS(2)
  150  CONTINUE
!
        CALL PUT4 ('NUT WAHR      ',NUTWAHR,int2(2),int2(2),int2(1))
      ENDIF
!
      IF (KNUTC .EQ. 1) THEN
!   Option to turn off Wahr nutation.
        DEPS(1) = 0.0D0
        DEPS(2) = 0.0D0
        DPSI(1) = 0.0D0
        DPSI(2) = 0.0D0
      ENDIF
!
!   Calculate the mean and true obliquities of the ecliptic and their CT time
!   derivatives in units of radians and radians per second.
!
        IF(TSKIP.EQ.1) GO TO 625
!   Compute the mean obliquity and its CT time derivative.
      EPSMN(1) = ( EC(1)
     .           + EC(2) * CENT
     .           + EC(3) * CENT**2
     .           + EC(4) * CENT**3 ) * CONVDS
      EPSMN(2) = ( EC(2)
     .           + EC(3) * ( 2.D0 * CENT )
     .           + EC(4) * ( 3.D0 * CENT**2 ) )
     .         * DCENT * CONVDS
!
!   Compute the true obliquity and its CT time derivative.
      EPS(1) = EPSMN(1)  +  DEPS(1)
      EPS(2) = EPSMN(2)  +  DEPS(2)
!
!   Pass the mean obliquity of the reference epoch J2000.0.
      EPSMNR = EC(1) * CONVDS
!
!   Construct the nutation portion of the complete crust fixed to J2000.0
!   rotation matrix if nutation is turned on.
      IF (KNUTC .EQ. 1) GO TO 599
!
  625  CONTINUE
!********************************************************************
!
        IF(TSKIP.EQ.1) GO TO 655
!
!  Compute IAU2000A nutation:
        CALL NU2KA (CENT,DCENT,XJD,CT,FA2K,FAD2K,DPSI2K,DEPS2K)
!
        NUT2000(1,1) = DPSI2K(1)
        NUT2000(1,2) = DPSI2K(2)
        NUT2000(2,1) = DEPS2K(1)
        NUT2000(2,2) = DEPS2K(2)
!
! Compute frame bias and rate between IAU2000A and IAU1980 nutations
        DPSIdif = (-299.65*CENT - 41.775) * CONVDS * 1.0D-3
        DEPSdif = ( -25.24*CENT - 6.8192) * CONVDS * 1.0D-3
! Compute differences between IAU2000A and IAU1980 nutation values
        NUTDIF(1,1) = NUTWAHR(1) - NUT2000(1,1) - DPSIdif
        NUTDIF(2,1) = NUTWAHR(2) - NUT2000(2,1) - DEPSdif
        NUTDIF(1,2) = NUTWAHR(3) - NUT2000(1,2)
        NUTDIF(2,2) = NUTWAHR(4) - NUT2000(2,2)
!
!   Add IAU2000A precession correction to IAU 1980 mean obliquity of
!    date and its CT time derivative.
!
!   Compute the mean obliquity and its CT time derivative.
!   (Old/Keep)
      EPSMN(1) = ( EC(1)
     .           + EC(2) * CENT
     .           + EC(3) * CENT**2
     .           + EC(4) * CENT**3 ) * CONVDS
      EPSMN(2) = ( EC(2)
     .           + EC(3) * ( 2.D0 * CENT )
     .           + EC(4) * ( 3.D0 * CENT**2 ) )
     .         * DCENT * CONVDS
!
! New: Correct for IAU2000A precession rate
      EPSA(1) = EPSMN(1) + (-0.02524D0)*CENT*CONVDS
      EPSA(2) = EPSMN(2) + (-0.02524D0)*DCENT*CONVDS
!
!   Construct the rotation matrix which rotates about the true
!   equinoctial line by an angle equal to [EPSA(1)+DEPS2K(1)].
      CALL ROTAT ( EPSA(1)+DEPS2K(1), int2(1), RN1)
!   Construct the rotation matrix which rotates about the mean
!   ecliptic pole by an angle equal to DPSI2K(1).
      CALL ROTAT ( DPSI2K(1), int2(3), RN2)
!   Construct the rotation matrix which rotates about the mean
!   equinoctial line by an angle equal to -EPSA(1).
      CALL ROTAT ( -EPSA(1), int2(1), RN3)
!   Complete the construction of the nutation rotation matrix.
      CALL MMUL3 ( RN3, RN2, RN1, RNC2K(1,1,1) )
!
!   Compute the CT time derivative of the nutation rotation matrix.
!
!   Construct the CT time derivative of the rotation matrix RN1.
      CALL DROTT ( EPSA(1)+DEPS2K(1), EPSA(2)+DEPS2K(2), int2(1), RN4)
!   Construct the CT time derivative of the rotation matrix RN2.
      CALL DROTT ( DPSI2K(1), DPSI2K(2), int2(3), RN5)
!   Construct the CT time derivative of the rotation matrix RN3.
      CALL DROTT ( -EPSA(1), -EPSA(2), int2(1), RN6)
!  Complete the construction of the CT time derivative of the nutation
!  matrix.
!   Compute the three terms necessary for the calculation.
      CALL MMUL3 ( RN6, RN2, RN1, R1N )
      CALL MMUL3 ( RN3, RN5, RN1, R2N )
      CALL MMUL3 ( RN3, RN2, RN4, R3N )
!  Add the three terms to complete the calculation.
      CALL MADD3 ( R1N, R2N, R3N, RNC2K(1,1,2) )
!
 655   Continue
!
        CALL PUT4 ('NUT2000A      ',NUT2000,int2(2),int2(2),int2(1))
!
!********************************************************************
!  Obtain the CEO-based bias-precession-nutation matrix.
!
        IF(TSKIP.EQ.1) GO TO 665
!
!   Obtain the IAU 2000A X, Y, and S nutation values.
      CALL XYS2KA (XJD, CT, CENT, DCENT, FA2K, FAD2K, Xn, Yn, Sn )
!
        NUTXYS (1,1) = Xn(1)
        NUTXYS (1,2) = Xn(2)
        NUTXYS (2,1) = Yn(1)
        NUTXYS (2,2) = Yn(2)
        NUTXYS (3,1) = Sn(1)
        NUTXYS (3,2) = Sn(2)
!
!   Compute the intermediate-to-GCRS rotation matrix
      CALL BPN2K(Xn, Yn, Sn, RBPN, dRBPN )
 1012  Format(1x,'NUTG/RBPN,dRBPN:',(6(/,3E25.15)))
      Do II = 1,3
       Do JJ = 1,3
        RPN2K(II,JJ,1) =  RBPN(II,JJ)
        RPN2K(II,JJ,2) = dRBPN(II,JJ)
       Enddo
      Enddo
!
 665   Continue
!
        CALL PUT4 ('NUT2KXYS      ', NUTXYS,int2(3),int2(2),int2(1))
!************************************************************
  599 CONTINUE
!
!  Check KNUTC to determine if the nutation module is to be turned off.
!     IMPORTANT NOTE -- Turning off the nutation module will also turn off
!     the nutation in obliquity portion of subroutine DIURNAL.  BEWARE!
!
      IF ( KNUTC .EQ. 1 )  THEN
!   Set the position portion of the nutation matrix to the identity matrix.
       CALL ROTAT ( 0.D0, int2(3), RPN2K(1,1,1))
!   Set the velocity portion of the nutation matrix to 0.0D0.
       DO 600 N=1,3
         DO 600 M=1,3
          RPN2K(M,N,2) = 0.0D0
  600  CONTINUE
      ENDIF
!
!
!   Check KNUTD for debug output.
  700 IF (KNUTD .EQ. 0) GO TO 800
      WRITE (6,9100)
 9100 FORMAT (1X, "Debug output for subroutine NUTG." )
    8 FORMAT(A,4D25.16/(7X,5D25.16))
      WRITE(6,8)' CENT    ',CENT
      WRITE(6,8)' DCENT   ',DCENT
      WRITE(6,8)' CENTJ   ',CENTJ
      WRITE(6,8)' SECDAY  ',SECDAY
      WRITE(6,8)' CONVDS  ',CONVDS
      WRITE(6,8)' EC      ',EC
      WRITE(6,8)' EPS     ',EPS
      WRITE(6,8)' EPSMN   ',EPSMN
      WRITE(6,8)' EPSMNR  ',EPSMNR
      WRITE(6,8)' EPSA    ',EPSA
      WRITE(6,8)' NUTWAHR ',NUTWAHR
      WRITE(6,8)' NUT2000 ',NUT2000
      WRITE(6,8)' NUT2000_FULL: ', NUT2000(1,1) + DPSIdif,
     .                             NUT2000(2,1) + DEPSdif
      WRITE(6,8)' NUTXYS  ',NUTXYS
      WRITE(6,8)' NUTDIF  ',NUTDIF
      Write(6,1017) RNC2K
 1017 Format(1x,'/RNC2K, New Classical Nutation Matrix:',(6(/,3E25.15)))
      Write(6,1019) RPN2K
 1019 Format(1x,'RPN2K, CEO-based Nutation Matrix:',(6(/,3E25.15)))
      WRITE(6,*) ' DPSIdif: ', DPSIdif/(CONVDS * 1.0D-3)
      WRITE(6,*) ' DEPSdif: ', DEPSdif/(CONVDS * 1.0D-3)
!
  800 RETURN
      END
!********************************************************************
        SUBROUTINE NUTP (CFBASE, EPS, Xn, Yn, Sn, DEPS2K, DPSI2K, EPSA,
     .                   GAST2K, STAR, RPN2K, RS2K, RW2K, RNC2K, RPC2K,
     .                   RSC2K, RFR2K, TSKIP, DNUpe)
        IMPLICIT None
!
! 5.    NUTP
!
! 5.1   NUTP PROGRAM SPECIFICATION
!
! 5.1.1 NUTP IS THE NUTATION MODULE PARTIAL DERIVATIVES SECTION. NUTP COMPUTES
!       THE PARTIAL DERIVATIVES OF THE DELAY AND OF THE DELAY RATE W.R.T. PSI
!       AND EPSILON AT THE EPOCH OF THE OBSERVATION.
!
! 5.1.2 RESTRICTIONS - NONE
!
! 5.1.3 REFERENCES - 1) ROBERTSON, GEODETIC AND ASTROMETRIC MEASUREMENTS
!                       WITH VLBI, NASA X-DOCUMENT X-922-77-228.
!                    2) KAPLAN, USNO NOTES.
!
! 5.2   NUTP PROGRAM INTERFACE
!
! 5.2.1 CALLING SEQUENCE -
!
!          INPUT VARIABLES:
!             1. CFBASE(3) -  THE CRUST FIXED BASELINE VECTOR. (M)
!             2. EPS(2)    -  THE TRUE OBLIQUITY OF THE ECLIPTIC AND ITS CT
!                             TIME DERIVATIVE. (RAD, RAD/SEC)
!             3. Xn(2)     -  X-component of the CIP (Celestial
!                             Intermediate Pole) in the GCRS (Geocentric
!                             Celestial Reference System), and its time
!                             derivative. (Radians, Radians/sec)
!             4. Yn(2)     -  Y-component of the CIP (Celestial
!                             Intermediate Pole) in the GCRS (Geocentric
!                             Celestial Reference System), and its time
!                             derivative. (Radians, Radians/sec)
!             5. Sn(2)     -  Position of the CEO (Celestial Ephemeris
!                             Origin) on the equator of the CIP, and its
!                             time derivative. (Radians, Radians/sec)
!             6. DEPS2K(2) -  THE NUTATION IN OBLIQUITY AND ITS CT TIME
!                             DERIVATIVE COMPUTED FROM WAHR OR TAKEN FROM THE
!                             DATA BASE. (RAD, RAD/SEC)
!             7. DPSI2K(2) -  THE NUTATION IN LONGITUDE AND ITS CT TIME
!                             DERIVATIVE COMPUTED FROM WAHR OR TAKEN FROM THE
!                             DATA BASE. (RAD, RAD/SEC)
!             8. EPSA(2)   -  THE MEAN OBLIQUITY OF THE ECLIPTIC at the
!                             reference epoch using IAU200A
!                             Precession/Nutation ???
!                             and its time derivative. ????
!             9. GAST2K(2) -  THE GREENWICH APPARENT SIDEREAL TIME AND ITS CT
!                             TIME DERIVATIVE. (RAD, RAD/SEC)
!            10. STAR(3)   -  THE J2000.0 SOURCE UNIT VECTOR. (UNITLESS)
!            11. RPN2K(3,3,2)-THE PRECESSION-NUTATION PORTION OF THE CRUST
!                             FIXED TO J2000.0 ROTATION MATRIX and it first
!                             time derivative. (unitless, 1/sec)
!            12. RS2K(3,3,3)- THE DIURNAL SPIN PORTION OF THE COMPLETE CRUST
!                             FIXED TO J2000.0 ROTATION MATRIX AND ITS FIRST
!                             two CT TIME DERIVATIVEs. (UNITLESS, 1/SEC)
!            13. RW2K(3,3,2)- THE WOBBLE PORTION OF THE COMPLETE CRUST FIXED
!                             TO J2000.0 ROTATION MATRIX and its time
!                             derivative. (unitless, 1/sec)
!            14. RNC2K(3,3,2)-THE NUTATION PORTION OF THE COMPLETE CRUST FIXED
!                             TO J2000.0 ROTATION MATRIX and it first time
!                             derivative. (unitless, 1/sec)
!            15. RPC2K(3,3,2)-THE PRECESSION PORTION OF THE COMPLETE CRUST
!                             FIXED TO J2000.0 ROTATION MATRIX AND ITS CT
!                             TIME DERIVATIVE, consistent with the IERS
!                             Conventions (2003). (UNITLESS, 1/SEC)
!            16. RSC2K(3,3,3)-THE DIURNAL SPIN PORTION OF THE COMPLETE CRUST
!                             FIXED TO J2000.0 ROTATION MATRIX AND ITS FIRST
!                             TWO CT TIME DERIVATIVES. (UNITLESS, 1/SEC,
!                             1/SEC**2)
!            17. RFR2K(3,3) - The frame bias rotation matrix
!            18. TSKIP      - Skip recomputations if TSKIP=1.
!
!          OUTPUT VARIABLES:
!             1. DNUpe(2,2) - PARTIAL DERIVATIVES OF THE DELAY AND RATE
!                             W.R.T DPSI2K AND DEPS2K. (SEC/RAD, SEC/SEC/RAD)
!
! 5.2.2 COMMON BLOCKS USED -
!
      INCLUDE 'ccon.i'
!           VARIABLES 'FROM':
!              1. KNUTC  -  THE NUTATION MODULE FLOW CONTROL FLAG.
!              2. KNUTD  -  THE NUTATION MODULE DEBUG OUTPUT FLAG.
!
      INCLUDE 'cphys.i'
!           VARIABLES 'FROM':
!              1.  VLIGHT    -  THE VELOCITY OF LIGHT IN VACUUM. (M/SEC)
!
      Real*8 RN1(3,3), RN2(3,3), RN3(3,3), RN4(3,3), RN5(3,3), RN6(3,3)
      COMMON / RN1to6 / RN1, RN2, RN3, RN4, RN5, RN6
!
! 5.2.3 PROGRAM SPECIFICATIONS -
      Real*8 CFBASE(3), EPS(2), RDNP(3,3), STAR(3),
     .       DPSI2K(2), DEPS2K(2), Xn(2), Yn(2), Sn(2), EPSA(2),
     .       GAST2K(2), RPN2K(3,3,2), RS2K(3,3,3), RW2K(3,3,2),
     .       RNC2K(3,3,2), RPC2K(3,3,2), RSC2K(3,3,3), RFR2K(3,3)
      Real*8 C, S, CT, ST, SINARG, COSARG, DOTP
!??   Real*8 C, S, CT, ST, SINARG, COSARG, DOTP, ROTAT, DROTT, DDROT,
!??  &       MMUL3, MADD3, MADD2, MADD4, MMUL5
      REAL*8 D1, RNCeps(3,3,2), RN1eps(3,3), RN4eps(3,3), R1Neps(3,3),
     .       R2Neps(3,3), R3Neps(3,3), RNCpsi(3,3,2), RN2psi(3,3),
     .       RN5psi(3,3), R1Npsi(3,3), R2Npsi(3,3), R3Npsi(3,3),
     .       RBPNX(3,3,2),RBPNY(3,3,2), RSCpsi(3,3,3), CEPSA, SEPSA
      REAL*8 R2KX(3,3,3), R2KY(3,3,3), RC2Keps(3,3,3), RC2Kpsi(3,3,3),
     .       pBL_X(3,2),pBL_Y(3,2),pBL_eps(3,2),pBL_psi(3,2),
     .       DNUXY(2,2), DNUpe(2,2)
      REAL*8 dRX1(3,3), dRX2(3,3), dRX3(3,3), ddRX1(3,3), ddRX2(3,3),
     .       ddRX3(3,3), ddRX4(3,3), ddRX11(3,3), ddRX12(3,3),
     .       ddRX13(3,3), dRY1(3,3), dRY2(3,3), dRY3(3,3), ddRY1(3,3),
     .       ddRY2(3,3), ddRY3(3,3), ddRY4(3,3), ddRY11(3,3),
     .       ddRY12(3,3), ddRY13(3,3), RC2K1(3,3), RC2K2(3,3)
      REAL*8 R2001e(3,3), R2002e(3,3), R2003e(3,3), R2004e(3,3),
     .       R2005e(3,3), R2006e(3,3), R2007e(3,3), R2008e(3,3),
     .       R2p1a(3,3), R2p1b(3,3), R2p1(3,3), R2p2a(3,3), R2p2b(3,3),
     .       R2p2(3,3), R2p3a(3,3), R2p3b(3,3), R2p3(3,3), R2p4a(3,3),
     .       R2p4b(3,3), R2p4(3,3), R2p5a(3,3), R2p5b(3,3), R2p5(3,3),
     .       R2p6a(3,3), R2p6b(3,3), R2p6(3,3), R2p7a(3,3), R2p7b(3,3),
     .       R2p7(3,3), R2p8a(3,3), R2p8b(3,3), R2p8(3,3), S2p1(3,3),
     .       R2e(3,3)
      Integer*4 TSKIP, N
       SAVE  R2KX, R2KY, RC2Keps, RC2Kpsi
!
! 5.2.4 DATA BASE ACCESS -
!        'PUT' VARIABLES:
!           1. DNUpe(2,2)  -  PARTIAL DERIVATIVES OF THE DELAY AND THE DELAY
!                             RATE W.R.T. DPSI AND DEPS. (SEC/RAD, SEC/SEC/RAD)
!           2. DNUXY(2,2)  -  Partial derivatives of the delay and rate
!                             w.r.t. X and Y CEO-based nutation.
!        ACCESS CODES:
!           1. 'NUT2000P'  -  The database access code for the classical
!                             nutation partial derivatives array.
!           2. 'NUT2KXYP'  -  The database access code for the CEO-based
!                             nutation partial derivatives array.
!
! 5.2.5 EXTERNAL INPUT/OUTPUT - POSSIBLE ERROR OUTPUT
!
! 5.2.6 SUBROUTINE INTERFACE -
!             CALLER SUBROUTINES: DRIVR
!             CALLED SUBROUTINES: DCOS, DSIN, MMUL5, MADD2, PUT4, VECRT
!
! 5.2.7 CONSTANTS USED - VLIGHT
!
! 5.2.8 PROGRAM VARIABLES -
!          1. RBPNX(3,3,2)  - Partial derivative of RPN2K w.r.t. X nutation.
!          2. RBPNY(3,3,2)  - Partial derivative of RPN2K w.r.t. Y nutation.
!          3. R2KX(3,3,3)   - Partial derivative of R2K w.r.t. X nutation.
!          4. R2KY(3,3,3)   - Partial derivative of R2K w.r.t. Y nutation.
!          5. RNCeps(3,3,2) - Partial derivative of RNC2K w.r.t. obliquity
!                             (epsilon).
!          6. RNCpsi(3,3,2) - Partial derivative of RNC2K w.r.t. longitude
!                             (psi).
!          7. RSCpsi(3,3,3) - Partial derivative of RSC2K (classical diurnal
!                             spin matrix) w.r.t. longitude (psi).
!          8. RC2Keps(3,3,2)- Partial derivative of RC2K w.r.t. obliquity
!                             (epsilon).
!          9. RC2Kpsi(3,3,2)- Partial derivative of RC2K w.r.t. longitude
!                             (psi).
!         10. pBL_X(3,2)    - Partial derivatives of the baseline vector
!                             w.r.t. X nutation.
!         11. pBL_Y(3,2)    - Partial derivatives of the baseline vector
!                             w.r.t. Y nutation.
!         12. pBL_eps(3,2)  - Partial derivatives of the baseline vector
!                             w.r.t. obliquity (epsilon).
!         13. pBL_psi(3,2)  - Partial derivatives of the baseline vector
!                             w.r.t. longitude (psi).
!
! 5.2.9 PROGRAMMER - DOUG ROBERTSON 09/07/82
!                    GEORGE KAPLAN 04/24/84
!                    DAVID GORDON 11/13/84 REDIMENSIONED NUT# PART AND DNUTPT
!                    Jim Ryan     89:10:05 CPHYS common made an include file
!                    Jim Ryan     89.12.12 UNIX-like database interface
!                                 implimented.
!                    David Gordon 94.04.15 Converted to Implicit None.
!                    David Gordon 95.12.11 Changed RW(3,3) to RW(3,3,2).
!                    David Gordon 95.12.12 RS improperly dimensioned as
!                                 (3,3,2). Changed to (3,3,3). RP and RN
!                                 improperly dimensioned as (3,3). Changed to
!                                 (3,3,2).
!                    David Gordon 98.02.03  Removed DNUTPT and computation
!                                 and PUT's of 'NT1 PART', 'NT2 PART', etc.
!                    Jim Ryan 2002.09 Integer*4 conversion.
!                    David Gordon 2003-2004 Mods for 2003 IERS Conventions.
!
! 5.3   NUTP PROGRAM STRUCTURE
!
!  Start New Code
!
!    Skip site independent computations if same time as previous obs.
       IF(TSKIP.EQ.1) GO TO 150
!
!    Compute partial derivatives of the IERS2000 CEO Bias/Prec/Nut
!     rotation matrix (RPN2K) w.r.t. X and Y.
      CALL BPN2KP(Xn,Yn,Sn,RBPNX,RBPNY)
!
!    Compute partial derivatives of the IERS2000 CEO-based transformation
!     matrix (R2K) w.r.t. the X and Y nutation quantities.
!
!   Partials of R2K(1,1,1).
      CALL MMUL3 (RBPNX(1,1,1), RS2K(1,1,1), RW2K(1,1,1), R2KX(1,1,1))
      CALL MMUL3 (RBPNY(1,1,1), RS2K(1,1,1), RW2K(1,1,1), R2KY(1,1,1))
!
!   Partials of first time derivative, R2K(1,1,2).
      CALL MMUL3 ( RBPNX(1,1,2), RS2K(1,1,1), RW2K(1,1,1), dRX1 )
      CALL MMUL3 ( RBPNX(1,1,1), RS2K(1,1,2), RW2K(1,1,1), dRX2 )
      CALL MMUL3 ( RBPNX(1,1,1), RS2K(1,1,1), RW2K(1,1,2), dRX3 )
      CALL MADD3 ( dRX1, dRX2, dRX3, R2KX(1,1,2 ) )
!
      CALL MMUL3 ( RBPNY(1,1,2), RS2K(1,1,1), RW2K(1,1,1), dRY1 )
      CALL MMUL3 ( RBPNY(1,1,1), RS2K(1,1,2), RW2K(1,1,1), dRY2 )
      CALL MMUL3 ( RBPNY(1,1,1), RS2K(1,1,1), RW2K(1,1,2), dRY3 )
      CALL MADD3 ( dRY1, dRY2, dRY3, R2KY(1,1,2 ) )
!
!!!!!!!!!!!!!!!! Second derivatives not needed !!!!!!!!!!!!!!!!!!!!!!!!!!!!
!   Partials of second time derivative, R2K(1,1,3).
!
      CALL MMUL3 ( RBPNX(1,1,2), RS2K(1,1,2), RW2K(1,1,1), ddRX1 )
      CALL MMUL3 ( RBPNX(1,1,2), RS2K(1,1,1), RW2K(1,1,2), ddRX2 )
       CALL MADD2 (ddRX1, ddRX2, ddRX11)
!     CALL MMUL3 ( RBPNX(1,1,2), RS2K(1,1,2), RW2K(1,1,1), ddRX1 )
      CALL MMUL3 ( RBPNX(1,1,1), RS2K(1,1,3), RW2K(1,1,1), ddRX3 )
      CALL MMUL3 ( RBPNX(1,1,1), RS2K(1,1,2), RW2K(1,1,2), ddRX4 )
       CALL MADD3 (ddRX1, ddRX3, ddRX4, ddRX12)
!     CALL MMUL3 ( RBPNX(1,1,2), RS2K(1,1,1), RW2K(1,1,2), ddRX2 )
!     CALL MMUL3 ( RBPNX(1,1,1), RS2K(1,1,2), RW2K(1,1,2), ddRX4 )
       CALL MADD2 (ddRX2, ddRX4, ddRX13)
!     Complete the second derivative
      CALL MADD3 ( ddRX11, ddRX12, ddRX13, R2KX(1,1,3) )
!
      CALL MMUL3 ( RBPNY(1,1,2), RS2K(1,1,2), RW2K(1,1,1), ddRY1 )
      CALL MMUL3 ( RBPNY(1,1,2), RS2K(1,1,1), RW2K(1,1,2), ddRY2 )
       CALL MADD2 (ddRY1, ddRY2, ddRY11)
!     CALL MMUL3 ( RBPNY(1,1,2), RS2K(1,1,2), RW2K(1,1,1), ddRY1 )
      CALL MMUL3 ( RBPNY(1,1,1), RS2K(1,1,3), RW2K(1,1,1), ddRY3 )
      CALL MMUL3 ( RBPNY(1,1,1), RS2K(1,1,2), RW2K(1,1,2), ddRY4 )
       CALL MADD3 (ddRY1, ddRY3, ddRY4, ddRY12)
!     CALL MMUL3 ( RBPNY(1,1,2), RS2K(1,1,1), RW2K(1,1,2), ddRY2 )
!     CALL MMUL3 ( RBPNY(1,1,1), RS2K(1,1,2), RW2K(1,1,2), ddRY4 )
       CALL MADD2 (ddRY2, ddRY4, ddRY13)
!     Complete the second derivative
      CALL MADD3 ( ddRY11, ddRY12, ddRY13, R2KY(1,1,3) )
!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!
!   Compute the partial derivative of RNC2K(i,j,1) w.r.t. the
!   obliquity angle DEPS2K(1).
!    Partials of RN2 and RN3 are zeros.
!    Partial of RN1 .w.r.t. DEPS2K(1):
         D1 = 1.D0
      CALL DROTT ( EPSA(1)+DEPS2K(1), D1, int2(1), RN1eps)
!  Complete the partial
      CALL MMUL3 ( RN3, RN2, RN1eps, RNCeps(1,1,1) )
!
!   Compute the partial derivative of RNC2K(i,j,2) w.r.t. the
!   obliquity angle DEPS2K(1).
!    Partials of RN5 and RN6 are zeros.
!    Partial of RN4 .w.r.t. DEPS2K(1):
      CALL DDROT ( EPSA(1)+DEPS2K(1),EPSA(2)+DEPS2K(2),int2(1),RN4eps)
!   Compute the three terms necessary for the calculation.
      CALL MMUL3 ( RN6, RN2, RN1eps, R1Neps )
      CALL MMUL3 ( RN3, RN5, RN1eps, R2Neps )
      CALL MMUL3 ( RN3, RN2, RN4eps, R3Neps )
!  Add the three terms to complete the calculation.
      CALL MADD3 ( R1Neps, R2Neps, R3Neps, RNCeps(1,1,2) )
!
!   Compute the partial derivative of RNC2K(i,j,1) w.r.t. the
!             angle DPSI2K(1).
!    Partials of RN1 and RN3 are zeros.
!    Partial of RN2 .w.r.t. DPSI2K(1):
      CALL DROTT ( DPSI2K(1), D1, int2(3), RN2psi)
!  Complete the partial
      CALL MMUL3 ( RN3, RN2psi, RN1, RNCpsi(1,1,1) )
!
!   Compute the partial derivative of RNC2K(i,j,2) w.r.t. the
!             angle DPSI2K(1).
!    Partials of RN4 and RN6 are zeros.
!    Partial of RN5 .w.r.t. DPSI2K(1):
      CALL DDROT ( DPSI2K(1), DPSI2K(2), int2(3), RN5psi)
!   Compute the three terms necessary for the calculation.
      CALL MMUL3 ( RN6, RN2psi, RN1, R1Npsi )
      CALL MMUL3 ( RN3, RN5psi, RN1, R2Npsi )
      CALL MMUL3 ( RN3, RN2psi, RN4, R3Npsi )
!  Add the three terms to complete the calculation.
      CALL MADD3 ( R1Npsi, R2Npsi, R3Npsi, RNCpsi(1,1,2) )
!
!
!   Compute partial derivatives of the IERS2000 classical diurnal
!   spin matrix, RSC2K, and its first two CT time derivatives w.r.t.
!   DPSI2K(2).
!
!   Partial of RSC2K(1,1,1):
        CEPSA = DCOS(EPSA(1))
        SEPSA = DSIN(EPSA(1))
      CALL DROTT ( -GAST2K(1), -CEPSA, int2(3), RSCpsi(1,1,1))
!
!   Partial of RSC2K(1,1,2):
      RSCpsi(1,1,2) = -DCOS(GAST2K(1)) * GAST2K(2) * CEPSA  +
     .                 DSIN(GAST2K(1)) * SEPSA*EPSA(2)
      RSCpsi(1,2,2) =  DSIN(GAST2K(1)) * GAST2K(2) * CEPSA  +
     .                 DCOS(GAST2K(1)) * SEPSA*EPSA(2)
      RSCpsi(1,3,2) =  0.D0
      RSCpsi(2,1,2) = -RSCpsi(1,2,2)
      RSCpsi(2,2,2) =  RSCpsi(1,1,2)
      RSCpsi(2,3,2) =  0.D0
      RSCpsi(3,1,2) =  0.D0
      RSCpsi(3,2,2) =  0.D0
      RSCpsi(3,3,2) =  0.D0
!
!!!!!!!!!!!!!!!! Second derivatives not needed !!!!!!!!!!!!!!!!!!!!!!!!!!!!
!   Partial of RSC2K(1,1,3):
      RSCpsi(1,1,3) =  DSIN(GAST2K(1)) * GAST2K(2)**2 * CEPSA  +
     .                 DCOS(GAST2K(1)) * 2.D0*GAST2K(2) * SEPSA*EPSA(2)
      RSCpsi(1,2,3) =  DCOS(GAST2K(1)) * GAST2K(2)**2 * CEPSA  -
     .                 DSIN(GAST2K(1)) * 2.D0*GAST2K(2) * SEPSA*EPSA(2)
      RSCpsi(1,3,3) =  0.D0
      RSCpsi(2,1,3) = -RSCpsi(1,2,3)
      RSCpsi(2,2,3) =  RSCpsi(1,1,3)
      RSCpsi(2,3,3) =  0.D0
      RSCpsi(3,1,3) =  0.D0
      RSCpsi(3,2,3) =  0.D0
      RSCpsi(3,3,3) =  0.D0
!
!
!    Compute partial derivative of the IERS2000 classical transformation
!     matrix, RC2K, w.r.t. DEPS2K(1) and DPSI2K(1).
!
!  Partials of RC2K(1,1,1) w.r.t. DEPS2K(1)
      CALL MMUL5 ( RFR2K, RPC2K(1,1,1), RNCeps(1,1,1), RSC2K(1,1,1),
     .             RW2K(1,1,1), RC2Keps(1,1,1) )
!
!  Partials of RC2K(1,1,2) w.r.t. DEPS2K(1)
      CALL MMUL5 ( RFR2K, RPC2K(1,1,2), RNCeps(1,1,1), RSC2K(1,1,1),
     .             RW2K(1,1,1), R2001e )
      CALL MMUL5 ( RFR2K, RPC2K(1,1,1), RNCeps(1,1,2), RSC2K(1,1,1),
     .             RW2K(1,1,1), R2002e )
      CALL MMUL5 ( RFR2K, RPC2K(1,1,1), RNCeps(1,1,1), RSC2K(1,1,2),
     .             RW2K(1,1,1), R2003e )
      CALL MMUL5 ( RFR2K, RPC2K(1,1,1), RNCeps(1,1,1), RSC2K(1,1,1),
     .             RW2K(1,1,2), R2004e )
!   Add the four terms to complete the calculation.
      CALL MADD4 ( R2001e, R2002e, R2003e, R2004e, RC2Keps(1,1,2) )
!
!!!!!!!!!!!!!!!! Second derivatives not needed !!!!!!!!!!!!!!!!!!!!!!!!!!!!
!  Partials of RC2K(1,1,3) w.r.t. DEPS2K(1)
      CALL MMUL5 ( RFR2K, RPC2K(1,1,2), RNCeps(1,1,1), RSC2K(1,1,2),
     .             RW2K(1,1,1), R2005e )
      CALL MMUL5 ( RFR2K, RPC2K(1,1,1), RNCeps(1,1,2), RSC2K(1,1,2),
     .             RW2K(1,1,1), R2006e )
      CALL MMUL5 ( RFR2K, RPC2K(1,1,1), RNCeps(1,1,1), RSC2K(1,1,3),
     .             RW2K(1,1,1), R2007e )
      CALL MMUL5 ( RFR2K, RPC2K(1,1,1), RNCeps(1,1,1), RSC2K(1,1,2),
     .             RW2K(1,1,2), R2008e )
!    (R2005e, R2006e, and R2008e need to be added twice)
      CALL MADD5 ( R2005e, R2005e, R2006e, R2006e, R2007e, R2e)
      CALL MADD3 (R2e, R2008e, R2008e, RC2Keps (1,1,3))
!
!
!  Partials of RC2K(1,1,1) w.r.t. DPSI2K(1)
      CALL MMUL5 ( RFR2K, RPC2K(1,1,1), RNCpsi(1,1,1), RSC2K(1,1,1),
     .             RW2K(1,1,1), RC2K1 )
      CALL MMUL5 ( RFR2K, RPC2K(1,1,1), RNC2K(1,1,1), RSCpsi(1,1,1),
     .             RW2K(1,1,1), RC2K2 )
      CALL MADD2 (RC2K1, RC2K2, RC2Kpsi (1,1,1))
!
!  Partials of RC2K(1,1,2) w.r.t. DPSI2K(1)
      CALL MMUL5 ( RFR2K, RPC2K(1,1,2), RNCpsi(1,1,1), RSC2K(1,1,1),
     .             RW2K(1,1,1), R2p1a )
      CALL MMUL5 ( RFR2K, RPC2K(1,1,2), RNC2K(1,1,1), RSCpsi(1,1,1),
     .             RW2K(1,1,1), R2p1b )
      CALL MADD2 (R2p1a, R2p1b, R2p1)
!
      CALL MMUL5 ( RFR2K, RPC2K(1,1,1), RNCpsi(1,1,2), RSC2K(1,1,1),
     .             RW2K(1,1,1), R2p2a )
      CALL MMUL5 ( RFR2K, RPC2K(1,1,1), RNC2K(1,1,2), RSCpsi(1,1,1),
     .             RW2K(1,1,1), R2p2b )
      CALL MADD2 (R2p2a, R2p2b, R2p2)
!
      CALL MMUL5 ( RFR2K, RPC2K(1,1,1), RNCpsi(1,1,1), RSC2K(1,1,2),
     .             RW2K(1,1,1), R2p3a )
      CALL MMUL5 ( RFR2K, RPC2K(1,1,1), RNC2K(1,1,1), RSCpsi(1,1,2),
     .             RW2K(1,1,1), R2p3b )
      CALL MADD2 (R2p3a, R2p3b, R2p3)
!
      CALL MMUL5 ( RFR2K, RPC2K(1,1,1), RNCpsi(1,1,1), RSC2K(1,1,1),
     .             RW2K(1,1,2), R2p4a )
      CALL MMUL5 ( RFR2K, RPC2K(1,1,1), RNC2K(1,1,1), RSCpsi(1,1,1),
     .             RW2K(1,1,2), R2p4b )
      CALL MADD2 (R2p4a, R2p4b, R2p4)
      CALL MADD4 (R2p1, R2p2, R2p3, R2p4, RC2Kpsi(1,1,2) )
!
!!!!!!!!!!!!!!!! Second derivatives not needed !!!!!!!!!!!!!!!!!!!!!!!!!!!!
!  Partials of RC2K(1,1,3) w.r.t. DPSI2K(1)
!  Compute the four largest terms.
      CALL MMUL5 ( RFR2K, RPC2K(1,1,2), RNCpsi(1,1,1), RSC2K(1,1,2),
     .             RW2K(1,1,1), R2p5a )
      CALL MMUL5 ( RFR2K, RPC2K(1,1,2), RNC2K(1,1,1), RSCpsi(1,1,2),
     .             RW2K(1,1,1), R2p5b )
      CALL MADD2 (R2p5a, R2p5b, R2p5)
!
      CALL MMUL5 ( RFR2K, RPC2K(1,1,1), RNCpsi(1,1,2), RSC2K(1,1,2),
     .             RW2K(1,1,1), R2p6a )
      CALL MMUL5 ( RFR2K, RPC2K(1,1,1), RNC2K(1,1,2), RSCpsi(1,1,2),
     .             RW2K(1,1,1), R2p6b )
      CALL MADD2 (R2p6a, R2p6b, R2p6)
!
      CALL MMUL5 ( RFR2K, RPC2K(1,1,1), RNCpsi(1,1,1), RSC2K(1,1,3),
     .             RW2K(1,1,1), R2p7a )
      CALL MMUL5 ( RFR2K, RPC2K(1,1,1), RNC2K(1,1,1), RSCpsi(1,1,3),
     .             RW2K(1,1,1), R2p7b )
      CALL MADD2 (R2p7a, R2p7b, R2p7)
!
      CALL MMUL5 ( RFR2K, RPC2K(1,1,1), RNCpsi(1,1,1), RSC2K(1,1,2),
     .             RW2K(1,1,2), R2p8a )
      CALL MMUL5 ( RFR2K, RPC2K(1,1,1), RNC2K(1,1,1), RSCpsi(1,1,2),
     .             RW2K(1,1,2), R2p8b )
      CALL MADD2 (R2p8a, R2p8b, R2p8)
!
!   Add the terms in the correct way to complete the calculation.
      CALL MADD5 ( R2p5, R2p5, R2p6, R2p6, R2p7, S2p1)
      CALL MADD3 (S2p1, R2p8, R2p8, RC2Kpsi(1,1,3))
!
 150   Continue
!
!   Compute partials of the baseline vector and its first time
!    derivative (second time derivative needed??) with respect to
!    X and Y, and psi and epsilon.
          CALL VECRT(R2KX(1,1,1),CFBASE,pBL_X(1,1))
          CALL VECRT(R2KX(1,1,2),CFBASE,pBL_X(1,2))
          CALL VECRT(R2KY(1,1,1),CFBASE,pBL_Y(1,1))
          CALL VECRT(R2KY(1,1,2),CFBASE,pBL_Y(1,2))
!
          CALL VECRT(RC2Keps(1,1,1),CFBASE,pBL_eps(1,1))
          CALL VECRT(RC2Keps(1,1,2),CFBASE,pBL_eps(1,2))
          CALL VECRT(RC2Kpsi(1,1,1),CFBASE,pBL_psi(1,1))
          CALL VECRT(RC2Kpsi(1,1,2),CFBASE,pBL_psi(1,2))
!
!   Compute the partials of the delay and delay rate with respect to
!    X and Y, and psi and epsilon.
!     X partials
          DNUXY(1,1) = DOTP(pBL_X(1,1),STAR) / VLIGHT
          DNUXY(1,2) = DOTP(pBL_X(1,2),STAR) / VLIGHT
!     Y partials
          DNUXY(2,1) = DOTP(pBL_Y(1,1),STAR) / VLIGHT
          DNUXY(2,2) = DOTP(pBL_Y(1,2),STAR) / VLIGHT
!     Psi partials
          DNUpe(1,1) = DOTP(pBL_psi(1,1),STAR) / VLIGHT
          DNUpe(1,2) = DOTP(pBL_psi(1,2),STAR) / VLIGHT
!     Epsilon partials
          DNUpe(2,1) = DOTP(pBL_eps(1,1),STAR) / VLIGHT
          DNUpe(2,2) = DOTP(pBL_eps(1,2),STAR) / VLIGHT
!
!   Call 'PUT4' to place the IERS 2003 Nutation partials into the database.
          CALL PUT4 ('NUT2KXYP      ',DNUXY,int2(2),int2(2),int2(1))
          CALL PUT4 ('NUT2000P      ',DNUpe,int2(2),int2(2),int2(1))
!
!   End New Code
!
 800      Continue
!
!     CHECK KNUTD TO DETERMINE IF DEBUG OUTPUT IS NECESSARY.
      If(KNUTD.EQ.0) GO TO 900
      WRITE ( 6, 9)
    9 FORMAT (1X, "Debug output for subroutine NUTP." )
       Write(6,1012) RBPNX
 1012 Format(' NUTP/RBPNX, Partials CEO Bias/Prec/Nut:',(6(/,3E25.15)))
       Write(6,1013) RBPNY
 1013 Format(' NUTP/RBPNY, Partials CEO Bias/Prec/Nut:',(6(/,3E25.15)))
       Write(6,1015) R2KX
 1015 Format(' NUTP/R2KX, X Partials R2K:',(6(/,3E25.15)))
       Write(6,1016) R2KY
 1016 Format(' NUTP/R2KY, Y Partials R2K:',(6(/,3E25.15)))
       Write(6,1028) RNCeps
 1028  Format(1x,' NUTP/Partial RNCeps:',(6(/,3E25.15)))
       Write(6,1029) RNCpsi
 1029  Format(1x,' NUTP/Partial RNCpsi:',(6(/,3E25.15)))
       Write(6,1031) RSCpsi
 1031  Format(1x,' NUTP/RSCpsi: ',(9(/,3E25.15)))
       Write(6,1033) RC2Keps
 1033  Format(1x,' NUTP/RC2Keps: ',(9(/,3E25.15)))
       Write(6,1034) RC2Kpsi
 1034  Format(1x,' NUTP/RC2Kpsi: ',(9(/,3E25.15)))
       Write(6,1035) pBL_X
 1035  Format(1x,' NUTP/pBL_X: ',(2(/,3E25.15)))
       Write(6,1036) pBL_Y
 1036  Format(1x,' NUTP/pBL_Y: ',(2(/,3E25.15)))
       Write(6,1037) pBL_psi
 1037  Format(1x,' NUTP/pBL_psi: ',(2(/,3E25.15)))
       Write(6,1038) pBL_eps
 1038  Format(1x,' NUTP/pBL_eps: ',(2(/,3E25.15)))
       Write(6,1039) DNUXY
 1039  Format(1x,' NUTP/DNUXY: ',(2(/,2E25.15)))
       Write(6,1040) DNUpe
 1040  Format(1x,' NUTP/DNUpe: ',(2(/,2E25.15)))
!
!   9.    NORMAL PROGRAM CONCLUSION.
!
  900 RETURN
      END
!***********************************************************************
      SUBROUTINE NUTC (NUTDIF, DNUpe)
      IMPLICIT None
!
! 1.    NUTC
!
! 1.1   NUTC PROGRAM SPECIFICATION
!
! 1.1.1 NUTC computes the contributions to convert from the IERS 1996 nutation
!       model back to the old IAU 1980 (Wahr) model, and places them in the
!       data base.
!
! 1.2   NUTC PROGRAM INTERFACE
! 1.2.1 CALLING SEQUENCE - NONE
!         Input variables:
!            1. NUTDIF(2,2) - Nutation difference: IAU1980 minus IAU2000A.
!                             First index over psi and epsilon; second
!                             index over difference and derivative of
!                             difference. (radians, radians/sec)
!            2. DNUpe(2,2)  - PARTIAL DERIVATIVES OF THE DELAY AND THE DELAY
!                             RATE W.R.T DPSI AND DEPS. (SEC/RAD,
!                             SEC/SEC/RAD)
!
        Real*8 NUTDIF(2,2), DNUpe(2,2)
!
! 1.2.2 COMMON BLOCKS USED
      INCLUDE 'ccon.i'
!            VARIABLES 'FROM':
!              1. KNUTC - THE NUTATION MODULE CONTROL FLAG.
!                 0 - IAU200A Nutation
!                 1 - NO NUTATION
!
! 1.2.3 PROGRAM SPECIFICATIONS
        Real*8 Wahr_cont(2)
!
! 1.2.4 DATA BASE ACCESS
!         'PUT' Variables:
!            1. Wahr_cont(2) - Delay and rate contributions to convert back to
!                              the IAU 1980 Nutation (Wahr) model.
!          Access Codes:
!            1. 'WAHRCONT'   - Data base access code for the contribution to
!                              convert to the IAU 1980 Nutation (Wahr) model.
!
! 1.2.5 EXTERNAL INPUT/OUTPUT - NONE
!
! 1.2.6 SUBROUTINE INTERFACE - NONE
!        Caller subroutine: DRIVR
!
! 1.2.7 CONSTANTS USED - NONE
!
! 1.2.8 PROGRAM VARIABLES
!         1. Wahr_cont(2) - Delay and rate contributions to convert back to
!                           the IAU 1980 Nutation (Wahr) model.
!
! 1.2.9 PROGRAMMER -
!         BRUCE SCHUPLER 11/2/77
!         D. Gordon 94.04.15 Converted to Implicit None.
!         D. Gordon 98.02.03  Documentation and code added to compute Wahr
!                   nutation contribution and place it in the data base.
!         D. Gordon 98.11.12  Computation and PUT of the delay and rate
!                   contributions for the Fukushima geodesic nutation in
!                   longitude correction, access code 'GDNUTCON'.
!         Jim Ryan 2002.09 Integer*4 conversion.
!         David Gordon 2003/2004 Mods for IERS Conventions 2003.
!
! 1.3   NUTC PROGRAM STRUCTURE
!
!      Skip if not using IAU2000 Nutation
         IF (KNUTC .ne. 0) Go to 999
!
!    Wahr delay contribution
       Wahr_cont(1) = Nutdif(1,1) * DNUpe(1,1) +
     .                Nutdif(2,1) * DNUpe(2,1)
!    Wahr rate contribution
       Wahr_cont(2) = Nutdif(1,1)*DNUpe(1,2) + Nutdif(1,2)*DNUpe(1,1)
     .              + Nutdif(2,1)*DNUpe(2,2) + Nutdif(2,2)*DNUpe(2,1)
!  Call PUT4 to place the Wahr contribution in the data base
         CALL PUT4 ('WAHRCONT      ',Wahr_cont,int2(2),int2(1),int2(1))
!
      If(KNUTD.EQ.0) GO TO 999
        WRITE ( 6, * ) ' Nutdif-psi: ', Nutdif(1,1), Nutdif(1,2)
        WRITE ( 6, * ) ' Nutdif-eps: ', Nutdif(2,1), Nutdif(2,2)
        WRITE ( 6, * ) ' Psi partials: ', DNUpe(1,1), DNUpe(1,2)
        WRITE ( 6, * ) ' Eps partials: ', DNUpe(2,1), DNUpe(2,2)
        WRITE ( 6, * ) ' Wahr_cont (sec): ', Wahr_cont
!
 999  Continue
      RETURN
      END
!***********************************************************************
      BLOCK DATA NUTCMB
      IMPLICIT None
!
! 7.    NUTBD
!
! 7.1   NUTBD PROGRAM SPECIFICATION
!
! 7.1.1 NUTBD IS THE NUTATION MODULE BLOCK DATA INITIALIZATION SECTION.
!       THE NUTATION SERIES IS ESTABLISHED HERE. THIS VERSION CONTAINS
!       THE 1980 IAU THEORY OF NUTATION, FROM THE WORK OF J. M. WAHR,
!       SPECIFICALLY, THE WAHR NUTATION SERIES FOR AXIS B OF GILBERT
!       DZIEWONSKI EARTH MODEL 1066A.
!
! 7.1.3 REFERENCES - 1) 'THE EXPLANATORY SUPPLEMENT TO THE AMERICAN
!                    EPHEMERIS AND NAUTICAL ALMANAC", P. 41-45, 98
!
!                    2) LIESKE, J.H., ET AL., EXPRESSIONS FOR THE
!                    PRECESSIONAL QUANTITIES BASED ON THE IAU (1976)
!                    SYSTEM OF ASTRONOMICAL CONSTANTS,
!                    ASTRON. ASTROPHYS. 58, 1-16, 1977.
!
!                    3) SEIDELMANN, P. K., 1980 IAU THEORY OF NUTATION:
!                    THE FINAL REPORT OF THE IAU WORKING GROUP ON
!                    NUTATION, CELEST. MECH. 27, PP. 79-106 (1982).
!
!                    4) WAHR, J. M., THE FORCED NUTATIONS OF ... EARTH,
!                    GEOPHYS. J. ROY. ASTR. SOC. 64, PP. 705-727 (1981).
!
! 7.2   NUTBD PROGRAM INTERFACE
!
! 7.2.1 CALLING SEQUENCE - NONE
!
! 7.2.2 COMMON BLOCK -
!
      REAL*8 X(9,120)
      COMMON / XWAHR / X
!
      Real*8 CENTJ, DJ2000, EC(4), ARGP(2,6)
      Integer*4 NOT, NOP, IDP(6)
      COMMON / NUTCM / CENTJ, DJ2000, EC, ARGP, NOT, NOP, IDP
!           VARIABLES 'TO':
!              1. CENTJ   - THE NUMBER OF COORDINATE TIME DAYS PER JULIAN
!                           CENTURY. (DAYS/CENTURY) (CENTJ = 36525.D0)
!              2. DJ2000  - THE JULIAN DATE OF THE EPOCH JANUARY 1.5, 2000.
!                           (DAYS) (DJ2000 = 2451545.D0)
!              3. EC(4)   - THE CONSTANTS APPEARING IN TERMS 1-4 IN THE
!                           CALCULATION OF THE MEAN OBLIQUITY OF THE ECLIPTIC.
!                           (ARCSEC) (SEE REFERENCES)
!                           ( EC(1) = +8.4381448D4, EC(2) = -46.815D0,
!                             EC(3) = -5.9D-4, EC(4) = +1.813D-3 )
!              4. NOT     - THE NUMBER OF TERMS IN THE NUTATION SERIES.
!                           (NOT = 106)
!              5. X(9,120)- THE ARRAY CONTAINING THE NUTATION SERIES.
!                           (X = 1980 IAU THEORY OF NUTATION)
!              6. NOP     - THE NUMBER OF NUTATION TERMS DESIGNATED FOR WHICH
!                           PARTIALS ARE TO BE COMPUTED. (NOP = 6)
!                                  (Obsolete?)
!              7. IDP(6)  - IDENTIFICATION NUMBERS (TERM NUMBERS) OF DESIGNATED
!                           NUTATION TERMS FOR WHICH PARTIALS ARE TO BE COMPUTED
!                           ( IDP(1) =  1, IDP(2) =  2, IDP(3) =  3,
!                             IDP(4) =  4, IDP(5) =  5, IDP(6) =  7 )
!              8. ARGP(2,6)-ARGUMENTS (COMBINATIONS OF FUNDAMENTAL ARGUMENTS)
!                           AND THEIR DERIVATIVES OF DESIGNATED NUTATION TERMS
!                           FOR WHICH PARTIALS ARE TO BE COMPUTED.
!                           (COMPUTED IN NUTW. SET TO 0.0D0 HERE)
!                                  (Obsolete?)
!
! 7.2.3 PROGRAM SPECIFICATIONS -
      Real*8  X1(180), X2(180), X3(180), X4(180), X5(180), X6(180)
      EQUIVALENCE (X(1,  1),X1(1))
      EQUIVALENCE (X(1, 21),X2(1))
      EQUIVALENCE (X(1, 41),X3(1))
      EQUIVALENCE (X(1, 61),X4(1))
      EQUIVALENCE (X(1, 81),X5(1))
      EQUIVALENCE (X(1,101),X6(1))
!
      DATA  CENTJ  / 36525.D0 /,
     .      DJ2000 / 2451545.D0 /,
     .      EC     / 8.4381448D4, -46.8150D0, -5.9D-4, 1.813D-3 /,
     .      NOT    / 106 /,
     .      NOP    / 6 /,
     .      IDP    / 1, 2, 3, 4, 5, 7 /,
     .      ARGP   / 12 * 0.0D0 /
!***********************************************************************
!
!               1980 IAU THEORY OF NUTATION (WAHR THEORY)
!           TABLE OF MULTIPLES OF ARGUMENTS AND COEFFICIENTS
!
!                   MULTIPLE OF            LONGITUDE        OBLIQUITY
!              L    L'   F    D  OMEGA   COEFF. OF SIN    COEFF. OF COS
      DATA X1/ 0.,  0.,  0.,  0.,  1., -171996., -174.2,  92025.,  8.9,
     .         0.,  0.,  2., -2.,  2.,  -13187.,   -1.6,   5736., -3.1,
     .         0.,  0.,  2.,  0.,  2.,   -2274.,   -0.2,    977., -0.5,
     .         0.,  0.,  0.,  0.,  2.,    2062.,    0.2,   -895.,  0.5,
     .         0.,  1.,  0.,  0.,  0.,    1426.,   -3.4,     54., -0.1,
     .         1.,  0.,  0.,  0.,  0.,     712.,    0.1,     -7.,  0.0,
     .         0.,  1.,  2., -2.,  2.,    -517.,    1.2,    224., -0.6,
     .         0.,  0.,  2.,  0.,  1.,    -386.,   -0.4,    200.,  0.0,
     .         1.,  0.,  2.,  0.,  2.,    -301.,    0.0,    129., -0.1,
     .         0., -1.,  2., -2.,  2.,     217.,   -0.5,    -95.,  0.3,
     .         1.,  0.,  0., -2.,  0.,    -158.,    0.0,     -1.,  0.0,
     .         0.,  0.,  2., -2.,  1.,     129.,    0.1,    -70.,  0.0,
     .        -1.,  0.,  2.,  0.,  2.,     123.,    0.0,    -53.,  0.0,
     .         1.,  0.,  0.,  0.,  1.,      63.,    0.1,    -33.,  0.0,
     .         0.,  0.,  0.,  2.,  0.,      63.,    0.0,     -2.,  0.0,
     .        -1.,  0.,  2.,  2.,  2.,     -59.,    0.0,     26.,  0.0,
     .        -1.,  0.,  0.,  0.,  1.,     -58.,   -0.1,     32.,  0.0,
     .         1.,  0.,  2.,  0.,  1.,     -51.,    0.0,     27.,  0.0,
     .         2.,  0.,  0., -2.,  0.,      48.,    0.0,      1.,  0.0,
     .        -2.,  0.,  2.,  0.,  1.,      46.,    0.0,    -24.,  0.0/
      DATA X2/ 0.,  0.,  2.,  2.,  2.,     -38.,    0.0,     16.,  0.0,
     .         2.,  0.,  2.,  0.,  2.,     -31.,    0.0,     13.,  0.0,
     .         2.,  0.,  0.,  0.,  0.,      29.,    0.0,     -1.,  0.0,
     .         1.,  0.,  2., -2.,  2.,      29.,    0.0,    -12.,  0.0,
     .         0.,  0.,  2.,  0.,  0.,      26.,    0.0,     -1.,  0.0,
     .         0.,  0.,  2., -2.,  0.,     -22.,    0.0,      0.,  0.0,
     .        -1.,  0.,  2.,  0.,  1.,      21.,    0.0,    -10.,  0.0,
     .         0.,  2.,  0.,  0.,  0.,      17.,   -0.1,      0.,  0.0,
     .         0.,  2.,  2., -2.,  2.,     -16.,    0.1,      7.,  0.0,
     .        -1.,  0.,  0.,  2.,  1.,      16.,    0.0,     -8.,  0.0,
     .         0.,  1.,  0.,  0.,  1.,     -15.,    0.0,      9.,  0.0,
     .         1.,  0.,  0., -2.,  1.,     -13.,    0.0,      7.,  0.0,
     .         0., -1.,  0.,  0.,  1.,     -12.,    0.0,      6.,  0.0,
     .         2.,  0., -2.,  0.,  0.,      11.,    0.0,      0.,  0.0,
     .        -1.,  0.,  2.,  2.,  1.,     -10.,    0.0,      5.,  0.0,
     .         1.,  0.,  2.,  2.,  2.,      -8.,    0.0,      3.,  0.0,
     .         0., -1.,  2.,  0.,  2.,      -7.,    0.0,      3.,  0.0,
     .         0.,  0.,  2.,  2.,  1.,      -7.,    0.0,      3.,  0.0,
     .         1.,  1.,  0., -2.,  0.,      -7.,    0.0,      0.,  0.0,
     .         0.,  1.,  2.,  0.,  2.,       7.,    0.0,     -3.,  0.0/
      DATA X3/-2.,  0.,  0.,  2.,  1.,      -6.,    0.0,      3.,  0.0,
     .         0.,  0.,  0.,  2.,  1.,      -6.,    0.0,      3.,  0.0,
     .         2.,  0.,  2., -2.,  2.,       6.,    0.0,     -3.,  0.0,
     .         1.,  0.,  0.,  2.,  0.,       6.,    0.0,      0.,  0.0,
     .         1.,  0.,  2., -2.,  1.,       6.,    0.0,     -3.,  0.0,
     .         0.,  0.,  0., -2.,  1.,      -5.,    0.0,      3.,  0.0,
     .         0., -1.,  2., -2.,  1.,      -5.,    0.0,      3.,  0.0,
     .         2.,  0.,  2.,  0.,  1.,      -5.,    0.0,      3.,  0.0,
     .         1., -1.,  0.,  0.,  0.,       5.,    0.0,      0.,  0.0,
     .         1.,  0.,  0., -1.,  0.,      -4.,    0.0,      0.,  0.0,
     .         0.,  0.,  0.,  1.,  0.,      -4.,    0.0,      0.,  0.0,
     .         0.,  1.,  0., -2.,  0.,      -4.,    0.0,      0.,  0.0,
     .         1.,  0., -2.,  0.,  0.,       4.,    0.0,      0.,  0.0,
     .         2.,  0.,  0., -2.,  1.,       4.,    0.0,     -2.,  0.0,
     .         0.,  1.,  2., -2.,  1.,       4.,    0.0,     -2.,  0.0,
     .         1.,  1.,  0.,  0.,  0.,      -3.,    0.0,      0.,  0.0,
     .         1., -1.,  0., -1.,  0.,      -3.,    0.0,      0.,  0.0,
     .        -1., -1.,  2.,  2.,  2.,      -3.,    0.0,      1.,  0.0,
     .         0., -1.,  2.,  2.,  2.,      -3.,    0.0,      1.,  0.0,
     .         1., -1.,  2.,  0.,  2.,      -3.,    0.0,      1.,  0.0/
      DATA X4/ 3.,  0.,  2.,  0.,  2.,      -3.,    0.0,      1.,  0.0,
     .        -2.,  0.,  2.,  0.,  2.,      -3.,    0.0,      1.,  0.0,
     .         1.,  0.,  2.,  0.,  0.,       3.,    0.0,      0.,  0.0,
     .        -1.,  0.,  2.,  4.,  2.,      -2.,    0.0,      1.,  0.0,
     .         1.,  0.,  0.,  0.,  2.,      -2.,    0.0,      1.,  0.0,
     .        -1.,  0.,  2., -2.,  1.,      -2.,    0.0,      1.,  0.0,
     .         0., -2.,  2., -2.,  1.,      -2.,    0.0,      1.,  0.0,
     .        -2.,  0.,  0.,  0.,  1.,      -2.,    0.0,      1.,  0.0,
     .         2.,  0.,  0.,  0.,  1.,       2.,    0.0,     -1.,  0.0,
     .         3.,  0.,  0.,  0.,  0.,       2.,    0.0,      0.,  0.0,
     .         1.,  1.,  2.,  0.,  2.,       2.,    0.0,     -1.,  0.0,
     .         0.,  0.,  2.,  1.,  2.,       2.,    0.0,     -1.,  0.0,
     .         1.,  0.,  0.,  2.,  1.,      -1.,    0.0,      0.,  0.0,
     .         1.,  0.,  2.,  2.,  1.,      -1.,    0.0,      1.,  0.0,
     .         1.,  1.,  0., -2.,  1.,      -1.,    0.0,      0.,  0.0,
     .         0.,  1.,  0.,  2.,  0.,      -1.,    0.0,      0.,  0.0,
     .         0.,  1.,  2., -2.,  0.,      -1.,    0.0,      0.,  0.0,
     .         0.,  1., -2.,  2.,  0.,      -1.,    0.0,      0.,  0.0,
     .         1.,  0., -2.,  2.,  0.,      -1.,    0.0,      0.,  0.0,
     .         1.,  0., -2., -2.,  0.,      -1.,    0.0,      0.,  0.0/
      DATA X5/ 1.,  0.,  2., -2.,  0.,      -1.,    0.0,      0.,  0.0,
     .         1.,  0.,  0., -4.,  0.,      -1.,    0.0,      0.,  0.0,
     .         2.,  0.,  0., -4.,  0.,      -1.,    0.0,      0.,  0.0,
     .         0.,  0.,  2.,  4.,  2.,      -1.,    0.0,      0.,  0.0,
     .         0.,  0.,  2., -1.,  2.,      -1.,    0.0,      0.,  0.0,
     .        -2.,  0.,  2.,  4.,  2.,      -1.,    0.0,      1.,  0.0,
     .         2.,  0.,  2.,  2.,  2.,      -1.,    0.0,      0.,  0.0,
     .         0., -1.,  2.,  0.,  1.,      -1.,    0.0,      0.,  0.0,
     .         0.,  0., -2.,  0.,  1.,      -1.,    0.0,      0.,  0.0,
     .         0.,  0.,  4., -2.,  2.,       1.,    0.0,      0.,  0.0,
     .         0.,  1.,  0.,  0.,  2.,       1.,    0.0,      0.,  0.0,
     .         1.,  1.,  2., -2.,  2.,       1.,    0.0,     -1.,  0.0,
     .         3.,  0.,  2., -2.,  2.,       1.,    0.0,      0.,  0.0,
     .        -2.,  0.,  2.,  2.,  2.,       1.,    0.0,     -1.,  0.0,
     .        -1.,  0.,  0.,  0.,  2.,       1.,    0.0,     -1.,  0.0,
     .         0.,  0., -2.,  2.,  1.,       1.,    0.0,      0.,  0.0,
     .         0.,  1.,  2.,  0.,  1.,       1.,    0.0,      0.,  0.0,
     .        -1.,  0.,  4.,  0.,  2.,       1.,    0.0,      0.,  0.0,
     .         2.,  1.,  0., -2.,  0.,       1.,    0.0,      0.,  0.0,
     .         2.,  0.,  0.,  2.,  0.,       1.,    0.0,      0.,  0.0/
      DATA X6/ 2.,  0.,  2., -2.,  1.,       1.,    0.0,     -1.,  0.0,
     .         2.,  0., -2.,  0.,  1.,       1.,    0.0,      0.,  0.0,
     .         1., -1.,  0., -2.,  0.,       1.,    0.0,      0.,  0.0,
     .        -1.,  0.,  0.,  1.,  1.,       1.,    0.0,      0.,  0.0,
     .        -1., -1.,  0.,  2.,  1.,       1.,    0.0,      0.,  0.0,
     .         0.,  1.,  0.,  1.,  0.,       1.,    0.0,      0.,  0.0,
     .                      126 *  0./
!
!***********************************************************************
!
! 7.2.4 CONSTANTS USED - CENTJ, DJ2000, EC(4), NOT, X(9,120),
!                    NOP, IDP(6), ARGP(2,6)
!
! 7.2.5 PROGRAMMER - DALE MARKHAM   01/13/77
!                    PETER DENATALE 07/12/77
!                    BRUCE SCHUPLER 12/22/77
!                    CHOPO MA       08/04/81
!                    GEORGE KAPLAN  04/24/84
!                    David Gordon   94.04.15 Converted to Implicit None.
!                    David Gordon   95.09.27 X(9,120) table changed from Real*4
!                                   to Real*8
!                    David Gordon   98.02.03 Removed X(9,120) from COMMON
!                                   /NUTCM/ and put it into COMMON /XWAHR/,
!                                   and removed it from most subroutines.
!
! 7.3   NUTBD PROGRAM STRUCTURE - NONE
!
      END
!
!**********************************************************************
      SUBROUTINE NUTW (CENT, FA, FAD, DPSI, DEPS)
      IMPLICIT None
!
! 1.    NUTW
!
! 1.1   NUTW PROGRAM SPECIFICATION
!
! 1.1.1 THIS SUBROUTINE EVALUATES THE NUTATION SERIES AND RETURNS THE
!       VALUES FOR NUTATION IN LONGITUDE AND NUTATION IN OBLIQUITY.
!       The fundamental arguments and derivatives are calculated
!       in NUTFA to be called prior to NUTW.
!
! 1.1.2 RESTRICTIONS - NONE
!
! 1.1.3 REFERENCES - 1) KAPLAN, G. H. (ED.), THE IAU RESOLUTIONS ... ,
!                       USNO CIRCULAR NO. 163, U. S. NAVAL OBSERV'Y (1981).
!
! 1.2   NUTW PROGRAM INTERFACE
!
! 1.2.1 CALLING SEQUENCE - CALL NUTW(CENT, fa, fad, DPSI, DEPS)
!    INPUT VARIABLES:
!      1. CENT   - The number of Julian centuries between the observation
!                  epoch and January 1.5, 2000. (centuries)
!      2. FA(5)  - Fundamental arguments. (arcseconds) (see NUTFA)
!      3. FAD(5) - The CT derivatives of the fundamental arguments. (see NUTFA)
!    OUTPUT VARIABLES:
!      1. DPSI(2) - THE NUTATION IN LONGITUDE AND ITS CT TIME DERIVATIVE
!                   (ARCSEC,ARCSEC/SEC)
!      2. DEPS(2) - THE NUTATION IN OBLIQUITY AND ITS CT TIME DERIVATIVE
!                   (ARCSEC,ARCSEC/SEC)
!
! 1.2.2 COMMON BLOCKS USED -
!
      Real*8           PI, TWOPI, HALFPI, CONVD, CONVDS, CONVHS, SECDAY
      COMMON / CMATH / PI, TWOPI, HALFPI, CONVD, CONVDS, CONVHS, SECDAY
!     VARIABLES 'FROM':
!       1. CONVDS - THE CONVERSION FACTOR FROM ARCSECONDS TO RADIANS.
!       2. SECDAY - THE NUMBER OF COORDINATE TIME SECONDS PER COORDINATE
!                   TIME DAY. (SECONDS/DAY)
!
      INCLUDE 'ccon.i'
!     VARIABLES 'FROM':
!       1. KNUTD - THE NUTATION MODULE DEBUG CONTROL FLAG
!
      REAL*8 fa(5), fad(5)
      Real*8 CENTJ, DJ2000, EC(4), ARGP(2,6)
      Integer*4 NOT, NOP, IDP(6)
      COMMON / NUTCM / CENTJ, DJ2000, EC, ARGP, NOT, NOP, IDP
      REAL*8 X(9,120)
      COMMON / XWAHR / X
!
!     VARIABLES 'FROM':
!        1. CENTJ - THE NUMBER OF COORDINATE TIME DAYS PER JULIAN CENTURY.
!                   (DAYS/CENTURY)
!        2. NOT   - THE NUMBER OF TERMS IN THE NUTATION SERIES.
!        3. X(9,120)-ARRAY CONTAINING THE NUTATION SERIES.
!        4. NOP   - THE NUMBER OF NUTATION TERMS DESIGNATED FOR WHICH
!                   PARTIALS ARE TO BE COMPUTED. (NOP .LE. 6)
!                       (obsolete?)
!        5. IDP(6)- IDENTIFICATION NUMBERS (TERM NUMBERS) OF DESIGNATED
!                   NUTATION TERMS FOR WHICH PARTIALS ARE TO BE COMPUTED.
!
!     VARIABLES 'TO':
!        1. ARGP(2,6) - ARGUMENTS (COMBINATIONS OF FUNDAMENTAL ARGUMENTS)
!                       AND THEIR DERIVATIVES OF DESIGNATED NUTATION TERMS
!                       FOR WHICH PARTIALS ARE TO BE COMPUTED.
!                           (obsolete?)
!
! 1.2.3 PROGRAM SPECIFICATIONS
!
      Real*8 DPSI(2),DEPS(2), CENT, SEC360, ARG, ARGDOT
      REAL*8 EL, ELP, F, D, OM, ELC(5), ELPC(5), FC(5),
     .       DC(5), OMC(5), CENT2, CENT3
      Integer*4 I, J, N
!
!  Code added from Calc 8!!!
!     Constants used -
!       ELC(5)   - COEFFICIENTS USED IN THE CALCULATION OF EL
!       ELPC(5)  - COEFFICIENTS USED IN THE CALCULATION OF ELP
!       FC(5)    - COEFFICIENTS USED IN THE CALCULATION OF F
!       DC(5)    - COEFFICIENTS USED IN THE CALCULATION OF D
!       OMC(5)   - COEFFICIENTS USED IN THE CALCULATION OF OM
!
!       COEFFICIENTS ARE IN THE ORDER CUBIC, QUDRATIC, LINEAR, CONSTANT,
!       AND INTEGRAL NUMBER OF TURNS.
!
      DATA ELC /0.064D0,31.310D0,715922.633D0,485866.733D0,1325.0D0/
      DATA ELPC /-0.012D0,-0.577D0,1292581.224D0,1287099.804D00,99.0D0/
      DATA FC /0.011D0,-13.257D0,295263.137D0,335778.877D0,1342.0D0/
      DATA DC /0.019D0,-6.891D0,1105601.328D0,1072261.307D0,1236.0D0/
      DATA OMC /0.008D0,7.455D0,-482890.539D0,450160.280D0,-5.0D0/
!
!   SEC360 = ARCSECONDS IN ONE TURN
      DATA SEC360 / 1296000.D0 /
!
!
! 1.2.4 DATA BASE ACCESS - NONE
!
! 1.2.5 EXTERNAL INPUT/OUTPUT - POSSIBLE DEBUG
!
! 1.2.6 SUBROUTINE INTERFACE
!       CALLER SUBROUTINES - NUTG
!       CALLED SUBROUTINES - DSIN, DCOS, DMOD
!
!
! 1.2.8 PROGRAM VARIABLES
!      1. ARG    - THE COMBINATION OF FUNDAMENTAL ARGUMENTS USED
!                  TO COMPUTE THE NUTATION (RADIANS)
!      2. ARGDOT - THE DERIVATIVE OF ARG (RADIANS/CENTURY)
!
! 1.2.9 PROGRAMMER:
!          810804  CHOPO MA
!          840000  GEORGE KAPLAN
!          930901  Norbert Zacharias: put fundamental arguments computation
!                  into subroutine NUTFA.
!          David Gordon 94.04.15 Converted to Implicit None.
!          David Gordon 95.09.27 Conversion of nutation series coefficients to
!                  double precision removed - no longer necessary.
!          Jim Ryan 2002.09 Integer*4 conversion.
!          David Gordon 2004.07.30 Adding computation of fundamental
!                  arguments consistent with Wahr nutation, taken from
!                  Calc 8 and IERS Standards (1992).
!
!  Code added to compute fundamental arguments consistent with Wahr nutation,
!   taken from Calc 8.
      CENT2 = CENT * CENT
      CENT3 = CENT * CENT2
!
!  Computation of the fundamental arguments and derivatives
!
      EL = ELC(1) * CENT3 + ELC(2) * CENT2 + ELC(3) * CENT
     .     + ELC(4) + DMOD( ELC(5) * CENT, 1.D0 ) * SEC360
      FA (1) = DMOD( EL, SEC360 )
      FAD(1) = 3.D0 * ELC(1) * CENT2 + 2.D0 * ELC(2) * CENT
     .         + ELC(3) + ELC(5) * SEC360
!
      ELP = ELPC(1) * CENT3 + ELPC(2) * CENT2 + ELPC(3) * CENT
     .     + ELPC(4) + DMOD( ELPC(5) * CENT, 1.D0 ) * SEC360
      FA (2) = DMOD( ELP, SEC360 )
      FAD(2) = 3.D0 * ELPC(1) * CENT2 + 2.D0 * ELPC(2) * CENT
     .         + ELPC(3) + ELPC(5) * SEC360
!
      F = FC(1) * CENT3 + FC(2) * CENT2 + FC(3) * CENT
     .     + FC(4) + DMOD( FC(5) * CENT, 1.D0 ) * SEC360
      FA (3) = DMOD( F, SEC360 )
      FAD(3) = 3.D0 * FC(1) * CENT2 + 2.D0 * FC(2) * CENT
     .         + FC (3) + FC(5) * SEC360
!
      D = DC(1) * CENT3 + DC(2) * CENT2 + DC(3) * CENT
     .     + DC(4) + DMOD( DC(5) * CENT, 1.D0 ) * SEC360
      FA (4) = DMOD( D, SEC360 )
      FAD(4) = 3.D0 * DC(1) * CENT2 + 2.D0 * DC(2) * CENT
     .         + DC(3) + DC(5) * SEC360
!
      OM = OMC(1) * CENT3 + OMC(2) * CENT2 + OMC(3) * CENT
     .     + OMC(4) + DMOD( OMC(5) * CENT, 1.D0 ) * SEC360
      FA (5) = DMOD( OM, SEC360 )
      FAD(5) = 3.D0 * OMC(1) * CENT2 + 2.D0 * OMC(2) * CENT
     .         + OMC(3) + OMC(5) * SEC360
!
!  INITIALIZE OUTPUT
      DPSI(1) = 0.0D0
      DPSI(2) = 0.0D0
      DEPS(1) = 0.0D0
      DEPS(2) = 0.0D0
!
!  SUM NUTATION SERIES TERMS, FROM SMALLEST TO LARGEST
!
      N = NOP
!
      DO J=1,NOT
        I = NOT + 1 - J
!
!   FORMATION OF MULTIPLES OF ARGUMENTS
!
        ARG = X(1,I) * FA(1)                    ! EL    arcseconds
     .      + X(2,I) * FA(2)                     ! ELP
     .      + X(3,I) * FA(3)                     ! F
     .      + X(4,I) * FA(4)                     ! D
     .      + X(5,I) * FA(5)                  ! OM
        ARG = DMOD(ARG,SEC360) * CONVDS     ! radian
!
!   FORMATION OF MULTIPLES FOR DERIVATIVES
!
        ARGDOT = X(1,I) * FAD(1)     ! ELD
     .         + X(2,I) * FAD(2)     ! ELPD
     .         + X(3,I) * FAD(3)     ! FD
     .         + X(4,I) * FAD(4)     ! DD
     .         + X(5,I) * FAD(5)  ! OMD
        ARGDOT = ARGDOT * CONVDS
!
!   STORE VALUES OF ARGUMENTS AND DERIVATIVES OF SPECIFIC TERMS
!
        IF (N.GT.0) THEN
          IF (IDP(N).EQ.I) THEN
            ARGP(1,N) = ARG
            ARGP(2,N) = ARGDOT
            N = N - 1
          ENDIF
        ENDIF
!
!   EVALUATE NUTATION AND DERIVATIVES OF NUTATION
!
        DPSI(1) = (X(6,I) + X(7,I)*CENT) * DSIN(ARG)
     .          + DPSI(1)
        DPSI(2) = DPSI(2) + X(7,I) * DSIN(ARG) + (X(6,I)
     .          + X(7,I) * CENT) * ARGDOT * DCOS(ARG)
        DEPS(1) = (X(8,I) + X(9,I)*CENT) * DCOS(ARG)
     .          + DEPS(1)
        DEPS(2) = DEPS(2) + X(9,I) * DCOS(ARG) - (X(8,I)
     .          + X(9,I) * CENT) * ARGDOT * DSIN(ARG)
!
      ENDDO  ! j=1,not
!
!   CONVERT TO PROPER UNITS
!
      DPSI(1) = DPSI(1) * 1.0D-4
      DPSI(2) = DPSI(2) * 1.0D-4 / (CENTJ * SECDAY)
      DEPS(1) = DEPS(1) * 1.0D-4
      DEPS(2) = DEPS(2) * 1.0D-4 / (CENTJ * SECDAY)
!
! SEE IF WE NEED DEBUG OUTPUT
      IF (KNUTD .NE. 0) THEN
        WRITE (6,9)
    9   FORMAT (1X,'Debug output for subroutine NUTW.')
    8   FORMAT(A,4D25.16/(7X,5D25.16))
        WRITE(6,8)' SEC360  ',SEC360
        WRITE(6,8)' ARG     ',ARG
        WRITE(6,8)' ARGDOT  ',ARGDOT
        WRITE (6,9200) CONVDS, CENTJ, SECDAY, CENT, DEPS, DPSI
9200    FORMAT (1X,'CONVDS = ',D25.16,/,1X,'CENTJ = ',D25.16,/,1X,
     .       'SECDAY = ',D25.16,/,1X,'CENT = ',D25.16,/,1X,
     .       'DEPS = ',2(D25.16,2X),/,1X,'DPSI = ',2(D25.16,2X), /)
      ENDIF
!
      RETURN
      END
!***************************************************************
      SUBROUTINE NUTFA (XJD, CT, CENT, FA2K, FAD2K)
      IMPLICIT NONE
!
!  NUTFA computes the number of Julian centuries since J2000 and the fundamental
!  arguments and derivatives to be used in the nutation series.
!
!  References: D.McCarthy, IERS Technical Note 32, IERS Conventions (2003).
!
!  Calling sequence:
!    input:
!           1. XJD  -  THE JULIAN DATE AT ZERO HOURS UTC OF THE DATE IN
!                      QUESTION. (DAYS)
!           2. CT   -  THE COORDINATE TIME FRACTION OF THE COORDINATE TIME DAY.
!                      (DAYS)
!    output:
!           1. CENT -  The number of Julian centuries elapsed since the epoch
!                      January 1.5 2000.(centuries)
!           4. FA2K(14)- The fundamental arguments for the nutation theory,
!                      updated for the IERS Conventions 2003.  (radians)
!               1 = mean anomaly of the moon
!                 = mean longitude of the moon minus the
!                   mean longitude of the moon's perigee     (l)
!               2 = mean anomaly of the sun
!                 = mean longitude of the sun minus the
!                   mean longitude of the sun's perigee      (l')
!               3 = mean longitude of the moon minus omega   (F)
!               4 = mean elongation of the moon from the sun (D)
!               5 = longitude of the asc.node of the moon's
!                   mean orbit on the ecliptic,
!                   measured from the mean equinox of date   (omega)
!               6-13 = Planetary longitudes, Mercury through Neptune.
!               14  = General accumulated precession in longitude.
!           3. FAD2K(14)- The CT time derivatives of the fundamental arguments.
!                      (radians/second)
!
      REAL*8 XJD, CT, CENT, EL, ELP, F, D, OM, SEC360, CENT2, CENT3,
     .       CENT4, DAYSJ, ELC2(5), ELPC2(5), FC2(5),
     .       DC2(5), OMC2(5), dTdt
      REAL*8 FA2K(14), FAD2K(14)
!
      Real*8 CENTJ, DJ2000, EC(4), ARGP(2,6)
      Integer*4 NOT, NOP, IDP(6)
      COMMON / NUTCM / CENTJ, DJ2000, EC, ARGP, NOT, NOP, IDP
!     Variables from:
!        1. CENTJ   -  The number of coordinate time days per Julian century.
!                      (days/century)
!        2. DJ2000  -  The Julian date of the epoch January 1.5, 2000. (days)
!
!
      Real*8           PI, TWOPI, HALFPI, CONVD, CONVDS, CONVHS, SECDAY
      COMMON / CMATH / PI, TWOPI, HALFPI, CONVD, CONVDS, CONVHS, SECDAY
!           VARIABLES 'FROM':
!              1. TWOPI   -  TWOPI
!              2. SECDAY  -  THE NUMBER OF COORDINATE TIME SECONDS PER
!                            COORDINATE TIME DAY. (SEC/DAY)
!
      INCLUDE 'ccon.i'
!     Variables from:
!        1. KNUTD  -  nutation module debug flag
!
!     Subroutine interface:
!       Caller subroutines: DRIVR, UT1I
!       Called subroutines: DMOD
!
!     Constants used -
!       ELC(5)   - COEFFICIENTS USED IN THE CALCULATION OF EL
!       ELPC(5)  - COEFFICIENTS USED IN THE CALCULATION OF ELP
!       FC(5)    - COEFFICIENTS USED IN THE CALCULATION OF F
!       DC(5)    - COEFFICIENTS USED IN THE CALCULATION OF D
!       OMC(5)   - COEFFICIENTS USED IN THE CALCULATION OF OM
!
!     DATA statements for the fundamental arguments.
!     Simons et al., 1994 values
!      -Conform to IERS Conventions (1996)-
!xx   DATA ELC    / -0.00024470d0,       0.051635d0,  31.8792d0,
!xx  .          1717915923.2178d0,  485868.249036d0/
!xx   DATA ELPC   / -0.00001149d0,      -0.000136d0,  -0.5532d0,
!xx  .           129596581.0481d0,  1287104.79305d0/
!xx   DATA FC     /  0.00000417d0,      -0.001037d0, -12.7512d0,
!xx  .          1739527262.8478d0,  335779.526232d0/
!xx   DATA DC     / -0.00003169d0,       0.006593d0,  -6.3706d0,
!xx  .          1602961601.2090d0,  1072260.70369d0/
!xx   DATA OMC    /-0.00005939d0,        0.007702d0,   7.4722d0,
!xx  .           -6962890.2665d0,   450160.398036d0/
!
      DATA SEC360 / 1296000.0D0 /       ! arcseconds in one turn
!
!     DATA statements for the revised fundamental arguments.
!     Simons et al., 1994 values
!      -Conform to IERS Conventions (2003)-
      DATA ELC2   / -0.00024470d0,       0.051635d0,  31.8792d0,
     .          1717915923.2178d0,  485868.249036d0/
      DATA ELPC2  / -0.00001149d0,      +0.000136d0,  -0.5532d0,
     .           129596581.0481d0,  1287104.793048d0/
      DATA FC2    /  0.00000417d0,      -0.001037d0, -12.7512d0,
     .          1739527262.8478d0,  335779.526232d0/
      DATA DC2    / -0.00003169d0,       0.006593d0,  -6.3706d0,
     .          1602961601.2090d0,  1072260.703692d0/
      DATA OMC2   /-0.00005939d0,        0.007702d0,   7.4722d0,
     .           -6962890.5431d0,   450160.398036d0/
!
!  Programmer:
!    93.09.01  Norbert Zacharias - Fundamental arguments computation put into
!              separate subroutine, taken from old NUTG subroutine.
!    98.01.28  David Gordon - Coefficients and computations modified to conform
!              to IERS Conventions (1996).
!                    Jim Ryan 2002.09 Integer*4 conversion.
!    2003.     David Gordon - Revised for IERS Conventions (2003)
!
!-------------------------------------------------------------------------------
!  Compute the number of Julian days elapsed since the epoch January 1.5, 2000.
      DAYSJ = XJD + CT - DJ2000
!  Derivative of CENT w.r.t. time (centuries/sec)
      dTdt = 1.D0/(CENTJ*86400.D0)
!
!  Compute the number of Julian centuries elapsed since the epoch January 1.5,
!   2000.
      CENT  = ((XJD - DJ2000) + CT) / CENTJ
      CENT2 = CENT * CENT
      CENT3 = CENT * CENT2
      CENT4 = CENT2 * CENT2
!
!-----------------------------------------------------------
!
!  Revised computation of the fundamental arguments and derivatives.
!   IERS Conventions (2003)
!
      EL = ELC2(1)*CENT4 + ELC2(2)*CENT3 + ELC2(3)*CENT2
     .   + ELC2(4)*CENT  + ELC2(5)
      FA2K(1)  = DMOD( EL, SEC360 )
      FAD2K(1) = 4.D0*ELC2(1)*CENT3 + 3.D0*ELC2(2)*CENT2
     .       + 2.D0*ELC2(3)*CENT  +      ELC2(4)
!
      ELP = ELPC2(1)*CENT4 + ELPC2(2)*CENT3 + ELPC2(3)*CENT2
     .    + ELPC2(4)*CENT  + ELPC2(5)
      FA2K (2) = DMOD( ELP, SEC360 )
      FAD2K(2) = 4.D0*ELPC2(1)*CENT3 + 3.D0*ELPC2(2)*CENT2
     .       + 2.D0*ELPC2(3)*CENT  +      ELPC2(4)
!
      F = FC2(1)*CENT4 + FC2(2)*CENT3 + FC2(3)*CENT2
     .  + FC2(4)*CENT  + FC2(5)
      FA2K (3) = DMOD( F, SEC360 )
      FAD2K(3) = 4.D0*FC2(1)*CENT3 + 3.D0*FC2(2)*CENT2
     .       + 2.D0*FC2(3)*CENT  +      FC2(4)
!
      D = DC2(1)*CENT4 + DC2(2)*CENT3 + DC2(3)*CENT2
     .  + DC2(4)*CENT  + DC2(5)
      FA2K (4) = DMOD( D, SEC360 )
      FAD2K(4) = 4.D0*DC2(1)*CENT3 + 3.D0*DC2(2)*CENT2
     .       + 2.D0*DC2(3)*CENT  +      DC2(4)
!
      OM = OMC2(1)*CENT4 + OMC2(2)*CENT3 + OMC2(3)*CENT2
     .   + OMC2(4)*CENT  + OMC2(5)
      FA2K (5) = DMOD( OM, SEC360 )
      FAD2K(5) = 4.D0*OMC2(1)*CENT3 + 3.D0*OMC2(2)*CENT2
     .       + 2.D0*OMC2(3)*CENT  +      OMC2(4)
!  Convert to radians and radians/sec:
      FA2K(1)  =  FA2K(1) * CONVDS
      FA2K(2)  =  FA2K(2) * CONVDS
      FA2K(3)  =  FA2K(3) * CONVDS
      FA2K(4)  =  FA2K(4) * CONVDS
      FA2K(5)  =  FA2K(5) * CONVDS
      FAD2K(1)  =  FAD2K(1) * CONVDS/(SECDAY*CENTJ)
      FAD2K(2)  =  FAD2K(2) * CONVDS/(SECDAY*CENTJ)
      FAD2K(3)  =  FAD2K(3) * CONVDS/(SECDAY*CENTJ)
      FAD2K(4)  =  FAD2K(4) * CONVDS/(SECDAY*CENTJ)
      FAD2K(5)  =  FAD2K(5) * CONVDS/(SECDAY*CENTJ)
!
!  Planetary longitudes, Mercury through Neptune (Souchay et al. 1999).
      FA2K( 6) = ( 4.402608842D0 + 2608.7903141574D0 * CENT )
        FA2K(6) = DMOD(FA2K(6),TWOPI)
      FA2K( 7) = ( 3.176146697D0 + 1021.3285546211D0 * CENT )
        FA2K(7) = DMOD(FA2K(7),TWOPI)
      FA2K( 8) = ( 1.753470314D0 +  628.3075849991D0 * CENT )
        FA2K(8) = DMOD(FA2K(8),TWOPI)
      FA2K( 9) = ( 6.203480913D0 +  334.0612426700D0 * CENT )
        FA2K(9) = DMOD(FA2K(9),TWOPI)
      FA2K(10) = ( 0.599546497D0 +   52.9690962641D0 * CENT )
        FA2K(10) = DMOD(FA2K(10),TWOPI)
      FA2K(11) = ( 0.874016757D0 +   21.3299104960D0 * CENT )
        FA2K(11) = DMOD(FA2K(11),TWOPI)
      FA2K(12) = ( 5.481293872D0 +    7.4781598567D0 * CENT )
        FA2K(12) = DMOD(FA2K(12),TWOPI)
      FA2K(13) = ( 5.311886287D0 +    3.8133035638D0 * CENT )
        FA2K(13) = DMOD(FA2K(13),TWOPI)
!  General accumulated precession in longitude.
      FA2K(14) = ( 0.024381750D0*CENT + 0.00000538691D0*CENT2)
!
      FAD2K( 6) = 2608.7903141574D0 * dTdt
      FAD2K( 7) = 1021.3285546211D0 * dTdt
      FAD2K( 8) = 628.3075849991D0 * dTdt
      FAD2K( 9) = 334.0612426700D0 * dTdt
      FAD2K(10) = 52.9690962641D0 * dTdt
      FAD2K(11) = 21.3299104960D0 * dTdt
      FAD2K(12) = 7.4781598567D0 * dTdt
      FAD2K(13) = 3.8133035638D0 * dTdt
      FAD2K(14) = (0.024381750D0 + 2.D0*0.00000538691D0*CENT) * dTdt
!
!  Debug output
      IF (knutd.NE.0) THEN
        WRITE (6,'(1x,a)') 'Debug output for subroutine NUTFA'
       WRITE (6,8) ' CENT,  ', cent
    8         FORMAT(A,4D25.16/(7X,5D25.16))
      Write (6,787) FA2K
 787  Format(' FA2K:',/,3(5E22.15,/))
      Write (6,788) FAD2K
 788  Format(' FAD2K:',/,3(5E22.15,/))
!
!     Write (6,778) FAD(1)*CONVDS/(SECDAY*CENTJ),
!    *   FAD(2)*CONVDS/(SECDAY*CENTJ),FAD(3)*CONVDS/(SECDAY*CENTJ),
!    *   FAD(4)*CONVDS/(SECDAY*CENTJ),FAD(5)*CONVDS/(SECDAY*CENTJ)
!778  Format(' NUTFA: FAD (rad/sec):',/, (5E22.15,/))
      ENDIF
!
      RETURN
      END
!*************************************************************************
      SUBROUTINE BPN2K ( X, Y, S, RBPN, dRBPN )
!
!  CEO-based bias-precession-nutation matrix.
!  Annexe to IERS Conventions 2000, Chapter 5
!
!  Given:
!     X(2),Y(2)        d      CIP coordinates and time derivatives
!     S(2)             d      the quantity s and time derivatives
!                             (radians)
!  Returned:
!     RBPN        d(3,3)   intermediate-to-celestial matrix ("Q")
!
!  Modified SOFA subroutine for use in Calc. Added time derivative
!  of bias-precession-nutation matrix.
!
!-----------------------------------------------------------------------
      IMPLICIT NONE
      DOUBLE PRECISION X(2), Y(2), S(2), RBPN(3,3)
      DOUBLE PRECISION X2, Y2, R2, R, Z, A, AXY, RR(3,3), RL(3,3)
      DOUBLE PRECISION dX2, dY2, dR2, dZ, dA, dAXY,dRR(3,3),dRL(3,3),
     .                 RR1(3,3), RR2(3,3), dRBPN(3,3)
      Integer*2 Int1,Int2,Int3
! - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
         Int1 = 1
         Int2 = 2
         Int3 = 3
!
!  Prepare to evaluate expression (10).
      X2 = X(1)*X(1)
       dX2 = 2.D0*X(1)*X(2)
!       print *,' X2,dX2: ', X2,dX2
      Y2 = Y(1)*Y(1)
       dY2 = 2.D0*Y(1)*Y(2)
!       print *,' Y2,dY2: ', Y2,dY2
      R2 = X2 + Y2
       dR2 = dX2 + dY2
!       print *,' R2,dR2: ', R2,dR2
      R = DSQRT (R2)
      Z = DSQRT (1.D0 - R2)
       dZ = -dR2/(2.D0*DSQRT(1D0 - R2))
!       print *,'  Z, dZ: ',  Z, dZ
      A = 1D0 / ( 1.D0 + Z )
       dA = -dZ/(1.D0 + Z)**2
!       print *,'  A, dA: ',  A, dA
      AXY = A*X(1)*Y(1)
       dAXY = dA*X(1)*Y(1) + A*X(2)*Y(1) + A*X(1)*Y(2)
!       print *,' AXY,dAXY: ',  AXY,dAXY
!
!  Right-hand matrix.
      CALL ROTAT(S(1),Int3,RR)
      CALL DROTT(S(1),S(2),Int3,dRR)
!
!  Left-hand matrix.
      RL(1,1) = 1.D0-A*X2
      RL(1,2) = -AXY
      RL(1,3) = X(1)
      RL(2,1) = -AXY
      RL(2,2) = 1.D0-A*Y2
      RL(2,3) = Y(1)
      RL(3,1) = -X(1)
      RL(3,2) = -Y(1)
      RL(3,3) = 1.D0-A*R2
!
      dRL(1,1) = -dA*X2 - A*dX2
      dRL(1,2) = -dAXY
      dRL(1,3) = X(2)
      dRL(2,1) = -dAXY
      dRL(2,2) = -dA*Y2 - A*dY2
      dRL(2,3) = Y(2)
      dRL(3,1) = -X(2)
      dRL(3,2) = -Y(2)
      dRL(3,3) = -dA*R2 - A*dR2
!
!  The result is the product of the two matrices.
      CALL MMUL2 (RL, RR, RBPN)
!  Derivative of RBPN:
      CALL MMUL2 (dRL, RR, RR1)
      CALL MMUL2 (RL, dRR, RR2)
      CALL MADD2 (RR1, RR2, dRBPN)
!      Write(6,1012) RBPN,dRBPN
 1012  Format(1x,'BPN2K/RBPN,dRBPN:',(6(/,3E25.15)))
!
      RETURN
      END
!***************************************************************************
      SUBROUTINE BPN2KP ( X, Y, S, RBPNX, RBPNY )
!
!  CEO-based bias-precession-nutation matrix.
!  Given:
!     X(2),Y(2)        d      CIP coordinates
!     S(2)             d      the quantity s (radians)
!  Returned:
!     RBPN        d(3,3)   intermediate-to-celestial matrix ("Q")
!
!
!  Modified SOFA subroutine for use in Calc. Computes partial derivatives
!  of the bias-precession-nutation matrix and its time derivative w.r.t.
!  X and Y nutation.
!-----------------------------------------------------------------------
      IMPLICIT NONE
      DOUBLE PRECISION X(2), Y(2), S(2), RBPNX(3,3,2), RBPNY(3,3,2)
      DOUBLE PRECISION X2, Y2, R2, R, Z, A, AXY, RR(3,3), RL(3,3)
      DOUBLE PRECISION dX2, dY2, dR2, dZ, dA, dAXY,dRR(3,3),dRL(3,3),
     .                 RR1(3,3), RR2(3,3), dRBPN(3,3)
      Double PRECISION p_X2_x, p_dX2_x,p_Y2_y,p_dY2_y,p_R2_x,
     .       p_R2_y,p_dR2_x,p_dR2_y,p_Z_x,p_Z_y,p_dZ_x,p_dZ_y,
     .       p_A_x,p_A_y,p_dA_x,p_dA_y,p_AXY_x,p_AXY_y,p_dAXY_x,
     .       p_dAXY_y, RLX(3,3), RLY(3,3),
     .       dRLX(3,3), dRLY(3,3)
      Integer*2 Int1,Int2,Int3
!
! - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
         Int1 = 1
         Int2 = 2
         Int3 = 3
!       print *,' X, Y, S: ',  X, Y, S
!
!  Prepare to evaluate expression (10).
      X2 = X(1)*X(1)
      dX2 = 2.D0*X(1)*X(2)
       p_X2_x = 2.D0*X(1)
       p_dX2_x = 2.D0*X(2)
!       print *,'X2,dX2: ', X2,dX2
!       print *,'p_X2_x,p_dX2_x: ', p_X2_x,p_dX2_x
!
      Y2 = Y(1)*Y(1)
      dY2 = 2.D0*Y(1)*Y(2)
       p_Y2_y = 2.D0*Y(1)
       p_dY2_y = 2.D0*Y(2)
!       print *,'Y2,dY2: ', Y2,dY2
!       print *,'p_Y2_y,p_dY2_y: ', p_Y2_y,p_dY2_y
!
      R2 = X2 + Y2
      dR2 = dX2 + dY2
       p_R2_x = p_X2_x
       p_R2_y = p_Y2_y
       p_dR2_x = p_dX2_x
       p_dR2_y = p_dY2_y
!       print *,'R2,dR2: ', R2,dR2
!       print *,'p_R2_x,p_dR2_x: ', p_R2_x,p_dR2_x
!       print *,'p_R2_y,p_dR2_y: ', p_R2_y,p_dR2_y
!
!     R = DSQRT (R2)
!
      Z = DSQRT (1.D0 - R2)
      dZ = -dR2/(2.D0*DSQRT(1D0 - R2))
       p_Z_x = -X(1)/Z
       p_Z_y = -Y(1)/Z
       p_dZ_x = -p_dX2_x/(2.D0*Z) + dR2*p_Z_x/(2.D0*(Z**2))
       p_dZ_y = -p_dY2_y/(2.D0*Z) + dR2*p_Z_y/(2.D0*(Z**2))
!       print *,'Z,dZ: ', Z,dZ
!       print *,'p_Z_x,p_dZ_x: ', p_Z_x,p_dZ_x
!       print *,'p_Z_y,p_dZ_y: ', p_Z_y,p_dZ_y
!
      A = 1D0 / ( 1.D0 + Z )
      dA = -dZ/(1.D0 + Z)**2
       p_A_x = -p_Z_x/(1.D0 + Z)**2
       p_A_y = -p_Z_y/(1.D0 + Z)**2
       p_dA_x = -p_dZ_x/(1.D0+Z)**2 + 2.D0*dZ*p_Z_x/(1.D0+Z)**3
       p_dA_y = -p_dZ_y/(1.D0+Z)**2 + 2.D0*dZ*p_Z_y/(1.D0+Z)**3
!       print *,'A,dA: ', A,dA
!       print *,'p_A_x,p_dA_x: ', p_A_x,p_dA_x
!       print *,'p_A_y,p_dA_y: ', p_A_y,p_dA_y
!
      AXY = A*X(1)*Y(1)
      dAXY = dA*X(1)*Y(1) + A*X(2)*Y(1) + A*X(1)*Y(2)
       p_AXY_x = p_A_x*X(1)*Y(1) + A*Y(1)
       p_AXY_y = p_A_y*X(1)*Y(1) + A*X(1)
       p_dAXY_x = p_dA_x*X(1)*Y(1) + dA*Y(1) + p_A_x*X(2)*Y(1) +
     .            p_A_x*X(1)*Y(2) + A*Y(2)
       p_dAXY_y = p_dA_y*X(1)*Y(1) + dA*X(1) + p_A_y*X(2)*Y(1) +
     .            A*X(2) + p_A_y*X(1)*Y(2)
!       print *,'AXY,dAXY: ', AXY,dAXY
!       print *,'p_AXY_x,p_dAXY_x: ', p_AXY_x,p_dAXY_x
!       print *,'p_AXY_y,p_dAXY_y: ', p_AXY_y,p_dAXY_y
!
!
!  Right-hand matrix.
      CALL ROTAT(S(1),Int3,RR)
      CALL DROTT(S(1),S(2),Int3,dRR)
!
!  Left-hand matrix w.r.t. X.
      RLX(1,1) = -p_A_x*X2 - A*p_X2_x
      RLX(1,2) = -p_AXY_x
      RLX(1,3) = 1.D0
      RLX(2,1) = -p_AXY_x
      RLX(2,2) = -p_A_x*Y2
      RLX(2,3) = 0.D0
      RLX(3,1) = -1.D0
      RLX(3,2) = 0.D0
      RLX(3,3) = -p_A_x*R2 - A*p_R2_x
!
      RLY(1,1) = -p_A_y*X2
      RLY(1,2) = -p_AXY_y
      RLY(1,3) = 0.D0
      RLY(2,1) = -p_AXY_y
      RLY(2,2) = -p_A_y*Y2 - A*p_Y2_y
      RLY(2,3) = 1.D0
      RLY(3,1) =  0.D0
      RLY(3,2) = -1.D0
      RLY(3,3) = -p_A_y*R2 - A*p_R2_y
!
!      dRLX(1,1) = -dA*X2 - A*dX2
      dRLX(1,1) = -p_dA_x*X2 - dA*p_X2_x - p_A_x*dX2 - A*p_dX2_x
      dRLY(1,1) = -p_dA_y*X2             - p_A_y*dX2
!      dRLX(1,2) = -dAXY
      dRLX(1,2) = -p_dAXY_x
      dRLY(1,2) = -p_dAXY_y
!      dRLX(1,3) =  X(2)
      dRLX(1,3) =  0.D0
      dRLY(1,3) =  0.D0
!      dRLX(2,1) = -dAXY
      dRLX(2,1) = -p_dAXY_x
      dRLY(2,1) = -p_dAXY_y
!     dRLX(2,2) = -dA*Y2 - A*dY2
      dRLX(2,2) = -p_dA_x*Y2 - p_A_x*dY2
      dRLY(2,2) = -p_dA_y*Y2 -dA*p_Y2_y - p_A_y*dY2 - A*p_dY2_y
!      dRLX(2,3) =  Y(2)
      dRLX(2,3) =  0.D0
      dRLY(2,3) =  0.D0
!      dRLX(3,1) = -X(2)
      dRLX(3,1) =  0.D0
      dRLY(3,1) =  0.D0
!      dRLX(3,2) = -Y(2)
      dRLX(3,2) =  0.D0
      dRLY(3,2) =  0.D0
!      dRLX(3,3) = -dA*R2 - A*dR2
      dRLX(3,3) = -p_dA_x*R2 - dA*p_R2_x - p_A_x*dR2 - A*p_dR2_x
      dRLY(3,3) = -p_dA_y*R2 - dA*p_R2_y - p_A_y*dR2 - A*p_dR2_y
!
!  The partials are the two matrices.
      CALL MMUL2 (RLX, RR, RBPNX(1,1,1))
      CALL MMUL2 (RLY, RR, RBPNY(1,1,1))
!  The partials of the derivative matrix w.r.t. X
      CALL MMUL2 (dRLX, RR, RR1)
      CALL MMUL2 (RLX, dRR, RR2)
      CALL MADD2 (RR1, RR2, RBPNX(1,1,2))
!  The partials of the derivative matrix w.r.t. Y
      CALL MMUL2 (dRLY, RR, RR1)
      CALL MMUL2 (RLY, dRR, RR2)
      CALL MADD2 (RR1, RR2, RBPNY(1,1,2))
!      Write(6,1012) RBPNdX
!1012  Format(1x,'RBPNdX:',(6(/,3E25.15)))
!      Write(6,1014) RBPNdY
!1014  Format(1x,'RBPNdY:',(6(/,3E25.15)))
      RETURN
      END
