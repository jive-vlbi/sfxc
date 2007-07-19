      SUBROUTINE AXOA()
      IMPLICIT None
!
! 1.    AXOA
!
! 1.1   AXOA PROGRAM SPECIFICATION
!
! 1.1.1 AXOA adds entries to the table of contents for the Axis Offset
!       text message, the axis offset flow control message, the axis
!       offset partial derivatives and contributions, the feedbox
!       rotation routine test message, the feedbox rotation angles, and
!       the group and phase delay rate corrections.
!
! 1.2   AXOA PROGRAM INTERFACE
!
! 1.2.4 DATA BASE ACCESS -
!           ACCESS CODES ADDED:
!              1.  'AXO MESS'  -  THE DATA BASE ACCESS CODE FOR THE
!                                 AXIS OFFSET MODULE TEXT MESSAGE.
!              2.  'AXO CFLG'  -  THE DATA BASE ACCESS CODE FOR THE AXIS
!                                 OFFSET CONTROL FLAG MESSAGE.
!              3.  'AXO PART'  -  THE DATA BASE ACCESS CODE FOR THE AXIS
!                                 OFFSET MODULE PARTIAL DERIVATIVES ARRAY.
!                                 (Using new, simple axis offset model)
!              4.  'AXO CONT'  -  The database access code for the new axis
!                                 offset module contributions array.
!                                 (Using the new, simple axis offset model.)
!              7.  'PAN MESS'  -  The database access code for the
!                                 feedhorn rotation routine text message.
!              8.  'PARANGLE'  -  The database access code for the
!                                 feedhorn rotation angles.
!              9.  'FEED.COR'  -  The database access code for the feedbox
!                                 rotation corrections for group delay and
!                                 phase delay rate.
!
! 1.2.6 SUBROUTINE INTERFACE -
!             CALLER SUBROUTINES: TOCUP
!             CALLED SUBROUTINES: ADDA, ADDR
!
! 1.2.9 PROGRAMMER - DALE MARKHAM  01/13/77
!                    PETER DENATALE 07/11/77
!                    BRUCE SCHUPLER 09/16/77
!                    SAVITA GOEL    06/03/87 (CDS FOR A900)
!                    Jim Ryan 86.29 Character stings used.
!                    Jim Ryan 89.12.12 UNIX-like database interface
!                    implemented.
!                    David Gordon 94.03.25 AXO2CONT and AXIS_OLD Lcodes added.
!                    David Gordon 94.07.22 Changed 'AXIS_OLD' to 'AXIS OLD' to
!                                 keep Chopo happy.
!                    David Gordon 98.11.12 Merged in code from subroutine
!                                 PANA for feedbox rotation stuff. 'AXIS OLD'
!                                 contribution removed.
!                    David Gordon 98.12.30 FEED.COR changed from (2,2) to
!                                 (2,1) dimensions.
!                    Jim Ryan 03.03.10 Kill replaced with terminate_solve
!                    Jim Ryan 02.Sept Integer*2/4 updates.
!                    D. Gordon 2004.05.19 Removed Lcode AXO2CONT, Addded Tilt
!                                 removing Lcode TILTRMVR.
!
! 1.3   AXOA PROGRAM STRUCTURE
!
!   ADD for axis offset module text message.
      CALL ADDA (int2(1),'AXO MESS','Axis Offset Message Definition  ',
     . int2(40), int2(1), int2(1))
!
!   ADD for axis offset module control flag message.
      CALL ADDA (int2(1),'AXO CFLG','Axis Offset Control flag mes def',
     . int2(40), int2(1), int2(1))
!
!   ADD for axis offset partial derivatives.
      CALL ADDR (int2(2),'AXO PART','Axis Offset partial deriv. def. ',
     . int2(2), int2(2), int2(1))
!
!   ADD for new axis offset contributions (one-term formula).
      CALL ADDR (int2(2),'AXO CONT','New Axis Offset Contributions   ',
     . int2(2), int2(2), int2(1))
!
!   Delete for incorrect Calc 9 contribution
       CALL DELR ( int2(2), 'AXO2CONT')
!
!     ADDA for feedbox rotation (paralactic angle) text message.
      CALL ADDA (int2(1),'PAN MESS','Feedhorn rot. angle mod. ident. ',
     . int2(40), int2(1), int2(1))
!
!     ADD for feedbox rotation angle (paralactic angle).
      CALL ADDR (int2(2),'PARANGLE','Feedhorn rot. angle, ref and rem',
     . int2(2), int2(1), int2(1))
!
!     ADD for feedbox corrections for standard observables.
      CALL ADDR (int2(2),'FEED.COR','Feedhorn corr. in CORFIL scheme ',
     . int2(2), int2(1), int2(1))
!
!     ADD for corrections to remove fixed axis tilt corrections.
      CALL ADDR (int2(2),'TILTRMVR','Axis Tilt Contribution Remover  ',
     . int2(2), int2(1), int2(1))
!
      RETURN
      END
!***********************************************************************
      SUBROUTINE AXOI()
      IMPLICIT None
!
! 2.    AXOI
!
! 2.1   AXOI PROGRAM SPECIFICATION
!
! 2.1.1 AXOI is the Axis Offset and Feedbox Rotation Module input and
!       initialization section.
!
! 2.2   AXOI PROGRAM INTERFACE
!
! 2.2.2 COMMON BLOCKS USED -
!
       INCLUDE 'ccon.i'
!      VARIABLES 'FROM'
!        1. KAXOC - THE AXIS OFFSET MODULE FLOW CONTROL FLAG
!                   = 0 ==> Axis offset corrections applied to theoreticals.
!                   = 1 ==> Axis offset corrections NOT applied to theoreticals.
!        1.  KPANC  -  The Feedbox Rotation flow control flag.
!
! 2.2.3 PROGRAM SPECIFICATIONS -
      INTEGER*2       LAXOM(40),      LON(40),    LOFF(40)
      CHARACTER*40  C_LAXOM(2),     C_LON(2),   C_LOFF(2)
      EQUIVALENCE(  C_LAXOM,LAXOM),(C_LON,LON),(C_LOFF,LOFF)
      INTEGER*2      LPANG_ON(40),  LPANG_OF(40)
      CHARACTER*40 C_LPANG_ON(2), C_LPANG_OF(2)
      EQUIVALENCE (  LPANG_ON, C_LPANG_ON)
      EQUIVALENCE (  LPANG_OF, C_LPANG_OF)
!
      DATA C_LAXOM /
     .'Axis Offset Module - Last modified 2004.',
     .'05.19, D. Gordon/GSFC.                  '/
      DATA C_LON  /
     .'Axis Offset Module is turned ON in CALC.',
     .'                                        '/
      DATA C_LOFF /
     .'Axis Offset Module is turned OFF in CALC',
     .'.                                       '/
!
      DATA C_LPANG_ON  /
     .'Feedbox Rotation Angle Module, Last Modi',
     .'fied 98NOV12, D. Gordon/GSFC.           '/
      DATA C_LPANG_OF  /
     .'Feedbox Rotation angle zeroed out.      ',
     .'                                        '/
!
! 2.2.4 DATA BASE ACCESS -
!           'PUT' VARIABLES:
!              1. LAXOM(40)    - THE AXIS OFFSET MODULE TEXT MESSAGE.
!              2. LON(40)      - THE AXIS OFFSET MODULE TURNED ON MESSAGE
!              3. LOFF(40)     - THE AXIS OFFSET MODULE TURNED OFF MESSAGE.
!              4. LPANG_ON(40) - The feedbox rotation (parallactic) angle
!                                text message.
!              5. LPANG_OF(40) - The feedbox rotation (parallactic) angle
!                                turned OFF message.
!           ACCESS CODES PUT:
!              1. 'AXO MESS'   -  THE DATA BASE ACCESS CODE FOR THE
!                                 AXIS OFFSET MODULE TEXT MESSAGE.
!              2. 'AXO CFLG'   -  THE DATA BASE ACCESS CODE FOR THE AXIS
!                                 OFFSET MODULE FLOW CONTROL MESSAGE.
!              3. 'PAN MESS'   -  The data base access code for the feedbox
!                                 rotation text message.
!
! 2.2.6 SUBROUTINE INTERFACE -
!             CALLER SUBROUTINES: INITL
!             CALLED SUBROUTINES: PUTA
!
! 2.2.9 PROGRAMMER - DALE MARKHAM  01/13/77
!                    PETER DENATALE 07/11/77
!                    BRUCE SCHUPLER 03/07/78
!                    Jim Ryan 89.06.29 Character stings used.
!                    Jim Ryan 89.12.12 UNIX-like database interface
!                             implemented.
!                    D. Gordon 98.11.12 Merged in code from subroutine PANI,
!                             the feedbox rotation (parallactic) angle
!                             initialization routine.
!                    Jim Ryan 02.Sept Integer*2/4 updates.
!                    D.Gordon 2004.05.19 Calc 10 updates.
!
! 2.3   AXOI PROGRAM STRUCTURE
!
!     Put Axis Offset Module text message in database
      CALL PUTA ( 'AXO MESS      ', LAXOM, int2(40), int2(1), int2(1))
!
!     Put Axis Offset Module flow control message into database header.
      IF (KAXOC .NE. 1) CALL PUTA('AXO CFLG      ',LON,int2(40),
     .                  int2(1),int2(1))
      IF (KAXOC .EQ. 1) CALL PUTA('AXO CFLG      ',LOFF,int2(40),
     .                  int2(1),int2(1))
!
!     PUT for the feedbox rotation (paralactic) angle routine text message.
      IF (KPANC .NE. 1) CALL PUTA ('PAN MESS      ',LPANG_ON,int2(40),
     .                  int2(1), int2(1))
      IF (KPANC .EQ. 1) CALL PUTA ('PAN MESS      ',LPANG_OF,int2(40),
     .                  int2(1), int2(1))
!
!     Normal conclusion.
      RETURN
      END
!************************************************************************
      SUBROUTINE AXOG (KAXIS, R2K, SITLAT, STAR, TCTOCF, SITEV, AXOFF,
     .           EARTH, AZ, ELEV, STAR_ABERRATED, SITHEIGHT, AXTILT,
     .           ROTAXIS, axis2000, daxis2000 )
      IMPLICIT None
!
! 3.    AXOG
!
! 3.1   AXOG PROGRAM SPECIFICATION
!
! 3.1.1 AXOG is the geometry section of the Axis Offset Module. AXOG computes
!       the 3 dimensional axis offset vector and its time derivative corrected
!       for aberration and atmospheric refraction at each site.
!       Beginning with Calc 9.0, AXOG also computes the feedbox rotation
!       angle and the group delay and phase delay corrections, using a new
!       algorithm that replaces subroutine PANG.
!
! 3.1.2 REFERENCES - NINER, EDGAR P., 'MEMORANDUM FOR RECORD', 25 OCTOBER
!                    1967 (REFRACTION CORRECTION).
!                    SMART, W.M., "TEXTBOOK ON SPHERICAL ASTRONOMY", 1965,
!                    P. 68.
!                    Gordon, David, "Axis offset VLBI delay correction",
!                    1994 March 7 memo, updated 1994 May 5.
!
! 3.2   AXOG PROGRAM INTERFACE
!
! 3.2.1 CALLING SEQUENCE -
!          INPUT VARIABLES:
!             1. KAXIS(2)       -  THE ANTENNA AXIS TYPES AT EACH OBSERVATION
!                                  SITE. (UNITLESS)
!             2. R2K(3,3,3)     -  THE ROTATION MATRIX WHICH ROTATES THE CRUST
!                                  FIXED REFERENCE FRAME TO THE J2000.0 FRAME
!                                  AND ITS FIRST TWO CT TIME DERIVATIVES.
!                                  (UNITLESS, 1/SEC, 1/SEC**2)
!             3. SITLAT(2)      -  THE GEODETIC LATITUDES AT EACH OBSERVATION
!                                  SITE. (RAD)
!             4. STAR(3)        -  THE J2000.0 SOURCE UNIT VECTOR. (UNITLESS)
!             5. TCTOCF(3,3,2)  -  THE ROTATION MATRIX WHICH ROTATES THE
!                                  TOPOCENTRIC REFERENCE SYSTEM TO THE CRUST
!                                  FIXED GEOCENTRIC REFERENCE SYSTEM AT EACH
!                                  OBSERVATION SITE. (UNITLESS)
!             6. SITEV(3,2)     -  THE J2000.0 GEOCENTRIC VELOCITY VECTORS OF
!                                  EACH OBSERVATION SITE. (M/SEC)
!             7. AXOFF(2)       -  THE ANTENNA AXIS OFFSETS AT EACH OBSERVATION
!                                  SITE. (M)
!             8. EARTH(3,3)     -  THE SSBC EARTH POSITION, VELOCITY, AND
!                                  ACCELERATION VECTORS. (M, M/SEC, M/SEC**2)
!             9. ELEV(2,2)      -  The elevation angle of the source corrrected
!                                  for aberration and its CT time derivative at
!                                  each site (rad,rad/sec)
!            10. AZ(2,2)        -  The azimuth angle of the source corrrected
!                                  for aberration and its CT time derivative
!                                  at each site (rad,rad/sec)
!            11. STAR_ABERRATED(3,2) - The J2000 source unit vector corrected
!                                  for annual and diurnal aberration at each
!                                  site. (unitless)
!            12. SITHEIGHT(2)   -  Station heights above the geoid. (meters)
!            13. AXTILT(2,2)    -  Antenna fixed axis tilts (arc-seconds).
!                                  First index runs over the two orthogonal
!                                  tilt directions (Alt-Az: 1 => East,
!                                  2 => North; (X-Y (N-S or E-W fixed) and
!                                  Equatorial: 1 => Az error, 2 => Elev error).
!                                  Second index runs over the two stations.
!            14. ROTAXIS(3,3,2) -  Topocentric rotation matrices representing
!                                  the fixed axis tilts for station 1 and
!                                  station 2 of the current observation.
!
!          OUTPUT VARIABLES:
!             1. axis2000(3,2) -  Vector axis offset of antenna in the J2000.0
!                                 frame (effect on baseline). First index is
!                                 X,Y,Z (meters), second runs over sites.
!             2. daxis2000(3,2) - Time derivative of axis2000, rate of change
!                                 of vector axis offset of antenna in the
!                                 J2000.0 frame (effect on baseline). First
!                                 index is velocity, second runs over sites.
!
! 3.2.2 COMMON BLOCKS USED -
!
      Real*8  DAXOP(2,2), DCOMP(2,2), RICHM(2),
     .        Tpartl(2), uaxis2000(3,2), duaxis2000(3,2)
      COMMON / AXOCM / DAXOP, DCOMP, RICHM,
     .                 Tpartl, uaxis2000, duaxis2000
!         VARIABLES 'TO':
!            1. DCOMP(2,2) - THE PARTIAL DERIVATIVES OF THE COMPONENT OF THE
!                            ANTENNA AXIS OFFSET IN THE APPARENT DIRECTION OF
!                            THE SOURCE AND THE CT TIME DERIVATIVE OF THAT
!                            COMPONENT WITH RESPECT TO THE ANTENNA AXIS
!                            OFFSETS AT EACH OBSERVATION SITE. THE FIRST
!                            INDEX RUNS OVER THE OBSERVATION SITES AND THE
!                            SECOND INDEX RUNS OVER THE DELAY AND THE DELAY
!                            RATE.  (UNITLESS, 1/SEC)
!            2. RICHM(2)   - THE AXES ORIENTATION FOR THE RICHMOND ANTENNA
!                            SYSTEM. IT IS ORIENTED AS FOR AN EQUATORIAL
!                            MOUNT AT LATITUDE+39.06 DEGREES AND ROTATED 0.12
!                            DEGREES WEST OF NORTH.
!
      INCLUDE 'cphys.i'
!           VARIABLES 'FROM':
!             1. VLIGHT  -  The velocity of light in a vacuum (m/sec).
!
      INCLUDE 'ccon.i'
!       VARIABLES 'FROM':
!         KAXOC - THE AXIS OFFSET MODULE FLOW CONTROL FLAG.
!                  = 0 ==> Axis offset corrections applied to theoreticals.
!                  = 1 ==> Axis offset corrections NOT applied to theoreticals.
!         KAXOD - THE AXIS OFFSET MODULE DEBUG OUTPUT FLAG.
!         KPANC - The feedhorn rotation module flow control flag.
!         KPAND - The feedhorn rotation module debug flag (not used).
!
      Real*8         PI,TWOPI,HALFPI,CONVD,CONVDS,CONVHS,SECDAY
      COMMON /CMATH/ PI,TWOPI,HALFPI,CONVD,CONVDS,CONVHS,SECDAY
!           VARIABLES 'FROM':
!              1. HALFPI - THE VALUE OF PI/2
!              2. CONVD  - THE CONVERSION FACTOR FROM DEGREES TO RADIANS
!                          (RAD/DEG)
!
      INCLUDE 'cobsn.i'
!          Variables from:
!            1. Nzero  -  Set to 1 or 2 if station 1 or 2 is at the geocenter,
!                         otherwise equals zero. For correlator usage.
!       DATA BASE ACCESS -
!            'PUT' VARIABLES:
!               1. PANGL(2) - Feedbox rotation angles at site 1 and 2 in
!                             degrees of phase.
!             ACCESS CODES:
!               1. 'PARANGLE' - The database access code for the feedbox
!                               rotation angles.
!               2. 'FEED.COR' - The database access code for the feedbox
!                               rotation correction to the group delay and
!                               the phase delay rate.
!
! 3.2.3 PROGRAM SPECIFICATIONS -
      DATA RICHM /39.06D0, 0.12D0/
      Integer*2 KAXIS(2), NDO(3), KERR
      Integer*4 I, L, LL, kx
      Real*8 CFSTAR(3,2), CFTOTC(3,3), CTCSTR(3), SITEV(3,2),
     .       R2K(3,3,3), SITLAT(2), STAR(3), TCAXIS(3),
     .       TCSTAR(3,2), TCTOCF(3,3,2), TR2K(3,3,2), AXTILT(2,2),
     .       AZ(2,2), ELEV(2,2), STAR_ABERRATED(3,2), SITHEIGHT(2),
     .       ZC, ZEN,        AZMUTH, ZENCOR, REF_FREQ, ROTAXIS(3,3,2)
      Real*8 axistc(3,2), daxistc(3,2), axiscf(3,2), daxiscf(3,2),
     .       AXOFF(2), xnormal, ccfstr(3), dccfstr(3), vectr(3),
     .       daxocnew(2,2), EARTH(3,3), c2000str(3), axis2000(3,2),
     .       daxis2000(3,2), dctcstr(3), vec1(3), dvec1(3), absvec1,
     .       dabsvec1, vec2(3), vec3(3)
      Real*8 VECMG, DOTP, AMAG, TCAXnew(3), Axdiff(3), vec1d(3),
     .       axistcd(3,2), axiscfd(3,2), uaxisd2k(3,2), TCAXold(3),
     .       absvecd
      Real*8 zen_K, tanel_K, azmuth_K, zencor_K
      Real*8 El_Rad, Sithit, Temp_K, X, Press_Hg, Humid_F, sbend
      Real*8 TNCP(3), ZA, DZA, DAZM, RANGL, DRANGL, PANGL(2),
     .       FEED_COR(2,2), FCONT(2)
      Real*8 VG(3), VE(3), TT
!
! 3.2.6 SUBROUTINE INTERFACE -
!          CALLER SUBROUTINES: DRIVG
!          CALLED SUBROUTINES: DCOS, DSIN, DTAN, MTRAN, VECRT, VECMG, VECAD,
!                              DOTP, CROSP, VECAD, FBOX
!
! 3.2.8 PROGRAM VARIABLES -
!           1. TR2K(3,3,2)    -  THE COMPLETE J2000.0 TO CRUST FIXED
!                                ROTATION MATRIX AND ITS FIRST CT
!                                TIME DERIVATIVE. (UNITLESS, 1/SEC)
!           2. CFTOTC(3,3)    -  THE 3x3 ROTATION MATRIX WHICH ROTATES
!                                THE GEOCENTRIC CRUST FIXED REFERENCE
!                                SYSTEM TO THE TOPOCENTRIC REFERENCE
!                                SYSTEM. (UNITLESS)
!           3. CFSTAR(3,2)    -  THE SOURCE UNIT VECTOR IN THE CRUST FIXED
!                                GEOCENTRIC REFERENCE SYSTEM CORRECTED FOR
!                                ABERRATION AND ITS CT TIME DERIVATIVE.
!                                (UNITLESS, 1/SEC)
!           4. TCSTAR(3,2)    -  THE SOURCE UNIT VECTOR IN THE TOPOCENTRIC
!                                SYSTEM CORRECTED FOR ABERRATION AND ITS CT
!                                TIME DERIVATIVE. (UNITLESS, 1/SEC)
!           5. CTCSTR(3)      -  THE SOURCE UNIT VECTOR IN THE TOPOCENTRIC
!                                REFERENCE SYSTEM CORRECTED FOR ABERRATION
!                                AND ATMOSPHERIC REFRACTION. (UNITLESS)
!           6. dctcstr(3)     -  Derivative of CTCSTR (above).
!           7. ccfstr(3)      -  Aberrated/refracted source unit vector in the
!                                crust fixed frame.
!           8. dccfstr(3)     -  Time derivative of ccfstr (above).
!           9. c2000str(3)    -  Aberrated/refracted source unit vector in the
!                                J2000.0 frame.
!          10. AZMUTH         -  THE AZIMUTH OF THE TOPOCENTRIC SOURCE UNIT
!                                VECTOR CORRECTED FOR ABERRATION. (RAD)
!          12. ZEN            -  THE ZENITH ANGLE OF THE SOURCE UNIT VECTOR.
!                                (RADIANS)
!          13. ZENCOR         -  THE CORRECTION ANGLE TO THE SOURCE UNIT VECTOR
!                                DUE TO ATMOSPHERIC REFRACTION. (RADIANS)
!          14. TCAXIS(3)      -  THE UNIT VECTOR REPRESENTING THE ANTENNA FIXED
!                                AXIS IN THE TOPOCENTRIC SYSTEM. (UNITLESS)
!          16. axistc(3,2)    -  Vector axis offset of the antenna in the
!                                topocentric frame (effect on baseline). First
!                                index is X,Y,Z position (meters); second runs
!                                over the sites.
!          17. daxistc(3,2)   -  Time derivative of axistc, rate of change of
!                                vector axis offset of antenna in the
!                                topocentric frame (effect on baseline). First
!                                index is velocity, second runs over sites.
!          18. axiscf(3,2)    -  Vector axis offset of the antenna in the
!                                geocentric frame (effect on baseline). First
!                                index is X,Y,Z position (meters); second runs
!                                over the sites.
!          19. daxiscf(3,2)   -  Time derivative of axiscf, rate of change of
!                                vector axis offset of the antenna in the
!                                geocentric frame (effect on baseline). First
!                                index is velocity, second runs over sites.
!          20. uaxis2000(3,2) -  Unit vector axis offset of antenna in the
!                                J2000.0 frame. First index is X,Y,Z, second
!                                runs over sites. (unitless) [See axis2000]
!          21. duaxis2000(3,2) - Time derivative of uaxis2000, rate of change
!                                of unit vector axis offset in the J2000.0
!                                frame. First index is velocity, second runs
!                                over sites. (unitless) [See daxis2000]
!          22. El_Rad          - Source elevation for use by function Sbend to
!                                compute the atmospheric bending. (radians
!          23. Sithit          - Station height above the geoid, for use in
!                                computing atmospheric bending. (meters)
!          24. Temp_K          - A priori value for absolute temperature, for
!                                use in computing atmospheric bending. (Kelvins)
!          25. Press_Hg        - A priori value for atmospheric pressure, for
!                                use in computing atmospheric bending. (mm of
!                                mercury)
!          26. Humid_F         - A priori value for station humidity (.5).
!          27. X               - Variable related to station height, for use in
!                                computing atmospheric bending.
!          28. PANGL(2)        - The clock-wise feed box rotation angles for
!                                stations 1 and 2. (Degrees)
!          29. FEED_COR(2,2)   - The corrections to group delay and phase
!                                delay rate due to feedbox rotation. First
!                                index runs over the sites, second index runs
!                                over delay and rate. Sec, Sec/sec)
!
! 3.2.9 PROGRAMMER - DALE MARKHAM  01/13/77
!                    PETER DENATALE 07/11/77
!                    DOUG ROBERTSON ??/??/84 (RICHMOND AXIS CODING)
!                    DAVID GORDON   08/14/84 (RICHMOND)
!                    Jim Ryan 89.06.29 Character stings used.
!                    David Gordon  June - Aug, 1993 Modified to compute 3-D
!                                  axis offset vector; Aberrated star vector,
!                                  and altitude and azimuth from subroutine
!                                  ATMG added; new computation of DCOMP.
!                    David Gordon  93.12.23 Corrected division by zero problem
!                                  in DCOMP.
!                    David Gordon  94.03.08 New axis offset delay computation.
!                    David Gordon  94.03.25 Second (simpler) new axis offset
!                                  delay computation.
!                    David Gordon  94.09.21 Added function Sbend to compute
!                                  atmospheric bending. Sbend comes from the
!                                  Mark III Field System. Changed computation
!                                  of second new axis offset contribution to
!                                  combine aberration and bending. SITHEIGHT(2)
!                                  added to subroutine input arguments.
!                    David Gordon  94.10.20 Changed computation of second axis
!                                  offset correction to apply aberration to
!                                  vacuum part, but not to aberrated/refracted
!                                  part.
!                    David Gordon  94.10.24 Code cleanup.
!                    David Gordon  98.08.05 Mods for geocenter station.
!                    David Gordon  98.11.12 Code added to call the new feedbox
!                                  rotation subroutine, FBOX, and to do PUT's
!                                  of the rotation angles and the group delay
!                                  and phase delay rate corrections. Code to
!                                  GET Reference frequency added.
!                    David Gordon  98.12.30 FEED.COR access code changed from
!                                  1 and 2 combined.
!                    Jim Ryan 02.Sept Integer*2/4 updates.
!                    D. Gordon     2004 May. Axis tilt and other Calc10 mods.
!                    D. Gordon     2004 Oct. Corrected delay and partial
!                                  derivative.
!
! 3.3   AXOG PROGRAM STRUCTURE
!
!   Get the reference frequency for use in the phase delay rate corrections.
!    Convert from MHz to Hz.
      CALL GET4('REF FREQ      ',REF_FREQ,int2(1),int2(1),int2(1),NDO,
     . KERR)
      IF(KERR.NE.0) then
        write(6,'("AXOG: Failure to obtain ref frequency.")')
        CALL TERMINATE_CALC( 'AXOG  ', int2(1), KERR)
      Endif
      REF_FREQ = REF_FREQ*1.D6
!
!
!    Loop through twice for the calculation of the axis offset vectors and
!    their time derivatives and the partial derivatives of the component of
!    the antenna axis offsets in the apparent direction of the source and the
!    time derivative of that component with respect to the antenna axis
!    offsets at each site.
!
      DO 800  L = 1,2
           LL = L
!
!  Check for geocenter station
      IF (L .eq. Nzero) Go to 750
!
!    Identify the unit vector representing the antenna fixed axis in a
!    topocentric reference system. The topocentric reference system sits on the
!    Earth's surface at the observation site with axes pointing 1) radially Up
!    (X-AXIS), 2) East (Y-axis), and 3) North (Z-axis).
!
!  Identify the antenna axis type in order to obtain the correct algorithm:
!
!    Equatorial mount - fixed axis points at the North Celestial Pole.
         If (KAXIS(L) .EQ. 1) Then
           TCAXIS(1) = DSIN ( SITLAT(L) )
           TCAXIS(2) = 0.D0
           TCAXIS(3) = DCOS ( SITLAT(L) )
           GO TO 310
         Endif
!
!    X-Y mount - antenna fixed axis points North-South. (i.e. the antenna fixed
!      axis is in the plane of the horizon pointed North-South.)
         If (KAXIS(L) .EQ. 2) Then
           TCAXIS(1) = 0.D0
           TCAXIS(2) = 0.D0
           TCAXIS(3) = 1.D0
           GO TO 310
         Endif
!
!    Alt-Az mount - the antenna fixed axis points at the zenith.
         If (KAXIS(L) .EQ. 3) Then
           TCAXIS(1) = 1.D0
           TCAXIS(2) = 0.D0
           TCAXIS(3) = 0.D0
           GO TO 310
         Endif
!
!    X-Y mount -  antenna fixed axis pointing East-West. (i.e. The antenna
!      fixed axis is in the plane of the horizon pointed East-West.)
         If (KAXIS(L) .EQ. 4) Then
           TCAXIS(1) = 0.D0
           TCAXIS(2) = 1.D0
           TCAXIS(3) = 0.D0
           GO TO 310
         Endif
!
!    Old Richmond antenna system - axes oriented as for an equatorial mount at
!      latitude +39.06 degrees and rotated 0.12 degrees West of North.
         If (KAXIS(L) .EQ. 5) Then
           ZC        =     DCOS(RICHM(1)*CONVD)
           TCAXIS(1) =     DSIN(RICHM(1)*CONVD)
           TCAXIS(2) = -ZC*DSIN(RICHM(2)*CONVD)
           TCAXIS(3) =  ZC*DCOS(RICHM(2)*CONVD)
           GO TO 310
         Endif
!
!    Antenna axis type invalid, a message is written and the program stops.
         IF ((KAXIS(L) .le. 0 ) .OR. (KAXIS(L) .gt. 5)) Then
           WRITE (6, 9300)  LL, KAXIS(LL)
           CALL TERMINATE_CALC ('AXOG  ',int2(0),int2(0))
         Endif
 9300 FORMAT (1X, "THE PROGRAM HAS TERMINATED IN SUBROUTINE AXOG.  ",
     .            'KAXIS (', I2, ') = ', I2, '.' )
!
  310    Continue
!
!   Rotate the fixed axis by the tilt amount.
      CALL VECRT (ROTAXIS(1,1,L), TCAXIS, TCAXnew)
!
!   Replace fixed axis with tilt-corrected fixed-axis.
       Do kx = 1, 3
        TCAXold(kx) = TCAXIS(kx)
        TCAXIS(kx) = TCAXnew(kx)
!       Axdiff(kx) = TCAXnew(kx) - TCAXold(kx)
       Enddo
!
! 310    Continue
!******************************************************************************
!    Rotate the J2000.0 source unit vector to the topocentric reference system.
!
!        Compute the rotation matrix which rotates from the geocentric crust
!        fixed reference system to the topocentric reference system.
           CALL MTRAN ( TCTOCF(1,1,L), CFTOTC )
!
!        Compute the rotation matrix which rotates from the J2000.0
!        reference system to the geocentric crust fixed system.
           CALL MTRAN ( R2K(1,1,1), TR2K(1,1,1) )
!
!        Rotate the aberrated J2000.0 source unit vector to the crust fixed
!        system and then to the topocentric system.
           CALL VECRT ( TR2K(1,1,1),STAR_ABERRATED(1,L),CFSTAR(1,1))
           CALL VECRT ( CFTOTC, CFSTAR(1,1), TCSTAR(1,1) )
!
!*****************************************************************************
!    Correct the aberrated topocentric source unit vector for atmospheric
!    refraction.
!
!   Compute the zenith angle of the aberrated source.
        ZEN =  HALFPI - ELEV(L,1)
!   Compute the azimuth of the aberrated source.
        AZMUTH = AZ(L,1)
!   Compute values needed for function sbend.
        El_Rad = elev(l,1)
        Sithit = Sitheight(l)
        Temp_K = 293.15 - (6.5D-3)*sithit
        X = 1.D0 - (6.5D-3)*sithit/293.15D0
        Press_Hg = 760.D0 * (X**5.26D0)
        Humid_F = .5D0
!   Compute atmospheric bending.
        zencor = sbend(El_rad,Temp_K,Humid_F,Press_Hg)
!
! *************************************************************************
!    Compute the corrected (aberrated/refracted) topocentric star unit
!    vector. (NOTE: This corrects the vector so that it is pointing nearer
!    to the zenith).
          CTCSTR(1) = DCOS ( ZEN - ZENCOR )
          CTCSTR(2) = DSIN ( ZEN - ZENCOR ) * DSIN ( AZMUTH )
          CTCSTR(3) = DSIN ( ZEN - ZENCOR ) * DCOS ( AZMUTH )
!       Make sure we still have a unit vector
          amag = vecmg(CTCSTR)
          do kx = 1,3
           CTCSTR(kx) = CTCSTR(kx)/amag
          enddo
!
!     Compute the CT time derivative of the aberrated source unit vector
!      in the topocentric reference system.
            CALL MTRAN ( R2K(1,1,2), TR2K(1,1,2) )
            CALL VECRT ( TR2K(1,1,2),STAR_ABERRATED(1,L),CFSTAR(1,2))
            CALL VECRT ( CFTOTC, CFSTAR(1,2), TCSTAR(1,2) )
!
! .............................................................................
!   Code for computing the three dimensional components of the axis offset
!    vector.  D. Gordon 93MAY13 thru 93JUL27
!
!  Compute topocentric axis offset vector (change in effective site position
!   due to axis offset orientation) and its time derivative:
!
!   DELAY COMPUTATION:
!    Take triple vector product to get direction of axis offset vector:
        call crosp(TCAXIS,CTCSTR,vectr)
        call crosp(vectr,TCAXIS,vec1)
!    Compute magnitude of vector so we can normalize it
       absvec1 = vecmg(vec1)
!    Normalize to get topocentric unit axis offset vector
       do kx=1,3
        axistc(kx,L) = vec1(kx) / absvec1
       enddo
!
!    Rotate axis offset vector to the crust fixed frame
        call vecrt(TCTOCF(1,1,L),axistc(1,L),axiscf(1,L))
!    Then rotate axis offset vector to the J2000 frame
        call vecrt(R2K(1,1,1),axiscf(1,L),uaxis2000(1,L))
!
!   Derivative of CTCSTR: (aberrated/refracted topocentric source unit vector)
!     Rotate atmosphere corrected aberrated source unit vector to the
!     crust fixed frame:
        call vecrt(TCTOCF(1,1,L),CTCSTR,ccfstr)
!    Then rotate to the J2000 frame:
        call vecrt(R2K(1,1,1),ccfstr,c2000str)
!
!    Take time derivative and rotate back to crust fixed frame
        call vecrt(TR2K(1,1,2),c2000str,dccfstr)
!
!    Rotate source vector time derivative back to topocentric frame
        call vecrt(CFTOTC,dccfstr,dctcstr)
!
!   RATE COMPUTATION:
!    Take derivative of triple vector product (TCAXIS is a constant)
        call crosp(TCAXIS,dctcstr,vectr)
        call crosp(vectr,TCAXIS,dvec1)
!    Derivative of magnitude of vec1
        dabsvec1 = Dotp(vec1,dvec1) / absvec1
!    Compute derivative of unit axis offset vector in topocentric frame
       do kx=1,3
        daxistc(kx,L) = dvec1(kx)/absvec1 -
     .   vec1(kx) * dabsvec1 / absvec1**2
       enddo
!
!    Rotate axis offset time derivative to crust fixed frame
        call vecrt(TCTOCF(1,1,L),daxistc(1,L),daxiscf(1,L))
!
!    Then rotate to J2000 frame:
        call vecrt(R2K(1,1,1),daxiscf(1,L),vec2)
        call vecrt(R2K(1,1,2), axiscf(1,L),vec3)
        call vecad(vec2,vec3,duaxis2000(1,L))
!
!   Convert unit axis offset vector and its derivative to actual axis offset
!     vector and its derivative.
       do kx=1,3
         axis2000(kx,L) =  uaxis2000(kx,L) * AXOFF(L)
        daxis2000(kx,L) = duaxis2000(kx,L) * AXOFF(L)
       enddo
!
!
! Compute offset without axis tilt correction and rotate to J2000.
!  Then compute partial component.
        call crosp(TCAXold,CTCSTR,vectr)
        call crosp(vectr,TCAXold,vec1d)
        absvecd = vecmg(vec1d)
       do kx=1,3
        axistcd(kx,L) = vec1d(kx) / absvecd
       enddo
        call vecrt(TCTOCF(1,1,L),axistcd(1,L),axiscfd(1,L))
        call vecrt(R2K(1,1,1),axiscfd(1,L),uaxisd2k(1,L))
!  Use the Consensus model definition
        DO I =1,3
          VG(I) = EARTH(I,2) + SITEV(I,2)
          VE(I) = EARTH(I,2)
        Enddo
!
         TT = 1.d0 + DOTP(STAR,VG)/VLIGHT
        Tpartl(L) = Dotp(uaxisd2k(1,L),STAR)/TT +
     .              Dotp(uaxis2000(1,L),VE)/VLIGHT
!
!        print *,' Tpartl/new ', Tpartl(L)
!       Tpartl(L) = Dotp(c2000str,uaxisd2k(1,L))
!        print *,' Tpartl/old ', Tpartl(L)
!        print *,' vec1d    ', vec1d
!        print *,' axistcd  ', axistcd(1,L), axistcd(2,L),
!    *                        axistcd(3,L)
!        print *,' axiscfd  ', axiscfd(1,L), axiscfd(2,L),
!    *                        axiscfd(3,L)
!        print *,' uaxisd2k ', uaxisd2k(1,L), uaxisd2k(2,L),
!    *                        uaxisd2k(3,L)
!        print *,' uaxis2000 ', uaxis2000(1,L), uaxis2000(2,L),
!    *                        uaxis2000(3,L)
!
! .............................................................................
!
!          Now do feedbox rotation computations
!
        IF(KPANC.EQ.1) THEN
          PANGL(L)      = 0.0D0
          FEED_COR(L,1) = 0.0D0
          FEED_COR(L,2) = 0.0D0
          GO TO 156
         ENDIF
!
!  Define vector TNCP(3), a unit vector in the direction of the North
!   Celestial pole.
           TNCP(1) = DSIN(SITLAT(L))
           TNCP(2) = 0.D0
           TNCP(3) = DCOS(SITLAT(L))
!
        ZA = ZEN - ZENCOR
        DZA = -ELEV(L,2)
        DAZM = AZ(l,2)
       If (KAXIS(L) .NE. 1) Then
        CALL FBOX (TNCP, TCAXIS, CTCSTR, AZMUTH, DAZM, ZA, DZA,
     .            Rangl, dRangl)
       Else
!           Equatorial Mount, no feedbox rotation
        Rangl = 0.0D0
        dRangl = 0.0D0
       Endif
!
!  Convert the feedbox rotation angle to degrees for this site.
       PANGL(L) = RANGL/CONVD
!  Compute the feedbox rotation corrections for group delay and phase delay
!   rate for this site. The units are seconds and sec/sec.
       FEED_COR(L,1) = 0.D0
       FEED_COR(L,2) = DRANGL/TWOPI/REF_FREQ
!
!    To correct the phases, either ADD the quantity [PANGL(2) - PANGL(1)]
!     to the total or residual observed phases, or SUBTRACT it from the
!     theoretical (model) phases.
!    Note that the group delay corrections are always zero.
!    To correct the phase delay rates, either ADD the quantity
!     [FEED_COR(2,2) - FEEDCOR(1,2)] to the observed rates, or SUBTRACT
!     it from the theoretical (model) rates.
!
 156   CONTINUE
!******************************************************************************
!     See if debug is needed.
      IF ( KAXOD .ne. 0 )  Then
!
      If (L.eq.1) then
        WRITE ( 6, 9100 )
      endif
!
      If (L.eq.2) then
        WRITE ( 6, 9101 )
        WRITE ( 6, 9200 ) KAXIS, R2K, SITLAT, STAR, TCTOCF
        WRITE(6,8)' TR2K    ', TR2K
        WRITE(6,8)' STAR_ABERRATED ', STAR_ABERRATED
        WRITE(6,8)' AXOFF   ', AXOFF
        WRITE(6,8)' PANGL ', PANGL
        WRITE(6,8)' FEED_COR  ', FEED_COR
      endif
!
 9100 FORMAT (20X, "Debug output for subroutine AXOG." )
 9200   FORMAT (1X, "KAXIS  = ", 2 ( I2, 10X ), /, 1X,
     .   "R2K    = ", 9 ( 3 ( D30.16, 10X ), /, 1X ), /, 1X,
     .   "SITLAT = ", 2 ( D30.16, 10X ), /, 1X,
     .   "STAR   = ", 3 ( D30.16, 4X ), /, 1X,
     .   "TCTOCF = ", 6 ( 3 ( D30.16, 10X ), /, 1X ) )
 9101 FORMAT (1X, "Dump for both stations:          " )
    8 FORMAT(A,3D25.16,/,5(7X,3D25.16))
!
      write(6,'(/,10x," ******** DUMP FOR SITE",i2," ******** ")') L
      WRITE(6,8)' TCAXIS  ',TCAXIS
      WRITE(6,8)' CFTOTC  ',CFTOTC
      WRITE(6,8)' CFSTAR  ',CFSTAR
      WRITE(6,8)' TCSTAR  ',TCSTAR
      WRITE(6,8)' ZEN     ',ZEN
      WRITE(6,8)' AZMUTH  ',AZMUTH
      WRITE(6,8)' ZENCOR  ',ZENCOR
      write(6,'("vec1:  ",3D23.14)') vec1
      write(6,'("dvec1: ",3D23.14)') dvec1
      write(6,'("absvec1: ",3D23.14)') absvec1
      write(6,'("dabsvec1: ",3D23.14)') dabsvec1
      write(6,'("axistc:  ",3D23.14)') axistc(1,L),axistc(2,L)
     .                                ,axistc(3,L)
      write(6,'("daxistc: ",3D23.14)') daxistc(1,L), daxistc(2,L),
     .                                 daxistc(3,L)
      write(6,'("axiscf:  ",3D23.14)') axiscf(1,L),axiscf(2,L)
     .                                ,axiscf(3,L)
      write(6,'("daxiscf: ",3D23.14)') daxiscf(1,L), daxiscf(2,L)
     .                                ,daxiscf(3,L)
      write(6,'("c2000str:  ",3D23.14)') c2000str
      write(6,'("ccfstr:  ",3D23.14)') ccfstr
      write(6,'("dccfstr:  ",3D23.14)') dccfstr
      write(6,'("CTCSTR:  ",3D23.14)') CTCSTR
      write(6,'("dctcstr:  ",3D23.14)') dctcstr
      write(6,'("uaxis2000:",3D23.14)') uaxis2000(1,L),uaxis2000(2,L)
     .                                ,uaxis2000(3,L)
      write(6,'("duaxis2000:",3D23.14)') duaxis2000(1,L),duaxis2000(2,L)
     .                                 ,duaxis2000(3,L)
      write(6,'("axis2000:",3D23.14)') axis2000(1,L),axis2000(2,L)
     .                                ,axis2000(3,L)
      write(6,'("daxis2000:",3D23.14)') daxis2000(1,L),daxis2000(2,L)
     .                                 ,daxis2000(3,L)
      write(6,'("DCOMP:   ",2D23.14)') DCOMP(L,1),DCOMP(L,2)
!
      ENDIF
       Go to 800
!
  750 CONTINUE
!   Handling of geocenter station, set axis offset quantities to zero.
       do kx=1,3
         axis2000(kx,L) = 0.0D0
        daxis2000(kx,L) = 0.0D0
       enddo
        DCOMP(L,1) = 0.0D0
        DCOMP(L,2) = 0.0D0
!    Ditto for the feedbox rotation quantities.
       PANGL(L)      = 0.0D0
       FEED_COR(L,1) = 0.0D0
       FEED_COR(L,2) = 0.0D0
!
!     Close loop running over the observing sites.
  800 CONTINUE
!
!  PUT the feedbox rotation angles and delay/rate corrections in the database.
!       FCONT(1) = FEED_COR(1,1) - FEEDCOR(2,1)
        FCONT(1) = 0.D0
        FCONT(2) = FEED_COR(1,2) - FEED_COR(2,2)
      CALL PUT4 ('PARANGLE      ', PANGL, int2(2), int2(1), int2(1))
!     CALL PUT4 ('FEED.COR      ', FEED_COR,2,2,1)
      CALL PUT4 ('FEED.COR      ', FCONT, int2(2), int2(1), int2(1))
!
      IF ( KAXOD .ne. 0 )  Then
       WRITE(6,'(" KAXIS, PANGL ",2I3,2F10.2)')  KAXIS, PANGL
       WRITE(6,'(" FEED_COR ",4D20.10)')  FEED_COR
       WRITE(6,'(" FCONT    ",2D20.10)')  FEED_COR
      ENDIF
!
!     Normal conclusion.
      RETURN
      END
!**************************************************************************
      SUBROUTINE AXOP (AXOFF, STAR, EARTH, SITEV)
      IMPLICIT None
!
! 5.    AXOP
!
! 5.1   AXOP PROGRAM SPECIFICATION
!
! 5.1.1 AXOP is the partial derivatives section of the axis offset
!       module. AXOP computes the partial derivatives of the delay
!       and rate with respect to the antenna axis offsets at the sites.
!
! 5.2   AXOP PROGRAM INTERFACE
!
! 5.2.1 CALLING SEQUENCE -
!       INPUT VARIABLES:
!          1. AXOFF(2)    -  THE ANTENNA AXIS OFFSETS AT EACH SITE. (M)
!          2. STAR(3)     -  THE J2000.0 SOURCE UNIT VECTOR. (UNITLESS)
!          3. EARTH(3,3)  -  THE SSBC EARTH POSITION, VELOCITY, AND
!                            ACCELERATION VECTORS. (M, M/SEC, M/SEC**2)
!          4. SITEV(3,2)  -  THE J2000.0 GEOCENTRIC VELOCITY VECTORS OF
!                            EACH OBSERVATION SITE. (M/SEC)
!
      Real*8 AXOFF(2), STAR(3), EARTH(3,3), SITEV(3,2)
      Real*8 DOTP, VG(3), VE(3), C1, C2, TT
!
! 5.2.2 COMMON BLOCKS USED -
!
      INCLUDE 'cphys.i'
!         VARIABLES 'FROM':
!            1.  VLIGHT - THE VELOCITY OF LIGHT IN VACUUM. (M/SEC)
!
      Real*8  DAXOP(2,2), DCOMP(2,2), RICHM(2),
     .        Tpartl(2), uaxis2000(3,2), duaxis2000(3,2)
      COMMON / AXOCM / DAXOP, DCOMP, RICHM,
     .                 Tpartl, uaxis2000, duaxis2000
!         VARIABLES 'FROM':
!            1. DCOMP(2,2)    -  THE PARTIAL DERIVATIVES OF THE COMPONENT OF
!                                THE ANTENNA AXIS OFFSET IN THE APPARENT
!                                DIRECTION OF THE SOURCE AND THE CT TIME
!                                DERIVATIVE OF THAT COMPONENT WITH RESPECT TO
!                                THE ANTENNA AXIS OFFSETS AT EACH SITE. THE
!                                FIRST INDEX RUNS OVER THE OBSERVATION SITES
!                                AND THE SECOND INDEX RUNS OVER THE DELAY AND
!                                THE DELAY RATE. (UNITLESS, 1/SEC)
!         VARIABLES 'TO':
!            1. DAXOP(2,2)     - THE PARTIAL DERIVATIVES OF THE DELAY AND THE
!                                DELAY RATE WITH RESPECT TO THE ANTENNA AXIS
!                                OFFSETS AT EACH SITE (old model). FIRST INDEX
!                                RUNS OVER THE OBSERVATION SITES AND THE
!                                SECOND RUNS OVER THE DELAY AND DELAY RATE.
!                                (SEC/M, SEC/SEC-M)
!
      INCLUDE 'ccon.i'
!         VARIABLES 'FROM':
!            1. KAXOC  -  THE AXIS OFFSET MODULE FLOW CONTROL FLAG.
!            2. KAXOD  -  THE AXIS OFFSET MODULE DEBUG OUTPUT FLAG.
!
! 5.2.3 PROGRAM SPECIFICATIONS -
!
! 5.2.4 DATA BASE ACCESS -
!       'PUT' VARIABLES:
!          1. DAXOP(2,2) - THE PARTIAL DERIVATIVES OF THE DELAY AND THE
!                          DELAY RATE WITH RESPECT TO THE ANTENNA AXIS
!                          OFFSETS AT EACH SITE (modified for new model).
!                          THE FIRST INDEX RUNS OVER THE OBSERVATION SITES
!                          AND THE SECOND RUNS OVER THE DELAY AND DELAY
!                          RATE. (SEC/M, SEC/SEC-M)
!       ACCESS CODES:
!          1. 'AXO PART' - THE DATA BASE ACCESS CODE FOR THE AXIS OFFSET
!                          MODULE PARTIAL DERIVATIVES ARRAY.
!
! 5.2.6 SUBROUTINE INTERFACE -
!          CALLER SUBROUTINES: DRIVP
!          CALLED SUBROUTINES: PUT4
!
! 5.2.7 CONSTANTS USED - VLIGHT
!
! 5.2.8 PROGRAM VARIABLES
      Integer*4 K, L
!
! 5.2.9 PROGRAMMER - DALE MARKHAM  01/13/77
!                    PETER DENATALE 07/11/77
!                    BRUCE SCHUPLER 03/07/78
!                    Jim Ryan 89.06.29 Character stings used.
!                    Jim Ryan 89.10.05 CPHYS common now an include file.
!                    Jim Ryan 89.12.12 UNIX-like database interface
!                    implimented.
!                    D. Gordon 94.03.25 Old partials multiplied by index of
!                              refraction and PUT into database (new, simple
!                              axis offset model). Added complete partials for
!                              more complex (3 terms) axis offset model.
!                    D. Gordon 94.08.03 Corrected old axis multiplication by
!                              N_air(site #). Was incorrrectly multiplying both
!                              delays by site 1 and both rates by site 2
!                              indices of refraction.
!                    Jim Ryan 02.Sept Integer*2/4 updates.
!                    D. Gordon 2004 October. Corrections to partials.
!
! 5.3   AXOP PROGRAM STRUCTURE
!
!    Compute the partial derivatives of the delay and the delay rate with
!    respect to the axis offset at each site. First two terms in the
!    relativity model included.
!
      Do K = 1,3
        VE(K) = EARTH(K,2)
        VG(K) = EARTH(K,2) + SITEV(K,2)
      Enddo
      TT = 1.D0 + DOTP(Star,VG)/VLIGHT
!
!   Delay and rate partials for L'th station
       Do L = 1,2
        DCOMP(L,1) = DOTP(uaxis2000(1,L),STAR)/TT +
     .                  DOTP(uaxis2000(1,L),VE)/VLIGHT
        DCOMP(L,2) = DOTP(duaxis2000(1,L),STAR)/TT +
     .                  DOTP(duaxis2000(1,L),VE)/VLIGHT
       Enddo
!
!   Axis offset partial compution
!    The minus sign appears due to the definition of the
!    baseline vector being the site#1 vector minus the site#2 vector.
      DO   K = 1,2                       ! looping over delay and rate
           DAXOP(1,K) = + DCOMP(1,K) / VLIGHT      ! site 1
           DAXOP(2,K) = - DCOMP(2,K) / VLIGHT      ! site 2
      ENDDO
!
!     PUT the axis offset partial derivatives array.
      CALL PUT4 ('AXO PART      ',DAXOP,int2(2),int2(2),int2(1))
!
!   See if debug is neccessary.
      IF (KAXOD .ne. 0) Then
       WRITE ( 6, 9100 )
 9100  FORMAT (1X, "Debug output for subroutine AXOP." )
   8   FORMAT(A,5D25.16/(9X,5D25.16))
!      WRITE(6,8)' AXOFF      ', AXOFF
       WRITE(6,8)' DCOMP      ', DCOMP
       WRITE(6,8)' DAXOP      ', DAXOP
       WRITE(6,8)' VE         ', VE
       WRITE(6,8)' VG         ', VG
       WRITE(6,8)' TT         ', TT
       WRITE(6,8)' VLIGHT     ', VLIGHT
      Endif
!
      RETURN
      END
!**************************************************************************
      SUBROUTINE AXOC (AXOFF, DAXOC)
      IMPLICIT None
!
! 6.    AXOC
!
! 6.1   AXOC PROGRAM SPECIFICATION
!
! 6.1.1 AXOC is the contributions section of the axis offset module. It
!       computes the contributions to the delay and rate due to the antenna
!       axis offsets at each site.
!
! 6.2   AXOC PROGRAM INTERFACE
!
! 6.2.1 CALLING SEQUENCE -
!          INPUT VARIABLES:
!             1. AXOFF(2)  -  THE ANTENNA AXIS OFFSETS AT EACH SITE. (M)
!          OUTPUT VARIABLES:
!             1. DAXOC(2,2) - THE CONTRIBUTIONS TO THE DELAY AND TO THE DELAY
!                             RATE DUE TO ANTENNA AXIS OFFSETS AT EACH SITE.
!                             THE FIRST INDEX RUNS OVER THE OBSERVATION SITES
!                             AND THE SECOND RUNS OVER THE DELAY AND THE DELAY
!                             RATE. (SEC, SEC/SEC)
!
! 6.2.2 COMMON BLOCKS USED -
!
      Real*8  DAXOP(2,2), DCOMP(2,2), RICHM(2),
     .        Tpartl(2), uaxis2000(3,2), duaxis2000(3,2)
      COMMON / AXOCM / DAXOP, DCOMP, RICHM,
     .                 Tpartl, uaxis2000, duaxis2000
!          VARIABLES 'FROM':
!            1. DAXOP(2,2)    -  THE PARTIAL DERIVATIVES OF THE DELAY AND THE
!                                DELAY RATE WITH RESPECT TO THE ANTENNA AXIS
!                                OFFSETS AT EACH SITE. THE FIRST INDEX RUNS
!                                OVER THE OBSERVATION SITES AND THE SECOND RUNS
!                                OVER THE DELAY AND THE DELAY RATE. (Old model)
!                                (SEC/M, SEC/SEC-M)
!
       INCLUDE 'ccon.i'
!         VARIABLES 'FROM':
!            1. KAXOC - THE AXIS OFFSET MODULE FLOW CONTROL FLAG.
!            2. KAXOD - THE AXIS OFFSET MODULE DEBUG OUTPUT FLAG.
!
      INCLUDE 'cphys.i'
!         VARIABLES 'FROM':
!            1.  VLIGHT - THE VELOCITY OF LIGHT IN VACUUM. (M/SEC)
!
!
!     Common /OBS/N_air, DAXOC_new, DAXOC_newer
!     Real*8 N_air(2), DAXOC_new(2,2), DAXOC_newer(2,2), Daxoc_old(2)
!
! 6.2.3 PROGRAM SPECIFICATIONS -
      Real*8  AXOFF(2), DAXOC(2,2)
      Real*8  Taxoc(2), Tcorrmv(2), DAXOC_old(2)
      Integer*4 K, L
!
! 6.2.4 DATA BASE ACCESS -
!         'PUT' VARIABLES:
!            1. DAXOC(2,2)  -  THE CONTRIBUTIONS TO THE DELAY AND THE DELAY
!                              RATE DUE TO ANTENNA AXIS OFFSETS AT EACH SITE.
!                              THE FIRST INDEX RUNS OVER THE OBSERVATION
!                              SITES AND THE SECOND RUNS OVER THE DELAY AND
!                              THE DELAY RATE. (SEC, SEC/SEC)
!         ACCESS CODES:
!            1. 'AXO CONT' -  The database access code for the new, simple
!                             version, axis offset contributions array.
!
! 6.2.6 SUBROUTINE INTERFACE -
!          CALLER SUBROUTINES: DRIVC
!          CALLED SUBROUTINES: PUT4
!
! 6.2.7 CONSTANTS USED - NONE
!
! 6.2.8 PROGRAM VARIABLES -
!            1. DAXOC(2,2) - The contributions to the delay and rate due
!                            to the antenna axis offsets. First index runs
!                            over sites, second runs over delay and rate.
!
! 6.2.9 PROGRAMMER - DALE MARKHAM  01/13/77
!                    PETER DENATALE 07/11/77
!                    BRUCE SCHUPLER 03/07/78
!                    Jim Ryan 89.06.29 Character stings used.
!                    Jim Ryan 89.12.12 UNIX-like database interface
!                    implimented.
!                    D. Gordon 94.03.25 Added computations for both new versions
!                              of the axis offset contributions. Made the
!                              simpler version the default (will be added to
!                              theoreticals in THERY). Computing corrections
!                              to convert to alternate new model and to old
!                              model ('on-the-fly' in SOLVE), and PUTting them
!                              into the database.
!                    D. Gordon 94.05.23 Changed $Include to Include.
!                    D. Gordon 94.08.04 Error corrected. Old axis offset was
!                              still the default. Corrected to make new/simple
!                              offset the default. Added variable DAXOC_old.
!                    D. Gordon 98.11.12 Removed PUT of old axis offset
!                              differential contribution.
!                    Jim Ryan 02.Sept Integer*2/4 updates.
!                    D. Gordon 2004.05.19 Tilt removing contribution added.
!                    D. Gordon 2004 October. Corrections to contributions.
!
! 6.3   AXOC PROGRAM STRUCTURE
!
!     Compute the axis offset contributions.
!      [L => site; K => delay and rate]
      DO 120  K = 1,2
           DO 110  L = 1,2
             DAXOC(L,K) = DAXOP(L,K) * AXOFF(L)
  110      CONTINUE
  120 CONTINUE
!
!     Check to determine if the axis offset module is to be turned off.
      IF (KAXOC .eq. 1) Then
       DO 320  K = 1,2
           DO 310  L = 1,2
                DAXOC(L,K) = 0.D0
  310      CONTINUE
  320  CONTINUE
      Endif
!
!     PUT the axis offset contributions.
  400 CALL PUT4 ('AXO CONT      ',DAXOC,int2(2),int2(2),int2(1))
!
!   Compute small correction to remove the tilt effect
!    (Relativity terms omitted)
        Daxoc_old(1) =  Tpartl(1)*AXOFF(1)/VLIGHT
        Daxoc_old(2) = -Tpartl(2)*AXOFF(2)/VLIGHT
!
        Taxoc(1) = Daxoc_old(1) - DAXOC(1,1)
        Taxoc(2) = Daxoc_old(2) - DAXOC(2,1)
         Tcorrmv(1) = Taxoc(1) + Taxoc(2)
         Tcorrmv(2) = 0.D0
      CALL PUT4 ('TILTRMVR      ',Tcorrmv,int2(2),int2(1),int2(1))
!
!     Check for debug output.
      IF (KAXOD .ne. 0)  Then
       WRITE ( 6, 9100 )
 9100  FORMAT (1X, "Debug output for subroutine AXOC." )
    8  FORMAT(A,5D25.16/(9X,5D25.16))
       WRITE(6,8)' AXOFF       ', AXOFF
       WRITE(6,8)' DAXOP       ', DAXOP
       WRITE(6,8)' DAXOC       ', DAXOC
       WRITE(6,8)' DAXOC_OLD   ', DAXOC_OLD
       WRITE(6,8)' Taxoc (ps)  ', Taxoc(1)*1.E12,Taxoc(2)*1.E12
       WRITE(6,8)' Tcorrmv     ', Tcorrmv
      Endif
!
  600 RETURN
      END
!**************************************************************************
      DOUBLE PRECISION FUNCTION SBEND(El_rad,Temp_K,Humid_F,Press_Hg)
      IMPLICIT None
!
! Input:
!   El_rad   -- elevation angle in radians
!   Press_Hg -- Pressure in mm of Mercury (Hg)
!   Temp_K   -- Temperature in Kelvins
!   Humid_F  -- relative humidity (percent)
!
! output   --
!   Sbend  -- bending angle in radians.
!
      Real*8 El_rad, Temp_K, Humid_F, Press_Hg
      Real*8 e(12),wp1(4),d3
      Real*8 fp,ft,fw,u,x,ad1,ad2,bd1,bd2,zd2,r,delta
      Real*8 a1,a2,b1,b2,c1,c2,e1,e2,e3,e4,e5,e6,e7,e8,e9
      Real*8 e10,e11,e12,p1,p2,t1,t2,z1,z2,w0,w1,w2,w3
      Real*8 conv
      Integer*4 I
!
      equivalence (e( 1), e1),(e( 2), e2),(e( 3), e3),(e( 4), e4),
     .            (e( 5), e5),(e( 6), e6),(e( 7), e7),(e( 8), e8),
     .            (e( 9), e9),(e(10),e10),(e(11),e11),(e(12),e12)
      equivalence (wp1(1),w0),(wp1(2),w1),(wp1(3),w2),(wp1(4),w3)
!
      data a1, a2 /     0.40816d0, 112.30d0  /
      data b1, b2 /     0.12820d0, 142.88d0  /
      data c1, c2 /     0.80000d0,  99.344d0 /
      data e   /    46.625d0  ,  45.375d0 ,     4.1572d0,  1.4468d0  ,
     .               0.25391d0,   2.2716d0,    -1.3465d0, -4.3877d0  ,
     .               3.1484d0 ,   4.5201d0,    -1.8982d0,  0.89000d0 /
      data p1 /   760.0d0 /
      data t1 /   273.0d0 /
      data wp1 / 22000.0d0    ,  17.149d0 ,  4684.1d0,    38.450d0   /
      data z1 /  91.870d0 /
      data conv/57.295779512d0/
!
      Real*8         PI,TWOPI,HALFPI,CONVD,CONVDS,CONVHS,SECDAY
      COMMON /CMATH/ PI,TWOPI,HALFPI,CONVD,CONVDS,CONVHS,SECDAY
!           VARIABLES 'FROM':
!              1. HALFPI - THE VALUE OF PI/2
!              2. CONVD  - THE CONVERSION FACTOR FROM DEGREES TO RADIANS
!                          (RAD/DEG)
!
! STATEMENT FUCNTION
      delta(ad1,ad2,bd1,bd2,zd2)=(ad2-ad1)*dexp(bd1*(zd2-bd2))
!
! CONVERT UNITS
!  Zenith angle in degrees
      z2 = 90.0d0 - El_Rad/CONVD
!  Temperature in Kelvins
      t2 = Temp_K
!  Fractional humidity (0.0 -> 1.0)
      r = Humid_F
!  Pressure in mm of Hg
      p2 = Press_Hg
!
!      WRITE(6,9956) R,T2,P2,Z2                                          DEBUG
!9956  FORMAT(" R,T2,P2,Z2",4F10.4)                                      DEBUG
!
! CALCULATE CORRECTIONS FOR PRES, TEMP, AND WETNESS
!      WRITE(6,9980) Z1,Z2,C1,C2                                         DEBUG
!9980  FORMAT(" Z1,Z2,C1,C2",4D15.6)                                     DEBUG
      d3=1.0d0+delta(z1,z2,c1,c2,z2)
      fp=(p2/p1)*(1.0d0-delta(p1,p2,a1,a2,z2)/d3)
      ft=(t1/t2)*(1.0d0-delta(t1,t2,b1,b2,z2)/d3)
      fw=1.0d0+(w0*r*dexp((w1*t2-w2)/(t2-w3))/(t2*p2))
!      WRITE(6,9957) D3,FP,FT,FW                                         DEBUG
!9957  FORMAT(" D3,FP,FT,FW ",4D12.6)                                    DEBUG
!
!  CALCULATE OPTICAL REFRACTION
      u=(z2-e1)/e2
      x=e11
      do 10 i=1,8
        x=e(11-i)+u*x
10    continue
!
!  COMBINE FACTORS AND FINISH OPTICAL FACTOR
      sbend=ft*fp*fw*(dexp(x/d3)-e12)
!      WRITE(6,9958) SBEND,X,U                                           DEBUG
!9958  FORMAT(" SBEND,X,U ",3D20.10)                                     DEBUG
!
! BACK TO RADIANS FROM ARC SECONDS
      sbend=(sbend/3600.0d0)*CONVD
!      WRITE(6,9959) SBEND                                               DEBUG
!9959  FORMAT(" SBEND (RADIANS) ",D20.10)                                DEBUG
      return
      end
!**************************************************************************
      SUBROUTINE FBOX ( TNCP, TCAXIS, CTCSTR, AZ, DAZ, ZA, DZA,
     .            RANGL, DRANGL )
      IMPLICIT NONE
!
!     FBOX computes the feedbox rotation angle, relative to the source.
!     It computes the angle, looking up at the sky and measured at the
!     source, between the North Celestial Pole and the antenna fixed axis.
!     An increase in the feedbox rotation angle means a clockwise
!     rotation of the feedbox, for an observer looking up at the sky.
!     The angle itself is arbitrary. The important quantity is the
!     difference in the feedbox rotations at the two antennas, and how it
!     changes from one observation to the next.
!
!     It's not clear what to do with these angles in all case. For RCP data
!     in geodesy, we correct the phase by adding the remote station
!     correction and subtracting the reference station correction.
!
!     The algorithm makes two coordinate rotations to put the star vector
!     along the X-axis. The azimuths of the NCP and the fixed axis in the
!     transformed Y-Z plane are then easily found and differenced.
!
!     This subroutine replaces subroutine PANG in Calc 9.0, and will handle
!     all azis types.
!
!     Input Variables:
!         1) CTCSTR(3)  - Topocentric unit vector in the direction of the
!                         source. [X = Up = COS(Zenith Angle). Y = East =
!                         SIN(Zenith Angle) * SIN(Azimuth). Z = North =
!                         SIN(Zenith Angle) * COS(Azimuth). For greatest
!                         accuracy, the effects of atmospheric refraction
!                         and aberration should be included.]
!         2) TCAXIS(3)  - Topocentric unit vector in the direction of the
!                         antenna's fixed axis. [Altazimuth: X = 1.0,
!                         Y = 0.0, Z = 0.0.  X/Y North (Gilcreek):
!                         X = 0., Y = 0., Z = 1.0.  X/Y East (HARTRAO):
!                         X = 0., Y = 1.0., Z = 0.]
!         3) TNCP(3)    - Topocentric unit vector in the direction of the
!                         North Celestial Pole. [X = SIN(Geodetic Latitude),
!                         Y = 0.0, Z = COS(Geodetic Latitude)]
!         4) AZ         - Azimuth angle of source, in radians.
!         5) DAZ        - Time derivative of Azimuth angle. (radians/sec)
!                         Zeros can be used here if the user does not care
!                         about the rate of change of the rotation angle, or
!                         about the correction to the phase delay rate.
!         6) ZA         - Zenith angle of source, in radians. Include
!                         refraction and aberration for greatest accuracy.
!         7) DZA        - Time derivative of zenith angle. (radians/sec)
!                         Zeros can be used here if the user does not care
!                         about the rate of change of the rotation angle, or
!                         about the correction to the phase delay rate.
!     Output Variables:
!         1) RANGL      - The clockwise rotation angle of the feedbox. (rad)
!         2) DRANGL     - The time derivative of RANGL. (rad/sec)
!
!
      Real*8         PI,TWOPI,HALFPI,CONVD,CONVDS,CONVHS,SECDAY
      COMMON /CMATH/ PI,TWOPI,HALFPI,CONVD,CONVDS,CONVHS,SECDAY
!           VARIABLES 'FROM':
!              1. HALFPI - THE VALUE OF PI/2
!              2. CONVD  - THE CONVERSION FACTOR FROM DEGREES TO RADIANS
!                          (RAD/DEG)
!
!    Program History:
!       D. Gordon 98.11.10 Subroutine written at GSFC. Replaces subroutine
!                 PANG.
!
!
      Real*8 TNCP(3), TCAXIS(3), CTCSTR(3), AZ, DAZ,
     .       ZA, DZA, Rangl, dRangl
      REAL*8 P1AZ(3,3), P2ZA(3,3), P12(3,3), DP1AZ(3,3), DP2ZA(3,3),
     .       DP12(3,3), TT1(3,3), TT2(3,3)
      REAL*8 NCP1(3), AXIS1(3), STAR1(3), DNCP1(3), DAXIS1(3)
      REAL*8 AZNCP, AZAXIS, DAZNCP, DAZAXIS
!
!  Construct rotation matrix to rotate about the X-axis by the angle -AZ
!    and its derivative.
      CALL ROTAT ( -AZ, int2(1), P1AZ)
       CALL DROTT ( -AZ, -DAZ, int2(1), DP1AZ)
!  Construct rotation matrix to rotate about the Y-axis by the angle -ZA
!    and its derivative.
      CALL ROTAT ( -ZA, int2(2), P2ZA)
       CALL DROTT ( -ZA, -DZA, int2(2), DP2ZA)
!  Combine the above rotation matrices and the derivative.
      CALL MMUL2 (P2ZA, P1AZ, P12)
       CALL MMUL2 (DP2ZA, P1AZ,  TT1)
       CALL MMUL2 (P2ZA, DP1AZ,  TT2)
       CALL MADD2 (TT1,    TT2, DP12)
!  Rotate the three topocentric vectors and take derivatives of first two.
      CALL VECRT (P12,   TNCP,  NCP1)
      CALL VECRT (P12, TCAXIS, AXIS1)
      CALL VECRT (P12, CTCSTR, STAR1)
       CALL VECRT (DP12,   TNCP,  DNCP1)
       CALL VECRT (DP12, TCAXIS, DAXIS1)
!  The STAR1 vector should now be along the X-axis.
!       print *,' STAR1  ', STAR1
!
!  Find the azimuth angle of the NCP and its time derivative in the new frame
       AZNCP = -ATAN2(NCP1(3),NCP1(2)) + HALFPI
       IF (AZNCP .LT. 0.D0) AZNCP = AZNCP + TWOPI
        IF(DABS(NCP1(2)) .GT. 1.D-16) THEN
          dAZNCP = -1.D0/(1.D0+((NCP1(3)/NCP1(2))**2)) *
     .      ( 1.D0/NCP1(2) * DNCP1(3) - NCP1(3)/NCP1(2)**2 * DNCP1(2) )
        ELSE
          dAZNCP = 0.D0
        ENDIF
!
!  Find the azimuth angle of the fixed axis and its time derivative
       AZAXIS = -ATAN2(AXIS1(3),AXIS1(2)) + HALFPI
       IF (AZAXIS .LT. 0.D0) AZAXIS = AZAXIS + TWOPI
        IF(DABS(AXIS1(2)) .GT. 1.D-16) THEN
          dAZAXIS = -1.D0/(1.D0+((AXIS1(3)/AXIS1(2))**2)) *
     .      ( 1.D0/AXIS1(2) * DAXIS1(3) - AXIS1(3)/AXIS1(2)**2 *
     .                                           DAXIS1(2) )
        ELSE
          dAZAXIS = 0.D0
        ENDIF
!  Take difference (NCP to AXIS) and express as a clock-wise rotation, as
!   seen from the ground looking up
       RANGL = AZNCP - AZAXIS
        DRANGL = DAZNCP - DAZAXIS
        IF (RANGL .LT. -PI) RANGL = RANGL + PI
        IF (RANGL .GT.  PI) RANGL = RANGL - PI
!
!  That's all!
      RETURN
      END
