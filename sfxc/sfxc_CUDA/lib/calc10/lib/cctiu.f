      SUBROUTINE CTIMA()
      IMPLICIT None
!
! 1.    CTIMA
!
! 1.1   CTIMA PROGRAM SPECIFICATION
!
! 1.1.1 CTIMA ADDS ENTRIES TO THE TABLE OF CONTENTS FOR THE CTIMG UTILITY
!       ROUTINE TEXT MESSAGE.
!
! 1.2   CTIMA PROGRAM INTERFACE
!
! 1.2.4 DATA BASE ACCESS -
!            ACCESS CODES:
!              1.  'CTI MESS'  -  THE DATA BASE ACCESS CODE FOR THE CTIMG
!                                 UTILITY ROUTINE TEXT MESSAGE.
!              2.  'CTI CFLG'  -  THE DATA BASE ACCESS CODE FOR THE CTIMG
!                                 UTILITY ROUTINE FLOW CONTROL MESSAGE.
!              3.  'CT SITE1'  -  THE DATA BASE ACCESS CODE FOR THE CTIMG
!                                 UTILITY ROUTINE CT TIME FRACTION AT SITE 1.
!
! 1.2.6 SUBROUTINE INTERFACE -
!             CALLER SUBROUTINES: TOCUP
!             CALLED SUBROUTINES: ADDA, ADDR
!
! 1.2.8 PROGRAM VARIABLES - NONE
!
! 1.2.9 PROGRAMMER - DALE MARKHAM   02/04/77
!                    PETER DENATALE 07/18/77
!                    CHOPO MA       08/06/81
!                    SAVITA GOEL    06/04/87 (CDS FOR A900)
!                    Jim Ryan 89.07.26 Documentation simplified.
!                    Jim Ryan 89.12.12 UNIX-like database interface
!                    implimented.
!                    Jim Ryan 03.03.10 Kill replaced with terminate_solve
!                    Jim Ryan Sept02 Integer*2/4 mods.
!
!     CTIMA Program Structure
!
!   ADD for the CTIMG utility text message.
      CALL ADDA (int2(1),'CTI MESS','CTIMG Message Definition       ',
     . int2(40), int2(1), int2(1))
!
!   ADD for the CTIMG flow contrl message.
      CALL ADDA (int2(1),'CTI CFLG','CTIMG Flow Control Message Der ',
     . int2(40), int2(1), int2(1))
!
!   ADD for putting the CT fraction of the day into the database.
      CALL ADDR (int2(2),'CT SITE1','CT fraction at obs site 1      ',
     . int2(1), int2(1), int2(1))
!
!   Normal conclusion.
      RETURN
      END
!**********************************************************************
      SUBROUTINE CTIMI()
      IMPLICIT None
!
! 1.    CTIMI
!
! 1.1   CTIMI PROGRAM SPECIFICATION
!
! 1.1.1 CTIMI IS THE SUBROUTINE FOR THE CTIMG UTILITY WHICH GETS NECESSARY
!       PARAMETERS FROM THE DATA BASE HEADER AND ENTERS MESSAGES INTO THE
!       HEADER.
!
! 1.2   CTIMI PROGRAM INTERFACE
!
! 1.2.1 CALLING SEQUENCE - CALL CTIMI
!
! 1.2.2 COMMON BLOCKS USED:
!
      Real*8           A1TAI, ATCTEP, D1950, ECCEN, XL(2), XM(2),
     .       D(2), XLLJ(2), XLLSA(2), XMJ(2), XMSA(2), ACT(15)
      COMMON / CTICM / A1TAI, ATCTEP, D1950, ECCEN, XL,    XM,
     .       D,    XLLJ,    XLLSA,    XMJ,    XMSA,    ACT
!        VARIABLES 'TO'-
!          1. A1TAI - THE OFFSET BETWEEN A1 AND TAI (SECONDS)
!
      INCLUDE 'ccon.i'
!        VARIABLES 'FROM' -
!          1. KCTIC - THE CTIMG UTILITY FLOW CONTROL FLAG
!          2. KCTID - THE CTIMG UTILITY DEBUG CONTROL FLAG
!
      Real*8  ATMUTC(3), ROTEPH(2,20), A1UTC(3), A1DIFF(3)
!     COMMON / ATICM / ATMUTC, ROTEPH, A1UTC, A1DIFF
      COMMON / EOPCM / ATMUTC, ROTEPH, A1UTC, A1DIFF
!           VARIABLES 'TO/FROM':
!            1. A1DIFF(3) - THE ARRAY CONTAINING THE EPOCH, OFFSET AND RATE
!                           OF CHANGE OF THE OFFSET BETWEEN A1 AND TAI
!                           (DAYS, SECONDS, SECONDS/DAY)
!
      INCLUDE 'inputs.i'
!            Variables from:
!              1. Input_EOP - T/F logical flag telling whether to use external
!                             EOP file input
!
! 1.2.3 PROGRAM SPECIFICATIONS -
!
!     Real*8    A1DIFF(3)
      Integer*2 NDO(3), KERR(1)
      INTEGER*2      LCTIU(40),   LON(40),   LOFFA(40),  LOFFP(40)
      CHARACTER*40 C_LCTIU(2),  C_LON(2),  C_LOFFA(2), C_LOFFP(2)
      EQUIVALENCE (C_LCTIU, LCTIU),(C_LON, LON),(C_LOFFA, LOFFA),
     .            (C_LOFFP, LOFFP)
!
      DATA C_LCTIU /
     .'CTIMG Utility routine - Version # 2, las',
     .'t modification - 08/06/81 Chopo Ma      '/
!
      DATA C_LON /
     .'CTIMG Utility routine is turned on.     ',
     .'                                        '/
!
      DATA C_LOFFA /
     .'CTIMG utility routine is turned off.    ',
     .'                                        '/
!
      DATA C_LOFFP  /
     .'CTIMG utility routine periodic terms tur',
     .'ned off.                                '/
!
! 1.2.4 DATA BASE ACCESS -
!
!      'GET' VARIABLES -
!         1. A1DIFF(3) - THE ARRAY CONTAINING THE EPOCH, OFFSET AND RATE
!                        OF CHANGE OF THE OFFSET BETWEEN A1 AND TAI
!                        (DAYS, SECONDS, SECONDS/DAY)
!
!      'PUT' VARIABLES -
!         1. LCTIU(40) - THE CTIMG UTILITY ROUTINE TEXT MESSAGE.
!         2. LON(40)   - THE CTIMG UTILITY 'TURNED ON' MESSAGE.
!         3. LOFFA(40) - THE CTIMG UTILITY 'TURNED OFF' MESSAGE.
!         4. LOFFP(40) - THE CTIMG UTILITY 'TURNED OFF PERIODIC TERMS' MESSAGE.
!
!      ACCESS CODES:
!         1. 'CTI MESS' - THE DATA BASE ACCESS CODE FOR THE CTIMG UTILITY
!                         ROUTINE TEXT MESSAGE.
!         2. 'CTI CFLG' - THE DATA BASE ACCESS CODE FOR THE CTIMG UTILITY FLOW
!                         CONTROL MESSAGE.
!         3. 'A1 - TAI' - THE DATA BASE ACCESS CODE FOR THE A1DIFF ARRAY.
!
! 1.2.5 EXTERNAL INPUT/OUTPUT - POSSIBLE DEBUG OUTPUT
!
! 1.2.6 SUBROUTINE INTERFACE:
!          CALLER SUBROUTINES - INITL
!          CALLED SUBROUTINES - PUTA, GET4
!
! 1.2.7 CONSTANTS USED - NONE
!
! 1.2.8 PROGRAM VARIABLES -
!       1. NDO(3)  - AN ARRAY USED TO HOLD THE DIMENSIONS FROM GET4_S
!       2. KERR(1) - THE ERROR RETURN CODE FROM GET4
!
! 1.2.9 PROGRAMMER - BRUCE SCHUPLER 02/21/78
!                    CHOPO MA       08/06/81
!                    Jim Ryan  89.07.26 Documentation simplified.
!                    D. Gordon 98.05.01 Added Include file 'inputs.i' and
!                              common block /EOPCM/. Added code to pass in
!                              A1DIFF(3) if using external EOP input.
!                    Jim Ryan Sept02 Integer*2/4 mods.
!
!    CTIMI Program Structure
!
!    PUT the CTIMG utility text message into the database.
      CALL PUTA ('CTI MESS      ',LCTIU,int2(40),int2(1),int2(1))
!
!   PUT the CTIMG utility flow control message into the database depending
!    on KCTIC. See strings above for meanings.
      IF (KCTIC .EQ. 0) CALL PUTA('CTI CFLG      ', LON, int2(40),
     .                  int2(1), int2(1))
      IF (KCTIC .EQ. 1) CALL PUTA('CTI CFLG      ', LOFFA, int2(40),
     .                  int2(1), int2(1))
      IF (KCTIC .EQ. 2) CALL PUTA('CTI CFLG      ', LOFFP, int2(40),
     .                  int2(1), int2(1))
!
      IF (.not. Input_EOP) THEN           !Already have A1DIFF?
!        Get the A1DIFF array and check for database error.
        CALL GET4('A1 - TAI      ',A1DIFF,int2(3),int2(1),int2(1),NDO,
     .   KERR(1))
        IF (KERR(1).NE.0) CALL TERMINATE_CALC('CTIMI ',int2(1),KERR(1))
      ENDIF                               !Already have A1DIFF?
!
!    Move the offset into A1TAI.
      A1TAI = A1DIFF(2)
!
!    See if debug output is needed.
      IF (KCTID .NE. 1) GO TO 600
      WRITE(6,9)
    9 FORMAT(1X,"Degug output from utility CTIMI")
      WRITE(6,8)' A1DIFF  ',A1DIFF
    8 FORMAT(A,4D25.16/(7X,5D25.16))
      WRITE(6,8)' A1TAI   ',A1TAI
600   CONTINUE
      RETURN
      END
!***********************************************************************
      SUBROUTINE CTIMG (AT, CFSITE, SITLON, UTC, XJD, CT, DATDCT,DLPGR)
      IMPLICIT None
!
! 1.    CTIMG
!
! 1.1   CTIMG PROGRAM SPECIFICATION
!
! 1.1.1 CTIMG is the utility routine which computes the coordinate time fraction
!       of the coordinate time day at observing site #1, the partial derivative
!       of the atomic time with respect to the coordinate time, and the partial
!       derivative of the long period terms in the 'AT mimus CT' offset with
!       respect to the coordinate time.
!
!       The algorithm is taken directly from the referenced Moyer paper. Moyer
!       derives the theory in terms of vector positions of the Earth, Moon, and
!       planets in a solar system barycenter coordinate system and he gives an
!       algorithm in that system. In part 2 of the paper he derives an alternate
!       algorithm which does not use vectors, but rather uses an analytic
!       planetary theory. The advantage of the second algorithm is that the only
!       required inputs are the geocentric, crust fixed position of the station
!       and the UTC and AT time tags. The second algorithm is implemented here.
!
!       NOTE 1. The algorithm calls for UT1, but UTC is used in its' place.
!       (Since 1972 UTC has been kept within 0.7 seconds of UT1).
!
!       NOTE 2. The equation numbers in the Moyer paper are identified below.
!
! 1.1.3 REFERENCES  -  MOYER, T. D., TRANSFORMATION FROM PROPER TIME ON
!                      EARTH TO COORDINATE TIME IN SOLAR SYSTEM BARYCENTRIC
!                      SPACE-TIME FRAME OF REFERENCE, CEL. MECH. 23,
!                      33-68, 1981. SEE PP. 66-67.
!
! 1.2   CTIMG PROGRAM INTERFACE
!
! 1.2.1 CALLING SEQUENCE -
!           INPUT VARIABLES:
!             1.  AT          - THE ATOMIC TIME FRACTION OF THE TAI TIME DAY.
!                               (DAYS)
!             2.  CFSITE(3,2) - CRUST FIXED STATION COORDINATES. (M)
!             3.  SITLON(2)   - STATION EAST LONGITUDES. (RAD)
!             4.  UTC         - UTC FRACTION OF DAY. (DAYS)
!             5.  XJD         - THE JULIAN DATE AT ZERO HOURS UTC OF THE DATE
!                               IN QUESTION.  (DAYS)
!           OUTPUT VARIABLES:
!             1.  CT      -  THE COORDINATE TIME FRACTION OF THE COORDINATE TIME
!                            DAY. (DAYS)
!             2.  DATDCT  -  THE PARTIAL DERIVATIVE OF THE ATOMIC TIME WITH
!                            RESPECT TO THE COORDINATE TIME. (SEC/SEC)
!             3.  DLPGR   -  THE PARTIAL DERIVATIVE OF THE LONG PERIOD TERMS IN
!                            THE 'AT MINUS CT' OFFSET WITH RESPECT TO THE
!                            COORDINATE TIME. (SEC/SEC)
!
! 1.2.2 COMMON BLOCKS USED -
!
      Real*8           A1TAI, ATCTEP, D1950, ECCEN, XL(2), XM(2),
     .       D(2), XLLJ(2), XLLSA(2), XMJ(2), XMSA(2), ACT(15)
      COMMON / CTICM / A1TAI, ATCTEP, D1950, ECCEN, XL,    XM,
     .       D,    XLLJ,    XLLSA,    XMJ,    XMSA,    ACT
!
!     VARIABLES 'FROM':
!       1. A1TAI   -  THE OFFSET BETWEEN A1 AND TAI (SECONDS) (NO LONGER USED)
!       2. ATCTEP  -  THE VALUE OF THE 'CT MINUS TAI' OFFSET. (SEC)
!       3. D1950   -  THE JULIAN DATE OF THE EPOCH JAN 1, 1950 0 HR UT.
!       4. ECCEN   -  ECCENTRICITY OF THE HELIOCENTRIC ORBIT OF THE EARTH-MOON
!                     BARYCENTER. (UNITLESS)
!       5. XL(2)   -  COEFFICIENTS FOR SUN'S MEAN LONGITUDE. (RAD,RAD/SEC)
!       6. XM(2)   -  COEFFICIENTS FOR EARTH-MOON BARYCENTER MEAN ANOMALY.
!                     (RAD,RAD/SEC)
!       7. D(2)    -  COEFFICIENTS FOR MEAN ELONGATION OF THE MOON FROM THE SUN.
!                     (RAD,RAD/SEC)
!       8. XLLJ(2) -  COEFFICIENTS FOR MEAN ELONGATION OF THE SUN FROM JUPITER.
!                     (RAD,RAD/SEC)
!       9. XLLSA(2)-  COEFFICIENTS FOR MEAN ELONGATION OF THE SUN FROM SATURN.
!                     (RAD,RAD/SEC)
!      10. XMJ(2)  -  COEFFICIENTS FOR JUPITER'S MEAN ANOMALY. (RAD,RAD/SEC)
!      11. XMSA(2) -  COEFFICIENTS FOR SATURN'S MEAN ANOMALY. (RAD,RAD/SEC)
!      12. ACT(15) -  AMPLITUDES OF TERMS IN CT-AT. (SEC)
!
      Real*8           PI, TWOPI, HALFPI, CONVD, CONVDS, CONVHS, SECDAY
      COMMON / CMATH / PI, TWOPI, HALFPI, CONVD, CONVDS, CONVHS, SECDAY
!          VARIABLES 'FROM':
!            1. SECDAY  -  THE CONVERSION FACTOR OF COORDINATE TIME SECONDS PER
!                          COORDINATE TIME DAY. (SEC/DAY)
!
      INCLUDE 'ccon.i'
!          VARIABLES 'FROM':
!            1. KCTIC  -  THE CTIMG UTILITY ROUTINE FLOW CONTROL FLAG.
!            2. KCTID  -  THE CTIMG UTILITY ROUTINE DEBUG OUTPUT FLAG.
!
! 1.2.3 PROGRAM SPECIFICATIONS -
!
      Real*8 AT, CFSITE(3,2), SITLON(2), UTC, XJD, CT, DATDCT, DLPGR
!
! 1.2.4 DATA BASE ACCESS -
!          'PUT' VARIABLES:
!            1. CT - THE COORDINATE TIME FRACTION OF THE COORDINATE TIME DAY
!                    AT OBSERVATION SITE #1. (DAYS)
!          ACCESS CODES:
!            1. 'CT SITE1' - THE DATA BASE ACCESS CODE FOR THE COORDINATE TIME
!                            FRACTION OF THE COORDINATE TIME DAY AT SITE #1.
!
! 1.2.5 EXTERNAL INPUT/OUTPUT - POSSIBLE DEBUG OUTPUT
!
! 1.2.6 SUBROUTINE INTERFACE -
!             CALLER SUBROUTINES: DRIVG
!             CALLED SUBROUTINES: DCOS, DSIN, PUT4
!
! 1.2.7 CONSTANTS USED - SECDAY, TWOPI, ATCTEP, D1950, ECCEN,
!                        XL, XM, D, XLLJ, XLLSA, XMJ, XMSA, ACT
!
! 1.2.8 PROGRAM VARIABLES -
!       1. XLE     -  SUN'S MEAN LONGITUDE. (RAD,RAD/SEC)
!       2. XME     -  EARTH-MOON BARYCENTER MEAN ANOMALY. (RAD,RAD/SEC)
!       3. DE      -  MEAN ELONGATION OF THE MOON FROM THE SUN. (RAD,RAD/SEC)
!       4. XLLJE   -  MEAN ELONGATION OF THE SUN FROM JUPITER. (RAD,RAD/SEC)
!       5. XLLSAE  -  MEAN ELONGATION OF THE SUN FROM SATURN. (RAD,RAD/SEC)
!       6. XMJE    -  JUPITER'S MEAN ANOMALY. (RAD,RAD/SEC)
!       7. XMSAE   -  SATURN'S MEAN ANOMALY. (RAD,RAD/SEC)
!       8. ANOMEC  -  THE ECCENTRIC ANOMALY OF THE HELIOCENTRIC ORBIT OF THE
!                     EARTH-MOON BARYCENTER. (RAD)
!       9. SECEP   -  THE NUMBER OF SECONDS ELAPSED SINCE THE EPOCH JAN 1, 1950
!                     0HR UT.
!      10. UTL     -  UTC + EAST LONGITUDE OF STATION 1. (RAD)
!      11. U       -  STATION 1 DISTANCE FROM SPIN AXIS. (KM)
!      12. V       -  STATION 1 DISTANCE ABOVE EQUATOR. (KM)
!      13. CTP     -  PERIODIC TERMS IN CT. (SEC)
!
! 1.2.9 PROGRAMMER - DALE MARKHAM   01/17/77
!                    PETER DENATALE 07/18/77
!                    BRUCE SCHUPLER 02/21/78
!                    CHOPO MA       08/06/81
!                    Jim Ryan 89.07.26 Documentation simplified.
!                    Gregg Cooke    90.02.14  Changed name to avoid conflict
!                                             with hp-ux libraries.
!
      Real*8  XLE, XME, DE, XLLJE, XLLSAE, XMJE, XMSAE, ANOMEC,
     .         SECEP, UTL, U, V, CTP
!
!     CTIMG Program Structure
!
!     Compute the number of seconds elapsed since epoch Jan 1, 1950
!     0 hours UT.
      SECEP = ( XJD + AT - D1950 ) * SECDAY
!
!     Compute the astronomical arguments.
!
!     Compute the mean anomaly of the heliocentric orbit of the
!     Earth-Moon barycenter. Moyer Eq. 44
      XME = XM(1) + XM(2) * SECEP
!
!     Compute the eccentric anomaly of the heliocentric orbit of the
!     Earth_moon barycenter.  Moyer Eq. 40
      ANOMEC = XME + ECCEN * DSIN ( XME )
!
!     Compute the sun's mean longitude.  Moyer Eq. 43
      XLE = XL(1) + XL(2) * SECEP
!
!     Compute mean elongation of the Moon from the Sun. Moyer Eq. 45
      DE = D(1) + D(2) * SECEP
!
!     Compute mean elongation of the Sun from Jupiter. Moyer Eq. 46
      XLLJE = XLLJ(1) + XLLJ(2) * SECEP
!
!     Compute mean elongation of the Sun from Saturn. Moyer Eq. 47
      XLLSAE = XLLSA(1) + XLLSA(2) * SECEP
!
!     Compute Jupiter's mean anomaly - approximation for eccentric anomaly
!     Moyer Eq. 48
      XMJE = XMJ(1) + XMJ(2) * SECEP
!
!     Compute Saturn's mean anomaly - approximation for eccentric anomaly
!     Moyer Eq. 49
      XMSAE = XMSA(1) + XMSA(2) * SECEP
!
!     Compute periodic contributions to coordinate time. The Moyer
!     Equation (No. 38) calls for UT1+LAMDA,i.e., UT1 plus the crust
!     fixed site longitude in the following 'UTL' is that quantity.
      UTL = UTC * TWOPI + SITLON(1)
!
!     Compute the station spin axis distance and Z-component in kilometers.
      U = DSQRT( CFSITE(1,1)**2 + CFSITE(2,1)**2 ) / 1.D3
      V = CFSITE(3,1) / 1.D3
!
!     Now evaluate Moyer's Equation # 43 with delta T sub A removed.
      CTP =   ACT(1) * DSIN( ANOMEC ) + ACT(2) * DSIN( DE )
     .      + ACT(3) * U * DSIN( UTL )
     .      + ACT(4) * U * DSIN( UTL - XME )
     .      + ACT(5) * U * DSIN( UTL - 2.D0*XME )
     .      + ACT(6) * U * DSIN( UTL + 2.D0*XLE )
     .      + ACT(7) * U * DSIN( UTL + 2.D0*XLE + XME )
     .      + ACT(8) * U * DSIN( UTL - DE ) + ACT(9) * V * DCOS( XLE )
     .      + ACT(10) * DSIN( XMJE ) + ACT(11) * DSIN( XMSAE )
     .      + ACT(12) * DSIN( XLLJE ) + ACT(13) * DSIN( XLLSAE )
     .      + ACT(14) * U * DSIN( UTL + XLLJE )
     .      + ACT(15) * U * DSIN( UTL + XLLSAE )
!
!     Compute coordinate time depending on flow control.
      IF (KCTIC.EQ.0) CT = AT + (ATCTEP + CTP) / SECDAY
      IF (KCTIC.EQ.1) CT = AT
      IF (KCTIC.EQ.2) CT = AT + ATCTEP / SECDAY
!
!     Compute the partial derivative of the long period term in
!     'AT minus CT' offset with respect to the coordinate time.
!     The long period terms are those without any diurnal signature,
!     specifically those withut UTL. The leading (-) is required
!     since the series above defines the long period terms for
!     'CT minus AT'.
!
      DLPGR = - ( ACT(1) * DCOS ( ANOMEC )
     .          * ( 1.D0  +  ECCEN * DCOS ( XME ) ) * XM(2)
     .        + ACT(2) * DCOS ( DE ) * D(2)
     .        - ACT(9) * V * DSIN( XLE ) * XL(2)
     .        + ACT(10) * DCOS( XMJE ) * XMJ(2)
     .        + ACT(11) * DCOS( XMSAE ) * XMSA(2)
     .        + ACT(12) * DCOS( XLLJE ) * XLLJ(2)
     .        + ACT(13) * DCOS( XLLSAE ) * XLLSA(2) )
!
      IF( KCTIC.NE.0) DLPGR = 0.D0
!
!     Compute the partial derivative of the atomic time with respect to
!     the coordinate time.  The (+) reverses the sign of DLPGR.
      DATDCT = 1.D0  +  DLPGR
!
!     PUT the coordinate time fracton of the coordinate time day into
!     the database.
      CALL PUT4 ('CT SITE1      ', CT, int2(1), int2(1), int2(1))
!
!     Check KCTID for debug output.
      IF ( KCTID .EQ. 0 )  GO TO 1100
      WRITE ( 6, 9)
    9 FORMAT(1X, "Debug output for subroutine CTIME." )
      WRITE(6,8)' XL      ',XL
    8 FORMAT(A,4D25.16/(7X,5D25.16))
      WRITE(6,8)' XLE     ',XLE
      WRITE(6,8)' XM      ',XM
      WRITE(6,8)' XME     ',XME
      WRITE(6,8)' D       ',D
      WRITE(6,8)' DE      ',DE
      WRITE(6,8)' XLLJ    ',XLLJ
      WRITE(6,8)' XLLJE   ',XLLJE
      WRITE(6,8)' XLLSA   ',XLLSA
      WRITE(6,8)' XLLSAE  ',XLLSAE
      WRITE(6,8)' XMJ     ',XMJ
      WRITE(6,8)' XMJE    ',XMJE
      WRITE(6,8)' XMSA    ',XMSA
      WRITE(6,8)' XMSAE   ',XMSAE
      WRITE(6,8)' ECCEN   ',ECCEN
      WRITE(6,8)' ACT     ',ACT
      WRITE(6,8)' ANOMEC  ',ANOMEC
      WRITE(6,8)' CTP     ',CTP
      WRITE(6,8)' D1950   ',D1950
      WRITE(6,8)' SECDAY  ',SECDAY
      WRITE(6,8)' SECEP   ',SECEP
      WRITE(6,8)' UTL     ',UTL
      WRITE(6,8)' U       ',U
      WRITE(6,8)' V       ',V
      WRITE(6,8)' A1TAI   ',A1TAI
      WRITE(6,8)' ATCTEP  ',ATCTEP
      WRITE ( 6, 9200 )  AT, CFSITE(1,1), CFSITE(2,1), SITLON(1),
     .                        UTC, XJD, CT, DATDCT, DLPGR
 9200 FORMAT (1X, "AT     = ", D30.16, /, 1X,
     .            "CFSITE(1,1) = ", D30.16, /, 1X,
     .            "CFSITE(2,1) = ", D30.16, /, 1X,
     .            "SITLON (1) = ", D30.16, /, 1X,
     .            "UTC    = ", D30.16, /, 1X,
     .            "XJD    = ", D30.16, /, 1X,
     .            "CT     = ", D30.16, /, 1X,
     .            "DATDCT = ", D30.16, /, 1X,
     .            "DLPGR  = ", D30.16 )
!
!   10.   NORMAL PROGRAM CONCLUSION.
!
 1100 RETURN
      END
!**********************************************************************
      BLOCK DATA CTICMB
      IMPLICIT None
!
! 7.    CTICM
!
! 7.1   CTICM PROGRAM SPECIFICATION
!
! 7.1.1 CTICM IS THE CTIMG UTILITY BLOCK DATA INPUT AND INITIALIZATION SECTION.
!
! 7.1.3 REFERENCES  -  MOYER, T. D., TRANSFORMATION FROM PROPER TIME ON
!                      EARTH TO COORDINATE TIME IN SOLAR SYSTEM BARYCENTRIC
!                      SPACE-TIME FRAME OF REFERENCE, CEL. MECH. 23,
!                      33-68, 1981.  SEE PP. 66-67.
!
! 7.2.2 COMMON BLOCKS - CTICM
      Real*8           A1TAI, ATCTEP, D1950, ECCEN, XL(2), XM(2),
     .       D(2), XLLJ(2), XLLSA(2), XMJ(2), XMSA(2), ACT(15)
      COMMON / CTICM / A1TAI, ATCTEP, D1950, ECCEN, XL,    XM,
     .       D,    XLLJ,    XLLSA,    XMJ,    XMSA,    ACT
!
!     VARIABLES 'TO':
!       1. ATCTEP   -  THE VALUE OF THE 'CT MINUS TAI' OFFSET. (SEC)
!       2. D1950    -  THE JULIAN DATE OF THE EPOCH JAN 1, 1950 0 HR UT.
!       3. ECCEN    -  ECCENTRICITY OF THE HELIOCENTRIC ORBIT OF THE EARTH-MOON
!                      BARYCENTER. (UNITLESS)
!       4. XL(2)    -  COEFFICIENTS FOR SUN'S MEAN LONGITUDE  (RAD,RAD/SEC)
!       5. XM(2)    -  COEFFICIENTS FOR EARTH-MOON BARYCENTER MEAN ANOMALY.
!                      (RAD,RAD/SEC)
!       6. D(2)     -  COEFFICIENTS FOR MEAN ELONGATION OF THE MOON FROM THE
!                      SUN. (RAD,RAD/SEC)
!       7. XLLJ(2)  -  COEFFICIENTS FOR MEAN ELONGATION OF THE SUN FROM JUPITER.
!                      (RAD,RAD/SEC)
!       8. XLLSA(2) -  COEFFICIENTS FOR MEAN ELONGATION OF THE SUN FROM SATURN.
!                      (RAD,RAD/SEC)
!       9. XMJ(2)   -  COEFFICIENTS FOR JUPITER'S MEAN ANOMALY. (RAD,RAD/SEC)
!      10. XMSA(2)  -  COEFFICIENTS FOR SATURN'S MEAN ANOMALY. (RAD,RAD/SEC)
!      11. ACT(15)  -  AMPLITUDES OF TERMS IN CT-AT, ORDERED AS ON P. 66 OF
!                      REFERENCE. (SEC)
!     VARIABLES 'PASSED' FROM OTHER MODULE SECTIONS:
!       1. A1TAI  -  (A1 - TAI) FROM THE DATA BASE. (SEC)
!
! 7.2.3 PROGRAM SPECIFICATIONS -
!
      DATA ATCTEP / 32.184D0 /
      DATA D1950 / 2433282.5D0 /
      DATA ECCEN / 0.01672D0 /
      DATA XL    / 4.888339D0, 1.99106383D-7 /
      DATA XM    / 6.248291D0, 1.99096871D-7 /
      DATA D     / 2.518411D0, 2.462600818D-6 /
      DATA XLLJ  / 5.652593D0, 1.82313637D-7 /
      DATA XLLSA / 2.125474D0, 1.92339923D-7 /
      DATA XMJ   / 5.286877D0, 1.6785063D-8 /
      DATA XMSA  / 1.165341D0, 0.6758558D-8 /
      DATA ACT   / 1.658D-3, 1.548D-6,
     .             3.17679D-10,
     .             5.312D-12,
     .             1.00D-13,
     .            -1.3677D-11,
     .            -2.29D-13,
     .             1.33D-13, -1.3184D-10,
     .             5.21D-6, 2.45D-6,
     .             20.73D-6, 4.58D-6,
     .             1.33D-13,
     .             2.9D-14 /
!
! 7.2.4 CONSTANTS USED - ATCTEP, D1950, ECCEN, XL, XM, D,
!                        XLLJ, XLLSA, XMJ, XMSA, ACT
!
! 7.2.5 PROGRAMMER - BRUCE SCHUPLER  01/07/80
!                    CHOPO MA        08/06/81
!
! 7.3   CTICM PROGRAM STRUCTURE - NONE
!
      END
