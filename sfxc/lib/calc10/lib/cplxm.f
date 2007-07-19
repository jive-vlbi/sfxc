      SUBROUTINE PLXA()
      IMPLICIT None
!
! 1.1.1 PLXA ADDS ENTRIES TO THE TABLE OF CONTENTS FOR THE PARALLAX MODULE TEXT
!       MESSAGE, AND PARTIAL DERIVATIVE AND CONTRIBUTION ARRAYS. IT ALSO ADDS
!       ENTRIES TO THE TABLE OF CONTENTS FOR THE FLOW CONTROL MESSAGE.
!
! 1.2   PLXA PROGRAM INTERFACE
!
! 1.2.2 COMMON BLOCKS USED -
       INCLUDE 'ccon.i'
!            VARIABLES 'FROM':
!             1. KPLXC - THE PARALLAX MODULE FLOW CONTROL FLAG.
!                         0 => Do NOT compute parallax delay and rate
!                              contributions.
!                         1 => DO compute parallax delay and rate
!                              contributions. (Lcode 'PRLXCONT'). A distance
!                              in parsecs must be supplied, either via Lcode
!                              'DISTPSEC', or via external source file input,
!                              for this computation to be meaningful.
!
! 1.2.4 DATA BASE ACCESS -
!           ACCESS CODES:
!             1. 'PLX MESS'  -  THE DATA BASE ACCESS CODE FOR THE
!                               PARALLAX MODULE TEXT MESSAGE.
!             2. 'PLX PART'  -  THE DATA BASE ACCESS CODE FOR THE PARALLAX
!                               MODULE PARTIAL DERIVATIVES ARRAY.
!             3. 'PLX CFLG'  -  THE DATA BASE ACCESS CODE FOR THE
!                               PARALLAX MODULE FLOW CONTROL MESSAGE.
!             4. 'PLX1PSEC'  -  New with Calc 9.x. Has two possible usages.
!                               1) Parallax delay and rate contributions for
!                               the source at 1 parsec distance. Divide by
!                               actual distance (parsecs) to get delay and
!                               rate corrections. 2) Partial derivatives of
!                               delay and rate w.r.t. inverse distance in
!                               parsecs. Use to solve for inverse distance.
!             5. 'PRLXCONT'  -  Optional/New with Calc 9.x. Delay and rate
!                               contributions for parallax (sec, sec/sec).
!                               Computed only if KPLXC=1. If distance not
!                               supplied, these will be zeros. Intended for
!                               correlator usage.
!
! 1.2.6 SUBROUTINE INTERFACE -
!             CALLER SUBROUTINES: TOCUP
!             CALLED SUBROUTINES: ADDA, ADDR
!
! 1.2.9 PROGRAMMER - C. A. KNIGHT  08/12/80
!                    SAVITA GOEL   06/04/87 (CDS FOR A900)
!                    89.08.15 Jim Ryan Documentation simplified.
!                    Jim Ryan 89.12.12 UNIX-like database interface
!                         implimented.
!                    David Gordon 94.04.14 Converted to Implicit None.
!                    David Gordon 98.09.11 Put in ADD's for 'PLX1PSEC' and
!                         'PRLXCONT', new parallax partials and contributions.
!                    Jim Ryan Sept 2002 Integer*2/4 mods.
!
! 1.3   PLXA PROGRAM STRUCTURE
!
!   ADD for the module text message.
      CALL ADDA (int2(1),'PLX MESS','Parallax message definition     ',
     . int2(40), int2(1), int2(1))
!
!   ADD for module flow control message.
      CALL ADDA (int2(1),'PLX CFLG','Parallax flow control mess def  ',
     . int2(40), int2(1), int2(1))
!
!   ADD for parallax partial derivatives. (Old partial, for annual parallax
!        of 1 radian.)
      CALL ADDR (int2(2),'PLX PART','Parallax partial deriv. def.    ',
     . int2(2), int2(1), int2(1))
!
!   New ADD for parallax partials. Use to solve for inverse distance (parsecs)
!     or divide by distance (in parsecs) to get delay and rate corrections.
      CALL ADDR (int2(2),'PLX1PSEC','Parallax partial/contr, 1 parsec',
     . int2(2), int2(1), int2(1))
!
!   Optional add for total parallax contributions.
      If (KPLXC .eq. 1) CALL ADDR (int2(2), 'PRLXCONT',
     . 'Parallax Contributions, sec, s/s', int2(2), int2(1), int2(1))
      RETURN
      END
!*********************************************************************
      SUBROUTINE PLXI()
      IMPLICIT None
!
! 3.1.1 PLXI IS THE PARALLAX MODULE INPUT AND INITIALIZATION SECTION.
!
! 3.2   PLXI PROGRAM INTERFACE
!
! 3.2.2 COMMON BLOCKS USED -
      INCLUDE 'ccon.i'
!           VARIABLES 'FROM':
!             1.  KPLXC  -  THE PARALLAX MODULE FLOW CONTROL FLAG.
!                           0 => Do not compute parallax contributions
!                           1 => DO compute parallax contributionsns
!             2.  KPLXD  -  THE PARALLAX MODULE DEBUG OUTPUT FLAG.
!
! 3.2.3 PROGRAM SPECIFICATIONS -
      INTEGER*2  NDO(3), idm7
      INTEGER*2      LON(40),    LOFF(40),     LPLXM(40)
      CHARACTER*40 C_LON(2) ,  C_LOFF(2) ,   C_LPLXM(2)
      EQUIVALENCE( C_LON,LON),(C_LOFF,LOFF),(C_LPLXM,LPLXM)
!
      DATA C_LPLXM  /
     .'Parallax Module, Last modified 98SEP11, ',
     .'D. Gordon/GSFC                          '/
!
      DATA C_LON  /
     .'Parallax Contributions OFF.             ',
     .'                                        '/
!
      DATA C_LOFF /
     .'Parallax Contributions ON.              ',
     .'                                        '/
!
! 3.2.4 DATA BASE ACCESS -
!           'GET' VARIABLES: none
!           'PUT' VARIABLES:
!             1. LPLXM(40)  -  THE PARALLAX MODULE TEXT MESSAGE.
!             2. LON(40)    -  THE PARALLAX MODULE 'TURNED ON' MESSAGE.
!             3. LOFF(40)   -  THE PARALLAX MODULE 'TURNED OFF' MESSAGE.
!           ACCESS CODES:
!             1. 'PLX MESS' -  THE DATA BASE ACCESS CODE FOR THE PARALLAX
!                              MODULE TEXT MESSAGE.
!             2. 'PLX CFLG' -  THE DATA BASE ACCES CODE FOR THE PARALLAX
!                              MODULE FLOW CONTROL MESSAGE.
!
! 3.2.6 SUBROUTINE INTERFACE -
!             CALLER SUBROUTINES: INITL
!             CALLED SUBROUTINES: PUTA
!
! 3.2.9 PROGRAMMER - C. A. KNIGHT  02/26/80
!                    89.08.15 Jim Ryan Documentation simplified.
!                    Jim Ryan 89.12.12 UNIX-like database interface
!                    implimented.
!                    David Gordon 94.04.14 Converted to Implicit None.
!                    David Gordon 98.09.11 Updated data base text messages.
!                    Jim Ryan Sept 2002 Integer*2/4 mods.
!
!    PLXI PROGRAM STRUCTURE
!
!   PUT the module text message.
      CALL PUTA ('PLX MESS      ',LPLXM,int2(40),int2(1),int2(1))
!
!   PUT the module flow control message.
      IF (KPLXC .EQ. 0) CALL PUTA('PLX CFLG      ', LON, int2(40),
     .    int2(1), int2(1))
      IF (KPLXC .NE. 0) CALL PUTA('PLX CFLG      ', LOFF, int2(40),
     .    int2(1), int2(1))
!
  500 RETURN
      END
!*********************************************************************
      SUBROUTINE PLXG()
      IMPLICIT None
!       PLXG is the Parallax Module geometry section. With Calc 9.0,
!        parallax contributions can be calculated, but the work will be
!        done in the partials and contributions sections.
!
      Integer*4 I
!
!   Normal program conclusion - a dummy executable statement included to make
!    the compiler happy
      I=1
!
      RETURN
      END
!*********************************************************************
      SUBROUTINE PLXP( SUN, DSTRP, CD, CRA, SD, SRA,
     .           EARTH, STAR, EPBASE, SITEV )
      IMPLICIT None
!
! 5.1.1 PLXP is the Parallax module partial derivatives section. PLXP computes
!       the partial derivatives of the delay and rate with respect to the
!       parallax parameter.
!
! 5.1.2 RESTRICTIONS - MUST BE CALLED AFTER STAR PARTIALS, SUN DATA, AND
!                      PHYSICAL CONSTANTS ARE AVAILABLE.
!
! 5.1.3 REFERENCES - EXPLANATORY SUPPLEMENT TO THE A.E.N.A. (GREEN BOOK), p. 64.
!
! 5.2   PLXP PROGRAM INTERFACE
!
! 5.2.1 CALLING SEQUENCE -
!         INPUT VARIABLES:
!            1. SUN(3,2)  - The J2000 geocentric Sun position and velocity
!                           vectors. (m, m/sec)
!            2. DSTRP(2,2)- Partial derivatives of the delay and delay rate with
!                           respect to source RA and Dec. First index runs over
!                           RA and Dec, second runs over delay and rate.
!                           (sec/rad, sec/sec-rad)
!            3. CD        - COSINE OF DECLINATION OF THE CURRENT SOURCE.
!            4. CRA       - COSINE OF RIGHT ASCENSION OF THE CURRENT SOURCE.
!            5. SD        - SINE OF DECLINATION OF THE CURRENT SOURCE.
!            6. SRA       - SINE OF RIGHT ASCENSION OF THE CURRENT SOURCE.
!            7. EARTH(3,3)- THE SOLAR SYSTEM BARYCENTRIC EARTH POSITION,
!                           VELOCITY, AND ACCELERATION VECTORS.
!                           (M, M/SEC, M/SEC**2)
!            8. STAR(3)   - THE J2000.0 SOURCE UNIT VECTOR. (UNITLESS)
!            9. EPBASE(3,2)-THE J2000.0 GEOCENTRIC BASELINE POSITION AND
!                           VELOCITY VECTORS. (M, M/SEC)
!           10. SITEV(3,2)- THE J2000.0 GEOCENTRIC VELOCITY VECTORS OF EACH
!                           OBSERVATION SITE. (M/SEC)
!
! 5.2.2 COMMON BLOCKS USED -
!
      INCLUDE 'ccon.i'
!            VARIABLES 'FROM':
!              1.  KPLXC  -  THE PARALLAX MODULE FLOW CONTROL FLAG.
!              2.  KPLXD  -  THE PARALLAX MODULE DEBUG OUTPUT FLAG.
!
      INCLUDE 'cphys.i'
!            VARIABLES FROM -
!              1. AU_meters - Astronomical Unit. (meters)
!              2. VLIGHT    - The velocity of light in vacuum.  (m/sec)
!
      Real*8         PI, TWOPI, HALFPI, CONVD, CONVDS, CONVHS, SECDAY
      COMMON /CMATH/ PI, TWOPI, HALFPI, CONVD, CONVDS, CONVHS, SECDAY
!            VARIABLES FROM -
!              1.  CONVDS - THE CONVERSION FACTOR FROM ARCSECONDS TO RADIANS
!                           (RAD/ARCSECOND)
!
      Real*8 Dparsec, t_prlx(2)
      Common /PRLX/ Dparsec, t_prlx
!       Variables From:
!         Dparsec - Distance of the current source in parsecs, either from
!                   Lcode 'DISTPSEC' or from an external source file. But
!                   usually this will be zero.
!       Variables To:
!         t_prlx(2) - Partial derivatives of the delay and rate w.r.t. the
!                     inverse source distance (in parsecs).
!
! 5.2.3 PROGRAM SPECIFICATIONS -
      Real*8 CD, CRA, SD, SRA, SUN(3,2)
      Real*8 DSTRP(2,2), SUNVCT(3), DPLXP(2), DRADPI, DDCDPI
      Real*8 EARTH(3,3), STAR(3), EPBASE(3,2), SITEV(3,2), DOTP
      Real*8 E(3), A(3), B(3), K(3), dK(3), B0(3), V0(3), Vpw2(3)
      INTEGER*4 I, J
!
! 5.2.4 DATA BASE ACCESS -
!            'PUT' VARIABLES:
!              1.  DPLXP(2)  - THE PARTIAL DERIVATIVES OF THE DELAY AND RATE
!              2.  t_prlx(2) - The partial derivatives of the delay and rate
!                              with respect to the inverse distance in parsecs.
!            ACCESS CODES:
!              1. 'PLX PART' - THE DATA BASE ACCESS CODE FOR THE PARALLAX
!                              MODULE PARTIAL DERIVATIVES ARRAY.
!              2. 'PLX1PSEC' - The data base access code for the parallax
!                              partial with respect to inverse distance in
!                              parsecs.
!
! 5.2.6 SUBROUTINE INTERFACE -
!             CALLER SUBROUTINES: DRIVR
!             CALLED SUBROUTINES: PUT4
!
! 5.2.8 PROGRAM VARIABLES -
!             1. DRADPI - THE PARTIAL DERIVATIVE OF SOURCE RIGHT ASCENSION
!                         WRT THE PARALLAX PARAMETER. (RAD/RAD)
!             2. DDCDPI - THE PARTIAL DERIVATIVE OF SOURCE DECLINATION WRT
!                         THE PARALLAX PARAMETER. (RAD/RAD)
!
! 5.2.9 PROGRAMMER - C. A. KNIGHT  02/26/80
!                    Jim Ryan      89:10:05 CPHYS common made an include file
!                    Jim Ryan 89.12.12 UNIX-like database interface
!                        implimented.
!                    D. Gordon 94.04.14 Converted to Implicit None.
!                    D. Gordon 95.05.02 Removed database GET4 calls for
!                        'SUN DATA' and 'STR PART'. Now passing them from the
!                        driver subroutine via variables SUN and DSTRP.
!                        Replaced (VLIGHT*SECPAU) with AU_meters from cphys.i.
!                        Changed SUNVCT(3,2) to SUNVCT(3).
!                    D. Gordon 96.02.27 Removed second declaration of
!                        DSTRP(2,2), found by Warwick Wilson, ATNF. Minor
!                        code cleanup.
!                    D. Gordon 98.09.11 Added EARTH, STAR, EPBASE, SITEV.
!                        Corrected computations to use Earth-barycenter
!                        vector instead of Earth-Sun vector.
!                        Adding partials w.r.t. inverse distance in parsecs
!                        and new Lcode 'PLX1PSEC'.
!                    Jim Ryan Sept 2002 Integer*2/4 mods.
!
! 5.3   PLXP PROGRAM STRUCTURE
!
!   The rectangular coordinates of the Sun are supplied in meters but are
!   required in A.U. - convert them using the constant AU_meters.
!    98.09.11 - Should be using Barycentric coordinates.
      do i=1,3
!**    SUNVCT(I) = SUN(I,1) / AU_meters
       SUNVCT(I) = -EARTH(I,1) / AU_meters
      enddo
!
! Compute the partial derivatives by the chain rule.
!
!   Compute derivatives of RA and DEC wrt the parallax
      DRADPI = (SUNVCT(2)*CRA-SUNVCT(1)*SRA)/CD
      DDCDPI = SUNVCT(3)*CD-SUNVCT(1)*CRA*SD-SUNVCT(2)*SRA*SD
!
!   Apply the chain rule.
      DPLXP(1)=DRADPI*DSTRP(1,1)+DDCDPI*DSTRP(2,1)
      DPLXP(2)=DRADPI*DSTRP(1,2)+DDCDPI*DSTRP(2,2)
!
!   PUT the parallax partials.
      CALL PUT4 ('PLX PART      ',DPLXP,int2(2),int2(1),int2(1))
!
! Test
!     DRADPI = DRADPI * CONVDS
!     DDCDPI = DDCDPI * CONVDS
!     dK(1) = -SD*DDCDPI*CRA - CD*SRA*DRADPI
!     dK(2) = -SD*DDCDPI*SRA + CD*CRA*DRADPI
!     dK(3) =  CD*DDCDPI
!   print dK in arc-seconds
!     WRITE(6,8)'test/dK: ', dK(1)/CONVDS, dK(2)/CONVDS,
!    *             dK(3)/CONVDS
!
!   Check for debug output.
      IF ( KPLXD .EQ. 0 )  GO TO 500
      WRITE ( 6, 9100 )
 9100 FORMAT (1X, "Debug output for subroutine PLXP." )
      WRITE(6,8)' DSTRP   ',DSTRP
    8 FORMAT(A,4D25.16/(7X,5D25.16))
      WRITE(6,8)' SUNVCT  ',SUNVCT
      WRITE(6,8)' VLIGHT  ',VLIGHT
      WRITE(6,8)' SECPAU  ',SECPAU
      WRITE(6,8)' SRA     ',SRA
      WRITE(6,8)' SD      ',SD
      WRITE(6,8)' CRA     ',CRA
      WRITE(6,8)' CD      ',CD
      WRITE(6,8)' DPLXP   ',DPLXP
!
 500  CONTINUE
!
!   New computation of parallax partial w.r.t. inverse distance (parsecs)
      Do J=1,3
       E(J) = EARTH(J,1)
       K(J) = STAR(J)
       B0(J) = EPBASE(J,1)
       V0(J) = EPBASE(J,2)
       Vpw2(J) = EARTH(J,2) + SITEV(J,2)
      Enddo
!   Compute vector to Earth that is perpendicular to star vector
       Call CROSP(E, K, A)
       Call CROSP(K, A, B)
!   Compute parallax correction to star vector at distance of 1 parsec
!    (Radians)
       dK(1) = B(1) / AU_meters * CONVDS
       dK(2) = B(2) / AU_meters * CONVDS
       dK(3) = B(3) / AU_meters * CONVDS
!   print dK in arc-seconds
!     WRITE(6,8)' dK: ', dK(1)/CONVDS, dK(2)/CONVDS,
!    *             dK(3)/CONVDS
!
!  Convert to arc-sec - parsec units
!     DPLXP(1) = DPLXP(1) * CONVDS
!     DPLXP(2) = DPLXP(2) * CONVDS
!   PUT the new parallax partials.
!     CALL PUT4 ('PLX1PSEC      ', DPLXP, 2, 1, 1 )
!
!
!  Compute delay and rate corrections using Consensus model equation (15)
      t_prlx(1) = -(DOTP(dK,B0)/VLIGHT) / (1.D0 + (DOTP(K,Vpw2)/VLIGHT))
     .         + ((DOTP(K,B0)/VLIGHT2) * DOTP(dK,Vpw2))
      t_prlx(2) = -(DOTP(dK,V0)/VLIGHT) / (1.D0 + (DOTP(K,Vpw2)/VLIGHT))
     .         + ((DOTP(K,V0)/VLIGHT2) * DOTP(dK,Vpw2))
      CALL PUT4 ( 'PLX1PSEC      ', t_prlx, int2(2), int2(1), int2(1))
!     WRITE(6,8)' t_prlx(parsec): ',  t_prlx
!
!     Normal termination.
      RETURN
      END
!*********************************************************************
      SUBROUTINE PLXC()
      IMPLICIT None
!
! 6.1.1 PLXC is the parallax module contribution section. Contributions will
!       be computed if KPLXC=1 and if the source distance is given. This is
!       intended for special cases at correlators only.
!
! 6.2   PLXC PROGRAM INTERFACE
!
! 6.2.1 CALLING SEQUENCE - NONE
!
! 6.2.2 COMMON BLOCKS USED
!
      Real*8 Dparsec, t_prlx(2)
      Common /PRLX/ Dparsec, t_prlx
!
      INCLUDE 'ccon.i'
!            VARIABLES 'FROM':
!              1.  KPLXC  -  THE PARALLAX MODULE FLOW CONTROL FLAG.
!              2.  KPLXD  -  THE PARALLAX MODULE DEBUG OUTPUT FLAG.
!
! 6.2.3 PROGRAM SPECIFICATIONS
!
        Real*8 PLXCON(2)
!
! 6.2.6 SUBROUTINE INTERFACE -
!             CALLER SUBROUTINES: DRIVC
!             CALLED SUBROUTINES: NONE
!
! 6.2.9 PROGRAMMER - C. A. KNIGHT  02/26/80
!                    David Gordon 98.09.11 Optional computation of parallax
!                    contributions added.
!                    Jim Ryan Sept 2002 Integer*2/4 mods.
!
! 6.3   PLXC PROGRAM STRUCTURE
!
      IF (KPLXC.eq.1) Then
       PLXCON(1) = t_prlx(1) / Dparsec
       PLXCON(2) = t_prlx(2) / Dparsec
!   PUT the parallax contributions.
        CALL PUT4 ('PRLXCONT      ',PLXCON,int2(2),int2(1),int2(1))
!       WRITE(6,8)' PLXCON  ', PLXCON
      ENDIF
!
    8 FORMAT(A,4D25.16/(7X,5D25.16))
!
      RETURN
      END
