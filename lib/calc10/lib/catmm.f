      SUBROUTINE ATMA()
      Implicit none
!
!     ATMA adds entries to the table of contents for the atmosphere module text
!     message, the flow control message, the partial derivatives, and the
!     contributions arrays.
!
!     ATMI Interface
!
!     Common blocks used - none
!
!            Data base access codes:
!              1. 'ATM MESS' - Access code for the atmosphere module message.
!              2. 'ATM CFLG' - Access code for ATM module control flag message.
!              3. 'EL-THEO ' - Access code for source elevation array.
!              4. 'AZ-THEO ' - Access code for the source azimuth array.
!              5. 'NDRYPART' - Access code for the NHMF2 (Niell) dry partial.
!              6. 'NWETPART' - Access code for the WHMF2 (Niell) wet partial.
!              7. 'NDRYCONT' - Access code for the NHMF2 (Niell) dry
!                              contributions.
!              8. 'NWETCONT' - Access code for the WHMF2 (Niell) wet
!                              contributions.
!              9. 'NGRADPAR' - Access code for the gradient partials using
!                              Niell dry scaling.
!            Old Chau access codes deleted:
!              1. 'ATM PART' - Access code for the Chao dry partial.
!                              (In CALC since first version.)
!              2. 'WET PART' - Access code for the Chao wet partial.
!                              (Added in CALC 7.4.)
!              3. 'ATM CONT' - Access code for Chau dry contributions.
!
!       External I/O - none
!
!       Subroutine Interface -
!             Caller subroutines: TOCUP
!             Called subroutines: ADDA, ADDR
!
!     Programmer - Dale Markham  01/13/77
!      77.07.07  Peter Denatale
!      77.03.06  Bruce Schupler
!      87.03.06  Savita Goel   CDS for A900.
!      89.07.27  Jim Ryan      Documentation simplified.
!      89.12.12  Jim Ryan      UNIX-like database interface implimented.
!      91.05.24  Jim Ryan      Wet partial added and documatation simplified.
!      94.02.03  David Gordon  Access codes for nhmf and whmf partials and
!                              contributions added.
!      94.03.04  David Gordon  Changed Niell Lcodes from: NHMFCONT to NDRYCONT,
!                              NHMFPART to NDRYPART, WHMFCONT to NWETCONT, and
!                              WHMFPART to NWETPART.
!      98.11.19  David Gordon  ADDR for Niell dry atmosphere gradient partial.
!                              Removing all old Chau access codes.
!      Sept 2002 Jim Ryan      Interger*2/4 mods.
!
!     ADD for Atmosphere module test message.
      CALL ADDA (int2(1),'ATM MESS','Atmosphere message definition   ',
     . int2(40), int2(1), int2(1))
!
!     ADD for Atmosphere module flow control message.
      CALL ADDA (int2(1),'ATM CFLG','Atmosphere control flag mess def',
     . int2(40), int2(1), int2(1))
!
!     ADD for Niell Dry (NHMF2) partial derivatives.
      CALL ADDR (int2(2),'NDRYPART','Nhmf2 dry partial deriv. def.   ',
     . int2(2), int2(2), int2(1))
!
!     ADD for Niell Wet (WHMF2) partial derivatives.
      CALL ADDR (int2(2),'NWETPART','Whmf2 wet partial deriv. def.   ',
     . int2(2), int2(2), int2(1))
!
!     ADD for Niell Dry (NHMF2) contributions.
      CALL ADDR (int2(2),'NDRYCONT','Nhmf (dry) atm. contribution    ',
     . int2(2), int2(2), int2(1))
!
!     ADD for Niell Wet (WHMF2) contributions.
      CALL ADDR (int2(2),'NWETCONT','Whmf (wet) atm. contribution    ',
     . int2(2), int2(2), int2(1))
!
!     ADD for Niell Dry atmosphere gradient partials.
      CALL ADDR (int2(2),'NGRADPAR','Niell dry atm. gradient partials',
     . int2(2), int2(2), int2(2))
!
!     ADD for source elevation array.
      CALL ADDR (int2(2),'EL-THEO ','Elevation array definition      ',
     . int2(2), int2(2), int2(1))
!
!     ADD for source azimuth array.
      CALL ADDR (int2(2),'AZ-THEO ','Azimuth array definition        ',
     . int2(2), int2(2), int2(1))
!
      RETURN
      END
!************************************************************************
      SUBROUTINE ATMI()
      Implicit none
!
!     ATMI is the atmosphere module input and initialization section.
!
      INCLUDE 'ccon.i'
!       Variables 'from':
!          1. KATMC - Atmosphere module flow control flag.
!                     = 0 (default), compute Niell dry and
!                       wet partials and contributions, but don't apply them to
!                       the theoreticals.
!                     = 1, as above but DO apply Niell dry to the theoreticals.
!
!     Program specifications -
      INTEGER*2      LATMM(40),      LON(40),    LOFF(40)
      CHARACTER*40 C_LATMM(2) ,    C_LON(2) ,  C_LOFF(2)
      EQUIVALENCE (C_LATMM,LATMM),(C_LON,LON),(C_LOFF,LOFF)
!
      DATA C_LATMM /
     .'Atmosphere Module - Last modification 99',
     .'OCT05, D. Gordon, GSFC.                 '/
!
      DATA C_LON /
     .'Atmosphere Module turned on, Niell dry c',
     .'ontributions applied to theoreticals.   '/
!
      DATA C_LOFF /
     .'Atmosphere Module is turned on - Contrib',
     .'utions NOT applied to theoreticals.     '/
!
!     Database access -
!            'Put' variables:
!              1.  LATMM(40)  -  The atmosphere module text message.
!              2.  LON(40)    -  Module flow control 'ON' message.
!              3.  LOFF(40)   -  Module flow control 'OFF' message.
!            Access codes:
!              1.  'ATM MESS'  -  Access code for module text message.
!              2.  'ATM CFLG'  -  Access code for module status message.
!
!      Subroutine Interface -
!         Caller subroutines: INITL
!         Called subroutines: PUTA
!
!      Programmer - Dale Markham  01/13/77
!       77.07.07  Peter Denatale
!       78.01.03  Bruce Schupler
!       85.01.03  David Grodon  Changed FLAG=1 definition.
!       87.03.06  Savita Goel   CDS for A900.
!       89.06.29  Jim Ryan      Character strings used.
!       89.12.12  Jim Ryan      UNIX-like database interface implimented.
!       91.05.24  Jim Ryan      Documentation simplified.
!       94.02.03  David Gordon  Mods for Nhmf2 and Whmf2 computations.
!       Sept 2002 Jim Ryan      Interger*2/4 mods.
!
!     PUT Atmosphere module text message.
      CALL PUTA ('ATM MESS      ',LATMM,int2(40),int2(1),int2(1))
!
!     PUT module control flag status message according to KATMC.
      IF (KATMC .EQ. 1) CALL PUTA('ATM CFLG      ',LON,int2(40),
     .                  int2(1),int2(1))
      IF (KATMC .NE. 1) CALL PUTA('ATM CFLG      ',LOFF,int2(40),
     .                  int2(1),int2(1))
!
      RETURN
      END
!************************************************************************
      SUBROUTINE ATMG ( R2K, STAR, EARTH, TCTOCF, SITEV, AZ, ELEV,
     .                  STAR_ABERRATED )
      Implicit none
!
!       ATMG
!
!       ATMG is the geometry section of the Atmosphere module. It computes the
!       aberrated source vector and the elevation of the aberrated source and
!       its CT time derivative at each site.
!
!       References - Smart, W.M., 'Textbook on Spherical Astronomy', 1965, P. 68
!
!       ATMG program interface
!
!       Calling sequence -
!           Input variables:
!             1. R2K(3,3,3)    - The complete crust fixed to J2000.0 rotation
!                                matrix and its first two CT time derivatives.
!                                (unitless, 1/sec, 1/sec**2)
!             2. STAR(3)       - The J2000.0 source unit vector. (unitless)
!             3. EARTH(3,3)    - The SSBC position, velocity and acceleration of
!                                the Earth. (m,m/s,m/s**2)
!             4. TCTOCF(3,3,2) - The rotation matrix which rotates the
!                                topocentric reference system to the crust fixed
!                                geocentric reference system at each observation
!                                site. (unitless)
!             5. SITEV(3,2)    - The J2000.0 geocentric velocity of each site.
!                                (m/sec)
!           Output variables:
!             1. STAR_ABERRATED(3,2) - The J2000 source unit vector with
!                            aberration applied at each observing site.
!             2. AZ(2,2)   - The azimuth angle of the source corrrected for
!                            aberration and its CT time derivative at each
!                            site (rad, rad/sec)
!             3. ELEV(2,2) - The elevation angle of the source corrected for
!                            aberration and its CT time derivative at each
!                            observing site (rad,rad/sec)
!
!     Common blocks used -
!
      Real*8 RF(2), RTRACE(2), wetcon(2), Datmp_hmf(2,2),
     .       Datmp_wmf(2,2), Zen_dry(2,2), Zen_wet(2,2)
      COMMON /ATMCM/ RF, RTRACE, wetcon, Datmp_hmf, Datmp_wmf, Zen_dry,
     .               Zen_wet
!           Variables 'from':
!             1. RF(2) - The constants appearing in terms 1 and 2 in the
!                        calculation of the correction angle to be added to
!                        the source zenith angle to find the apparent source
!                        direction due to tropospheric refraction. - See
!                        references. (Radians)
!
      INCLUDE 'ccon.i'
!           Variables 'from':
!             1. KATMC  -  The atmosphere module flow control flag.
!             2. KATMD  -  The atmosphere module debug output flag.
!
      Include 'cphys.i'
!           Variables from:
!             1. VLIGHT  -  The speed of light. (m/s)
!
      INCLUDE 'cobsn.i'
!          Variables from:
!            1. Nzero  -  Set to 1 or 2 if station 1 or 2 is at the geocenter,
!                         otherwise equals zero. For correlator usage.
!
      Real*8         PI,TWOPI,HALFPI,CONVD,CONVDS,CONVHS,SECDAY
      COMMON /CMATH/ PI,TWOPI,HALFPI,CONVD,CONVDS,CONVHS,SECDAY
!           Variables 'from' :
!             1. TWOPI  -  The mathematical constant 2 * PI
!
!     Program specifications -
      Real*8     CFSTAR(3,2), CFTOTC(3,3), R2K(3,3,3), SITLAT(2),
     .           STAR(3), TCSTAR(3,2), TCTOCF(3,3,2), TR2000(3,3,2),
     .           AZ(2,2),TAZ(2,2),ELEV(2,2), STAR_ABERRATED(3,2)
      Real*8 DOTP, EARTH(3,3), VR, SITEV(3,2), Earthplus(3)
      Real*8 Vecmg, starcheck, K_Unit_Aberr(3)
      Real*8 AZQUAD
      Integer*4 I, L
!
!      Database access  -
!      PUT variables:
!        1. ELEV(2,2) - The elevation angle of the source corrrected for
!                       aberration and its CT time derivative at each
!                       site. (rad,rad/sec)
!        2. AZ(2,2)   - The azimuth angle of the source corrrected for
!                       aberration and its CT time derivative at each
!                       site. (rad,rad/sec)
!
!      External I/O - possible debug output.
!
!      Subroutine interface -
!             Caller subroutines: DRIVG
!             Called subroutines: DASIN, DCOS, MTRAN, VECRT, DATAN2, PUT4
!
!      Constants used - none
!
!      Program variables -
!           1. CFSTAR(3,2)   - The aberratted source unit vector in the crust
!                              fixed geocentric reference system and its CT
!                              time derivative. (unitless, 1/sec)
!           2. CFTOTC(3,3)   - The 3x3 rotation matrix which rotates the
!                              geocentric crust fixed reference system to the
!                              topocentric reference system. (unitless)
!           3. TCSTAR(3,2)   - The aberratted source unit vector in the
!                              topocentric reference system and its CT time
!                              derivative.
!                              (unitless, 1/sec)
!           4. TR2000(3,3,2) - The complete J2000.0 to crust fixed rotation
!                              matrix and its first CT time derivative.
!                              (unitless, 1/sec)
!           5. TAZ(2,2)      - A temporary array used in the computation of AZ
!                              (rad,rad/sec)
!           6. AZQUAD        - A variable used to force the source azimuth into
!                              the range 0 to TWOPI rather than -PI to PI.
!           7. VR            - Barycentric speed of the Earth in the direction
!                              of the source.
!
!       Programmer - Dale Markham  01/13/77
!        77.11.07  Peter Denatale
!        78.02.13  Bruce Schupler
!        87.06.03  Savita Goel   CDS for A900)
!        89.05.22  Gregg Cooke
!        89.07.27  Jim Ryan      Documentation simpfilied.
!        89.12.12  Jim Ryan      UNIX-like database interface implimented.
!        89.05.25  Jim Ryan      Documentation simplified again.
!        91:11:05  Jim Ryan      Logic to account for source position
!                                aberration added to the elevation and
!                                azimuth computation.
!        91:12:30  Jim Ryan      Two equations involving VR have had the sign
!                                flipped to make to use of VR consistent with
!                                it definition.
!        93.05.19  David Gordon  STAR_ABERRATED spelling corrected, changed to
!                                a (3,2) variable, diurnal aberration added,
!                                and renormalized to a unit vector. Replaced
!                                derivative of STAR with STAR_ABERRATED for
!                                derivatives of azimuth and elevation
!                                computation.
!        94.03.03  David Gordon  Added computation for unmodified (unaberrated)
!                                source elevations and azimuths.
!        94.10.24  David Gordon  Removed computation of unmodified (unaberrated)
!                                source elevations and azimuths.
!        95.12.18  David Gordon  SITEV added to debug printout.
!        98.08.05  David Gordon  Mods for geocenter station.
!        Sept 2002 Jim Ryan      Interger*2/4 mods.
!
!  ATMG program structure
!
!   Loop twice for the calculation of the elevation and azimuth angles
!   and their CT time derivatives at each site.
!
      DO 500  L = 1,2
!
!  Check for geocenter station
       IF (L .eq. Nzero) Go to 450
!
!  Rotate the J2000.0 source unit vector to the topocentric system.
!   (NOTE: The topocentric system sits at the observation site with the axes
!    pointing Up, East and North.
!
!  Compute the rotation matrix which rotates from the geocentric crust fixed
!    system to the topocentric system.
            CALL MTRAN ( TCTOCF(1,1,L), CFTOTC )
!
!  Compute the rotation matrix which rotates from the J2000.0 system to the
!    geocentric crust fixed system.
            CALL MTRAN ( R2K(1,1,1), TR2000(1,1,1) )
!
!  Apply aberration to the J2000 unit vector. We now add in the diurnal
!    component to each site's velocity, whereas previously only an annual
!    term was used.
            call vecad(EARTH(1,2),SITEV(1,L),Earthplus)
            VR = DOTP(STAR,Earthplus)
           Do I = 1,3
             STAR_ABERRATED(I,L) = STAR(I)
     .       + (Earthplus(i) - VR*STAR(I)) / VLIGHT
           Enddo
!  Normalize for further use here, but keep initial vector for later use
!    in the axis offset module
             Call VUNIT(Star_Aberrated(1,L), K_Unit_Aberr)
!
!
!  Rotate to the crust fixed system:
!           CALL VECRT (TR2000(1,1,1),STAR_ABERRATED(1,L),CFSTAR(1,1))
            CALL VECRT (TR2000(1,1,1),K_Unit_Aberr       ,CFSTAR(1,1))
!  Rotate to the topocentric system:
            CALL VECRT (CFTOTC, CFSTAR(1,1), TCSTAR(1,1))
!
!  Compute the elevation angle of the aberrated source.
            ELEV(L,1) = DASIN ( TCSTAR(1,1) )
!
!     Compute the azimuth angle of the aberrated source.
            TAZ(L,1) = DATAN2(TCSTAR(2,1),TCSTAR(3,1))
            AZQUAD = 0.0D0
            IF (TAZ(L,1) .LT. 0.0D0) AZQUAD = TWOPI
            AZ(L,1) = TAZ(L,1) + AZQUAD
!
!     Compute the CT time derivative of the aberrated source unit vector
!      in the topocentric system.
            CALL MTRAN (R2K(1,1,2), TR2000(1,1,2) )
!           CALL VECRT (TR2000(1,1,2),STAR_ABERRATED(1,L),CFSTAR(1,2))
            CALL VECRT (TR2000(1,1,2),K_Unit_Aberr       ,CFSTAR(1,2))
            CALL VECRT (CFTOTC, CFSTAR(1,2), TCSTAR(1,2) )
!
!     Compute time derivatives of elevation and azimuth
            ELEV(L,2) = TCSTAR(1,2) / DCOS ( ELEV(L,1) )
!
            AZ(L,2) = ( (TCSTAR(2,2) / TCSTAR(3,1)) -
     .                (TCSTAR(2,1) * TCSTAR(3,2) / TCSTAR(3,1)**2) ) /
     .                (1.0D0 + (TCSTAR(2,1) / TCSTAR(3,1))**2)
!
       GO TO 500
!
  450 CONTINUE
!      Geocenter station special handling
!           WRITE ( 6, * ) 'ATMG/Nzero: SITEV = ', SITEV
            call vecad(EARTH(1,2),SITEV(1,L),Earthplus)
            VR = DOTP(STAR,Earthplus)
           Do I = 1,3
             STAR_ABERRATED(I,L) = STAR(I)
     .       + (Earthplus(i) - VR*STAR(I)) / VLIGHT
           Enddo
            ELEV(L,1) = HALFPI
            AZ(L,1)   = 0.0D0
            ELEV(L,2) = 0.0D0
            AZ(L,2)   = 0.0D0
!
!     Close the loop which runs over the sites.
  500 CONTINUE
!
!   PUT the elevation and its time derivative into the data base.
      CALL PUT4('EL-THEO      ', ELEV, int2(2), int2(2), int2(1))
!
!   PUT the azimuth and its time derivative into the data base.
      CALL PUT4('AZ-THEO      ', AZ, int2(2), int2(2), int2(1))
!
!   Check for debug output.
      IF ( KATMD .ne. 0 )  Then
      WRITE ( 6, 9100 )
 9100 FORMAT (1X, "Debug output for subroutine ATMG." )
    8 FORMAT(A,3D25.16/(7X,3D25.16))
      WRITE(6,8)' CFSTAR  ',CFSTAR
      WRITE(6,8)' CFTOTC  ',CFTOTC
      WRITE(6,8)' TCSTAR  ',TCSTAR
      WRITE(6,8)' TR2000  ',TR2000
      write(6,8) ' SITEV  ', SITEV
      write(6,8) ' EARTH  ', EARTH
      write(6,8) ' Earthplus ', Earthplus
      write(6,8) ' VR        ', VR
      write(6,8) ' STAR           ', STAR
      write(6,8) ' STAR_ABERRATED ', STAR_ABERRATED
      WRITE(6,8)' ELEV    ',ELEV
      WRITE(6,8)' AZ      ',AZ
      WRITE(6,8)' TAZ     ',TAZ
      WRITE(6,8)' AZQUAD  ',AZQUAD
      WRITE ( 6, 9200 )  R2K, STAR, TCTOCF,TWOPI
 9200 FORMAT (1X, "R2K    = ", 9 ( 3 ( D30.16, 10X ), /, 1X ), /, 1X,
     .            "STAR   = ", 3 ( D30.16, 4X ), /, 1X,
     .            "TCTOCF = ", 6 ( 3 ( D30.16, 10X ), /, 1X ),1X,
     .            "TWOPI  = ",D30.16)
!
      Endif
!
      RETURN
      END
!************************************************************************
      SUBROUTINE ATMP (ELEV, AZ, SITLAT, SITHEIGHT, XJD, CT, dATMCdh)
      Implicit none
!
!     ATMP is the atmosphere module partial derivatives section. ATMP computes
!     the partial derivatives of the delay and rate with respect to the zenith
!     electrical path delay at each observing site. Separate computations are
!     made for the Chau and the Niell (Nhmf and Whmf) models.
!
!     Calling sequence:
!       'Input' variables:
!          1. ELEV(2,2) - The elevation angle of the source and its CT time
!                         derivative, corrected for aberration, at each
!                         observation site (rad,rad/sec)
!          2. AZ(2,2)   - The azimuth angle of the source and its CT time
!                         derivative, corrected for aberration, at each
!                         observation site (rad,rad/sec)
!          2. SITLAT(2) - The geodetic site latitudes. (rad)
!          3. SITHEIGHT(2)-The site heights above the geoid. (meters)
!          4. XJD       - Julian date at zero hours UTC of the date in question.
!          5. CT        - The coordinate time fraction of the coordinate time
!                         day.
!       'Output' variables:
!          1. dATMCdh(2,2)-Derivative of the Niell dry atmosphere contribution
!                          with respect to station height. First index runs over
!                          the sites and the second over the delay and rate.
!                          (sec/meter, sec/sec/meter)
!
      Real*8 ELEV(2,2), AZ(2,2), SITLAT(2), SITHEIGHT(2), XJD, CT,
     .       dATMCdh(2,2)
!
!   Common blocks used:
!
      Real*8 N_air(2),DAXOC_new(2,2), Daxoc_newer(2,2)
      Common /OBS/N_air,DAXOC_new, Daxoc_newer
!
      Real*8 RF(2), RTRACE(2), wetcon(2), Datmp_hmf(2,2),
     .       Datmp_wmf(2,2), Zen_dry(2,2), Zen_wet(2,2)
      COMMON /ATMCM/ RF, RTRACE, wetcon, Datmp_hmf, Datmp_wmf, Zen_dry,
     .               Zen_wet
!        Variables 'from':
!          1. RTRACE(2) - The constants appearing in the calculation of the
!                         partial derivatives of the delay and the delay
!                         rate with respect to the zenith path delay at each
!                         site. These constants were determined from the
!                         best fit of the ray tracing algorithm for
!                         atmospheric thickness from the zenith to the
!                         horizon. (unitless)  (See references.)
!          2. wetcon(2) - The same as RTRACE, except that RTRACE is
!                         appropriate for the dry atmosphere and wetcon is
!                         appropriate for the wet.
!        Variables 'to':
!          1. Datmp_hmf(2,2) - The Niell (Nhmf) dry atmosphere partial
!                          derivatives of the delay and rate with respect to
!                          the zenith path delays at each site. The first
!                          index runs over the sites and the second runs
!                          over the delay and rate. (sec,sec/sec)
!          2. Datmp_wmf(2,2) - Same as Datmp_hmf, except for the wet Niell
!                          (Whmf) atmosphere partials.
!          3. Zen_dry(2,2)- Dry (hydrostatic) zenith delay (meters), and its
!                          rate of change (m/sec) from Niell. First index runs
!                          over sites, second runs over delay and rate.
!          4. Zen_wet(2,2)- Wet zenith delay (meters), and its rate of change
!                          (m/sec) from Niell. First index runs over sites,
!                          second runs over delay and rate.
!
      INCLUDE 'ccon.i'
!        Variables 'from':
!          1. KATMC  -  The atmosphere module flow control flag.
!          2. KATMD  -  The atmosphere module debug output flag.
!
      INCLUDE 'cphys.i'
!        Variables 'from':
!          1. VLIGHT - Velocity of light in a vacuum. (m/sec)
!
      INCLUDE 'cobsn.i'
!          Variables from:
!            1. Nzero  -  Set to 1 or 2 if station 1 or 2 is at the geocenter,
!                         otherwise equals zero. For correlator usage.
!
      INCLUDE 'cuser.i'
!          Variables from:
!            1. Calc_user - Calc user type. 'A' for Calc/SOLVE analysis.
!                           'C' for VLBI correlator.
!
!     Program specifications -
!
!     Database access -
!        'PUT' variables:
!          1. Datmp_hmf(2,2)     - See above.
!          2. Datmp_wmf(2,2)     - See above.
!          3. Ngrad(2,2,2)       - Atmosphere gradient partials, scaled using
!                                  the Niell dry mapping function. First
!                                  index runs over sites, second over North
!                                  and East components, third over delay and
!                                  rate.
!        Access codes:
!          1. 'NDRYPART' - Access code for dry Niell (Nhmf) partials.
!          2. 'NWETPART' - Access code for wet Niell (Whmf) partials.
!          3. 'NGRADPAR' - Access code for dry Niell atmosphere gradient
!                          partials.
!
!    Subroutine interface -
!        Caller subroutines: DRIVP
!        Called subroutines: DCOS, DSIN, DTAN, PUT4
!
!    Program variables:
      Real*8  hmf(2), wmf(2), hmf2(2,2), wmf2(2,2), epoch, el
      Real*8  Temp, Tdot, Press, Pdot, Relhum, Rdot, X, ZD, ZDdot,
     .        ZW, ZWdot, sithit, Rlat, dXdh, dPdh, dZDdh,
     .        SurPR(2,2), SurTP(2,2), SurHM(2,2), Ngrad(2,2,2)
      Integer*4 N, metPR, metTP, metHM
      Integer*2 ND0(3), Kerr
!
!          1. Press  - Atmospheric presure. (mbar)
!          2. Pdot   - Rate of change of pressure (set to zero).
!          3. Relhum - Relative humidity. (fraction)
!          4. Temp   - Temperature. (Celsius)
!          5. ZD,ZDdot-Dry (hydrostatic) zenith delay, and rate of change,
!                      from Niell model. (meters, meters/sec)
!          6. ZW,ZWdot-Wet zenith delay, and rate of change, from Niell model.
!                      (meters, meters/sec)
!          7. dPdh   - Derivative of pressure with respect to station height.
!          8. dZDdh  - Derivative of zenith delay with respect to station
!                      height.
!          9. hmf2(2,2)-Hydrstatic (dry) mapping function and its derivative
!                      with respect to source elevation. First index runs over
!                      sites, second over the function and the derivative.
!          10. wmf2(2,2)-Wet delay mapping function and its derivative with
!                      respect to source elevation. First index runs over sites,
!                      second over the function and the derivative.
!
! 5.2.9 PROGRAMMER - DALE MARKHAM  01/13/77
!    77.07.11  Peter Denatale
!    89.07.27  Gregg Cooke
!    89.07.27  Jim Ryan  Documentation simpfilied.
!    89.12.13  Jim Ryan  UNIX-like database interface implemented.
!    89.05.24  Jim Ryan  Wet partial introduced and dry partial recode
!                        to simplify. Documetation simplified.
!    94.02.03  David Gordon - Put in ADDR's for 'NDRYPART', 'NWETPART',
!              access codes for Arthur Niell's NHMF2 and WHMF2 dry and wet
!              mapping function partials. Added SITLAT, SITHEIGHT, XJD, and CT
!              to ATMP calling sequence. DATMP changed to Datmp_Chau. Wetp
!              changed to Datmp_wetChau. Added Datmp_hmf(2,2) and Datmp_wmf(2,2)
!              to Common block ATMCM; these are the Niell model dry and wet
!              atmosphere partials w.r.t. the zenith path delay. Calls to
!              subroutines Nhmf2 and Nwmf2 added to get Niell dry and wet
!              mapping functions. PUT4's added for NDRYPART and NWETPART.
!    94.03.23  David Gordon - Moved calls to SASTD and SASTW to compute zenith
!              delays here instead of in ATMC. Added dATMCdh to calling sequence
!              and moved computation to here instead of ATMC.
!    94.03.25  David Gordon - N_air(2) computation added, a priori index of
!              refraction at each site.
!    95.12.18  David Gordon - Added about 30 variables to debug printout.
!    98.08.05  David Gordon - Mods for geocenter station.
!    98.10.01  David Gordon - New include file 'cuser.i' to tell if user is
!              analysis center or correlator. If correlator, allowed to use
!              measured pressures, temperatures, and humidities in
!              computation of Saastomoinen zenith dry and/or wet delays.
!    98.11.19  David Gordon - Adding computation and put for atmosphere
!              gradient partials, using the Niell dry mapping function.
!              Removed all old Chau computations and PUT's.
!    Sept 2002 Jim Ryan - Interger*2/4 mods.
!
! 5.3   ATMP PROGRAM STRUCTURE
!
!  Check to see if we are to use surface met data:
      metPR = 0
      metTP = 0
      metHM = 0
       IF (Calc_user .eq. 'C') Then
         CALL GET4 ('ATM PRES      ',SurPR,int2(2),int2(2),int2(1),ND0,
     .    Kerr)
           If (Kerr .eq. 0) metPR = 1
         CALL GET4 ('TEMP C        ',SurTP,int2(2),int2(2),int2(1),ND0,
     .    Kerr)
           If (Kerr .eq. 0) metTP = 1
         CALL GET4 ('REL.HUM.      ',SurHM,int2(2),int2(2),int2(1),ND0,
     .    Kerr)
           If (Kerr .eq. 0) metHM = 1
       ENDIF
!
!  Now do Arthur Niell's mapping functions:
       epoch = XJD + CT
       Do N=1,2                                      ! Loop over sites
!
!   First check for geocenter station
        IF (N .eq. Nzero) Go to 550
!
        Rlat = SITLAT(N)
        sithit = SITHEIGHT(N)
        el = elev(N,1)
!
        call NHMF2(epoch, rlat, sithit, el, hmf)
        call NWMF2(rlat, el, wmf)
!
!   Partials with respect to zenith path delays:
      if(N.eq.1) then             ! reverse sign for station 1)
        Datmp_hmf(N,1) = -hmf(1)            ! hydrostatic mapping function
        Datmp_hmf(N,2) = -hmf(2)*ELEV(N,2)  ! derivative of above w.r.t. elev.
        Datmp_wmf(N,1) = -wmf(1)            ! wet mapping function
        Datmp_wmf(N,2) = -wmf(2)*ELEV(N,2)  ! derivative of above w.r.t. elev.
      else
        Datmp_hmf(N,1) = hmf(1)             ! hydrostatic mapping function
        Datmp_hmf(N,2) = hmf(2)*ELEV(N,2)   ! derivative of above w.r.t. elev.
        Datmp_wmf(N,1) = wmf(1)             ! wet mapping function
        Datmp_wmf(N,2) = wmf(2)*ELEV(N,2)   ! derivative of above w.r.t. elev.
      endif
!
!    Atmosphere gradient partials using Niell mapping function
!        Delay terms
       Ngrad(N,1,1) = DCOS(AZ(N,1)) / DTAN(ELEV(N,1)) * hmf(1)          !North
       Ngrad(N,2,1) = DSIN(AZ(N,1)) / DTAN(ELEV(N,1)) * hmf(1)          !East
!        Rate terms
       Ngrad(N,1,2)= -DSIN(AZ(N,1))/DTAN(ELEV(N,1))*hmf(1)*AZ(N,2)
     .              - DCOS(AZ(N,1))/DSIN(ELEV(N,1))**2*hmf(1)*ELEV(N,2)
     .              + DCOS(AZ(N,1))/DTAN(ELEV(N,1))*hmf(2) * ELEV(N,2)
       Ngrad(N,2,2) = DCOS(AZ(N,1))/DTAN(ELEV(N,1))*hmf(1)*AZ(N,2)
     .              - DSIN(AZ(N,1))/DSIN(ELEV(N,1))**2*hmf(1)*ELEV(N,2)
     .              + DSIN(AZ(N,1))/DTAN(ELEV(N,1))*hmf(2) * ELEV(N,2)
!     print *,' Ngrad  ', Ngrad(N,1,1),Ngrad(N,2,1),
!    *                    Ngrad(N,1,2),Ngrad(N,2,2)
!
!   Now do the Niell dry and wet (Nhmf and Whmf) contributions:
!
!     Met values, either measured or a priori computed:
        If (metTP .eq. 1 .and. SurTP(N,1).ne.-999.D0) Then
          Temp = SurTP(N,1)
          Tdot = SurTP(N,2)
        Else
          Temp = 293.15D0 - (6.5D-3)*Sithit - 273.16D0
          Tdot = 0.D0
        Endif
!
        If (metPR .eq. 1 .and. SurPR(N,1).ne.-99900.D0) Then
          Press = SurPR(N,1)
          Pdot  = SurPR(N,2)
        Else
          X = 1.D0 - (6.5D-3)*Sithit  / 293.15D0
          Press = 1013.25D0 * (X**5.26D0)
           Pdot = 0.D0
        Endif
!
        If (metHM .eq. 1 .and. SurHM(N,1).ne.-999.D0) Then
          Relhum = SurHM(N,1)
          Rdot   = SurHM(N,2)
        Else
          Relhum = .5D0
           Rdot  = 0.D0
        Endif
!
!    Compute index of refraction in air at each site
        N_air(N) = 77.6D-6*Press/(Temp+273.16D0) + 1.D0
!
!   94Feb15 - Take partials with respect to height
         dXdh = - 6.5D-3 / 293.15D0
         dPdh = 1013.25D0 * 5.26 * (X**4.26D0) * dXdh
!
!  Compute zenith dry and wet delays
      call SASTD(Press,Pdot,Rlat,Sithit,dPdh,ZD,ZDDOT,dZDdh)
      call SASTW(Relhum,Temp,Rdot,Tdot,ZW,ZWDOT)
!
      Zen_dry(N,1) = ZD / VLIGHT
      Zen_dry(N,2) = ZDDOT / VLIGHT
      Zen_wet(N,1) = ZW / VLIGHT
      Zen_wet(N,2) = ZWDOT / VLIGHT
!
!  Find atmosphere contribution partial with respect to height (sec/meter) and
!    its time derivative
        dATMCdh(N,1) = Datmp_hmf(N,1) * dZDdh / VLIGHT
        dATMCdh(N,2) = Datmp_hmf(N,2) * dZDdh / VLIGHT
       Go to 600
!
 550   Continue
!   Geocenter handling:
        Datmp_hmf(N,1) = 0.0D0
        Datmp_hmf(N,2) = 0.0D0
        Datmp_wmf(N,1) = 0.0D0
        Datmp_wmf(N,2) = 0.0D0
        Zen_dry(N,1)   = 0.0D0
        Zen_dry(N,2)   = 0.0D0
        Zen_wet(N,1)   = 0.0D0
        Zen_wet(N,2)   = 0.0D0
        dATMCdh(N,1)   = 0.0D0
        dATMCdh(N,2)   = 0.0D0
        Ngrad(N,1,1)   = 0.0D0
        Ngrad(N,2,1)   = 0.0D0
        Ngrad(N,1,2)   = 0.0D0
        Ngrad(N,2,2)   = 0.0D0
!
 600   Continue
!
      Enddo                                          ! Loop over sites
!
!   'PUT' the Niell partials.
      CALL PUT4 ('NDRYPART      ',Datmp_hmf,int2(2),int2(2),int2(1))
      CALL PUT4 ('NWETPART      ',Datmp_wmf,int2(2),int2(2),int2(1))
      CALL PUT4 ('NGRADPAR      ',Ngrad,int2(2),int2(2),int2(2))
!************
!    Check for debug output.
      IF ( KATMD .ne. 0 )  Then
      WRITE ( 6, 9100 )
 9100 FORMAT (1X, "Debug output for subroutine ATMP." )
    8 FORMAT(A,4D25.16/(7X,5D25.16))
      WRITE(6,8)' ELEV       ',ELEV
      write(6,8)' SITLAT     ',SITLAT
      write(6,8)' SITHEIGHT  ',SITHEIGHT
      write(6,8)' XJD, CT    ',XJD, CT
      write(6,8)' epoch      ',epoch
      write(6,8)' RLAT, Sithit, el ',RLAT, Sithit, el
      write(6,8)' hmf        ',hmf
      write(6,8)' wmf        ',wmf
      write(6,8)' Temp, Press, Relhum ',Temp, Press, Relhum
      write(6,8)' X          ',X
      write(6,8)' N_air      ',N_air
      write(6,8)' dXdh, dPdh ',dXdh, dPdh
      write(6,8)' ZD, ZDDOT, dZDdh ',ZD, ZDDOT, dZDdh
      write(6,8)' ZW, ZWDOT        ',ZW, ZWDOT
      WRITE(6,8)' RTRACE     ',RTRACE
      write(6,8)' wetcon     ',wetcon
      WRITE(6,8)' Datmp_hmf  ',Datmp_hmf
      WRITE(6,8)' Datmp_wmf  ',Datmp_wmf
      WRITE(6,8)' Zen_dry    ', Zen_dry
      WRITE(6,8)' Zen_wet    ', Zen_wet
      WRITE(6,8)' dATMCdh    ', dAtmcdh
      WRITE(6,8)' Ngrad      ', Ngrad
      Endif
!
      RETURN
      END
!************************************************************************
      SUBROUTINE ATMC (ZPATH, DATMC)
      Implicit none
!
!     ATMC is the atmosphere module contributions section. ATMC computes the
!     contributions to the delay and rate due to tropospheric refraction at
!     each observation site.
!
      Real*8 ZPATH(2), DATMC(2,2)
!
!     Calling sequence -
!          Input variables:
!            1. ZPATH(2)  - The nominal zenith atmosphere path delay at each
!                           site. (sec)
!          Output variables:
!            1. DATMC(2,2) - The contributions to the delay and rate due to
!                            tropospheric refraction at each site. The first
!                            index runs over the sites and the second over the
!                            delay and rate. (sec, sec/sec)
!
!     Common blocks used -
!
      Real*8 RF(2), RTRACE(2), wetcon(2), Datmp_hmf(2,2),
     .       Datmp_wmf(2,2), Zen_dry(2,2), Zen_wet(2,2)
      COMMON /ATMCM/ RF, RTRACE, wetcon, Datmp_hmf, Datmp_wmf, Zen_dry,
     .               Zen_wet
!          Variables 'from':
!            1. Datmp_hmf(2,2) - The Niell (Nhmf) dry atmosphere partial
!                            derivatives of the delay and rate with respect to
!                            the zenith path delays at each site. The first
!                            index runs over the sites and the second runs
!                            over the delay and rate. (sec,sec/sec)
!            2. Datmp_wmf(2,2) - Same as Datmp_hmf, except for the wet Niell
!                            (Whmf) atmosphere partials.
!            3. Zen_dry(2,2)- Dry (hydrostatic) zenith delay (meters), and its
!                            rate of change (m/sec) from Niell. First index runs
!                            over sites, second runs over delay and rate.
!            4. Zen_wet(2,2)- Wet zenith delay (meters), and its rate of change
!                            (m/sec) from Niell. First index runs over sites,
!                            second runs over delay and rate.
!
      INCLUDE 'ccon.i'
!          Variables 'from':
!            1. KATMC  -  The atmosphere module flow control flag.
!            2. KATMD  -  the atmosphere module debug output flag.
!
!     INCLUDE 'cphys.i'
!          Variables 'from':
!            1. VLIGHT - Velocity of light in a vacuum. (m/sec)
!
       INCLUDE 'cuser.i'
!          Variables from:
!            1. Calc_user - Calc user type. 'A' for Calc/SOLVE analysis.
!                           'C' for VLBI correlator.
!            2. Wet_atm   - Correlator option to combine dry and wet
!                           atmosphere corrections.
!                           = 'Y' => combine and output in 'NDRYCONT' access
!                             code. Set 'NWETCONT' access code to zero.
!                           = 'N' => don't combine.
!
!    Program specifications -
!    Local variables:
      Integer*4 L,K,N
      Real*8  Datmc_hmf(2,2), Datmc_wmf(2,2)
!
!     Database access -
!         'PUT' variables:
!           1. Datmc_hmf(2,2) - The contributions to the delay and rate due
!                           to the Niell (Nhmf) dry tropospheric refraction 
!                           at each site. The first index runs over the sites
!                           and the second over the delay and rate.
!                           (sec, sec/sec)
!           2. Datmc_hmf(2,2) - The contributions to the delay and rate due
!                           to the Niell (Whmf) wet tropospheric refraction
!                           at each site. The first index runs over the sites
!                           and the second over the delay and rate.
!                           (sec, sec/sec)
!         Access codes:
!           2. 'NDRYCONT' - Access code for the NHMF2 (Niell) dry contributions.
!           3. 'NWETCONT' - Access code for the WHMF2 (Niell) wet contributions.
!
!     Subroutine interface -
!             Caller subroutines: DRIVC
!             Called subroutines: PUT4
!
!     PROGRAMMER - Dale Markham  01/13/77
!      77.07.07  Peter Denatale
!      87.06.03  David Gordon Changed flag definition.
!      87.06.03  Savita Goel  CDS for A900)
!      89.05.22  Gregg Cooke
!      89.07.27  Jim Ryan     Documentation simpfilied.
!      89.12.12  Jim Ryan     UNIX-like database interface implimented.
!      91.05.24  Jim Ryan     Documentation further simplified.
!      94.02.04  David Gordon Added SITLAT and SITHEIGHT to calling sequence.
!                             Added subroutines Sastd and Sastw and calls to
!                             to get Niell model dry and wet Zenith
!                             contributions. PUT4's added for NDRYCONT and
!                             NWETCONT. DATMC set equal to Datmc_hmf for use
!                             elsewhere in Calc.
!      94.03.04  David Gordon Changed dATMCdh to a (2,2) array and added
!                             computation for time derivative of Niell dry
!                             contribution.
!      94.03.23  David Gordon Moved Niell zenith delay and dATMCdh computations
!                             to ATMP.
!      98.11.19  David Gordon Removed all old Chau computations and PUT's.
!      99.10.05  David Gordon Added correlator option to use dry and
!                             wet atmosphere corrections in theory module.
!      Sept 2002 Jim Ryan     Interger*2/4 mods.
!
!     ATMC Program Structure
!
!   Do the Niell dry and wet (Nhmf and Whmf) contributions:
      Do N=1,2
       Datmc_hmf(N,1) = Datmp_hmf(N,1) * Zen_dry(N,1)
       Datmc_hmf(N,2) = Datmp_hmf(N,2) * Zen_dry(N,1)
       Datmc_wmf(N,1) = Datmp_wmf(N,1) * Zen_wet(N,1)
       Datmc_wmf(N,2) = Datmp_wmf(N,2) * Zen_wet(N,1)
!
!  Store the Niell dry atmosphere delays for use elsewhere:
       DATMC(N,1) = Datmc_hmf(N,1)
       DATMC(N,2) = Datmc_hmf(N,2)
      Enddo
!
!  Mods for correlator usage. Add wet component to DATMC for use elsewhere
      If (Calc_user .eq. 'C' .and. Wet_atm .eq. 'Y') Then
       Do N=1,2
        DATMC(N,1) = DATMC(N,1) + Datmc_wmf(N,1)
        DATMC(N,2) = DATMC(N,2) + Datmc_wmf(N,2)
       Enddo
      Endif
!
!  'PUT' the Niell dry and wet contributions.
      CALL PUT4 ('NDRYCONT      ',Datmc_hmf,int2(2),int2(2),int2(1))
      CALL PUT4 ('NWETCONT      ',Datmc_wmf,int2(2),int2(2),int2(1))
!
!     Check for debug output.
      IF ( KATMD .ne. 0 )  Then
      WRITE ( 6, 9100 )
 9100 FORMAT (1X, "Debug output for subroutine  ATMC." )
    8 FORMAT(A,4D25.16/(7X,5D25.16))
      WRITE(6,8)' ZPATH        ',ZPATH
      WRITE(6,8)' Zen_dry   ', Zen_dry
      WRITE(6,8)' Zen_wet   ', Zen_wet
      WRITE(6,8)' Datmc_hmf ',Datmc_hmf
      WRITE(6,8)' Datmc_wmf ',Datmc_wmf
      WRITE(6,8)' DATMC        ',DATMC
      Endif
!
      RETURN
      END
!************************************************************************
      Subroutine NHMF2(epoch,latitude,height,elev,hmf)
      Implicit none
!
!     Routine to compute the hydrostatic mapping function nhmf2 which depends on
!     the day of the year (DOY), station latitude, and height above the geoid.
!
      Integer*4 I
!
!   a,b,c       - the a,b,and c coeffiecents in the continued fraction
!                 form of Marini
!   beta        - intermediate term in calculation
!   gamma       - intermediate term in calculation
!   sine        - sine of elevation angle
!   cose        - cos of elevation angle
!   hmf(1)      - delay mapping function
!   hmf(2)      - d_mapping_function/d_elevation (dhmf2/d_el)
!   topcon      - constant of top of mapping function to ensure
!                 that value is 1.0000 at zenith
!
      Real*8 a,b,c, beta, cose, hmf(2), gamma, sine, topcon
!
!   height     - Height of site above geoid (meters)
!   hs_km      - Height of site in kms
!   latitude   - Latitude (radians)
!   latituded  - Latitude (deg)
!   l          - Absolute value of latitude
!   dl         - Incremental latitude from last lat_hmf
!   elev       - Elevation (radians)
!   epoch      - Julian date of observation, used to get day of year
!   doy        - Days since Dec 31
!   doy_atm    - Doy for atmosphere relative to Jan 28
!   doyr_atm   - Doy_atm in radians;
!   cost       - Cosine(day of year)
!   doy2rad    - Convert doy to radians
!
      Real*8 epoch, doy, latitude,latituded, height, elev
      Real*8 hs_km, l, dl, doy_atm, doyr_atm, cost
!
!   lat_hmf(5)  - Latitudes at which coefficients are defined
!   abc_avg     - Continued fraction coefficients at latitudes lat_hmf
!   abc_amp     - Amplitude of annual variation of abc_avg
!   daavg, daamp, etc - Incremental values for interpolation
!   aavg,  aamp,  etc - Average and amplitude at latitude
!
      Real*8 lat_hmf(5), abc_avg(5,3), abc_amp(5,3), a_ht, b_ht, c_ht
      Common / hmf2_coef/ lat_hmf, abc_avg, abc_amp, a_ht, b_ht, c_ht
!
      Real*8 daavg, daamp, dbavg, dbamp, dcavg, dcamp
!
!   a_ht, b_ht, c_ht - Parameters for continued fraction for height corr'n.
!
!   Define parameters used for calculating coefficients.
!
!     Data lat_hmf / 15, 30, 45, 60, 75/
!
!     Data abc_avg /
!    .1.2769934D-3,1.2683230D-3,1.2465397D-3,1.2196049D-3,1.2045996D-3,
!    .2.9153695D-3,2.9152299D-3,2.9288445D-3,2.9022565D-3,2.9024912D-3,
!    .62.610505D-3,62.837393D-3,63.721774D-3,63.824265D-3,64.258455D-3/
!
!     Data abc_amp /
!    .  0.0D0, 1.2709626D-5, 2.6523662D-5, 3.4000452D-5, 4.1202191D-5,
!    .  0.0D0, 2.1414979D-5, 3.0160779D-5, 7.2562722D-5, 11.723375D-5,
!    .  0.0D0, 9.0128400D-5, 4.3497037D-5, 84.795348D-5, 170.37206D-5/
!
!     Data a_ht / 2.53D-5/
!    .     b_ht / 5.49D-3/
!    .     c_ht / 1.14D-3/
!
      Real*8 aavg,  aamp,  bavg,  bamp,  cavg,  camp
      Real*8 ht_corr_coef, ht_corr
      real*8 dhmf_ht_del, dht_corr_coef_del, dht_corr_del               ! CSJ
!
!   Common Blocks:
!
      COMMON /CMATH/ PI,TWOPI,HALFPI,CONVD,CONVDS,CONVHS,SECDAY
      Real*8         PI,TWOPI,HALFPI,CONVD,CONVDS,CONVHS,SECDAY
!
      INCLUDE 'ccon.i'
!            Variables 'from':
!              1. KATMC  -  The atmosphere module flow control flag.
!              2. KATMD  -  the atmosphere module debug output flag.
!
!   Programmers:
!    00.00.00  Arthur Niell  Subroutine created.
!    93.05.17  Arthur Niell  Use phase of 28 days (winter extremum corresponds
!                            to Jan 28) based on least-square fit to raytrace
!                            of radiosonde data for DRT, ELP, ALB, CHH, FAI,
!                            MUN, and LIH.
!    94.01.27  Dan MacMillan Changed input latitude and elevation to radians,
!                            changed input time to Julian Day.
!    95.12.15  David Gordon  Installed mods by Chris Jacobs (JPL) to compute
!                            height correction for hmf(2). Cleanup/additions to
!                            documentation and debug printout.
!
!   Convert height in meters to kilometers
      hs_km  = height/1000.d0
!
!   If Julian date is used for epoch, then calculate day of year;
!      use 1980 Jan 0 as reference epoch.
      doy = epoch - 2444238.5
!
!   To account for the six month difference in seasons between hemispheres,
!   add 365.25/2 days to doy if station is in the southern hemisphere.
      latituded = latitude/CONVD                 ! convert to degrees
      l = abs(latituded)
      if (latituded .lt. 0) doy = doy + 365.25/2
!
! mod aen 930517 Use phase of 28 days (winter extremum corresponds to Jan 28)
!                based on least-square fit to raytrace of radiosonde data for
!                DRT, ELP, ALB, CHH, FAI, MUN, and LIH.
      doy_atm  = doy - 28.
!  convert to radians
      doyr_atm = doy_atm * TWOPI/365.25
!
      cost = cos(doyr_atm)
!
!   Coefficients for the continued fraction expansion for each latitude.
!
!   For latitudes less than 15 degrees:
      if (l .le. lat_hmf(1)) then
         a = abc_avg(1,1)
         b = abc_avg(1,2)
         c = abc_avg(1,3)
      endif
!
!   For latitudes between 15 and 75  degrees:
      do i = 1,4
          if (l .gt. lat_hmf(i) .and. l .le. lat_hmf(i+1)) then
             dl = (l-lat_hmf(i))/(lat_hmf(i+1)-lat_hmf(i))
             daavg =   abc_avg(i+1,1)-abc_avg(i,1)
             daamp =   abc_amp(i+1,1)-abc_amp(i,1)
             aavg  =   abc_avg(i,1) + dl*daavg
             aamp  =   abc_amp(i,1) + dl*daamp
             a     = aavg - aamp*cost
!
             dbavg =   abc_avg(i+1,2)-abc_avg(i,2)
             dbamp =   abc_amp(i+1,2)-abc_amp(i,2)
             bavg  =   abc_avg(i,2) + dl*dbavg
             bamp  =   abc_amp(i,2) + dl*dbamp
             b     = bavg - bamp*cost
!
             dcavg =   abc_avg(i+1,3)-abc_avg(i,3)
             dcamp =   abc_amp(i+1,3)-abc_amp(i,3)
             cavg  =   abc_avg(i,3) + dl*dcavg
             camp  =   abc_amp(i,3) + dl*dcamp
             c     = cavg - camp*cost
!
          endif
      end do
!
!   for latitudes greater than 75 degrees:
      if (l .ge. lat_hmf(5)) then
         a = abc_avg(5,1)
         b = abc_avg(5,2)
         c = abc_avg(5,3)
      endif
!
!   Now the coefficients exist; calculate the mapping function, hmf(1), and the
!    change of mapping function with elevation, dhmf/d_el = hmf(2).
!   To get delay-rate correction d_tau/dt:
!      d_tau/dt = d_tau-zen/dt*hmf(1) + tau-zen*dhmf/d_el*d_el/dt
!
      sine   = sin(elev )
      cose   = cos(elev )
      beta   = b/(sine + c )
      gamma  = a/(sine + beta)
      topcon = (1.d0 + a/(1.d0 + b/(1.d0 + c)))
!
      hmf(1) = topcon / ( sine + gamma )
!
      hmf(2) = -topcon / ( sine + gamma )**2 *
     .            ( cose - a/( sine + beta)**2 * cose *
     .            ( 1.d0 - b/( sine + c )**2 ) )
!
!   Apply height correction to mapping function only
!         (not to dmf/d_el since this is a small correction):
!      1) height correction coefficient is
!         1/sine(elev) - continued fraction(a_ht,b_ht,c_ht).
!      2) height correction is ht_corr_coef times height in km.
!
      beta   = b_ht/( sine + c_ht )
      gamma  = a_ht/( sine + beta)
      topcon = (1.d0 + a_ht/(1.d0 + b_ht/(1.d0 + c_ht)))
      ht_corr_coef = 1/sine - topcon/(sine + gamma)
      ht_corr      = ht_corr_coef * hs_km
      hmf(1)       = hmf(1) + ht_corr
!
!************************************************
!   CSJ additions December 1995:
!     Calculate the component of d hmf/ d el    due to ht_corr.
!     This will be needed for the second term in delay rate equation,
!      d_tau/dt = d_tau-zen/dt*hmf(1) + tau-zen * dhmf/d_el * d_el/dt
!
!     Assume
!      1) hs_km is constant.
!      2) d el /dt, hs_km, tau-zen are supplied outside this routine
!      3) For convenience of notation define  eldot = d el /dt
!
!     d ht_corr / dt   = d ht_corr      / d el         * eldot
!     d ht_corr / dt   = d ht_corr_coef / d el * hs_km * eldot
!
!     Thus we need d ht_corr_coef / d el
!     Define hmf_ht = topcon/(sinel+gamma)
!
!     d ht_corr_coef / d el  = d csc(el) / d el  - d hmf_ht /d el
!
!     d csc(el) / d el  =   - ( cosel / sinel**2 )
!
!     d hmf_ht / d el = (-topcon*cosel/( sinel + gamma)**2) *
!   (1.d0 -  (a_ht/( sinel + betm )**2) * (1.d0 -  (b_ht/( sinel + c_ht )**2)))
!   1        2     3              3   2   2        3     4              4   321
!   The numbers above show levels of parenthesis
!
      dhmf_ht_del = (-topcon*cose/( sine + gamma)**2)
     .            * (1.d0 -  (a_ht/( sine + beta )**2)
     .            * (1.d0 -  (b_ht/( sine + c_ht )**2) ) )      ! CSJ
!
      dht_corr_coef_del = -cose/sine**2 - dhmf_ht_del           ! CSJ
      dht_corr_del      = dht_corr_coef_del * hs_km             ! CSJ
      hmf(2)            = hmf(2) + dht_corr_del                 ! CSJ
!
!************************************************
!     Check for debug output.
      IF ( KATMD .ne. 0 )  Then
        WRITE ( 6, 9100 )
 9100   FORMAT (1X, "Debug output for subroutine NHMF2." )
    8   FORMAT(A,4D25.16/(7X,5D25.16))
        write(*,'("doy, doy_atm, doyr_atm = ", 3f20.9)') doy, doy_atm,
     .       doyr_atm
        WRITE(6,8)' epoch, elev    ', epoch, elev
        WRITE(6,8)' Height, hs_km  ', Height, hs_km
        WRITE(6,8)' Latitude, Latituded ', Latitude, Latituded
        WRITE(6,8)' a, b, c ', a, b, c
        write(*,'("sine, cose, beta, gamma, topcon = ", 5f10.5)')
     .             sine, cose, beta, gamma, topcon
        write(6,8)' ht_corr_coef, ht_corr ', ht_corr_coef, ht_corr
        WRITE(6,8)' dhmf_ht_del       ', dhmf_ht_del
        WRITE(6,8)' dht_corr_coef_del ', dht_corr_coef_del
        WRITE(6,8)' dht_corr_del      ', dht_corr_del
        write(6,8)' hmf(1), hmf(2)    ', hmf
      Endif
!
      Return
      End
!************************************************************************
      Subroutine NWMF2(latitude, elev, wmf)
      Implicit none
!
!    NWMF2: Routine to compute the new WMF2.0 mapping function which depends
!    only on latitude and elevation.
!
!   Input variables:
!       1. latitude - Latitude (radians)
!       2. elev     - Source selevation. (radians)
!
!   Output variables:
!       1. wmf(1) - Wet delay mapping function.
!       2. wmf(2) - Derivative of the wet delay mapping function (wmf(1) with
!                   respect to elevation.
!
!   Program variables:
!       1. a,b,c      - The a,b,and c coefficients in the continued fraction
!                       form of Marini
!       2. beta       - Intermediate term in calculation
!       3. gamma      - Intermediate term in calculation
!       4. sine       - Sine of elevation angle
!       5. cose       - Cos of elevation angle
!       6. topcon     - Constant of top of mapping fuinction to ensure
!                       value is 1.0000 at zenith
!       7. latituded  - latitude (degrees)
!       8. l          - absolute latitude
!       9. dl         - incremental latitude from last lat_wmf
!      10. dl,da,db,dc- used for interpolation
!
      Real*8 a,b,c, beta, cose, wmf(2), gamma, sine, topcon
      Real*8 lat_wmf(5), abc_w2p0(5,3)
      Real*8 dl, da, db, dc
      Real*8 latitude, latituded, l, elev
      Integer*4 I
!
!   Define parameters used for calculating coefficients.
      Data lat_wmf / 15.D0, 30.D0, 45.D0, 60.D0, 75.D0/
!
!   Coefficients are from fits to raytraces of the standard atmospheres
!   for July for latitudes 15, 45, 60, and 75 degrees latitude and for
!   January for 30 degrees latitude (930517).
      Data abc_w2p0 /
     . 5.8021897D-4,5.6794847D-4,5.8118019D-4,5.9727542D-4,6.1641693D-4,
     . 1.4275268D-3,1.5138625D-3,1.4572752D-3,1.5007428D-3,1.7599082D-3,
     . 4.3472961D-2,4.6729510D-2,4.3908931D-2,4.4626982D-2,5.4736038D-2/
!
!   Common Blocks:
!
      INCLUDE 'ccon.i'
!
      COMMON /CMATH/ PI,TWOPI,HALFPI,CONVD,CONVDS,CONVHS,SECDAY
      Real*8         PI,TWOPI,HALFPI,CONVD,CONVDS,CONVHS,SECDAY
!
!   Programmers:
!     93.05.17  Arthur Niell  Subroutine created.
!     94.01.27  Dan MacMillan Changed input latitude and elevation to radians.
!     95.12.18  David Gordon  Documentation made 'Calc'-like, debug section
!                             added.
!
      latituded = latitude/CONVD             ! convert to degrees
      l = abs(latituded)
!
!   Coefficients for the continued fraction expansion for each latitude.
!
!   for latitudes less than 15 degrees:
      if (l .le. lat_wmf(1)) then
         a = abc_w2p0(1,1)
         b = abc_w2p0(1,2)
         c = abc_w2p0(1,3)
      endif
!
!   for latitudes between 15 and 75  degrees:
      do i = 1,4
          if (l .gt. lat_wmf(i) .and. l .le. lat_wmf(i+1)) then
             dl = (l-lat_wmf(i))/(lat_wmf(i+1)-lat_wmf(i))
             da  =   abc_w2p0(i+1,1)-abc_w2p0(i,1)
             a   =   abc_w2p0(i,1) + dl*da
!
             db  =   abc_w2p0(i+1,2)-abc_w2p0(i,2)
             b   =   abc_w2p0(i,2) + dl*db
!
             dc  =   abc_w2p0(i+1,3)-abc_w2p0(i,3)
             c   =   abc_w2p0(i,3) + dl*dc
!
          endif
      end do
!
!   for latitudes greater than 75 degrees:
      if (l .ge. lat_wmf(5)) then
         a = abc_w2p0(5,1)
         b = abc_w2p0(5,2)
         c = abc_w2p0(5,3)
      endif
!
!   Now the coefficients exist; calculate the mapping function, wmf(1), and the
!    change of mapping function with elevation, dwmf/d_el =wmf(2).
!   To calculate the delay-rate correction, d_tau/dt:
!       d_tau/dt = d_tau_zen/dt * wmf(1) + tau_zen * dwmf/d_el * d_el/dt
!
      sine  = sin( elev )
      cose  = cos( elev )
      beta  = b/( sine + c )
      gamma = a/( sine + beta)
      topcon = (1.d0 + a/(1.d0 + b/(1.d0 + c)))
!
      wmf(1) = topcon / ( sine + gamma )
!
      wmf(2) = -topcon / ( sine + gamma )**2 *
     .         ( cose - a/( sine + beta)**2 * cose *
     .         ( 1.d0 - b/( sine + c )**2 ) )
!
!     Check for debug output.
      IF ( KATMD .ne. 0 )  Then
        WRITE ( 6, 9100 )
 9100   FORMAT (1X, "Debug output for subroutine NWMF2." )
    8   FORMAT(A,4D25.16/(7X,5D25.16))
        WRITE(6,8)' latitude, elev ', latitude, elev
        WRITE(6,8)' a, b, c        ', a, b, c
        WRITE(6,8)' dl, da, db, dc ', dl, da, db, dc
        write(*,'("sine, cose, beta, gamma, topcon = ", 5f10.5)')
     .           sine, cose, beta, gamma, topcon
         write(6,8)' wmf(1), wmf(2) ', wmf
      Endif
!
      return
      end
!************************************************************************
      SUBROUTINE SASTD(P,PDOT,RLAT,SITEHT,dPdh,ZD,ZDDOT,dZDdh)
      IMPLICIT NONE
!
!     SASTD PROGRAM SPECIFICATION
!
!     Calculate zenith delay for "hydrostatic" component of the atmosphere using
!     Saastamoinen formulation and constants from Davis et al.
!
!     REFERENCES:
!       1) Saastamoinen, J. "The Use of Artificial Satellites for
!          Geodesy", Geophys. Monograph Series, vol. 15, ed. by
!          S.W.Henriksen et al, AGU, Wash., D.C., pp 247-251, 1972.
!       2) Davis, J.L., et al., "Geodesy by Radio Interferometry:
!          Effects of Atmospheric Modeling Errors On Estimates Of
!          Baseline Length", Radio Science, 20, 1593-1607, 1985.
!
      REAL*8 P, PDOT, RLAT, SITEHT, dPdh, ZD, ZDDOT, dZDdh
!
!  Calling Sequence:
!
!    Input Variables:
!      1. P      - Pressure. (mbar)
!      2. PDOT   - Rate of change of pressure, set to 0.0. (mbar/sec)
!      3. RLAT   - Latitude. (radians)
!      4. SITEHT - Height of station above geoid. (meters)
!      5. dPdh   - Derivative of pressure with respect to station height.
!
!    Output Variables:
!      1. ZD    - Zenith delay of "hydrostatic" component. (meters)
!      2. ZDDOT - Rate of change of zenith delay. (meters/second)
!      3. dZDdh - Derivative of ZD with respect to station height.
!
!     SUBROUTINE INTERFACE
!       CALLING SUBROUTINES - ATMP
!       CALLED SUBROUTINES  - None
!
      INCLUDE 'ccon.i'
!
      REAL*8 F, dFdh
!
!     Program Variables
!       1. F    - Intermediate result in zenith delay calculation
!       2. dFdh - Derivative of F with respect to height.
!
!     Programmers:
!      ??.??.??  JMG,AEN??  Created?
!      94.02.15  D. Gordon  Added partials of ZD with respect to height.
!      95.12.18  D. Gordon  Documentation made 'Calc'-like, debug printout
!                           section added.
!
!     SASTD PROGRAM STRUCTURE
!
!-----Calculate variation of gravity with station position.
!
      F = 1.0D0-0.00266D0*COS(2.0D0*RLAT) - 0.00028D0*SITEHT/1000.0D0
      ZD = 0.0022768D0*P/F
      ZDDOT = ZD*PDOT/P
!
! 94FEB15 - take partials with respect to height -DG-
      dFdh = -2.8D-7
      dZDdh = 0.0022768D0 * (dPdh  - P*dFdh/F) / F
!
      If (KATMD .ne. 0)  Then
        WRITE ( 6, 9100 )
 9100   FORMAT (1X, "Debug output for subroutine SASTD." )
    8   FORMAT(A,4D25.16/(7X,5D25.16))
        WRITE(6,8)' P, PDOT, dPdh ', P, PDOT, dPdh
        WRITE(6,8)' RLAT, SITEHT  ', RLAT, SITEHT
        WRITE(6,8)' ZD, ZDDOT, dZDdh ', ZD, ZDDOT, dZDdh
        WRITE(6,8)' F, dFdh   ', F, dFdh
      Endif
!
      RETURN
      END
!************************************************************************
      SUBROUTINE SASTW(RH,TC,RHDOT,TCDOT,ZW,ZWDOT)
      IMPLICIT NONE
!
!     SASTW PROGRAM SPECIFICATION
!
!     Calculate zenith delay due to the 'wet' (non-hydrostatic) component of
!     the atmosphere using Saastamoinen formula (19a) (Ref. 1).  The saturation
!     vapor pressure is calculated using the formula found in MET03,  which is
!     much simpler than that found in "Methods of Experimental Physics B"
!    (1976) p. 187, but agrees to 0.3% over the range 0 - 40 deg. C.
!
!     REFERENCES:
!       1) Saastamoinen, J. "The Use of Artificial Satellites for
!          Geodesy", Geophys. Monograph Series, vol. 15, ed. by
!          S.W.Henriksen et al, AGU, Wash, D.C., pp 247-251, 1972.
!       2) Davis, J.L., et al., "Geodesy by Radio Interferometry:
!          Effects of Atmospheric Modeling Errors on Estimates of
!          Baseline Length", Radio Science, 20, 1593-1607, 1985.
!
!     SASTW INTERFACE
!
      REAL*8 RH, TC, RHDOT, TCDOT, ZW, ZWDOT
!
!     Input Variables:
!       1. RH - Relative humidity (set equal to .5).  (0 <= RH <= 1.0)
!       2. TC - Temperature (a function of height). (Celsius)
!       3. RHDOT - Time derivative of RH (set to 0.0).
!       4. TCDOT - Time derivative of TC (set to 0.0).
!
!     Output Variables:
!       1. ZW    - Zenith path delay of wet component. (meters)
!       2. ZWDOT - Time derivative of zenith delay. (meters/sec)
!
!     SUBROUTINE INTERFACE
!       CALLING SUBROUTINES - ATMP
!       CALLED SUBROUTINES  - None
!
      INCLUDE 'ccon.i'
!
      REAL*8 ESAT, TEMP, ESATDOT
!
!     Program Variables:
!       1. ESAT    - Saturation vapor pressure.
!       2. ESATDOT - Time derivative of saturation pressure.
!       3. TEMP    - Used for intermediate values in calculations.
!
!     Programmers:
!      ??.??.??  ?????????  Created?
!      95.12.18  D. Gordon  Documentation made 'Calc'-like, debug printout
!                           section added.
!
!    SASTW PROGRAM STRUCTURE
!
!---Calculate zenith delay due to'wet' (non-hydrostatic) component
!     of atmosphere using Saastamoinen formula 19a. (Ref 1).
!
!     The saturation vapor pressure is calculated using the formula
!     found in MET03 which is much simpler than that found in Methods
!     of Experimental Physics B (1976) p. 187, but agrees to 0.3%
!     over the range 0 - 40 deg. C.
!
!---Calculate saturation vapor pressure, and time derivative.
!     ESAT = 6.11 * 10**(7.5*TC/(TC+237.3))
      TEMP = TC+237.3D0
      ESAT = 6.11D0 * EXP(17.269D0*TC/TEMP)
      ESATDOT = ESAT * (17.269D0/TEMP - 17.269D0*TC/(TEMP*TEMP))*TCDOT
!
!---Calculate zenith path delay
      ZW = 0.002277D0 * (1255.D0/(TC+273.16D0) + 0.05D0) * RH * ESAT
!
!   ... AND RATE OF CHANGE OF ZENITH PATH DELAY
!       CHANGED 255. TO 1255. IN TCDOT CONTRIBUTION       12-28-87 WEH
!       CHANGED RHDOT CONTRIBUTION FROM ZW*RHDOT/RH       12-28-87  WEH
      TEMP = 273.16D0+TC
      ZWDOT = - 0.002277D0 * (TCDOT*1255.D0/(TEMP*TEMP))*RH*ESAT
     .  + 0.002277D0 * (1255.D0/(TC+273.16D0) + 0.05D0) * RHDOT * ESAT
     .  + ZW * ESATDOT/ESAT
!
      If (KATMD .ne. 0)  Then
        WRITE ( 6, 9100 )
 9100   FORMAT (1X, "Debug output for subroutine SASTW." )
    8   FORMAT(A,4D25.16/(7X,5D25.16))
        WRITE(6,8)' RH, RHDOT ', RH, RHDOT
        WRITE(6,8)' TC, TCDOT ', TC, TCDOT
        WRITE(6,8)' ZW, ZWDOT ', ZW, ZWDOT
        WRITE(6,8)' ESAT, TEMP, ESATDOT ', ESAT, TEMP, ESATDOT
      Endif
!
      RETURN
      END
!************************************************************************
      BLOCK DATA ATMCMB
      Implicit none
!
!     ATMBD is the atmosphere module block data input and initialization 
!     section.
!
!     References - Hopfield, H.S., 'Radio Science', 6, 357, 1971,
!                    'JSR', 74, 4487, 1969.
!                - Chau, C.C., 'JPL Technical Manual', #391-129, 1970.
!                - SMART, W.M., 'Textbook on Spherical Astronomy', 1965, P. 68.
!                - Chau, C.C., 'The Tropospheric Calibration Model for Mariner
!                    Mars 1971", JPL Technical Report 32-1587, (NASA Sci & Tech
!                    Info Facility N&$-16983) p. 75, eq. 19 - wet formulation.
!
!     Common blocks:
!
      Real*8 RF(2), RTRACE(2), wetcon(2), Datmp_hmf(2,2),
     .       Datmp_wmf(2,2), Zen_dry(2,2), Zen_wet(2,2)
      COMMON /ATMCM/ RF, RTRACE, wetcon, Datmp_hmf, Datmp_wmf, Zen_dry,
     .               Zen_wet
!
!        Variables 'to':
!           1. RF(2)     - The constants appearing in terms 1 and 2 of the
!                          calculation of the correction angle to be added to
!                          the source elevation angle to find the apparent
!                          source direction due to tropospheric refraction.
!                           RF(1) = 2.826172873D-4 radians
!                           RF(2) = -3.23855539D-7 radians
!           2. RTRACE(2) - The constants appearing in the calculation of the
!                          partial derivatives of the delay and rate with
!                          respect to the zenith path delay at each observing
!                          site. These constants were determined from the
!                          best fit of the ray tracing algorithm for
!                          atmospheric thickness from the zenith to the
!                          horizon. (unitless)  (See references)
!                           RTRACE(1) = 1.43D-3
!                           RTRACE(2) = 4.45D-2
!           3. wetcon(2) - The same as RTRACE, except that RTRACE is
!                          appropriate for the dry atmosphere and wetcon is
!                          appropriate for the wet.
!           4. lat_hmf(5)- Latitudes at which coefficients are defined for the
!                          Niell mapping function.
!           5. abc_avg(5,3) - Continued fraction coefficients at latitudes
!                          lat_hmf
!           6. abc_amp(5,3) - Amplitude of annual variation of abc_avg.
!           7. a_ht, b_ht, c_ht - Parameters for continued fraction for height
!                          correction.
!
      Real*8 lat_hmf(5), abc_avg(5,3), abc_amp(5,3), a_ht, b_ht, c_ht
      Common / hmf2_coef/ lat_hmf, abc_avg, abc_amp, a_ht, b_ht, c_ht
!
!     Program specifications -
!
!   Define parameters used for calculating coefficients.
!
      Data lat_hmf / 15.D0, 30.D0, 45.D0, 60.D0, 75.D0/
!
      Data abc_avg /
     .1.2769934D-3,1.2683230D-3,1.2465397D-3,1.2196049D-3,1.2045996D-3,
     .2.9153695D-3,2.9152299D-3,2.9288445D-3,2.9022565D-3,2.9024912D-3,
     .62.610505D-3,62.837393D-3,63.721774D-3,63.824265D-3,64.258455D-3/
!
      Data abc_amp /
     .  0.0,   1.2709626D-5, 2.6523662D-5, 3.4000452D-5, 4.1202191D-5,
     .  0.0,   2.1414979D-5, 3.0160779D-5, 7.2562722D-5, 11.723375D-5,
     .  0.0,   9.0128400D-5, 4.3497037D-5, 84.795348D-5, 170.37206D-5/
!
      Data a_ht / 2.53D-5/
      Data b_ht / 5.49D-3/
      Data c_ht / 1.14D-3/
!
      Data  RF     / 2.826172873D-4, -3.23855539D-7 /
      Data  RTRACE / 1.43D-3, 4.45D-2 /
      Data  wetcon / 0.35d-3, 1.7d-2 /
!
!    Constants used - RF, RTRACE
!
!    Programmer - Dale Markham  01/13/77
!     77.07.11  Peter Denatale
!     89.05.24  Jim Ryan - Wet partials info added.
!
      END
