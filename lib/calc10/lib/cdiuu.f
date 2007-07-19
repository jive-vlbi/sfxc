      SUBROUTINE DIRNA()
      IMPLICIT NONE
!
!   Now nothing but Deletes, 2004.04.21.
!
!       PROGRAMMER:
!         93.09.01 Norbert Zacharias - Equation of the equinoxes GAST update
!         94.01.04 David Gordon - Cleaned up TOC entries.
!         98.01.20 David Gordon - New TOC entries for New definition of the
!                                 equation of the equinoxes. Removed Lcodes
!                                 for previous EQE (calc 8.x) contributions.
!         2002.09  Jim Ryan       Integer*4 conversion.
!         2003/2004 D. Gordon     Calc 10 mods for IERS Conventions (2003).
!
       INCLUDE 'ccon.i'
!            VARIABLES 'FROM':
!              1. KDIUC - THE DIURNAL SPIN UTILITY ROUTINE FLOW CONTROL FLAG.
!                         - Does nothing!
!
        CALL DELR (int2(2), 'EQE DIFF')
        CALL DELR (int2(2), 'EQE CONT')
        CALL DELR (int2(2), 'OLDEQCON')
        CALL DELR (int2(2), 'NEWEQCON')
!
      Return
      END
!***********************************************************************
      SUBROUTINE DIRNL ( DATDCT, DUT1AT, EPS, FA2K, FAD2K, UT1,
     .           XJD, CT, DUTCAT, CENT, DEPS2K, DPSI2K, EPSA,  
     .           ERA2K, DERA2K, pERA2K, RS2K, GAST2K, GMST2K, RSC2K)
      IMPLICIT None
!
! 1.    DIRNL
!
! 1.1   DIRNL PROGRAM SPECIFICATION
!
! 1.1.1 DIRNL is the utility routine which computes the diurnal spin portion of
!       the complete crust fixed to J2000.0 rotation matrix and its first two CT
!       time derivatives. DIRNL also computes the diurnal angular velocity of
!       the Earth, the Greenwich Mean Sidereal Time, and the Greenwich Apparent
!       Siderial Time (GAST) and its CT time derivative.
!
! 1.1.2 RESTRICTIONS - NONE
!
! 1.1.3 REFERENCES - 1) 'THE EXPLANATORY SUPPLEMENT TO THE AMERICAN
!                       EPHEMERIS AND NAUTICAL ALMANAC", P.72-76,
!                    2) AOKI, S. ET AL., "THE NEW DEFINITION OF UNIVERSAL
!                       TIME", ASTRON. ASTROPHYS., ????,1980.
!                    3) McCarthy, D., IERS Technical Note 13, Paris 1992
!                    4) McCarthy, D., IERS Technical Note 32, IERS
!                       Conventions (2003).
!
! 1.2   DIRNL PROGRAM INTERFACE
!
! 1.2.1 CALLING SEQUENCE -
!        INPUT VARIABLES:
!             1. DATDCT   -  THE PARTIAL DERIVATIVE OF ATOMIC TIME WITH
!                            RESPECT TO COORDINATE TIME. (SEC/SEC)
!             2. DUT1AT   -  THE PARTIAL DERIVATIVE OF UT1 TIME WITH
!                            RESPECT TO ATOMIC TIME. (SEC/SEC)
! Not Used??  3. EPS(2)   -  THE TRUE OBLIQUITY OF THE ECLIPTIC AND ITS
!                            CT TIME DERIVATIVE. (RAD, RAD/SEC)
!             4. FA2K(5)  -  The fundamental nutation arguments (see NUTFA)
!             5. FAD2K(5) -  The time derivative of th fundamental
!                            nutation arguments (see NUTFA)
!             6. UT1      -  THE UT1 TIME OF THE DAY. (SEC)
!             7. XJD      -  THE JULIAN DATE AT ZERO HOURS UTC OF THE
!                            DATE IN QUESTION. (DAYS)
!             8. CT       -  Coordinate time, fraction of the day. (days)
!             9. DUTCAT    - Partial derivative of the UTC time with
!                            respect to the atomic time.
!            10. CENT      - Number of Julian centuries elapsed since the
!                            epoch January 1.5, 2000. (centuries)
!            11. DEPS2K(2) - THE NUTATION IN OBLIQUITY AND ITS CT TIME
!                            DERIVATIVE COMPUTED FROM WAHR OR TAKEN FROM
!                            THE DATA BASE. (RAD, RAD/SEC)
!            12. DPSI2K(2) - THE NUTATION IN LONGITUDE AND ITS CT
!                            TIME DERIVATIVE. (RAD, RAD/SEC)
!
!        OUTPUT VARIABLES:
!             2. EPSA(2)   -  True obliquity (????) using IAU200A
!                             Precession/Nutation
!             3. ERA2K     -  Earth rotation angle (Radians).
!             4. DERA2K    -  Time derivative of Earth rotation angle.
!             5. pERA2K    -  Partial derivative of the Earth rotation
!                             angle (ERA2K) w.r.t. UT1.
!             5. RS2K(3,3,3)- 'New paradigm' diurnal spin rotation
!                             matrix, and its first two CT time
!                             derivatives. (unitless, 1/sec, 1/sec**2)
!             7. GAST2K(2) -  IERS2000 Greenwich apparent sidereal time
!                             and its CT (?) time derivative. (rad, rad/sec)
!             8. GMST2K     - THE GREENWICH MEAN SIDEREAL TIME. (RAD)
!             9. RSC2K(3,3,3)-'Classical' diurnal spin matrix using the
!                             IERS2000 Greenwich apparent sidereal time,
!                             and its first two CT time derivatives.
!                             (unitless, 1/sec, 1/sec**2)
!
      Real*8 DATDCT, DUT1AT, EPS(2), UT1, XJD, CT, DUTCAT,
     .       ERA2K, DERA2K, RS2K(3,3,3), GAST2K(2), RSC2K(3,3,3),
     .       CENT, DPSI2K(2), DEPS2K(2), EPSA(2), FA2K(14), FAD2K(14)
!
! 1.2.2 COMMON BLOCKS USED -
!
      Real*8           PI, TWOPI, HALFPI, CONVD, CONVDS, CONVHS, SECDAY
      COMMON / CMATH / PI, TWOPI, HALFPI, CONVD, CONVDS, CONVHS, SECDAY
!            VARIABLES 'FROM':
!              1.  CONVHS  -  THE CONVERSION FACTOR OF RADIANS PER TIME-SECOND.
!                             (RAD/TIME-SEC)
           INCLUDE 'ccon.i'
!            VARIABLES 'FROM':
!              1.  KDIUC  -  THE DIURNAL SPIN UTILITY ROUTINE FLOW CONTROL FLAG.
!              2.  KDIUD  -  THE DIURNAL SPIN UTILITY ROUTINE DEBUG OUTPUT FLAG.
!              3.  KNUTC  -  THE NUTATION MODULE FLOW CONTROL FLAG. USED TO
!                            DETERMINE WHETHER OR NOT DPSI SHOULD BE ZEROED.
!
      Real*8 DJ2000, DAYSJ, CENJ
      Real*8 UT1f, F, T, ERA, GST2000, EE2000, XJD2, T2, dT2, GST,
     .       dGST, EECT2000, EPSAx, dEPSA, EE2K, dEE2K, EECT2K,   
     .       dEECT2K, GMST2K(2), pERA2K, dERA2K1
      Integer*4 I
!
      DATA  DJ2000 /  2451545.D0 /
!
! 1.2.4 DATA BASE ACCESS - NONE
!
! 1.2.5 EXTERNAL INPUT/OUTPUT -
!            INPUT VARIABLES - NONE
!            OUTPUT VARIABLES - POSSIBLE DEBUG OUTPUT
!
! 1.2.6 SUBROUTINE INTERFACE -
!             CALLER SUBROUTINES: DRIVR
!             CALLED SUBROUTINES: DDROT, DROTT, ROTAT
!
! 1.2.7 CONSTANTS USED - CONVHS, DJ2000
!
! 1.2.7.1 CONSTANTS INITIALIZED IN THIS UTILITY ROUTINE -
!           2. DJ2000    -  THE JULIAN DATE OF THE EPOCH JANUARY 1.5, 2000.
!                           (DJ2000 = 2451545.D0 DAYS)
! 1.2.8 PROGRAM VARIABLES -
!
!
! 1.2.9 PROGRAMMER
!         770207  DALE MARKHAM
!         770718  PETER DENATALE
!         771128  BRUCE SCHUPLER
!         790201  BRUCE SCHUPLER
!         810804  CHOPO MA
!         870604  SAVITA GOEL
!         890726  Jim Ryan:          Documentation simplified.
!         930901  Norbert Zacharias: Equation of equinoxes GAST update
!        2002.09  Jim Ryan  Integer*4 conversion.
!        2003-2004 D.Gordon Updated for IERS Conventions (2003).
!
!  DIRNL Program Structure
!
!   Compute the Earth rotation angle, THETA, for use in the new IERS2000
!   tranformations
       DAYSJ = XJD  -  DJ2000
       UT1f = UT1/86400.D0
       T = UT1f + DAYSJ
       F = DMOD(UT1f,1.D0) + DMOD (XJD,1.D0)
       ERA   =  TWOPI * ( F + 0.7790572732640D0
     .          + 0.00273781191135448D0 * T )
       ERA2K = DMOD (ERA,TWOPI)
       IF (ERA2K .lt. 0.D0) ERA2K = ERA2K + TWOPI
       DERA2K = TWOPI*(DUT1AT + 0.00273781191135448D0*DUTCAT)/86400.D0
!
!   Construct the Earth rotation angle matrix and its first two CT
!     time derivatives.
!   Construct the Earth rotation matrix.
      CALL ROTAT ( -ERA2K, int2(3), RS2K(1,1,1))
!   Construct the first CT time derivative of the diurnal spin matrix
      CALL DROTT ( -ERA2K, -DERA2K, int2(3), RS2K(1,1,2))
!   Construct the second CT time derivative of the diurnal spin matrix.
      CALL DDROT ( -ERA2K, DERA2K**2, int2(3), RS2K(1,1,3))
!
!   Compute the partials of ERA2K and DERA2K w.r.t. UT1
       pERA2K = TWOPI * 1.00273781191135448D0/86400.D0
!************************************************************************
!
!   Compute the Greenwich Apparent Siderial Time consistent with the
!   IERS 2003 classical transformations.
        T2 = CENT
        dT2 = 1.D0/(36525.D0*86400.D0)
!
!  Equation of the equinoxes.
      CALL EECT (T2, FA2K, FAD2K, EECT2K, dEECT2K)
      EE2K = DPSI2K(1)*COS(EPSA(1)) + EECT2K
      dEE2K = DPSI2K(2)*COS(EPSA(1)) - DPSI2K(1)*SIN(EPSA(1))*EPSA(2)
     .        + dEECT2K
!
       GAST2K(1) = ERA2K + ( 0.014506D0 + 4612.15739966D0*T2 +
     .        1.39667721D0*T2**2 - 0.00009344D0*T2**3 +
     .        0.00001882D0*T2**4 ) * CONVDS
     .      + EE2K
       GAST2K(2) = dERA2K + ( 4612.15739966D0 +
     .        2.D0*1.39667721D0*T2 - 3.D0*0.00009344D0*T2**2 +
     .        4.D0*0.00001882D0*T2**3 ) * CONVDS*dT2
     .      + dEE2K
! Greenwich mean siderial time
        GMST2K(1) = GAST2K(1) - EE2K
        GMST2K(2) = GAST2K(2) - dEE2K
!
!   Construct the IERS2000 classical diurnal spin matrix and its first
!    two CT time derivatives.
!   Construct the diurnal spin matrix.
      CALL ROTAT ( -GAST2K(1), int2(3), RSC2K(1,1,1))
!   Construct the first CT time derivative of the diurnal spin matrix
      CALL DROTT ( -GAST2K(1), -GAST2K(2), int2(3), RSC2K(1,1,2))
!   Construct the second CT time derivative of the diurnal spin matrix.
      CALL DDROT ( -GAST2K(1), GAST2K(2)**2, int2(3), RSC2K(1,1,3))
!
!************************************************************************
!   Check KDIUD for debug output.
      IF ( KDIUD .NE. 0 )  THEN
        WRITE ( 6, 9)
    9   FORMAT (1X, "Debug output for utillity DIRNL." )
    8   FORMAT(A,4D25.16/(7X,5D25.16))
        WRITE(6,8)' DJ2000  ',DJ2000
        WRITE(6,8)' CONVHS  ',CONVHS
       write(6,233)  UT1f,F,T
 233   format (' DIURNL/UT1f,F,T: ', 3E30.20)
       write(6,340)  ERA2K,DERA2K
 340   format(' DIRNL/ERA2K,DERA2K: ',F22.18,E30.20)
       WRITE ( 6, * ) 'DIURNL/EE_PRC: ', GAST2K(1) - ERA2K - EE2K
       Write(6,1024) RS2K
 1024  Format(1x,'DIRNL/RS2K  ',(9(/,3E25.15)))
       write(6,341)  EE2K, dEE2K
 341   format(' DIRNL/EE2K,dEE2K: ',E30.20)
       write(6,342)  GAST2K
 342   format(' DIRNL/GAST2K: ',F22.18,E30.20)
       WRITE ( 6, * ) ' DIRNL/GMST2K:  ', GMST2K
       Write(6,1026) RSC2K
 1026  Format(1x,'DIRNL/RSC2K: ',(9(/,3E25.15)))
       WRITE ( 6, * ) 'DIURNL/pERA2K: ', pERA2K
       WRITE ( 6, * ) 'DIURNL/T2: ', T2
       WRITE ( 6, * ) 'DIURNL/DAYSJ: ', DAYSJ
!
        WRITE ( 6, 9200 )  DATDCT, DPSI2K, DUT1AT, EPSA, UT1, XJD,
     .                     GAST2K, GMST2K, RS2K
 9200   FORMAT (1X, "DATDCT = ", D30.16, /, 1X,
     .            "DPSI2K = ", 2 ( D30.16, 10X ), /, 1X,
     .            "DUT1AT = ", D30.16, /, 1X,
     .            "EPSA   = ", 2 ( D30.16, 10X ), /, 1X,
     .            "UT1    = ", D30.16, /, 1X,
     .            "XJD    = ", D30.16, /, 1X,
     .            "GAST2K = ", 2 ( D30.16, 10X ), /, 1X,
     .            "GMST2K = ", D30.16, /, 1X,
     .            "RS2K   = ", 9 ( 3 ( D30.16, 10X ), /, 1X ) )
       WRITE ( 6, * ) 'DPSI2K(1), EPSA(1) = ', DPSI2K(1), EPSA(1)
       WRITE ( 6, * ) 'DPSI2K(1)*COS(EPSA(1)), EECT2K = ',
     .            DPSI2K(1)*COS(EPSA(1)), EECT2K 
!
      ENDIF   ! debug output flag
!
      Return
      END
!*************************************************************************
      SUBROUTINE EECT (T, FA2K, FAD2K, EECT2K, dEECT2K)
      IMPLICIT NONE
!
!  Equation of the equinoxes complementary terms, consistent with
!  IAU 2000 resolutions.
!  Modified form of SOFA subroutine EECT2000.
!
!  Given:
!     XJD, CT       d    TT date (JD = DATE1+DATE2)
!
!  Returned:
!     EECT2K        d    Complementary terms (radians)
!     dEECT2K       d    Time derivative of complementary terms
!                         (radians/sec)
!
!-----------------------------------------------------------------------
!
      REAL*8 EECT2K, dEECT2K
!  2Pi
      REAL*8           D2PI
      PARAMETER ( D2PI = 6.283185307179586476925287D0 )
!  Arcseconds to radians
      REAL*8           DAS2R
      PARAMETER ( DAS2R = 4.848136811095359935899141D-6 )
!  Reference epoch (J2000), JD
      REAL*8           DJ0
      PARAMETER ( DJ0 = 2451545D0 )
!  Days per Julian century
      REAL*8           DJC
      PARAMETER ( DJC = 36525D0 )
!  Time since J2000, in Julian centuries
      REAL*8           T
!  Miscellaneous
      INTEGER I, J
      REAL*8           A, S0, S1, dA, dS0, dS1
      REAL*8           ANMP
!
!  Fundamental arguments
!     DOUBLE PRECISION FA(14)
      REAL*8 FA2K(14), FAD2K(14)
!     COMMON / NFA2K / FA2K, FAD2K
!
!  -----------------------------------------
!  The series for the EE complementary terms
!  -----------------------------------------
!
!  Number of terms in the series
      INTEGER NE0, NE1
      PARAMETER ( NE0=  33, NE1=  1 )
!
!  Coefficients of l,l',F,D,Om,LMe,LVe,LE,LMa,LJu,LSa,LU,LN,pA
      INTEGER KE0 ( 14, NE0 ),
     .        KE1 ( 14, NE1 )
!
!  Sine and cosine coefficients
      REAL*8           SE0 ( 2, NE0 ),
     .                 SE1 ( 2, NE1 )
!
!  Argument coefficients for t^0
      DATA ( ( KE0(I,J), I=1,14), J =    1,   10 ) /
     .  0,  0,  0,  0,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     .  0,  0,  0,  0,  2,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     .  0,  0,  2, -2,  3,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     .  0,  0,  2, -2,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     .  0,  0,  2, -2,  2,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     .  0,  0,  2,  0,  3,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     .  0,  0,  2,  0,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     .  0,  0,  0,  0,  3,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     .  0,  1,  0,  0,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     .  0,  1,  0,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0 /
      DATA ( ( KE0(I,J), I=1,14), J =   11,   20 ) /
     .  1,  0,  0,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     .  1,  0,  0,  0,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     .  0,  1,  2, -2,  3,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     .  0,  1,  2, -2,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     .  0,  0,  4, -4,  4,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     .  0,  0,  1, -1,  1,  0, -8, 12,  0,  0,  0,  0,  0,  0,
     .  0,  0,  2,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     .  0,  0,  2,  0,  2,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     .  1,  0,  2,  0,  3,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     .  1,  0,  2,  0,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0 /
      DATA ( ( KE0(I,J), I=1,14), J =   21,   30 ) /
     .  0,  0,  2, -2,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     .  0,  1, -2,  2, -3,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     .  0,  1, -2,  2, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     .  0,  0,  0,  0,  0,  0,  8,-13,  0,  0,  0,  0,  0, -1,
     .  0,  0,  0,  2,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     .  2,  0, -2,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     .  1,  0,  0, -2,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     .  0,  1,  2, -2,  2,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     .  1,  0,  0, -2, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     .  0,  0,  4, -2,  4,  0,  0,  0,  0,  0,  0,  0,  0,  0 /
      DATA ( ( KE0(I,J), I=1,14), J =   31,  NE0 ) /
     .  0,  0,  2, -2,  4,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     .  1,  0, -2,  0, -3,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     .  1,  0, -2,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0 /
!
!  Argument coefficients for t^1
      DATA ( ( KE1(I,J), I=1,14), J =    1,  NE1 ) /
     .  0,  0,  0,  0,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0 /
!
!  Sine and cosine coefficients for t^0
      DATA ( ( SE0(I,J), I=1,2), J =    1,   10 ) /
     .            +2640.96D-6,          -0.39D-6,
     .              +63.52D-6,          -0.02D-6,
     .              +11.75D-6,          +0.01D-6,
     .              +11.21D-6,          +0.01D-6,
     .               -4.55D-6,          +0.00D-6,
     .               +2.02D-6,          +0.00D-6,
     .               +1.98D-6,          +0.00D-6,
     .               -1.72D-6,          +0.00D-6,
     .               -1.41D-6,          -0.01D-6,
     .               -1.26D-6,          -0.01D-6 /
      DATA ( ( SE0(I,J), I=1,2), J =   11,   20 ) /
     .               -0.63D-6,          +0.00D-6,
     .               -0.63D-6,          +0.00D-6,
     .               +0.46D-6,          +0.00D-6,
     .               +0.45D-6,          +0.00D-6,
     .               +0.36D-6,          +0.00D-6,
     .               -0.24D-6,          -0.12D-6,
     .               +0.32D-6,          +0.00D-6,
     .               +0.28D-6,          +0.00D-6,
     .               +0.27D-6,          +0.00D-6,
     .               +0.26D-6,          +0.00D-6 /
      DATA ( ( SE0(I,J), I=1,2), J =   21,   30 ) /
     .               -0.21D-6,          +0.00D-6,
     .               +0.19D-6,          +0.00D-6,
     .               +0.18D-6,          +0.00D-6,
     .               -0.10D-6,          +0.05D-6,
     .               +0.15D-6,          +0.00D-6,
     .               -0.14D-6,          +0.00D-6,
     .               +0.14D-6,          +0.00D-6,
     .               -0.14D-6,          +0.00D-6,
     .               +0.14D-6,          +0.00D-6,
     .               +0.13D-6,          +0.00D-6 /
      DATA ( ( SE0(I,J), I=1,2), J =   31,  NE0 ) /
     .               -0.11D-6,          +0.00D-6,
     .               +0.11D-6,          +0.00D-6,
     .               +0.11D-6,          +0.00D-6 /
!
!  Sine and cosine coefficients for t^1
      DATA ( ( SE1(I,J), I=1,2), J =    1,  NE1 ) /
     .               -0.87D-6,          +0.00D-6 /
!
! - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
!  Interval between fundamental epoch J2000.0 and current date (JC).
!     T = ( ( DATE1-DJ0 ) + DATE2 ) / DJC
!         print *,' EECT2K/T= ', T
!
!
!  Evaluate the EE complementary terms.
      S0 = 0D0
      S1 = 0D0
      dS0 = 0D0
      dS1 = 0D0
!
      DO I = NE0,1,-1
        A = 0.D0
        dA = 0.D0
         DO J=1,14
            A = A + DBLE(KE0(J,I))*FA2K(J)
            dA = dA + DBLE(KE0(J,I))*FAD2K(J)
         END DO
        S0 = S0 + ( SE0(1,I)*SIN(A) + SE0(2,I)*COS(A) )
        dS0 = dS0 + ( SE0(1,I)*COS(A)*dA - SE0(2,I)*SIN(A)*dA )
      END DO
!      print *,' EECT/S0,dS0 ', S0,dS0
!
      DO I = NE1,1,-1
         A = 0.D0
         dA = 0.D0
         DO J=1,14
            A = A + DBLE(KE1(J,I))*FA2K(J)
            dA = dA + DBLE(KE1(J,I))*FAD2K(J)
         END DO
         S1 = S1 + ( SE1(1,I)*SIN(A) + SE1(2,I)*COS(A) )
         dS1 = dS1 + ( SE1(1,I)*COS(A)*dA - SE1(2,I)*SIN(A)*dA )
      END DO
!      print *,' EECT/S1,dS1 ', S1,dS1
!
      EECT2K = ( S0 + S1 * T ) * DAS2R
      dEECT2K = ( dS0 + dS1*T/(36525.D0*86400D0) ) * DAS2R
!      print *,' EECT/EECT2K,dEECT2K ', EECT2K,dEECT2K
!
      RETURN
      END
