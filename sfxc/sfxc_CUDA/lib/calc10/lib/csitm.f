      SUBROUTINE SITA()
      IMPLICIT None
!
! 1.    SITA
!
! 1.1   SITA PROGRAM SPECIFICATION
!
! 1.1.1 SITA ADDs entries to the Table of Contents for the
!       Site Module text message and the partial derivatives array.
!
! 1.2   SITA PROGRAM INTERFACE
!
! 1.2.1 CALLING SEQUENCE - NONE
!
! 1.2.2 COMMON BLOCKS USED -
      INCLUDE 'inputs.i'
!            Variables from:
!              1. Input_sites - T/F logical flag telling whether to use
!                               external site a priori input.
      INCLUDE 'cmxst.i'
!            Variables from:
!              1. NUMSIT   - Number of sites in the data base.
!              2. Max_Stat - Maximun number of stations allowed.
!
      INCLUDE 'cuser.i'
!       Variables from:
!         1. Calc_user  - Calc user type. 'A' for Mark III/SOLVE analysis.
!                         'C' for VLBI correlator.
!
! 1.2.3 PROGRAM SPECIFICATIONS - NONE
!
! 1.2.4 DATA BASE ACCESS -
!            ACCESS CODES ADDED:
!              1.  'SIT MESS'  -  THE DATA BASE ACCESS CODE FOR THE
!                                 SITE MODULE TEXT MESSAGE.
!              2.  'SIT PART'  -  THE DATA BASE ACCESS CODE FOR THE SITE
!                                 MODULE TEXT MESSAGE.
!
! 1.2.5 EXTERNAL INPUT/OUTPUT -  None
!
! 1.2.6 SUBROUTINE INTERFACE -
!           CALLER SUBROUTINES: TOCUP
!           CALLED SUBROUTINES: ADDA, ADDR, ADDI
!
! 1.2.7 CONSTANTS USED - NONE
!
! 1.2.8 PROGRAM VARIABLES - NONE
!
! 1.2.9 PROGRAMMER - DALE MARKHAM  01/13/77
!                    PETER DENATALE 07/13/77
!                    SAVITA GOEL    06/03/87 (CDS FOR A900)
!                    89.07.20 Jim Ryan Documentation simplified.
!                    Jim Ryan 89.12.12 UNIX-like database interface
!                        implimented.
!                    David Gordon 94.04.16 Converted to Implicit None.
!                    David Gordon 94.06.08 Corrected format statements all
!                         subroutines, where single and double quotes reversed.
!                    David Gordon 98.03.17 Mods and ADD's for source a priori's
!                         and ocean loading in the case of external site inputs
!                    David Gordon 98.11.05 Mods/ADD for 'SITEXYZS', velocity
!                         corrected site coordinates. For correlator use only.
!                    David Gordon 99.10.27 Extraneous printout removed.
!                    David Gordon 2001.01.05 Code to add Lcodes 'OCE STAT'
!                         and 'TECTPLNM'.
!                    Jim Ryan 03.03.10 Kill replaced with terminate_solve
!                    Jim Ryan 02.Sept Integer*2/4 Updates.
!                    D. Gordon 2004.05.18 Axis tilt code added.
!
!     SITA Program Structure
!
!     ADD for module text message.
      CALL ADDA (int2(1),'SIT MESS','Site Module Message Definition  ',
     . int2(40), int2(1), int2(1))
!
!     ADD for Site module partials.
      CALL ADDR (int2(2),'SIT PART','Site partial derivative def.    ',
     . int2(3), int2(2), int2(2))
!
!   Do adds to replace the site a priori's in the data base in the case of
!    external file inputs
      If (Input_sites) Then
!**     print *, 'ADDs for external site catalog'
       CALL ADDR(int2(1),'SITERECS','Site cartesian coords (m).      ',
     .  int2(3), Numsit, int2(1))
       CALL ADDR(int2(1),'SITEZENS','Site zenith path delays (nsec). ',
     .  Numsit, int2(1), int2(1))
       CALL ADDA(int2(1),'TECTPLNM','4-char tectonic plate names.    ',
     .  int2(2), Numsit, int2(1))
       CALL ADDI(int2(1),'AXISTYPS','Axis type (1-eq,2-xy,3-azel,4,5)',
     .  Numsit, int2(1), int2(1))
       CALL ADDR(int2(1),'AXISOFFS','Axis offsets (m).               ',
     .  Numsit, int2(1), int2(1))
      Endif
!
!   Do adds to replace the ocean loading coefficients in the data base in
!    the case of external file inputs
      If (Input_ocean) Then
       CALL ADDR(int2(1),'SITOCAMP','Vert ocean loading ampltudes (m)',
     .  int2(11), Numsit, int2(1))
       CALL ADDR(int2(1),'SITOCPHS','Vert ocean loading phases (rad).',
     .  int2(11), Numsit, int2(1))
       CALL ADDR(int2(1),'SITHOCAM','Horz ocean loading ampltudes (m)',
     .  int2(11), int2(2), Numsit)
       CALL ADDR(int2(1),'SITHOCPH','Horz ocean loading phases (rad).',
     .  int2(11), int2(2), Numsit)
       CALL ADDA(int2(1),'OCE STAT','Ocean loading station status.   ',
     .  int2(2), Numsit, int2(1))
      Endif
!
!   Do adds to create/replace the axis tilts in the data base in the case of
!    external file inputs
      If (Input_tilts) Then
       CALL ADDR(int2(1),'AXISTILT','Fixed axis tilts (arc-sec).     ',
     .  int2(2), Numsit, int2(1))
      Endif
!
! Do add for modified/unmodified site positions for correlator usage.
!  Correlator users must capture this access code for downstream analysis
!  (in AIPS, HOPS, etc.) (in place of 'SITERECS') if they use the 'SITERECV'
!  access code for site input. The number of stations will probably not be
!  known at this time. To avoid unnecessary complications, we allow for the
!  maximum number of stations, as defined in 'cmxst.i'.
       If (calc_user .eq. 'C') Then
        CALL ADDR(int2(1),'SITEXYZS','Site Coords To Date (meters)    ',
     .   int2(3), Max_Stat, int2(1))
       Endif
!
!     Normal conclusion.
      RETURN
      END
!*************************************************************************
      SUBROUTINE SITI()
      IMPLICIT None
!
! 3.    SITI
!
! 3.1   SITI PROGRAM SPECIFICATION
!
! 3.1.1 SITI is the Site Module input and initialization section.
!
! 3.1.2 RESTRICTIONS - NONE
!
! 3.1.3 REFERENCES - SMART, W.M., 'TEXTBOOK ON SPHERICAL ASTRONOMY',
!                    1965, P. 195-198
!                    MARKHAM'S X-DOCUMENT
!
! 3.2   SITI PROGRAM INTERFACE
!
! 3.2.1 CALLING SEQUENCE - NONE
!
! 3.2.2 COMMON BLOCKS USED -
!
      INCLUDE 'cphys.i'
!            VARIABLES 'FROM':
!              1. EFLAT  - THE FLATTENNING OF THE ELLIPSOID APPROXIMATING
!                          THE SHAPE OF THE EARTH.  (UNITLESS)
!              2. REARTH - THE EQUATORIAL RADIUS OF THE EARTH. (M)
!
      Real*8           PI, TWOPI, HALFPI, CONVD, CONVDS, CONVHS, SECDAY
      COMMON / CMATH / PI, TWOPI, HALFPI, CONVD, CONVDS, CONVHS, SECDAY
!
      INCLUDE 'cmxst.i'
!
      INCLUDE 'ccon.i'
!            VARIABLES 'FROM':
!              1. KSITC - THE SITE MODULE FLOW CONTROL FLAG.
!              2. KSITD - THE SITE MODULE DEBUG OUTPUT FLAG.
!
      INCLUDE 'inputs.i'
!            Variables from:
!              1. Input_sites - T/F logical flag telling whether to use
!                               external site a priori input
!              2. Input_ocean - T/F logical flag telling whether to
!                               use external ocean loading a priori input
!              3. Ex_sites    - File name for sites external file input.
!                               If 'NONE' or blank, external site input
!                               will not be done.
!              4. Ex_ocean    - File name for ocean loading external file.
!                               If 'NONE' or blank, external ocean loading
!                               input will not be done.
!
      INCLUDE 'cuser.i'
!       Variables from:
!         1. Calc_user  - Calc user type. 'A' for Mark III/SOLVE analysis.
!                         'C' for VLBI correlator.
!
      INCLUDE 'cmxut.i'
!        Variables 'to':
!           1. Intrvl(5,2) - First and last time tag of data in the current
!                            data base. (First index: year, month, day,
!                            hour, minute. Second index: first, last.)
      INCLUDE 'param.i'
!       Variables from:
!           1. A_tilts    - Antenna tilts file name (second priority)
!
! 3.2.3 PROGRAM SPECIFICATIONS -
!
      Integer*2 KERR(12), LSITM(40), NDO(3), NDI(3), NN, numsit_local,
     .                       KERX(2)
      Integer*4 N, I, J, K, L, ik, kj, Imdoy(12), Intmov, Krr
      REAL*8 XLAT_DUMMY,XLON_DUMMY, RY(3,3), RZ(3,3), TCROT_DUMMY(3,3),
     .       RGY(3,3), GLAT_DUMMY, RTROT_DUMMY(3,3), Sitxyzv(7,Max_stat),
     .       T1(3,3), T2(3,3)
      Real*8 xlatlonht(3),Pie,A,Fl,Xyz(3), Xdoy1, Xepoch, X_frac
      CHARACTER*40 C_LSITM(2)
      EQUIVALENCE (LSITM,C_LSITM)
      CHARACTER*1 ITEST, idum(3)
      Character*80 xtlt
!     Logical*4 Input_sites, Input_ocean
!
      DATA C_LSITM /
     .'Site Module - Last modified 2004.05.20, ',
     .'D. Gordon, GSFC                         '/
!
      Data Imdoy /0,31,59,90,120,151,181,212,243,273,304,334/
!
! 3.2.4 DATA BASE ACCESS -
!
!    'GET' VARIABLES:
!      1. KTYPE(Max_Stat)       -  THE ANTENNA AXIS TYPES.  (UNITLESS)
!      2. LNSITE(4,Max_Stat)    -  THE EIGHT ALPHAMERIC CHARACTER SITE NAMES
!                                  OF THE SITES IN THE SITE CATALOG.
!      3. NUMSIT                -  THE NUMBER OF SITES IN THE SITE CATALOG.
!      4. SITAXO(Max_Stat)      -  THE SITE ANTENNA AXIS OFFSETS. (M)
!      5. SITOAM(11,Max_Stat)   -  THE SITE VERTICAL OCEAN LOADING
!                                  AMPLITUDES. (M)
!      6. SITOPH(11,Max_Stat)   -  THE SITE VERTICAL OCEAN LOADING PHASES.
!                                  (RAD)
!      7. SITHOA(11,2,Max_Stat) -  THE SITE HORIZONTAL OCEAN LOADING
!                                  AMPLITUDES. (M)
!      8. SITHOP(11,2,Max_Stat) -  THE SITE HORIZONTAL OCEAN LOADING PHASES.
!                                  (RAD)
!      9. SITXYZ(3,Max_Stat)    -  THE SITE CRUST FIXED X, Y, & Z
!                                  COORDINATES. (M, M, M )
!     10. SITZEN(Max_Stat)      -  THE ZENITH ELECTRICAL PATH LENGTH
!                                  AT EACH OBSERVATION SITE. (SEC)
!     11. Sitxyzv(7,Max_Stat)   -  Optional correlator input via access code
!                                  'SITERECV' in place of SITXYZ (via
!                                  'SITERECS'). First index runs over site crust
!                                  fixed coordinates (X, Y, Z), epoch for
!                                  those coordinates (4-digit and fractional
!                                  year, such as 1998.5), and site velocities
!                                  (X-velocity, Y-velocity, Z-velocity).
!                                  Second index runs over the sites. For
!                                  geocenter station make sure X-velocity =
!                                  Y-velocity = Z-velocity = 0.D0.
!                                  (m, m, m, yrs, m/sec, m/sec, m/sec)
!
!    'PUT' VARIABLES:
!      1. LSITM(40)  -  THE SITE MODULE TEXT MESSAGE.
!      2. SITXYZ(3,Max_Stat) - Correlator output of site crust fixed
!                       coordinates, modified if access code 'SITERECV'
!                       supplied.
!
!    ACCESS CODES:
!      1. 'SIT MESS'  -  THE DATA BASE ACCESS CODE FOR THE SITE MODULE TEXT
!                        MESSAGE.
!      2. 'AXISTYPS'  -  THE DATA BASE ACCESS CODE FOR THE ARRAY OF SITE
!                        ANTENNA TYPES.
!      3. 'SITNAMES'  -  THE DATA BASE ACCESS CODE FOR THE ARRAY OF SITE NAMES.
!      4. '# SITES '  -  THE DATA BASE ACCESS CODE FOR THE NUMBER OF
!                        OBSERVATION SITES.
!      5. 'AXISOFFS'  -  THE DATA BASE ACCESS CODE FOR THE ARRAY OF SITE
!                        ANTENNA AXIS OFFSETS.
!      6. 'SITERECS'  -  THE DATA BASE ACCESS CODE FOR THE  ARRAY OF SITE
!                        X,Y,Z COORDINATES.
!      7. 'SITEZENS'  -  THE DATA BASE ACCESS CODE FOR THE SITE ZENITH
!                        ELECTRICAL PATH LENGTH.
!      8. 'SITOCAMP'  -  THE DATA BASE ACCESS CODE FOR THE VERTICAL OCEAN
!                        LOADING AMPLITUDES.
!      9. 'SITOCPHS'  -  THE DATA BASE ACCESS CODE FOR THE VERTICAL OCEAN
!                        LOADING PHASES.
!     10. 'SITHOCAM'  -  THE DATA BASE ACCESS CODE FOR THE HORIZONTAL OCEAN
!                        LOADING AMPLITUDES.
!     11. 'SITHOCPH'  -  THE DATA BASE ACCESS CODE FOR THE HORIZONTAL OCEAN
!                        LOADING PHASES.
!
! 3.2.6 SUBROUTINE INTERFACE -
!           CALLER SUBROUTINES: INITL
!           CALLED SUBROUTINES: DCOS, DSIN, DSQRT, GETA, GETI,
!                    GET4, TERMINATE_CALC, MMUL2, PUTA, ROTATE, bkplh
!
! 3.2.7 CONSTANTS USED - EFLAT, REARTH
!
! 3.2.8 PROGRAM VARIABLES -
!           1. KERR(12) - THE DATA BASE ERROR RETURN FLAGS.
!           2. NDO(3)   - THE DATA BASE RETURN ARRAY INDICES.
!           3. RY(3,3)  - THE ROTATION MATRIX WHICH PERFORMS A COORDINATE
!                         SYSTEM ROTATION ABOUT THE TOPOCENTRIC Y-AXIS (EAST)
!                         THROUGH AN ANGLE EQUAL TO THE GEODETIC LATITUDE OF
!                         THE CURRENT SITE. (UNITLESS)
!           4. RZ(3,3)  - THE ROTATION MATRIX WHICH PERFORMS A COORDINATE
!                         SYSTEM ROTATION ABOUT THE TOPOCENTRIC Z-AXIS (NORTH)
!                         THROUGH AN ANGLE EQUAL TO THE NEGATIVE EAST LONGITUDE
!                         OF THE CURRENT SITE. (UNITLESS)
!
! 3.2.9 PROGRAMMER - DALE MARKHAM    01/13/77
!                    PETER DENATALE  07/13/77
!                    BRUCE SCHUPLER  03/08/78
!                    CHOPO MA        08/06/81
!                    HAROLD M. SCHUH 10/08/83
!                    SAVITA GOEL 06/03/87 (CDS FOR A900)
!                    LOTHAR MOHLMANN 03/23/89
!                    89.07.20 Jim Ryan Documentation simplified.
!                    Jim Ryan 89:10:05 CPHYS common made an include file.
!                    Jim Ryan 89.12.12 UNIX-like database interface
!                                      implimented.
!                    MSW 93.03.30 SITCM common and maximum number of station
!                                 variable put into newly created include file
!                                 "cmxst.i".
!                    David Gordon 93.10.12 Call to subroutine bkplh added for
!                                 geodetic latitude, longitude, and height;
!                                 Fixed debug printout errors.
!                    David Gordon 94.02.04 HEIGHT(Max_Stat) addded and put into
!                                 cmxst.i; heights above geoid (meters).
!                    David Gordon 94.04.16 Converted to Implicit None.
!                    David Gordon 98.01.22 Modified for station at or near the
!                         geocenter, set topocentric stuff to -999 or so.
!                    David Gordon 98.03.13 Mods for external site input via
!                         an Ascii file.
!                    David Gordon 98.03.17 Mods for source a priori's and ocean
!                         loading coefficients in the case of external file
!                         inputs.
!                    David Gordon 98.06.26 Add code to compute the 'radial-
!                         transverse rotation matrices at each site, for use
!                         in the Earth tide module.
!                    David Gordon 98.10.26 Add code for optional correlator
!                         usage of site velocities. Need input Lcodes
!                         'INTERVAL' and 'SITERECV'. New output Lcode is
!                         'SITERECS' - modified or not.
!                    David Gordon 98.11.05 Put in code to use proposed new
!                         Lcode 'INTRVAL4', the start/stop interval
!                         (yr/month/day/hr/min) using a 4-digit year. If
!                         not there will use 'INTERVAL' (2-digit year).
!                         Correlator output site position Lcode changed to
!                         'SITEXYZS', dimensions (3,Max_stat). 10 year limit
!                         set on velocity interpolation.
!                    Jim Ryan 02.Sept Integer*2/4 Updates.
!                    D. Gordon 2004.05.18 Axis tilt code added.
!                    D. Gordon 2006.01.26 Bug fix for Richmond tilt matrix.
!
!
! 3.3   SITI PROGRAM STRUCTURE
!
!   Initialize site positions for correlator usage
       If (calc_user .eq. 'C') Then
         do ik=1,Max_Stat
          do kj=1,3
           SITXYZ(kj,ik) = 0.D0
          enddo
         enddo
       Endif
!
!     PUT the module text message.
      CALL PUTA ( 'SIT MESS      ', LSITM, int2(40), int2(1), int2(1))
!
!     Use GETA, GETI, & GET4 to obtain the site information from the
!       database. All accessed site info goes into COMMON/SITCM/ in cmxst.i.
!
      CALL GETI ('# SITES       ',NUMSIT,int2(1),int2(1),int2(1),NDO,
     . KERR(1))
      NUMSIT_LOCAL = NUMSIT
      CALL GETA ('SITNAMES      ',LNSITE,int2(4),NUMSIT_LOCAL,int2(1),
     . NDO, KERR(2))
!
!-----------------------------------------------------------------------------
!  Data base input or external file input for sites?
      IF (Input_sites) THEN                    ! Get site info
!
       CALL SITBLK(Kerr)
!
      ELSE                                     ! Get site info
!
       CALL GETI ('AXISTYPS      ',KTYPE,NUMSIT_LOCAL,int2(1),int2(1),
     .  NDO, KERR(3))
       CALL GET4 ('AXISOFFS      ',SITAXO,NUMSIT_LOCAL,int2(1),int2(1),
     .  NDO, KERR(4))
       CALL GET4 ('SITEZENS      ',SITZEN,NUMSIT_LOCAL,int2(1),int2(1),
     .  NDO, KERR(6))
!
!           Look for 4-digit year interval
          CALL GETI ('INTRVAL4      ',Intrvl,int2(5),int2(2),int2(1),
     .     NDO, KERX(2))
!           If no 4-digit year interval, get 2-digit year interval
         If (KERX(2).ne.0) then
          CALL GETI ('INTERVAL      ',Intrvl,int2(5),int2(2),int2(1),
     .     NDO, KERX(2))
!            Convert 2-digit year to 4-digit year
          If (Intrvl(1,1) .ge. 70 .and. Intrvl(1,1) .le. 99)
     .        Intrvl(1,1) = Intrvl(1,1)+1900
          If (Intrvl(1,1) .ge.  0 .and. Intrvl(1,1) .le. 69)
     .        Intrvl(1,1) = Intrvl(1,1)+2000
         Endif
!
!***********************************************
!  Expanded for optional correlator usage of site velocities
       If (calc_user .eq. 'C') Then
!         'SITERECV' should contain X, Y, Z, epoch, X-dot, Y-dot, Z-dot
!           for each station, all Real*8. Epoch must be a 4-digit year and
!           fraction  thereof (1998.0, 2001.5, etc.) If Epoch is zero for a
!           site, then velocity corrections will be turned off for that site.
!           If X=Y=Z=0.D0 (geocenter), then velocity corrections will
!           automatically be turned off.
        CALL GET4 ('SITERECV      ',Sitxyzv,int2(7),NUMSIT_LOCAL,
     .             int2(1), NDO, KERX(1))
!
! !  Apply site velocities from new L-code 'SITERECV'
        If (KERX(1) .eq. 0)  Then
!           Look for 4-digit year interval
!         CALL GETI ('INTRVAL4      ',Intrvl,int2(5),int2(2),int2(1),
!    #     NDO, KERX(2))
!           If no 4-digit year interval, get 2-digit year interval
!        If (KERX(2).ne.0) then
!         CALL GETI ('INTERVAL      ',Intrvl,int2(5),int2(2),int2(1),
!    #     NDO, KERX(2))
!            Convert 2-digit year to 4-digit year
!         If (Intrvl(1,1) .ge. 70 .and. Intrvl(1,1) .le. 99)
!    *        Intrvl(1,1) = Intrvl(1,1)+1900
!         If (Intrvl(1,1) .ge.  0 .and. Intrvl(1,1) .le. 69)
!    *        Intrvl(1,1) = Intrvl(1,1)+2000
!        Endif
!
!  Convert start time to year and fraction, not worrying about leap years:
           xdoy1 = imdoy(Intrvl(2,1)) + Intrvl(3,1) + Intrvl(4,1)/24.d0
     .             + Intrvl(5,1)/1440.d0
           Xepoch = Intrvl(1,1) + xdoy1/365.D0
             Do ik = 1, Numsit_local
              X_frac = Xepoch - Sitxyzv(4,ik)
!              Turn off corrections if no position epoch or at the geocenter
                If (Sitxyzv(4,ik) .eq. 0.D0) X_frac = 0.D0
                If ( (DABS(Sitxyzv(1,ik)) .le. 1.D-6) .and.
     .               (DABS(Sitxyzv(2,ik)) .le. 1.D-6) .and.
     .               (DABS(Sitxyzv(3,ik)) .le. 1.D-6) )  X_frac = 0.D0
!               Make sure interpolation is over no more than 10 years!!
                 If (DABS(X_frac) .gt. 10.D0) Then
                  Write(6,147) ik, Xepoch, Sitxyzv(4,ik)
 147              Format(' Problem in SITI for Site #',I2,
     .             ', Data epoch = ',F10.2, ', Site epoch = ',F10.2,/,
     .             ' Maximum difference allowed is 10 years!! Check',
     .             ' input codes SITERECV and INTRVAL4/INTERVAL. ')
                  CALL TERMINATE_CALC ( 'SITI  ', int2(0), int2(0))
                  STOP
                 Endif
!
               Do kj = 1, 3
!                   Add motion rounded to nearest 0.1 mm
                 Intmov = Sitxyzv(kj+4,ik)*X_frac*1.D4 + .49D0
                SITXYZ(kj,ik) = Sitxyzv(kj,ik) + Intmov/1.D4
               Enddo
             Enddo
! !  No site velocities, use old L-code
        Else
          CALL GET4 ('SITERECS      ',SITXYZ,int2(3),NUMSIT_LOCAL,
     .         int2(1), NDO, KERR(5))
        Endif
!
!   Write out new/old site coordinates for Correlator capture
        CALL PUTR('SITEXYZS      ',SITXYZ,int2(3),Max_Stat,int2(1))
!
       Else
        CALL GET4 ('SITERECS      ',SITXYZ,int2(3),NUMSIT_LOCAL,int2(1),
     .   NDO, KERR(5))
       Endif
!***********************************************
!
      ENDIF                                    ! Get site info
!
!  Check for a geocenter station and set flag if so
        Zero_site = 0
      Do I = 1, Numsit
        If ( (DABS(SITXYZ(1,I)) .le. 1.D-6) .and.
     .       (DABS(SITXYZ(2,I)) .le. 1.D-6) .and.
     .       (DABS(SITXYZ(3,I)) .le. 1.D-6) )  Then
          If (Zero_site .eq. 0) Then
           Zero_site = I
c           Write(6,'("SITI: Goecenter site = site # ",I3)') Zero_site
          Else      ! More than one geocenter site! Not allowed!
           Write(6,'("SITBLK: More than 1 geocenter site! Quitting!")')
           CALL TERMINATE_CALC ( 'SITI  ', int2(0), int2(0))
          Endif
        Endif
      Enddo
!
!-----------------------------------------------------------------------------
!
!  Data base input or external file input for ocean loading?
      IF (Input_OCEAN) THEN        ! Database/external file ocean loading?
!
       CALL OCNIN(Kerr)
!
      ELSE                         ! Database/external file ocean loading?
!
       CALL GET4 ('SITOCAMP      ',SITOAM,int2(11),NUMSIT_LOCAL,int2(1),
     .  NDI, KERR(7))
       CALL GET4 ('SITOCPHS      ',SITOPH,int2(11),NUMSIT_LOCAL,int2(1),
     .  NDI, KERR(8))
       CALL GET4 ('SITHOCAM      ',SITHOA,int2(11),int2(2),NUMSIT_LOCAL,
     .  NDI, KERR(9))
       CALL GET4 ('SITHOCPH      ',SITHOP,int2(11),int2(2),NUMSIT_LOCAL,
     .  NDI, KERR(10))
       IF(KERR(9).NE.0  .OR. KERR(10).NE.0) THEN
         DO I = 1,11
           DO J = 1,2
             DO K = 1,NUMSIT_LOCAL
               SITHOA(I,J,K) = 0.D0
               SITHOP(I,J,K) = 0.D0
             ENDDO
           ENDDO
         ENDDO
        KERR(9)  = 0
        KERR(10) = 0
!
        If(KOCEC.ne.3) Then       !See if the user wants to go on.
          WRITE(6,'(///,
     .    "WARNING: This database does not contain horizontal ocean loa"
     .    ,"ding amplitudes and",/,
     .    "phases.  Arrays for all sites zeroed out.",//,
     .    "Continue (y/(n)) ?",$)')
          ITEST = 'N'
          READ(5,'(A)') ITEST
          IF(ITEST.ne.'Y' .and. ITEST.ne.'y') THEN
            KERR(9)  = 1
            KERR(10) = 1
          ENDIF
!
        Else         !If the ocean loading module control flag is 3, proceed!
          Write(6,'(
     .    " Proceeding with null horizontal ocean loading catalog!")')
        Endif
       ENDIF
!
      ENDIF                        ! Database/external file ocean loading?
!
!-----------------------------------------------------------------------------
!
!  Data base input or external file input for antenna tilts?
      IF (Input_tilts) THEN                 ! Get tilt info from file
       CALL ANTILT(Krr)
         Kerr(11) = Krr
      ELSE                                  ! Get tilt info from database?
       CALL GET4 ('AXISTILT      ',Dbtilt,int2(2),NUMSIT_LOCAL,int2(1),
     .  NDO, KERR(11))
       If (KERR(11) .ne. 0) Then
        Do I = 1, Max_stat
         Dbtilt(1,I) = 0.0
         Dbtilt(2,I) = 0.0
        Enddo
       Endif
!
      ENDIF
!
!  Compute topocentric rotation matrices for each antenna axis tilt.
       Do I = 1, NUMSIT_LOCAL
!
!           Alt-Az case:
        If (KTYPE(I) .eq. 3) Then
         Call ROTAT(-Dbtilt(1,I)*CONVD/60.D0, int2(3), T1)
         Call ROTAT( Dbtilt(2,I)*CONVD/60.D0, int2(2), T2)
         Call MMUL2(T1, T2, Rotilt(1,1,I))
        Endif
!
!           Equatorial, X/Y N-S, or Richmond case:
        If (KTYPE(I) .eq. 1 .or. KTYPE(I) .eq. 2 .or.    
     .      KTYPE(I) .eq. 5) Then
         Call ROTAT( Dbtilt(1,I)*CONVD/60.D0, int2(1), T1)
         Call ROTAT(-Dbtilt(2,I)*CONVD/60.D0, int2(2), T2)
         Call MMUL2(T1, T2, Rotilt(1,1,I))
        Endif
!
!           X/Y E-W case:
        If (KTYPE(I) .eq. 4) Then
         Call ROTAT( Dbtilt(1,I)*CONVD/60.D0, int2(1), T1)
         Call ROTAT( Dbtilt(2,I)*CONVD/60.D0, int2(3), T2)
         Call MMUL2(T1, T2, Rotilt(1,1,I))
        Endif
!
       Enddo
!
!-----------------------------------------------------------------------------
!
      IF (.Not. Input_sites) THEN
!
!     If only one site zenith path delay, copy for all stations.
       IF( NDO(1) .NE. NUMSIT ) THEN
         DO 210 N = 2,NUMSIT
 210       SITZEN(N) = SITZEN(1)
       ENDIF
!
!    Check for database interface errors. If an error is found, TERMINATE_CALC.
       DO N = 1,5
          NN = N
          IF (KERR(NN) .NE. 0) CALL TERMINATE_CALC('SITI  ',NN,KERR(NN))
       ENDDO
!
!     There may be only one site zenith atmosphere delay.
       IF( KERR(6) .NE. 0 .AND. KERR(6) .NE. 2 )
     .     CALL TERMINATE_CALC ('SITI  ',int2(6),KERR(6))
!
      ENDIF
!
!     Check for database interface errors _ ocean loading.
!
      DO 302 N = 7,10
        NN = N
        IF( KERR(N) .EQ. 0) GO TO 302
          CALL TERMINATE_CALC ('SITI  ',NN, KERR(NN))
 302  CONTINUE
!
!     Calculate the neccesary site geometry.
!      Mod added 98JAN22: Dummy out topocentric type variables for station at
!      or near the geocenter, for correlator usage.
!
!     Loop once for each station in the site catalog.
      DO 490  N = 1,NUMSIT
!
!       Compute the site spherical radii.
        CFRAD(N) = DSQRT ( SITXYZ(1,N)**2  +  SITXYZ(2,N)**2  +
     .                     SITXYZ(3,N)**2  )
!
!   Check for geocenter
         If (Zero_site .eq. N) Go to 491
!
!   Compute geocentric latitudes
         GLAT(N) = DASIN( SITXYZ(3,N) / CFRAD(N) )
!
!   93OCT12. Call subroutine bkplh to compute
!    geodetic latitude, geodetic longitude, and height. DG
         pie = pi
         fl = eflat
         a = rearth
         do i=1,3
          xyz(i)=SITXYZ(i,N)
         enddo
        call bkplh(xyz,xlatlonht,pie,a,fl)
! keep longitudes between -PI and +PI
         if (xlatlonht(2) .gt. pi)
     .       xlatlonht(2) = xlatlonht(2) - 2.D0*pi
         XLAT(N)  =  xlatlonht(1)
         XLON(N)  =  xlatlonht(2)
         Height(N) = xlatlonht(3)     ! height in meters
!
!       Compute the site normal unit vectors.
        SNRM(1,N) = DCOS ( XLAT(N) ) * DCOS ( XLON(N) )
        SNRM(2,N) = DCOS ( XLAT(N) ) * DSIN ( XLON(N) )
        SNRM(3,N) = DSIN ( XLAT(N) )
!
!       Compute the partial derivatives of the crust fixed site
!       coordinates with respect to the East longitudes.
        PLON(1,N) = - SITXYZ(2,N)
        PLON(2,N) =   SITXYZ(1,N)
        PLON(3,N) =   0.D0
!
!       Compute the partial derivatives of the crust fixed site
!       coordinates with respect to the geodetic latitudes.
!       (NOTE: The following equations are actually for the geocentric partial
!       derivatives, however, these partials are sufficiently close to the
!       geodetic partials for the purposes of CALC use.)
        PLAT(1,N) = - SITXYZ(3,N) * DCOS (XLON(N) )
        PLAT(2,N) = - SITXYZ(3,N) * DSIN (XLON(N) )
        PLAT(3,N) = + CFRAD(N) * DCOS (XLAT(N) )
!
!     Compute the topocentric-to-crust-fixed rotation matrices by rotating
!     about the geodetic latitude and the longitude. Also now compute a
!     "radial-transverse" rotation matrix by rotating about the geocentric
!     latitude and the longitude.
!
        XLAT_DUMMY = XLAT(N)
        CALL ROTAT ( XLAT_DUMMY, int2(2), RY)
!      write(6,8) ' XLAT? ',  xlat(n)*57.29578
!
        XLON_DUMMY = XLON(N)
        CALL ROTAT ( -XLON_DUMMY, int2(3), RZ)
!      write(6,8) ' XLON? ',  xlon(n)*57.29578
!
        GLAT_DUMMY = GLAT(N)
        CALL ROTAT ( GLAT_DUMMY, int2(2), RGY)
!      write(6,8) ' GLAT? ',  glat(n)*57.29578
!
!       DO I=1,3
!         DO J=1,3
!           TCROT_DUMMY(I,J) = TCROT(I,J,N)
!           RTROT_DUMMY(I,J) = RTROT(I,J,N)
!         ENDDO
!       ENDDO
        CALL MMUL2 ( RZ, RY, TCROT_DUMMY(1,1) )
        CALL MMUL2 ( RZ, RGY,RTROT_DUMMY(1,1) )
        DO I=1,3
          DO J=1,3
            TCROT(I,J,N) = TCROT_DUMMY(I,J)
            RTROT(I,J,N) = RTROT_DUMMY(I,J)
          ENDDO
        ENDDO
!      write(6,8) ' TCROT ',  TCROT_DUMMY
!      write(6,8) ' RTROT ',  RTROT_DUMMY
!
      IF (KSITD .ne. 0) Then  !Station debug printout
       if (N.eq.1) Then
        WRITE ( 6, 1)
        WRITE(6,8)' EFLAT   ',EFLAT
        WRITE(6,8)' REARTH  ',REARTH
        WRITE(6,7)' NUMSIT  ',NUMSIT
       endif
    1  FORMAT (1X, 'Debug output for subroutine SITI.' )
       write(6,'(" For site #",i2)') N
       WRITE(6,4)' RY   ',((RY(J,K),J=1,3),K=1,3)
       WRITE(6,4)' RZ   ',((RZ(J,K),J=1,3),K=1,3)
       WRITE(6,4)' RGY  ',((RGY(J,K),J=1,3),K=1,3)
       WRITE (6,8)' Geoid Height  ',  xlatlonht(3)
      Endif          !Station debug printout
      GO TO 490
!
  491 CONTINUE
!    Dummy out the above topocentric quantities, they have no meaning at the
!     geocenter
        XLAT(N)   = -999.D0
        XLON(N)   = -999.D0
        Height(N) = -999.D0
        SNRM(1,N) = 0.D0
        SNRM(2,N) = 0.D0
        SNRM(3,N) = 0.D0
        GLAT(N)   = -999.D0
        PLON(1,N) = 0.D0
        PLON(2,N) = 0.D0
        PLON(3,N) = 0.D0
        PLAT(1,N) = 0.D0
        PLAT(2,N) = 0.D0
        PLAT(3,N) = 0.D0
        SITAXO(N) = 0.D0
        KTYPE(N)  = 0
        SITZEN(N) = 0.D0
        DO I=1,3
         DO J=1,3
          TCROT(I,J,N) = 0.D0
          RTROT(I,J,N) = 0.D0
         ENDDO
        ENDDO
        DO I=1,11
          SITOAM(I,N) = 0.D0
          SITHOA(I,1,N) = 0.D0
          SITHOA(I,2,N) = 0.D0
          SITOPH(I,N) = 0.D0
          SITHOP(I,1,N) = 0.D0
          SITHOP(I,2,N) = 0.D0
        ENDDO
!
!     Close the loop which runs over the sites in the catalog.
  490 CONTINUE
!
!     Initialize the integer variable NLAST to zero.
      NLAST(1) = 0
      NLAST(2) = 0
!
!     Check KSITD for debug output.
      IF ( KSITD .ne. 0 ) Then  !Debug printout
!     WRITE ( 6, 11)
   11 FORMAT (1X, 'Station Debug for subroutine SITI.' )
      WRITE(6,8)' CFRAD   ',(CFRAD(J),J=1,NUMSIT)
    8 FORMAT(A,4D25.16/(7X,5D25.16))
      WRITE(6,7)' KTYPE   ',(KTYPE(J),J=1,NUMSIT)
    7 FORMAT(/,A,15I8/(7X,15I8))
      WRITE(6,7)' NLAST   ',NLAST
      WRITE(6,4)' PLAT    ',(( PLAT(J,K),J=1,3),K=1,NUMSIT)
    4 FORMAT(/,A,3D25.16/(9X,3D25.16))
      WRITE(6,4)' PLON    ',(( PLON(J,K),J=1,3),K=1,NUMSIT)
      WRITE(6,8)' SITAXO  ',( SITAXO(J),J=1,NUMSIT)
      WRITE(6,9)' SITOAM, ',((SITOAM(J,K),J=1,11),K=1,NUMSIT)
      WRITE(6,9)' SITOPH, ',((SITOPH(J,K),J=1,11),K=1,NUMSIT)
      WRITE(6,9)' SITHOA, ',(((SITHOA(J,L,K),J=1,11),L=1,2),K=1,
     .NUMSIT)
      WRITE(6,9)' SITHOP, ',(((SITHOP(J,L,K),J=1,11),L=1,2),K=1,
     .NUMSIT)
    9 FORMAT(/,A,11F9.4,/,(9X,11F9.4))
      WRITE(6,6)' SITXYZ  ',((SITXYZ(J,K),J=1,3),K=1,NUMSIT)
    6 FORMAT(/,A,3F20.4,/,(9X,3F20.4))
      WRITE(6,8)' SITZEN  ',(SITZEN(K),K=1,NUMSIT)
      WRITE(6,4)' SNRM    ',((SNRM(I,J),I=1,3),J=1,NUMSIT)
      WRITE(6,4)' TCROT   ',(((TCROT(I,J,K),I=1,3),J=1,3),K=1,
     .NUMSIT)
    5 FORMAT(/,A,/,3(3F20.4,/)/)
      WRITE(6,8)' XLAT    ',(XLAT(J),J=1,NUMSIT)
      WRITE(6,8)' XLON    ',(XLON(J),J=1,NUMSIT)
      WRITE(6,8)' HEIGHT    ',(HEIGHT(J),J=1,NUMSIT)
      WRITE(6,3)' LNSITE  ',((LNSITE(J,K),J=1,4),K=1,NUMSIT)
    3 FORMAT (/,A,4A2,/, 9X,4A2/)
       Do I = 1, NUMSIT_LOCAL
         Write(6,1012) I, Dbtilt(1,I), Dbtilt(2,I),
     .     ((Rotilt(kj,ik,I), ik=1,3), kj=1,3)
 1012    Format('Station #',I2,2x,2F10.5,/,'Rotilt: ',3F20.10,  
     .          /,8X,3F20.10,/,8X,3F20.10)
       Enddo
!
!
      Endif          !Debug printout
!
!     Normal conclusion.
      RETURN
      END
!*************************************************************************
      SUBROUTINE SITG (AXOFF, CFBASE, CFLAT, CFLON, CFSITE, CFSITN,
     .                 KAXIS, OCEAMP, OCEPHS,  SITLAT, SITLON, SITRAD,
     .                 TCTOCF, RTTOCF, ZPATH, SITHEIGHT, GEOLAT,
     .                 AXTILT, ROTAXIS)
      IMPLICIT None
!
! 4.    SITG
!
! 4.1   SITG PROGRAM SPECIFICATION
!
! 4.1.1 SITG is the Site Module geometry section. SITG calculates the site
!       geometry for the stations participating in the current observation.
!
! 4.1.2 RESTRICTIONS - NONE
!
! 4.1.3 REFERENCES - MARKHAM'S X-DOCUMENT
!
! 4.2   SITG PROGRAM INTERFACE
!
! 4.2.1 CALLING SEQUENCE -
!
!         OUTPUT VARIABLES:
!           1. AXOFF(2)      - THE ANTENNA AXIS OFFSETS AT EACH SITE. (M)
!           2. CFBASE(3)     - THE CRUST FIXED BASELINE VECTOR. (M)
!           3. CFLAT(3,2)    - THE PARTIAL DERIVATIVES OF THE SITE CRUST FIXED
!                              VECTOR COMPONENTS WITH RESPECT TO THE GEODETIC
!                              LATITUDES AT EACH OBSERVATION SITE. (M/RAD)
!           4. CFLON(3,2)    - THE PARTIAL DERIVATIVES OF THE SITE CRUST FIXED
!                              VECTOR COMPONENTS WITH RESPECT TO THE EAST
!                              LONGITUDES AT EACH OBSERVATION SITE.  (M/RAD)
!           5. CFSITE(3,2)   - THE CRUST FIXED SITE VECTORS AT EACH SITE. (M)
!           6. CFSITN(3,2)   - THE CRUST FIXED SITE NORMAL UNIT VECTORS AT
!                              EACH OBSERVATION SITE. (UNITLESS)
!           7. KAXIS(2)      - THE ANTENNA AXIS TYPES FOR EACH SITE. (UNITLESS)
!           8. OCEAMP(11,3,2)- THE TOPOCENTRIC OCEAN LOADING AMPLITUDES FOR
!                    ( J,K,L)  THE 11 MAIN TIDES (J=1,11),
!                                      K=1 : VERTICAL,
!                                      K=2 : EAST-WEST, AND
!                                      K=3 : NORTH-SOUTH DIRECTION
!                               FOR EACH OBSERVATION SITE (L=1,2). (M)
!           9. OCEPHS(11,3,2)- THE OCEAN LOADING PHASES AT EACH SITE. (RAD)
!          10. SITLAT(2)     - THE GEODETIC LATITUDE AT EACH SITE. (RAD)
!          11. SITLON(2)     - THE EAST LONGITUDE AT EACH SITE. (RAD)
!          12. SITRAD(2)     - THE SPHERICAL EARTH RADIUS OF EACH SITE. (M)
!          13. TCTOCF(3,3,2) - THE ROTATION MATRIX WHICH ROTATES THE
!                              TOPOCENTRIC REFERENCE SYSTEM TO THE CRUST FIXED
!                              REFERENCE SYSTEM AT EACH SITE. (UNITLESS)
!          14. RTTOCF(3,3,2) - The rotation matrix which rotates the
!                              'radial-transverse' reference system to the
!                              crust fixed reference system at each site.
!          15. ZPATH(2)      - THE ZENITH ELECTRICAL PATH LENGTH AT EACH
!                              OBSERVATION SITE.  (SEC)
!          16. SITHEIGHT(2)  - The height above the geoid at each site. (m)
!          17. GEOLAT(2)     - The geocentric latitude at each site. (rad)
!          18. AXTILT(2,2)   - Antenna fixed axis tilts (arc-seconds).
!                              First index runs over the two orthogonal
!                              tilt directions (Alt-Az: 1 => East,
!                              2 => North; (X-Y (N-S or E-W fixed) and
!                              Equatorial: 1 => Az error, 2 => Elev error).
!                              Second index runs over the two stations.
!
      INCLUDE 'cmxst.i'
!
      INCLUDE 'cobsn.i'
!          Variables from:
!            1. Nzero  -  Set to 1 or 2 if station 1 or 2 is at the geocenter,
!                         then used downstream. Otherwise equals zero. For
!                         correlator usage.
!
      INCLUDE 'ccon.i'
!          VARIABLES 'FROM':
!            1.  KSITC  -  THE SITE MODULE FLOW CONTROL FLAG.
!            2.  KSITD  -  THE SITE MODULE DEBUG OUTPUT FLAG.
!
! 4.2.3 PROGRAM SPECIFICATIONS -
!
      Real*8  AXOFF(2),CFBASE(3),CFLAT(3,2),CFLON(3,2),CFSITE(3,2),
     .        CFSITN(3,2),OCEAMP(11,3,2),OCEPHS(11,3,2),SITLAT(2),
     .        SITLON(2),SITRAD(2),TCTOCF(3,3,2),ZPATH(2),SITHEIGHT(2),
     .        RTTOCF(3,3,2), GEOLAT(2), AXTILT(2,2), ROTAXIS(3,3,2)
      Integer*2  KAXIS(2), LNBASE(4,2), NDO(3), KERR
      Integer*4  I, J, K, L, N, NN, ix, jx
!
! 4.2.4 DATA BASE ACCESS -
!          'GET' VARIABLES:
!            1. LNBASE(4,2) - THE EIGHT CHARACTER SITE NAMES OF THE BASELINE
!                             OF THE CURRENT OBSERVATION. (ALPHAMERIC)
!          ACCESS CODES:
!            1. 'BASELINE' - THE DATA BASE ACCESS CODE FOR THE BASELINE
!                            IDENTIFICATION OF THE CURRENT OBSERVATION.
!
! 4.2.6 SUBROUTINE INTERFACE -
!           CALLER SUBROUTINES: DRIVG
!           CALLED SUBROUTINES: GETA, TERMINATE_CALC, VECSB
!
! 4.2.7 CONSTANTS USED - NONE
!
! 4.2.8 PROGRAM VARIABLES -
!            1.  KERR     -  THE DATA BASE ERROR RETURN FLAG.
!            2.  NDO(3)   -  THE DATA BASE RETURN ARRAY INDICES.
!            3.  LOC...(4) - CONTAINS THE NAMES OF THE STATIONS WITH KNOWN
!                            PARAMETERS OF HORIZONTAL DISPLACEMENT DUE TO
!                            OCEAN LOADING.
!
! 4.2.9 PROGRAMMER - DALE MARKHAM    01/13/77
!                    PETER DENATALE  07/13/77
!                    CHOPO MA        08/06/81
!                    HAROLD M. SCHUH 10/08/83
!                    SAVITA GOEL 06/03/87 (CDS FOR A900)
!                    LOTHAR MOHLMANN 03/23/89
!                    89.07.20 Jim Ryan Documentation simplified.
!                    Jim Ryan 89.12.12 UNIX-like database interface
!                                      implimented.
!                    93.03.30 MSW SITCM common and maximum number of station
!                             variable put into newly created include file
!                             "cmxst.i".
!                    94.02.04 David Gordon SITHEIGHT(2) added, station heights
!                             above geoid (meters).
!                    David Gordon 94.04.16 Converted to Implicit None.
!                    David Gordon 98.06.26 Adding rotation matrix RTTOCF(3,3,2)
!                             and geocentric latitudes GEOLAT(2), for use later
!                             in the solid Earth tide module.
!                    David Gordon 98.07.29 Added 'Include cobsn.i' with
!                             variable Nzero, and code to determine when a
!                             station is at the geocenter.
!                    Jim Ryan 02.Sept Integer*2/4 Updates.
!                    D. Gordon 2004.05.18 Axis tilt code added.
!
!     SITG program structure.
!
!     GET the baseline name. Check for errors.
      CALL GETA ('BASELINE      ',LNBASE,int2(4),int2(2),int2(1),NDO,
     . KERR)
      IF ( KERR .EQ. 0 ) GO TO 310
      CALL TERMINATE_CALC ('SITG  ', int2(1), KERR)
!
!   Set geocenter indicator to zero
      Nzero = 0
!
!     Construct the arrays to hold the geometry of the stations participating
!     in the current observation so that this information can be passed to the
!     rest of the program.
!
!     Loop for sites 1 and 2.
  310 DO 3130  L = 1,2
!
!       Determine the identification of the stations participating in
!       the current observation. If the baseline identification is not
!       successful, a message is written and the program terminates.
!
        DO 320  NN = 1, NUMSIT
          N = NN
          IF   ( ( LNSITE(1,NN) .EQ. LNBASE(1,L) )
     .    .AND.  ( LNSITE(2,NN) .EQ. LNBASE(2,L) )
     .    .AND.  ( LNSITE(3,NN) .EQ. LNBASE(3,L) )
     .    .AND. ( LNSITE(4,NN) .EQ. LNBASE(4,L) ) )  GO TO 330
  320   CONTINUE
!
        GO TO 700
!
!       Check to see if the ID of the baseline has changed from that of the
!       previous observation. If not, then retain the current site geometry.
!
  330   IF ( NLAST(L) .EQ. N )  GO TO 3130
        NLAST(L) = N
!
!       Construct the array to hold the crust fixed site vectors.
        CFSITE(1,L) = SITXYZ(1,N)
        CFSITE(2,L) = SITXYZ(2,N)
        CFSITE(3,L) = SITXYZ(3,N)
!
!       Construct the array to hold the site spherical radii.
        SITRAD(L) = CFRAD(N)
!
!       Construct the array to hold the site normal unit vectors.
        CFSITN(1,L) = SNRM(1,N)
        CFSITN(2,L) = SNRM(2,N)
        CFSITN(3,L) = SNRM(3,N)
!
!       Construct the arrays to hold the geodetic latitudes and the East
!       longitudes. Also now an array for geocentric latitude.
        SITLAT(L) = XLAT(N)
        SITLON(L) = XLON(N)
        SITHEIGHT(L) = HEIGHT(N)
        GEOLAT(L) = GLAT(N)
!
!       Construct arrays to hold the partial derivatives of the crust fixed site
!       coordinates with respect to the longitudes and the geodetic latitudes.
!
        CFLON(1,L) = PLON(1,N)
        CFLON(2,L) = PLON(2,N)
        CFLON(3,L) = PLON(3,N)
!
        CFLAT(1,L) = PLAT(1,N)
        CFLAT(2,L) = PLAT(2,N)
        CFLAT(3,L) = PLAT(3,N)
!
!       Construct the array to hold the site antenna axis offsets.
        AXOFF(L) = SITAXO(N)
!
!       Construct the array to hold the site antenna types.
        KAXIS(L) = KTYPE(N)
!
!       Construct the array to hold the site antenna axis tilts.
        AXTILT(1,L) = Dbtilt(1,N)
        AXTILT(2,L) = Dbtilt(2,N)
!
!       Construct arrays to hold the fixed axis rotation matrices
        Do ix = 1,3
         Do jx = 1,3
          ROTAXIS(ix,jx,L) = Rotilt(ix,jx,N)
         Enddo
        Enddo
!
!       Construct the array to hold the topocentric to crust fixed
!       rotation matrices. Now also an array for the radial-transverse
!       rotation matrices.
        DO 3112  J = 1,3
          DO 3111  I = 1,3
            TCTOCF(I,J,L) = TCROT(I,J,N)
            RTTOCF(I,J,L) = RTROT(I,J,N)
 3111     CONTINUE
 3112   CONTINUE
!
!       Construct the array to hold the zenith electrical path lengths
        ZPATH(L) = SITZEN(N)
!
!       Construct the arrays to hold the site ocean loading amplitudes
!       and phases.
        DO J = 1, 11
          OCEAMP(J,1,L) = SITOAM(J,N)
          OCEAMP(J,2,L) = SITHOA(J,1,N)
          OCEAMP(J,3,L) = SITHOA(J,2,N)
!
          OCEPHS(J,1,L) = SITOPH(J,N)
          OCEPHS(J,2,L) = SITHOP(J,1,N)
          OCEPHS(J,3,L) = SITHOP(J,2,N)
        ENDDO
!
!  Check for geocenter station
       If(Zero_site .ne. 0 .and. N .eq. Zero_site) Then
        Nzero = N
c        WRITE ( 6, * )  ' SITG: Geocenter Site Found, Nzero =  ', Nzero
       Endif
!
!     Close the loop which runs over the sites.
 3130 CONTINUE
!
!     Construct the array to hold the crust fixed baseline vector.
      CALL VECSB ( CFSITE(1,1), CFSITE(1,2), CFBASE )
!
!     Check KSITD for debug output.
      IF ( KSITD .EQ. 0 )  GO TO 600
      WRITE ( 6, 1)
    1 FORMAT (1X, 'Debug output for subroutine SITG.' )
      WRITE(6,8)' CFRAD   ',(CFRAD(J),J=1,NUMSIT)
    8 FORMAT(A,4D25.16/(7X,5D25.16))
      WRITE(6,7)' KTYPE   ',(KTYPE(J),J=1,NUMSIT)
    7 FORMAT(/,A,15I8/(7X,15I8))
      WRITE(6,7)' NLAST   ',NLAST
      WRITE(6,7)' NUMSIT  ',NUMSIT
      WRITE(6,4)' PLAT    ',(( PLAT(J,K),J=1,3),K=1,NUMSIT)
    4 FORMAT(/,A,3D25.16/(9X,3D25.16))
      WRITE(6,4)' PLON    ',(( PLON(J,K),J=1,3),K=1,NUMSIT)
      WRITE(6,8)' SITAXO  ',( SITAXO(J),J=1,NUMSIT)
      WRITE(6,9)' SITOAM, ',((SITOAM(J,K),J=1,11),K=1,NUMSIT)
      WRITE(6,9)' SITOPH, ',((SITOPH(J,K),J=1,11),K=1,NUMSIT)
      WRITE(6,9)' SITHOA, ',(((SITHOA(J,L,K),J=1,11),L=1,2),K=1,
     .NUMSIT)
      WRITE(6,9)' SITHOP, ',(((SITHOP(J,L,K),J=1,11),L=1,2),K=1,
     .NUMSIT)
    9 FORMAT(/,A,11F7.4,/,(9X,11F7.4))
      WRITE(6,6)' SITXYZ  ',((SITXYZ(J,K),J=1,3),K=1,NUMSIT)
    6 FORMAT(/,A,3F20.4,/,(9X,3F20.4))
      WRITE(6,8)' SITZEN  ',(SITZEN(K),K=1,NUMSIT)
      WRITE(6,4)' SNRM    ',((SNRM(I,J),I=1,3),J=1,NUMSIT)
      WRITE(6,4)' TCROT   ',(((TCROT(I,J,K),I=1,3),J=1,3),K=1,
     .NUMSIT)
    5 FORMAT(/,A,/,3(3F20.4,/)/)
      WRITE(6,8)' XLAT    ',(XLAT(J),J=1,NUMSIT)
      WRITE(6,8)' XLON    ',(XLON(J),J=1,NUMSIT)
      WRITE(6,8)' GLAT    ',(GLAT(J),J=1,NUMSIT)
      WRITE(6,8)' HEIGHT  ',(HEIGHT(J),J=1,NUMSIT)
!
      WRITE ( 6, 9200 )  AXOFF, CFBASE, CFLAT, CFLON, CFSITE,
     .           CFSITN, KAXIS, OCEAMP, OCEPHS, SITLAT, SITLON,
     .           SITRAD, TCTOCF, ZPATH,  LNBASE
 9200 FORMAT (1X, 'AXOFF  = ', 2 ( D30.16, 10X ), /, 1X,
     .            'CFBASE = ', 3 ( D30.16, 10X ), /, 1X,
     .            'CFLAT  = ',/, 2 ( 3 ( D30.16, 10X ), /, 1X ),
     .            'CFLON  = ',/, 2 ( 3 ( D30.16, 10X ), /, 1X ),
     .            'CFSITE = ',/, 2 ( 3 ( D30.16, 10X ), /, 1X ),
     .            'CFSITN = ',/, 2 ( 3 ( D30.16, 10X ), /, 1X ),
     .            'KAXIS  = ',/, 2 ( I2, 10X ), /, 1X,
     .            'OCEAMP = ',/,2( 3( 11F10.4,/ ),/),/,1X,
     .            'OCEPHS = ',/,2( 3( 11F10.4,/ ),/,1X ),/,1X,
     .            'SITLAT = ', 2 ( D30.16, 10X ), /, 1X,
     .            'SITLON = ', 2 ( D30.16, 10X ), /, 1X,
     .            'SITRAD = ', 2 ( D30.16, 10X ), /, 1X,
     .            'TCTOCF = ',/,6( 3 ( D30.16, 10X ), /, 1X ),
     .            'ZPATH  = ', 2 ( D30.16, 10X ), /, 1X,
     .            'LNBASE = ', 4A2,1X,4A2)
!
!     Normal conclusiton.
  600 RETURN
!
!     Abnormal conclusion.       .
  700 WRITE ( 6, 9300 )
 9300 FORMAT (1X, 'CALC has been terminated in subroutine SITG.  ',
     .            'The baseline identification was not successful.' )
      CALL TERMINATE_CALC ( 'SITG  ', int2(0), int2(0))
      END
!*************************************************************************
      SUBROUTINE SITP (R2K, STAR, EARTH, SITEV)
      IMPLICIT None
!
! 5.    SITP
!
! 5.1   SITP PROGRAM SPECIFICATION
!
! 5.1.1 SITP is the Site Module partial derivatives section. SITP
!       computes the partial derivatives of the delay and the rate with
!       respect to the site crust fixed vector components at each site.
!
! 5.1.2 RESTRICTIONS - NONE
!
! 5.1.3 REFERENCES - MARKHAM'S X-DOCUMENT
!
! 5.2   SITP PROGRAM INTERFACE
!
! 5.2.1 CALLING SEQUENCE -
!
!         INPUT VARIABLES:
!           1.  R2K(3,3,3) - THE COMPLETE CRUST FIXED TO J2000.0 ROTATION
!                            MATRIX AND ITS FIRST TWO CT TIME DERIVATIVES.
!                            (UNITLESS, 1/SEC, 1/SEC**2)
!           2.  STAR(3)    - THE J2000.0 SOURCE UNIT VECTOR. (UNITLESS)
!           3.  EARTH(3,3) - The position, velocity, and acceleration of the
!                            Earth relative to the SSBC. (m, m/s, m/s**2)
!           4. SITEV(3,2)  - THE J2000.0 GEOCENTRIC VELOCITY VECTORS OF EACH
!                            OBSERVATION SITE. (M/SEC)
!
!         OUTPUT VARIABLES: NONE
!
! 5.2.2 COMMON BLOCKS USED -
!
      INCLUDE 'cphys.i'
!          VARIABLES 'FROM':
!            1.  VLIGHT  -  THE VELOCITY OF LIGHT IN VACUUM. (M/SEC)
!            2.  VLIGHT2 -  THE VELOCITY OF LIGHT SQUARED.
!
      INCLUDE 'ccon.i'
!          VARIABLES 'FROM':
!            1.  KSITC  -  THE SITE MODULE FLOW CONTROL FLAG.
!            2.  KSITD  -  THE SITE MODULE DEBUG OUTPUT FLAG.
!
! 5.2.3 PROGRAM SPECIFICATIONS -
!
      Real*8 R2K(3,3,3), STAR(3), EARTH(3,3), SITEV(3,2), DBDX1(3,2),
     .       DBDX2(3,2), DBDY1(3,2), DBDY2(3,2), DBDZ1(3,2),
     .       DBDZ2(3,2), DSITP(3,2,2), VG(3), VE(3), c1, c2, tt, DOTP
      Integer*4 I, K
!
! 5.2.4 DATA BASE ACCESS -
!
!          'GET' VARIABLES: NONE
!
!          'PUT' VARIABLES:
!            1. DSITP(3,2,2) - THE PARTIAL DERIVATIVES OF THE DELAY AND THE
!                              DELAY RATE WITH RESPECT TO THE CRUST FIXED SITE
!                              COORDINATES AT EACH OBSERVATION SITE. THE FIRST
!                              INDEX RUNS OVER THE SITE COORDINATES, THE SECOND
!                              INDEX RUNS OVER THE SITES, AND THE THIRD RUNS
!                              OVER THE DELAY AND THE DELAY RATE.
!                              (SEC/M, SEC/SEC-M)
!
!          ACCESS CODES:
!            1. 'SIT PART'  -  THE DATA BASE ACCESS CODE FOR THE SITE MODULE
!                              PARTIAL DERIVATIVES ARRAY.
!
! 5.2.5 EXTERNAL INPUT/OUTPUT -  POSSIBLE DEBUG OUTPUT
!
! 5.2.6 SUBROUTINE INTERFACE -
!           CALLER SUBROUTINES: DRIVP
!           CALLED SUBROUTINES: DOTP, PUT4
!
! 5.2.7 CONSTANTS USED - VLIGHT
!
! 5.2.8 PROGRAM VARIABLES -
!       1.  DBDX1(3,2)  -  THE PARTIAL DERIVATIVES OF THE J2000.0 BASELINE
!                          POSITION AND VELOCITY VECTORS WITH RESPECT TO THE
!                          X-COMPONENT OF THE CRUST FIXED SITE VECTOR AT
!                          OBSERVATION SITE #1. (M/M, M/M-SEC)
!       2.  DBDX2(3,2)  -  THE PARTIAL DERIVATIVES WITH RESPECT TO THE
!                          X-COMPONENT AT SITE #2.  (M/M, M/M-SEC)
!       3.  DBDY1(3,2)  -  THE PARTIAL DERIVATIVES WITH RESPECT TO THE
!                          Y-COMPONENT AT SITE #1.  (M/M, M/M-SEC)
!       4.  DBDY2(3,2)  -  THE PARTIAL DERIVATIVES WITH RESPECT TO THE
!                          Y-COMPONENT AT SITE #2.  (M/M, M/M-SEC)
!       5.  DBDZ1(3,2)  -  THE PARTIAL DERIVATIVES WITH RESPECT TO THE
!                          Z-COMPONENT AT SITE #1.  (M/M, M/M-SEC)
!       6.  DBDZ2(3,2)  -  THE PARTIAL DERIVATIVES WITH RESPECT TO THE
!                          Z-COMPONENT AT SITE #2.  (M/M, M/M-SEC)
!       7.  VE(3)       -  A local copy of the velocity of the Earth
!                          relative to the SSBC.
!       9.  ci          -  1.d0/VLIGHT
!      10.  tt          -  A term common to all partials.
!
! 5.2.9 PROGRAMMER - DALE MARKHAM   01/13/77
!                    PETER DENATALE 07/13/77
!                    CHOPO MA       08/06/81
!                    89.07.20 Jim Ryan Documentation simplified.
!                    Jim Ryan 89:10:05 CPHYS common made an include file.
!                    Jim Ryan 89.12.12 UNIX-like database interface
!                                   implimented.
!                    Jim Ryan 91.11.22 Next term from the delay and rate
!                                   theoreticals added to partials computation.
!                    David Gordon 94.04.16 Converted to Implicit None.
!                    David Gordon 98.10.15 Added SITEV to input arguments.
!                                   Changed site delay and rate partials
!                                   computations to use the Consensus formula.
!                                   Differences are very small and probably
!                                   not noticeable.
!                    Jim Ryan 02.Sept Integer*2/4 Updates.
!
!     SITP program structure
!
!     Loop twice for delay and rate partials for both sites.
!      [Index K runs over the delays and rates.]
      DO 300  K = 1,2
!
!   Loop three times for the calculation of the partials with respect to the
!    crust fixed vector components.
        DO 140  I = 1,3
!
!    Compute the partial derivatives of the J2000.0 baseline position and
!    velocity vectors with respect to the crust fixed vector coordinates
!    at site #2.
          DBDX2(I,K) = R2K(I,1,K)
          DBDY2(I,K) = R2K(I,2,K)
          DBDZ2(I,K) = R2K(I,3,K)
!
!    Compute the partial derivatives of the J2000.0 baseline position and
!    velocity vectors with respect to the crust fixed site coordinates
!    at site #1.
          DBDX1(I,K) = - R2K(I,1,K)
          DBDY1(I,K) = - R2K(I,2,K)
          DBDZ1(I,K) = - R2K(I,3,K)
!
!    Close the loop running over the vector components.
  140   CONTINUE
!
!    Complete the calculation of the partial derivatives of the delay and the
!    rate with respect to the crust fixed site vector components at each site.
!
!    First make a local copy of the velocity of the Earth.
!       DO I =1,3
!         VG(I) = EARTH(I,2)
!       Enddo
!       c1 = 1.d0/VLIGHT
!       c2 = 1.d0/VLIGHT2
!       tt = 1.d0 - c1*DOTP(star,vg)
!
!       DSITP(1,1,K)=-c1*DOTP(DBDX1(1,K),STAR)*tt-c2*DOTP(DBDX1(1,K),VG)
!       DSITP(2,1,K)=-c1*DOTP(DBDY1(1,K),STAR)*tt-c2*DOTP(DBDY1(1,K),VG)
!       DSITP(3,1,K)=-c1*DOTP(DBDZ1(1,K),STAR)*tt-c2*DOTP(DBDZ1(1,K),VG)
!       DSITP(1,2,K)=-c1*DOTP(DBDX2(1,K),STAR)*tt-c2*DOTP(DBDX2(1,K),VG)
!       DSITP(2,2,K)=-c1*DOTP(DBDY2(1,K),STAR)*tt-c2*DOTP(DBDY2(1,K),VG)
!       DSITP(3,2,K)=-c1*DOTP(DBDZ2(1,K),STAR)*tt-c2*DOTP(DBDZ2(1,K),VG)
!     WRITE(6,8)' Old DSITP ', DSITP(1,1,K), DSITP(2,1,K),DSITP(3,1,K)
!     WRITE(6,8)' Old DSITP ', DSITP(1,2,K), DSITP(2,2,K),DSITP(3,2,K)
!
! Change to use the Consensus model definition
        DO I =1,3
          VG(I) = EARTH(I,2) + SITEV(I,2)
          VE(I) = EARTH(I,2)
        Enddo
         tt = 1.d0 + DOTP(STAR,VG)/VLIGHT
!
        DSITP(1,1,K) = -DOTP(DBDX1(1,K),STAR)/VLIGHT/tt
     .                 - DOTP(DBDX1(1,K),VE)/VLIGHT2
        DSITP(2,1,K) = -DOTP(DBDY1(1,K),STAR)/VLIGHT/tt
     .                 - DOTP(DBDY1(1,K),VE)/VLIGHT2
        DSITP(3,1,K) = -DOTP(DBDZ1(1,K),STAR)/VLIGHT/tt
     .                 - DOTP(DBDZ1(1,K),VE)/VLIGHT2
        DSITP(1,2,K) = -DOTP(DBDX2(1,K),STAR)/VLIGHT/tt
     .                 - DOTP(DBDX2(1,K),VE)/VLIGHT2
        DSITP(2,2,K) = -DOTP(DBDY2(1,K),STAR)/VLIGHT/tt
     .                 - DOTP(DBDY2(1,K),VE)/VLIGHT2
        DSITP(3,2,K) = -DOTP(DBDZ2(1,K),STAR)/VLIGHT/tt
     .                 - DOTP(DBDZ2(1,K),VE)/VLIGHT2
!     WRITE(6,8)' New DSITP ', DSITP(1,1,K), DSITP(2,1,K),DSITP(3,1,K)
!     WRITE(6,8)' New DSITP ', DSITP(1,2,K), DSITP(2,2,K),DSITP(3,2,K)
!
!    Close the loop which runs over the partials of the delay and rate.
  300 CONTINUE
!
!    PUT the site module partials.
      CALL PUT4 ('SIT PART      ',DSITP,int2(3),int2(2),int2(2))
!
!    Check KSITD for debug output.
      IF ( KSITD .EQ. 0 )  GO TO 700
      WRITE ( 6, 9)
    9 FORMAT (1X, 'Debug output for subroutine SITP.' )
    8 FORMAT(A,4D25.16/(7X,5D25.16))
      WRITE(6,8)' DBDX1   ',DBDX1
      WRITE(6,8)' DBDX2   ',DBDX2
      WRITE(6,8)' DBDY1   ',DBDY1
      WRITE(6,8)' DBDY2   ',DBDY2
      WRITE(6,8)' DBDZ1   ',DBDZ1
      WRITE(6,8)' DBDZ2   ',DBDZ2
      WRITE(6,8)' DSITP   ',DSITP
      WRITE(6,8)' VLIGHT  ',VLIGHT
      WRITE(6,8)' c1      ',c1
      WRITE(6,8)' c2      ',c2
      WRITE(6,8)' tt      ',tt
      WRITE(6,8)' vg      ',vg
      WRITE ( 6, 9200 )  R2K, STAR
 9200 FORMAT (1X, 'R2K =  ', 9 ( 3 ( D30.16, 10X ), /, 1X ),
     .            'STAR = ',     3 ( D30.16, 10X ) )
!
!     Normal conclusion.
  700 RETURN
      END
!*************************************************************************
      SUBROUTINE bkplh(XYZ,PLH,PI,A,FL)
      IMPLICIT NONE
!
! NAME             bkplh.f
!
! VERSION          93.01.27
!
! WRITTEN          B. Archinal, USNO, July 20-23, 1990.
!                  Name changed from "borkow" to "bkplh", and arguments
!                  adapted for efficient use by Calc (csitm), Dbcal,
!                  and Solve.  BA, 93.01.27.
!
! PURPOSE          Converts XYZ coordinates to Phi, Lambda, H
!                  ellipsoidal coordinates.
!
! References       Borkowski, K. M. (1989).  "Accurate Algorithms to
!                  transform geocentric to geodetic coordinates"
!                  *Bulletin Geodesique*, v. 63, pp. 50-56.  Also see
!                  Borkowski, K. M. (1987).  "Transformation of
!                  Geocentric to Geodetic Coordinates without
!                  Approximations", *Astrophysics and Space Science*,
!                  v. 139, n. 1, pp. 1-4.  Correction in (1988), v. 146,
!                  n. 1, p. 201.
!
! Note             Will not work for points on the Z axis, i.e. if
!                  if X=Y=0 (Phi = +/- 90 degrees).
!
! Calling sequence CALL bkplh ( XYZ, PLH, PI, A, FL )
!
! ARGUMENT LIST
!
!  PARM       TYPE DESCRIPTION
!
!  XYZ(3)     D    INPUT - XYZ Cartesian coordinates of point.
!                  XYZ(1) and XYZ(2) must not both be zero.  Units are
!                  those of A below.
!  PLH(3)     D    OUTPUT - Ellipsoidal coordinates of point, in
!                  geodetic latitude, longitude, and height.  Units
!                  for latitude and longitude are in radians, units
!                  of height are those of A below.
!  PI         D    INPUT - Ratio of circumference to diameter of circle.
!                  Unitless.
!  A          D    INPUT - Semi-major axis of ellipsoid.  Units are
!                  of distance (meters, kilometers, miles, etc.).
!  FL         D    INPUT - Flattening of ellipsoid.  Unitless.
!
!
! SUBPROGRAMS USED
!  Fortran         DABS      DACOS     DATAN     DATAN2   DCOS
!                  DSIN      DSQRT
!
! COMMON BLOCKS    None.
!
! INPUT            None.
!
! OUTPUT           None, unless diagnostic printout uncommented.
!
! LANGUAGE         Fortran 77.
!
!===================================================================
!
      Real*8 A,B,D,DABS,DACOS,DATAN,DATAN2,DCOS,DSIN,
     .       DSQRT,E,F,FL,G,P,PI,Q,R,T,V,X,Y,Z,ZLONG
      Real*8 XYZ(3),PLH(3)
!     INTEGER IOUT
!
!--- XYZ.
      X=XYZ(1)
      Y=XYZ(2)
      Z=XYZ(3)
!--- Semi-minor axis.
      B=A*(1.D0-FL)
!--- Set sign of B to that of Z in order to get sign of Phi correct.
      IF(Z.LT.0.D0) B=-B
!--- Intermediate Values for Latitude.
      R=DSQRT(X*X+Y*Y)
      E=(B*Z-(A*A-B*B))/(A*R)
      F=(B*Z+(A*A-B*B))/(A*R)
      P=4.D0/3.D0 * (E*F+1)
      Q=2.D0 * (E*E - F*F)
      D=P*P*P+Q*Q
      IF(D.GE.0.D0) then
        V=(DSQRT(D)-Q)**(1.D0/3.D0) - (DSQRT(D)+Q)**(1.D0/3.D0)
        else
        V=2.D0 * DSQRT(-P) * DCOS (1.D0/3.D0 *
     .  DACOS(Q/(P * DSQRT(-P))))
        endif
!   (Improve V - not really necessary except near axes.)
      IF(V*V.LT.DABS(P)) V=-(V*V*V + 2.D0*Q)/(3.D0*P)
      G=(DSQRT(E*E+V)+E)/2.D0
      T=DSQRT( G*G  + (F-V*G)/(2.D0*G-E) ) - G
      PLH(1)=DATAN( (A*(1.D0-T*T))/(2.D0*B*T) )
!--- HEIGHT.
      PLH(3)=(R-A*T)*DCOS(PLH(1)) + (Z-B)*DSIN(PLH(1))
!--- LONGITUDE.
      ZLONG=DATAN2(Y,X)
      IF(ZLONG.LT.0.D0) ZLONG=ZLONG+2.D0*PI
      PLH(2)=ZLONG
!
!   Diagnostic output.
!
!     IOUT=11
!     WRITE(IOUT,901) A,F,B
! 901 FORMAT(' A,F,B:',3D25.16)
!     WRITE(IOUT,902) X, Y, Z
! 902 FORMAT(' X, Y, Z:',3D25.16)
!     WRITE(IOUT,903) R,E,F
! 903 FORMAT(' R, E, F:',3D25.16)
!     WRITE(IOUT,904) P,Q,D
! 904 FORMAT(' P, Q, D:',3D25.16)
!     WRITE(IOUT,905) V,G,T
! 905 FORMAT(' V, G, T:',3D25.16)
!--- Check.
!     CHK1=T*T*T*T + 2.D0 * E *T*T*T + 2.D0 * F *T - 1.D0
!     CHK2=V*V*V + 3.D0*P*V + 2.D0*Q
!     WRITE(IOUT,906) CHK1,CHK2
! 906 FORMAT('Check values (=0):',2D25.16)
      RETURN
      END
!*************************************************************************
      SUBROUTINE SITBLK(Kerr)
      Implicit None
!
      INCLUDE 'cmxst.i'
      INCLUDE 'inputs.i'
!
      Integer*2 Getunit, Kerr(10)
      Integer*4 I, II, Jsite(Max_stat), Iunit, An_typ, ios,
     .          Iquit, Index
      Real*8 Sit_X, Sit_Y, Sit_Z, Ax_off, S_zen
!
      Character*80 Inbuf
      Character*8  Dbsites(Max_stat)
      Equivalence (LNSITE(1,1), Dbsites(1))
!
      Character*4 T_plate(Max_stat), plate
      Integer*2   L_plate(2,Max_stat)
      Equivalence (T_plate,L_plate)
!
!  Programmer/History:
!       David Gordon 1998.03.17 Program created.
!       David Gordon 2001.01.05 Code to PUT Lcode TECTPLNM.
!       David Gordon 2006.03.30 Revised missing station message.
!       David Gordon 2006.04.03 Revised missing station message again.
!
!   Initialize station counter
       Do I = 1, Max_stat
         Jsite(i) = 0
       Enddo
!
!  Open the input sites data file
       Iunit = getunit()
       Open (Unit=Iunit, File=Ex_sites, Status='old',
     .       Err=240, Iostat=ios)
!
      If ( Index(Ex_sites,'blokq') .gt. 0) Then
!  If it is a blokq.dat file, find the site catalog section
  50   Continue
       Read(iunit,'(A80)') Inbuf
!      If (Inbuf(1:18) .eq. '$$ STATION CATALOG') Go to 100
       If (Inbuf(1:4) .eq. '$$//') Go to 100
       Go to 50
 100   Continue
      Endif
!
 110   Continue
       Read(iunit,'(A80)',end=200) Inbuf
!   Skip comments and illegal lines
!      If (Inbuf(1:2) .eq. '$$'  ) Go to 110
       If (Inbuf(1:4) .ne. '    ') Go to 110
       If (Inbuf(13:13) .ne. ' ' ) Go to 110
!
!   Finished site catalog
       If (Inbuf(1:2) .eq. '//') Go to 200
!
       Do I = 1, Numsit
         If (Inbuf(5:12) .eq. Dbsites(I)) Then
!         print *, 'Site matched: ', Dbsites(I)
!  Matched I'th station in data base station list, save station particulars
           II = I
           Read(Inbuf(14:80),*) Sit_X, Sit_Y, Sit_Z, An_typ, Ax_off,
     .                          S_zen, plate
!
           SITXYZ(1,II) = Sit_X
           SITXYZ(2,II) = Sit_Y
           SITXYZ(3,II) = Sit_Z
           SITAXO(II)   = Ax_off
           KTYPE(II)    = An_typ
           SITZEN(II)   = S_zen
           T_plate(II)  = plate
!
           Jsite(II)    = II
!
         Endif
!
       Enddo
!
       Go to 110
!
 200   Continue
!
       Close(Iunit)
!
!   Verify that we have site a priori's for all stations. If not, we must
!    quit here and tell the user to fix the problem.
!
        Iquit = 0
      DO I = 1, Numsit
        If (Jsite(i) .eq. 0) Then
          If (iquit.eq.0) Write(6,'(/)')
          Write(6,'(" SITBLK: Missing station: ",A8)') Dbsites(I)
          Iquit = Iquit + 1
        Endif
         If (Iquit .gt. 0) Then
             Write(6,'(/,"!!! Missing sites!!! Update file ",/,10X,A80, 
     .        /,3X, " and rerun Calc!!!",/ )') Ex_sites
             Call TERMINATE_CALC( 'SITBLK',int2(0), int2(0))
         Endif
      ENDDO
!
       If (Iquit.eq.0) Then
        Kerr(3) = 0
        Kerr(4) = 0
        Kerr(5) = 0
        Kerr(6) = 0
       Else
        Call TERMINATE_CALC( 'SITBLK', int2(0), int2(0))
       Endif
!
!       print *, ' SITXYZ ', SITXYZ
!       print *, ' SITAXO ', SITAXO
!       print *, ' KTYPE  ', KTYPE
!       print *, ' SITZEN ', SITZEN
!
!  Now we must replace the site a priori's in the data base
      CALL PUTR('SITERECS      ', SITXYZ, int2(3), Numsit, int2(1))
      CALL PUTR('SITEZENS      ', SITZEN, Numsit, int2(1), int2(1))
      CALL PUTI('AXISTYPS      ', KTYPE, Numsit, int2(1), int2(1))
      CALL PUTR('AXISOFFS      ', SITAXO, Numsit, int2(1), int2(1))
      CALL PUTA('TECTPLNM      ', L_plate, int2(2), Numsit, int2(1))
!
       Go to 270
!
!   error on OPEN
 240  Continue
      WRITE ( 6, * )  'Error on OPEN of blokq file '
!
 270  Continue
      Return
      End
!*************************************************************************
      SUBROUTINE OCNIN(Kerr)
      Implicit None
!
      INCLUDE 'cmxst.i'
      INCLUDE 'inputs.i'
!
      Real*8           PI, TWOPI, HALFPI, CONVD, CONVDS, CONVHS, SECDAY
      COMMON / CMATH / PI, TWOPI, HALFPI, CONVD, CONVDS, CONVHS, SECDAY
!         VARIABLES 'FROM':
!            1. CONVD - THE CONVERSION FACTOR FROM DEGREES TO RADIANS (RAD/DEG)
!
      Integer*2 Getunit, Kerr(10)
      Integer*4 I, II, Jsite(Max_stat), Iunit, ios, Iquit, Index, K
      Real*8    OC11(11)
!
      Character*80 Inbuf
      Character*8  Dbsites(Max_stat)
      Equivalence (LNSITE(1,1), Dbsites(1))
!
      Character*4 Ocestat(Max_stat)
      Integer*2 L_ocestat(2,Max_stat)
      Equivalence (Ocestat,L_ocestat)
!
!  Programmer/History:
!       David Gordon 98.03.17 - Program created.
!       David Gordon 99.11.19 - Bug fix - corrected ocean loading external
!                               file site name.
!       David Gordon 2000.01.05 - Determine ocean loading statuses (must be
!                               'YES' for all stations) and PUT them into
!                               the type 1 records, Lcode 'OCE STAT'.
!       David Gordon 2006.03.30 Revised missing station message.
!       David Gordon 2006.04.03 Revised missing station message again.
!
!   Initialize station counter
       Do I = 1, Max_stat
         Jsite(i) = 0
       Enddo
!
!  Open the Ocean loading data file
       Iunit = getunit()
       Open (Unit=Iunit, File=Ex_ocean, Status='old',
     .       Err=240, Iostat=ios)
!
      If ( Index(Ex_ocean,'blokq') .gt. 0) Then
!  Blokq.dat file, find the site catalog
  50   Continue
       Read(iunit,'(A80)') Inbuf
       If (Inbuf(1:2) .eq. '//') Go to 100
       Go to 50
 100   Continue
      Endif
!
 110   Continue
       Read(iunit,'(A80)',end=200) Inbuf
!   Skip comments and illegal lines
       If (Inbuf(1:2) .eq. '$$') Go to 110
       If (Inbuf(1:2) .ne. '  ') Go to 110
!      If (Inbuf(11:11) .ne. ' ' ) Go to 110
!
!   Finished site catalog
       If (Inbuf(1:2) .eq. '//') Go to 200
!
! See if this station is in the database list
       Do I = 1, Numsit
         If (Inbuf(3:10) .eq. Dbsites(I)) Then
!         print *, 'Site matched: ', Dbsites(I)
!  Matched I'th station in data base station list, save station particulars
           II = I
!
!  Skip comments
 170    Continue
        Read(iunit,'(A80)',end=200) Inbuf
        If (Inbuf(1:2) .eq. '$$') Go to 170
!
!   Read 6 data lines
        Read(Inbuf,*,err=180) OC11
          Do k=1,11
           SITOAM(k,II) = OC11(k)
          Enddo
!
        Read(Iunit,*,err=180) OC11
          Do k=1,11
           SITHOA(k,1,II) = OC11(k)
          Enddo
        Read(Iunit,*,err=180) OC11
          Do k=1,11
           SITHOA(k,2,II) = OC11(k)
          Enddo
!
        Read(Iunit,*,err=180) OC11
          Do k=1,11
           SITOPH(k,II) = OC11(k) * CONVD
          Enddo
        Read(Iunit,*,err=180) OC11
          Do k=1,11
           SITHOP(k,1,II) = OC11(k) * CONVD
          Enddo
        Read(Iunit,*,err=180) OC11
          Do k=1,11
           SITHOP(k,2,II) = OC11(k) * CONVD
          Enddo
!
           Jsite(II)    = II
!
         Endif
       Enddo
!
       Go to 110
!
 200   Continue
!
       Close(Iunit)
!
!   Verify that we have ocean loading a priori's for all stations, except any
!    site at the geocenter. If not, we must quit here and tell the user to fix
!    the problem.
!
        Iquit = 0
!
      DO I = 1, Numsit
        If (Jsite(i) .eq. 0) Then
           If (I .ne. Zero_site) Then
            If (iquit.eq.0) Write(6,'(/)')
            Write(6,'(" OCNIN: No ocean loading for ",A8)') Dbsites(I)
            Iquit = Iquit + 1
           Endif
        Endif
         If (Iquit .gt. 0) Then
             Write(6,'(/,"!!! Missing ocean loading!!! Update file ",/,  
     .       10X, A80,/,3X, " and rerun Calc!!!",/ )') Ex_ocean
             Call TERMINATE_CALC( 'OCNIN ',int2(0), int2(0))
         Endif
      ENDDO
!
       If (Iquit.eq.0) Then
        Kerr(7)  = 0
        Kerr(8)  = 0
        Kerr(9)  = 0
        Kerr(10) = 0
         Do I = 1, Numsit
          Ocestat(I) = 'YES '
         Enddo
       Else
        Call TERMINATE_CALC( 'OCNIN ', int2(0), int2(0))
       Endif
!
!       print *, ' SITOAM ', SITOAM
!       print *, ' SITOPH ', SITOPH
!       print *, ' SITHOA ', SITHOA
!       print *, ' SITHOP ', SITHOP
!
!  Now we must replace the ocean loading a priori's in the data base
      CALL PUTR('SITOCAMP      ', SITOAM, int2(11), Numsit, int2(1))
      CALL PUTR('SITOCPHS      ', SITOPH, int2(11), Numsit, int2(1))
      CALL PUTR('SITHOCAM      ', SITHOA, int2(11), int2(2), Numsit)
      CALL PUTR('SITHOCPH      ', SITHOP, int2(11), int2(2), Numsit)
      CALL PUTA('OCE STAT      ', L_ocestat, int2(2), Numsit, int2(1))
!
       Go to 270
!
!   error on Read
 180  Continue
      WRITE ( 6, * )  'OCNIN: Error on read of ocean loading file '
        Call TERMINATE_CALC('OCNIN ', int2(0), int2(0))
!
!   error on OPEN
 240  Continue
      WRITE ( 6, * )  'OCNIN: Error on OPEN of ocean loading file '
        Call TERMINATE_CALC('OCNIN ', int2(0), int2(0))
!
 270  Continue
      Return
      End
!*************************************************************************
      SUBROUTINE ANTILT(Krr)
      Implicit None
!
      INCLUDE 'cmxst.i'
!        Variables 'from':
!           1. LNSITE(4,Max_Stat)  - THE EIGHT CHARACTER SITE NAMES
!                                    IN THE DATABASE. (ALPHAMERIC)
!           2. NUMSIT             -  THE NUMBER OF SITES IN THE DATABASE.
!        Variables 'to':
!           2. Dbtilt(2,Max_Stat)  - Antenna fixed axis tilts, in arc-minutes.
!                                    For alt-az mounts, 1 => East tilt,
!                                    2 => North tilt.
      INCLUDE 'inputs.i'
!        Variables 'from':
!           1. Ex_tilts -  Ascii name of the antenna fixed axis tilts file.
!
      INCLUDE 'cmxut.i'
!        Variables 'to':
!           1. Intrvl(5,2) - First and last time tag of data in the current
!                            data base. (First index: year, month, day,
!                            hour, minute. Second index: first, last.)
!
      Character*80 Inbuf
      Character*8  Dbsites(Max_stat), Asite, Bsite, slash, Ablnk,
     .             Bblnk
      Equivalence (LNSITE(1,1), Dbsites(1))
!
      Real*4 Tyear, Tdate, Tilt1, Tilt2, Tilt1a, Tilt2a, Tilt1b,
     .       Tilt2b, Tdatea, Tdateb
      Real*4 XDOY, XYR
      Integer*4 Iyear, Imonth, Iday, Krr, Nsites, Mstat
      Integer*4 I, II, Iunit, Ios, Index, I4, ICAL(12), LCAL(12)
      Integer*2 Getunit
!
      DATA ICAL /0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334/
      DATA LCAL /0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335/
!
!  Programmer/History:
!       David Gordon 2004.05.17 Program created.
!
!   Use Integer*4 quantities
       Nsites = Numsit
       Mstat = Max_stat
!
!   Initialize station tilts
       Do I = 1, Mstat
         Dbtilt(1,I) = 0.0D0
         Dbtilt(2,I) = 0.0D0
       Enddo
!
        Iyear  = Intrvl(1,1)
        Imonth = Intrvl(2,1)
        Iday   = Intrvl(3,1)
         If (Iyear .ge.  0 .and. Iyear .le. 69) Iyear = Iyear+2000
         If (Iyear .ge. 70 .and. Iyear .le. 99) Iyear = Iyear+1900
!         Write ( 6, * ) ' Intrvl ', Intrvl
!         Write ( 6, * ) ' Iyear,Imonth,Iday ', Iyear,Imonth,Iday
!
!   Find epoch, in years at start of session.
!    Get day of year number
        IF (MOD(Iyear,4).NE. 0) Then
          XDOY = ICAL(Imonth) + Iday
          XYR = 365.
        Else
          XDOY = LCAL(Imonth) + Iday
          XYR = 366.
        Endif
         Tyear = Float(Iyear) + XDOY/XYR
!          Write ( 6, * ) ' XDOY,XYR,Tyear ', XDOY,XYR,Tyear
!
!  Open the input file of antenna tilts
       Iunit = getunit()
       Open (Unit=Iunit, File=Ex_tilts, Status='old',
     .       Err=240, Iostat=ios)
!
!   Skip first 4 lines
       Do I = 1, 4
        Read (Iunit,'(A80)') Inbuf
       Enddo
!
 110   Continue
       Read(iunit,1001,end=300) Asite, Bsite, Tdate, Tilt1,
     .                                  Tilt2, slash
 1001   Format(2X,A8,1X,A8,F11.3,6X,F8.2,7X,F8.2,A8)
!
       IF ( Index(slash, '/') .gt. 0) Then
          Tilt1a = Tilt1
          Tilt2a = Tilt2
          Tdatea = Tdate
          Tilt1b = Tilt1
          Tilt2b = Tilt2
          Tdateb = Tdate
          Go to 200
       Endif
!
!             More lines for this station
          Tilt1a = Tilt1
          Tilt2a = Tilt2
          Tdatea = Tdate
       If (Tyear .le. Tdate) Then
          Tilt1b = Tilt1
          Tilt2b = Tilt2
          Tdateb = Tdate
!           Read till '/' found
  180     Continue
          Read(iunit,1001) Ablnk, Bblnk, Tdate, Tilt1, Tilt2, slash
          IF (Index(slash, '/') .gt. 0) Go to 200
          Go to 180
       Endif
!
  182      Continue
          Read(Iunit,1001) Ablnk, Bblnk, Tdate, Tilt1, Tilt2, slash
          If (Tyear .le. Tdate) Then
          Tilt1b = Tilt1
          Tilt2b = Tilt2
          Tdateb = Tdate
          IF (Index(slash, '/') .gt. 0) Go to 200
  184      Continue
           Read(iunit,1001) Ablnk, Bblnk, Tdate, Tilt1, Tilt2, slash
           IF (Index(slash, '/') .gt. 0) Go to 200
           Go to 184
!
          Else
            IF (Index(slash, '/') .le. 0) Then
             Tilt1a = Tilt1
             Tilt2a = Tilt2
             Tdatea = Tdate
             Go to 182
            Else
             Tilt1b = Tilt1
             Tilt2b = Tilt2
             Tdateb = Tdate
            Endif
!
          Endif
!
 200   Continue
!       Write (6, *) ' Asite, Bsite, Tyear: ', Asite, Bsite, Tyear
!       Write (6, *) ' Tdatea,Tilt1a,Tilt2a: ', Tdatea,Tilt1a,Tilt2a
!       Write (6, *) ' Tdateb,Tilt1b,Tilt2b: ', Tdateb,Tilt1b,Tilt2b
!
!  Match I'th station and find its tilt for this epoch.
      Do I = 1, Nsites
       If (Asite .eq. Dbsites(I) .or. Bsite .eq. Dbsites(I)) Then
         II = I
         Go to 155
       Endif
      Enddo
       Go to 110
!
 155  Continue
        If ( ABS(Tdateb-Tdatea) .lt. .002 ) Then
          Dbtilt(1,II) = Tilt1a
          Dbtilt(2,II) = Tilt2a
        Else
          Dbtilt(1,II) = Tilt1a + (Tyear-Tdatea)*(Tilt1b-Tilt1a)/
     .                   (Tdateb-Tdatea)
          Dbtilt(2,II) = Tilt2a + (Tyear-Tdatea)*(Tilt2b-Tilt2a)/
     .                   (Tdateb-Tdatea)
        Endif
!       Write (6, *) ' Interpolated tilt: ', Dbtilt(1,II),Dbtilt(2,II)
!
       Go to 110
!
 300   Continue
       Close(Iunit)
!
!  Now we place the tilts in the data base
      CALL PUTR ('AXISTILT      ', Dbtilt, int2(2), Numsit, int2(1))
       Krr = 0
       Go to 270
!
!   error on OPEN
 240  Continue
      Krr = -1
      WRITE ( 6, * )  'Error on OPEN of blokq file '
!
 270  Continue
      Return
      End
