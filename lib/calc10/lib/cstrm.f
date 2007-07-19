      SUBROUTINE STRA()
      IMPLICIT None
!
! 1.1.1 STRA adds entries to the table of contents for the
!       STAR Module text message and the partial derivative array.
!
! 1.2   STRA PROGRAM INTERFACE
!
! 1.2.2 COMMON BLOCKS USED -
!
      INCLUDE 'ccon.i'
!            VARIABLES 'FROM':
!             1. KSTRC  -  THE STAR MODULE FLOW CONTROL FLAG.
!                          0 => Do not add proper motion constants to data
!                               base when in external file mode (Lcode
!                               'PRMOTION'). Do not compute any proper
!                               motion contributions. Remove any previous
!                               proper motion contribution Lcodes.
!                          1 => Add proper motion constants ('PRMOTION') if
!                               in external file input mode. Compute proper
!                               motion contributions if proper motion
!                               constants are available and insert in data
!                               base under Lcode 'PMOTNCON'. Do NOT add
!                               these contributions to the theoretical.
!                               [They can be ADDED in SOLVE to get proper
!                               motion corrected delays and rates.]
!                          2 => Add proper motion constants ('PRMOTION') if
!                               in external file input mode. Also, ADD the
!                               proper motions to source vector and use the
!                               proper motion corrected source vector
!                               throughout all computations. Compute a
!                               contribution (ADD to theoretical) that will
!                               (approximately) return the delays and rates
!                               to their non-proper motion corrected values,
!                               and put in Lcode 'PMOT2CON'. For cases where
!                               there is a large accumulated proper motion
!                               (greater than ~1 arcsec). Intended for
!                               correlator useage only. USE WITH CAUTION!!
!             2. KSTRD  -  THE STAR MODULE DEBUG OUTPUT FLAG.
!             3. KPLXC  -  THE PARALLAX MODULE FLOW CONTROL FLAG.
!                          0 => Do not insert source distances into data
!                               base when in external file input mode, even
!                               if distances given in external source file.
!                          1 => Insert source distances into data base in
!                               external file input mode, even if all zeros,
!                               using Lcode 'DISTPSEC'.
!
      INCLUDE 'inputs.i'
!            Variables from:
!              1. Input_stars - T/F logical flag telling whether to use
!                               external star (radio sources) a priori input.
!
      INCLUDE 'cmxsr.i'
!           VARIABLES 'TO':
!             1. NUMSTR - The number of stars (radio sources) in the Star
!                         catalog.
!
! 1.2.4 DATA BASE ACCESS:
!            ACCESS CODES:
!              1. 'STR MESS' - THE DATA BASE ACCESS CODE FOR THE
!                              STAR MODULE TEXT MESSAGE.
!              2. 'STR CFLG' - THE DATA BASE ACCESS CODE FOR THE
!                              STAR MODULE FLOW CONTROL MESSAGE.
!              3. 'STR PART' - THE DATA BASE ACCESS CODE FOR THE
!                              STAR MODULE PARTIAL DERIVATIVES ARRAY.
!              4. 'PRMOTION' - The data base access code for the proper
!                              motion array. First variable runs over
!                              RA velocity (arc-sec/year), Declination
!                              velocity (arc-sec/year), and epoch for
!                              which the J2000 coordinates in 'STAR2000'
!                              precessed to date are correct (epoch for
!                              which corrections should be zero). Second
!                              index runs over the sources, same order as
!                              in 'STRNAMES'. Zeros imply unknown.
!              5. 'DISTPSEC' - The data base access code for the source
!                              distance array. Units are parsecs; zero
!                              implies unknown.
!              6. 'PMOTNCON' - Proper motion contributions (sec, sec/sec).
!                              Add to normal delays and rates to correct for
!                              proper motions to date of observation. Exists
!                              only if KSTRC=1.
!              7. 'PMOT2CON' - Proper motion contributions (sec, sec/sec)
!                              to return to non-proper motion delays and
!                              rates. Exists only if KSTRC=2. Add to proper
!                              motion corrected delays and rates to return
!                              to the uncorrected values.
!              8. 'RADECADD' - Proper motion offsets (if proper motion turned
!                              on) in RA and Dec. Add to RA and Dec to get
!                              corrected values. Used if KSTRC = 1.
!              9. "STARPRMO' - RA and Declinations after correcting for proper
!                              motion. Used if KSTRC = 2.
!
! 1.2.6 SUBROUTINE INTERFACE -
!             CALLER SUBROUTINES: TOCUP
!             CALLED SUBROUTINES: ADDA, ADDR
!
! 1.2.9 PROGRAMMER - DALE MARKHAM  01/13/77
!                    PETER DENATALE 07/14/77
!                    SAVITA GOEL    06/03/87 (CDS FOR A900)
!                    Jim Ryan 89.07.09 Documentation simplied.
!                    Jim Ryan 89.12.12 UNIX-like database interface
!                        implimented.
!                    David Gordon 94.04.15 Converted to Implicit None
!                    David Gordon 98.03.19 Modified to do ADDR for source
!                        positions in the case of external source a priori
!                        input.
!                    David Gordon 98.09.08 Adds for 'PRMOTION' and 'DISTPSEC'
!                        for the case of external source a priori input.
!                    David Gordon 98.09.14 Adds and Deletes for 'PMOTNCON'
!                        and 'PMOT2CON', and various mods for proper motion
!                        contibutions.
!                    David Gordon 98.11.25 Adds/Deletes for 'RADECADD' and
!                        'STARPRMO', depending on value of KSTRC.
!                    Jim Ryan Sept 2002 Integer*2/4 mods.
!                    Jim Ryan 03.03.10 Kill replaced with terminate_solve
!
!     STRA PROGRAM STRUCTURE
!
!     ADD for the Star Module text message.
      CALL ADDA (int2(1),'STR MESS','Star module message definition  ',
     . int2(40), int2(1), int2(1))
!
!   ADD for the Star Module flow control message.
      CALL ADDA (int2(1),'STR CFLG','Parallax flow control mess def  ',
     . int2(40), int2(1), int2(1))
!
!     ADD for the STAR Module partial derivatives.
      CALL ADDR (int2(2),'STR PART','Star partial derivatives def.   ',
     . int2(2), int2(2), int2(1))
!
!   Do ADDR to replace the source positions in the data base in the case of
!    external file input
      IF (Input_stars) THEN
       CALL ADDR(int2(1),'STAR2000','J2000 source RAs, decs (rd,rd). ',
     .  int2(2), NUMSTR, int2(1))
       CALL ADDA(int2(1),'STAR REF','Source of coordinate values.    ',
     .  int2(10), NUMSTR, int2(1))
!
       IF (KSTRC.eq.1 .or. KSTRC.eq.2) CALL ADDR(int2(1), 'PRMOTION',
     .  'Proper motions/Epoch (a-sec, yr)', int2(3), NUMSTR, int2(1))
!
       IF (KPLXC.eq.1) CALL ADDR(int2(1), 'DISTPSEC',
     .  'Source Distances (parsecs)      ', NUMSTR, int2(1), int2(1))
      ENDIF
!
       IF (KSTRC.eq.0) Then
        CALL DELR( int2(2), 'PMOTNCON')
        CALL DELR( int2(2), 'PMOT2CON')
       ENDIF
!
       If (KSTRC.eq.1) Then
        CALL ADDR(int2(1),'RADECADD','Add to RA/Dec for proper motion ',
     .   int2(2), Numstr, int2(1))
        CALL ADDR(int2(2),'PMOTNCON','Proper Motion contribs, sec, s/s',
     .   int2(2), int2(1), int2(1))
        CALL DELR( int2(1), 'STARPRMO')
        CALL DELR( int2(2), 'PMOT2CON')
       Endif
!
       If (KSTRC.eq.2) Then
        CALL ADDR(int2(1),'STARPRMO','Proper motion corrected RA/Dec  ',
     .   int2(2), Numstr, int2(1))
        CALL ADDR(int2(2),'PMOT2CON','Proper Motion Remover,  sec, s/s',
     .   int2(2), int2(1), int2(1))
        CALL DELR( int2(1), 'RADECADD')
        CALL DELR( int2(2), 'PMOTNCON')
       Endif
!
!     Normal conclusion.
      RETURN
      END
!***********************************************************************
      SUBROUTINE STRI()
      IMPLICIT None
!
! 3.1.1 STRI is the STAR Module input and initialization section.
!
! 3.2.2 COMMON BLOCKS USED -
!
      INCLUDE 'cmxsr.i'
!           VARIABLES 'TO':
!             1. LNSTAR(4,MAX_ARC_SRC) - THE EIGHT ALPHANUMERIC CHARACTER NAMES
!                                        OF THE STARS IN THE STAR CATALOG.
!             2. NUMSTR                - THE NUMBER OF STARS IN THE STAR
!                                        CATALOG.
!             3. RADEC(2,MAX_ARC_SRC)  - THE RIGHT ASCENSIONS AND DECLINATIONS
!                                        OF THE STARS IN THE STAR CATALOG.
!                                        (RAD, RAD)
!             4. P_motion(3,Max_arc_src)-The RA and Dec proper motions and
!                                        appropriate epoch for stars in the
!                                        star catalog. (arc-sec/yr, arc-sec/yr,
!                                        year (i.e. 1995.5, etc.))
!             5. D_psec(Max_arc_src)   - Distances, if known, for stars in the
!                                        star catalog. Zero => unknown.
!                                        (parsecs)
!             6. PRcorr(2,Max_arc_src) - Proper motion corrections in RA and
!                                        Declination for each star. (Radians)
!
      INCLUDE 'ccon.i'
!            VARIABLES 'FROM':
!             1. KSTRC  -  THE STAR MODULE FLOW CONTROL FLAG.
!                          0 => Do not add proper motion constants to data
!                               base when in external file mode (Lcode
!                               'PRMOTION'). Do not compute any proper
!                               motion contributions. Remove any previous
!                               proper motion contribution Lcodes.
!                          1 => Add proper motion constants ('PRMOTION') if
!                               in external file input mode. Compute proper
!                               motion contributions if proper motion
!                               constants are available and insert in data
!                               base under Lcode 'PMOTNCON'. Do NOT add
!                               these contributions to the theoretical.
!                               [They can be ADDED in SOLVE to get proper
!                               motion corrected delays and rates.]
!                               The proper motion RA and Delination offsets,
!                               in radians, will be stored for each source in
!                               the type 1 Lcode 'RADECADD'.
!                          2 => Add proper motion constants ('PRMOTION') if
!                               in external file input mode. Also, ADD the
!                               proper motions to source vector and use the
!                               proper motion corrected source vector
!                               throughout all computations. Compute a
!                               contribution (ADD to theoretical) that will
!                               (approximately) return the delays and rates
!                               to their non-proper motion corrected values,
!                               and put in Lcode 'PMOT2CON'. For cases where
!                               there is a large accumulated proper motion
!                               (greater than ~1 arcsec). Intended for
!                               correlator useage only. USE WITH CAUTION!!
!                               The RA's and Dec's for each star, AFTER
!                               correcting for proper motion offsets, are
!                               output in the type 1 Lcode 'STARPRMO'. These
!                               should be captured and used in any post
!                               processing adjustments, etc.
!             2. KSTRD  -  THE STAR MODULE DEBUG OUTPUT FLAG.
!             3. KPLXC  -  THE PARALLAX MODULE FLOW CONTROL FLAG.
!                          0 => Do not insert source distances into data
!                               base when in external file input mode, even
!                               if distances given in external source file.
!                          1 => Insert source distances into data base in
!                               external file input mode, even if all zeros,
!                               using Lcode 'DISTPSEC'.
!
      Real*8         PI, TWOPI, HALFPI, CONVD, CONVDS, CONVHS, SECDAY
      COMMON /CMATH/ PI, TWOPI, HALFPI, CONVD, CONVDS, CONVHS, SECDAY
!         CONVDS - THE CONVERSION FACTOR FROM ARCSECONDS TO RADIANS
!                 (RAD/ARCSECOND)
!
      INCLUDE 'inputs.i'
!            Variables from:
!              1. Input_stars - T/F logical flag telling whether to use star
!                               a priori external input
!              2. Ex_stars    - File name for stars external file input.
!                               If 'NONE' or blank, then no external inputs
!
! 3.2.3 PROGRAM SPECIFICATIONS -
!
      Real*8    Xdoy1, Xepoch, X_frac
      Integer*4 I, J, N, NN, Imdoy(12)
      INTEGER*2 KERR(5), LSTRM(40), LOFF(4), LON1(40),LON2(40),NDO(3),
     .          Intrvl(5,2), KERX, idum2
      CHARACTER*40 C_LSTRM(2), C_OFF(2), C_ON1(2), C_ON2(2)
      EQUIVALENCE (C_LSTRM,LSTRM), (C_OFF,LOFF), (C_ON1,LON1),
     .            (C_ON2,LON2)
!
      Data Imdoy /0,31,59,90,120,151,181,212,243,273,304,334/
!
      DATA C_LSTRM / 'Star Module - Last modification 98.09.15',
     .               ', D. Gordon, GSFC                       '/
!
      DATA C_OFF / 'Proper Motion Corrections OFF.          ',
     .             '                                        '/
!
      DATA C_ON1 / 'Proper Motion Corrections ON, but NOT in',
     .             'cluded in theoreticals.                 '/
!
      DATA C_ON2 / 'Proper Motion Corrections ON, AND includ',
     .             'ed in theoreticals.                     '/
!
! 3.2.4 DATA BASE ACCESS -
!          'GET' VARIABLES:
!             1. LNSTAR(4,MAX_ARC_SRC) - THE EIGHT ALPHANUMMERIC CHARACTER
!                                        NAMES OF THE STARS IN THE STAR CATALOG
!                                        (ALPHANUMERIC).
!             2. NUMSTR                - THE NUMBER OF STARS IN THE STAR
!                                        CATALOG.
!             3. RADEC(2,MAX_ARC_SRC)  - THE RIGHT ASCENSIONS AND DECLINATIONS
!                                        OF THE STARS IN THE STAR CATALOG
!                                        (RAD, RAD).
!           'PUT' VARIABLES:
!             1. LSTRM(40)  -  THE STAR MODULE TEXT MESSAGE.
!
!           ACCESS CODES:
!             1. 'STR MESS' - THE DATA BASE ACCESS CODE FOR THE STAR MODULE
!                             TEXT MESSAGE.
!             2. 'STAR2000' - THE DATA BASE ACCESS CODE FOR THE ARRAY OF STAR
!                             RIGHT ASCENSIONS AND DECLINATIONS IN J2000.0
!                             COORDINATES.
!             3. 'STRNAMES' - THE DATA BASE ACCESS CODE FOR THE ARRAY OF STAR
!                             NAMES.
!             4. '# STARS ' - DATA BASE ACCESS CODE FOR THE NUMBER OF STARS.
!             5. 'PRMOTION' - The data base access code for the proper
!                             motion array. First variable runs over
!                             RA velocity (arc-sec/year), Declination
!                             velocity (arc-sec/year), and epoch for
!                             which the J2000 coordinates in 'STAR2000'
!                             precessed to date are correct (epoch for
!                             which corrections should be zero). Second
!                             index runs over the sources, same order as
!                             in 'STRNAMES'. Zeros imply unknown.
!             6. 'DISTPSEC' - The data base access code for the source
!                             distance array. Units are parsecs; zero
!                             implies unknown.
!             7. 'RADECADD' - Proper motion offsets (if proper motion turned
!                             on) in RA and Dec. Add to RA and Dec to get
!                             corrected values. Used if KSTRC = 1.
!             8. "STARPRMO' - RA and Declinations after correcting for proper
!                             motion. Used if KSTRC = 2.
!
! 3.2.6 SUBROUTINE INTERFACE -
!             CALLER SUBROUTINES: INITL
!             CALLED SUBROUTINES: GETA, GETI, GET4, TERMINATE_CALC, PUTA, STRIN
!
! 3.2.8 PROGRAM VARIABLES -
!             1.  KERR(3) - THE DATA BASE ERROR RETURN FLAGS.
!             2.  NDO(3)  - THE DATA BASE RETURN ARRAY INDICES.
!
! 3.2.9 PROGRAMMER - DALE MARKHAM   01/13/77
!                    PETER DENATALE 07/14/77
!                    CHOPO MA       08/05/81
!                    Jim Ryan 89.07.09 Documentation simplied.
!                    Jim Ryan 89.12.12 UNIX-like database interface
!                        implimented.
!                    David Gordon 94.04.15 Converted to Implicit None
!                    B. Archinal  95.11.13 Max # sources set in cmxsr.i.
!                    David Gordon 98.03.19 Mods for external file input of
!                        source coordinates
!                    David Gordon 98.09.15 Mods for proper motions and
!                        distances.
!                    David Gordon 98.11.25 Moved proper motion source offset
!                        computation here so that the RA/Dec offsets or
!                        corrected RA/Dec's could be output for capture by
!                        correlator users. Added Lcodes 'RADECADD' and
!                        'STARPRMO'.
!                    Jim Ryan Sept 2002 Integer*2/4 mods.
!
!     STRI PROGRAM STRUCTURE
!
!   PUT the Star Module text message.
      CALL PUTA ( 'STR MESS      ', LSTRM, int2(40), int2(1),int2(1))
!   PUT the module flow control message.
      IF (KSTRC .EQ. 0) CALL PUTA('STR CFLG      ', LOFF, int2(40),
     .                  int2(1), int2(1))
      IF (KSTRC .EQ. 1) CALL PUTA('STR CFLG      ', LON1, int2(40),
     .                  int2(1), int2(1))
      IF (KSTRC .EQ. 2) CALL PUTA('STR CFLG      ', LON2, int2(40),
     .                  int2(1), int2(1))
!
!   GET the star catalog from the input database.
      CALL GETI('# STARS       ',NUMSTR,int2(1),int2(1),int2(1),NDO,
     . KERR(1))
      CALL GETA('STRNAMES      ',LNSTAR,int2(4),int2((NUMSTR)),int2(1),
     . NDO, KERR(2))
!
       Pmotion = 0
       Dpsec   = 0
!  Get source positions either from the data base or from an external file
      IF (Input_stars) Then                         !Get Star Info
        Call STRIN(Kerr)
      ELSE                                          !Get Star Info
        CALL GET4('STAR2000      ',RADEC,int2(2),NUMSTR,int2(1),NDO,
     .   KERR(3))
!
!  Also get proper motions, if they are in the data base
       IF(KSTRC.eq.1 .or. KSTRC.eq.2) Then          !Get proper motions
        CALL GET4('PRMOTION      ', P_motion, int2(3), NUMSTR,
     .       int2(1), NDO, KERR(4))
        If (Kerr(4) .eq. 0) Then
!   Lcode exists, but check that values are not all zero
         Do N=1,Numstr
          If((P_motion(3,N).ge.1900.D0) .AND.
     .     (DABS(P_motion(1,N)) .gt. 1.D-12  .or.
     .      DABS(P_motion(2,N)) .gt. 1.D-12)) Pmotion = Pmotion + 1
         Enddo
        Else
!  No Lcode, therefore no proper motions, zero out the array.
         Do N=1,Numstr
          P_motion(1,N) = 0.D0
          P_motion(2,N) = 0.D0
          P_motion(3,N) = 0.D0
         Enddo
          Pmotion = 0
        Endif
       ENDIF                                        !Get proper motions
!
!  Also get distances, if they are in the data base
       IF(KPLXC.eq.1) Then                          !Get distances
        CALL GET4('DISTPSEC      ',D_Psec,NUMSTR,int2(1),int2(1),NDO,
     .   KERR(5))
        If (Kerr(5) .eq. 0) Then
!   Check that values not all zero
         Do N=1,Numstr
          If( D_psec(N).ge.1.D0 ) Dpsec = Dpsec + 1
         Enddo
        Else
!  No distances, zero out the array.
         Do N=1,Numstr
          D_psec(N) = 0.D0
         Enddo
          Dpsec = 0
        Endif
       ENDIF                                        !Get distances
!
      ENDIF                                         !Get Star Info
!      print *,' STRI: Pmotion, Dpsec = ', Pmotion, Dpsec
!
!  Proper Motions: If proper motions applied, do the work here. RA and Dec
!   offsets will be computed once for each source, and will remain constant
!   throughout this Calc run. If proper motions are added to the source
!   coordinates (KSTRC = 2), those new coordinates will be output in the
!   type 1 Lcode 'STARPRMO' for capture and later use. If proper motions
!   are used only to compute a proper motion contribution but not added
!   to the source coordinates (KSTRC = 1), then those offsets will be
!   output in the type 1 Lcode 'RADECADD'.
!
       IF (KSTRC.eq.1 .or. KSTRC.eq.2) THEN           !Do proper motions
!
        IF (Pmotion .eq. 0) Then
         Do N = 1, Numstr
          PRcorr(1,N) = 0.D0
          PRcorr(2,N) = 0.D0
         Enddo
         Go to 250
        ENDIF
!
!           Look for 4-digit year interval
          CALL GETI ('INTRVAL4      ',Intrvl,int2(5),int2(2),int2(1),
     .     NDO, KERX)
!           If no 4-digit year interval, get 2-digit year interval
         If (KERX.ne.0) then
          CALL GETI ('INTERVAL      ',Intrvl,int2(5),int2(2),int2(1),
     .     NDO, KERX)
!            Convert 2-digit year to 4-digit year
          If (Intrvl(1,1) .ge. 70 .and. Intrvl(1,1) .le. 99)
     .        Intrvl(1,1) = Intrvl(1,1)+1900
          If (Intrvl(1,1) .ge.  0 .and. Intrvl(1,1) .le. 69)
     .        Intrvl(1,1) = Intrvl(1,1)+2000
         Endif
!
!  Convert start time to year and fraction, not worrying about leap years:
           xdoy1 = imdoy(Intrvl(2,1)) + Intrvl(3,1) + Intrvl(4,1)/24.d0
     .             + Intrvl(5,1)/1440.d0
           Xepoch = Intrvl(1,1) + xdoy1/365.D0
             Do N = 1, Numstr
              X_frac = Xepoch - P_motion(3,N)
               If (P_motion(3,N).eq.0.D0) X_frac = 0.D0
              PRcorr(1,N) = P_motion(1,N)*X_frac*CONVDS/DCOS(RADEC(2,N))
              PRcorr(2,N) = P_motion(2,N)*X_frac*CONVDS
                If (KSTRC.eq.2) Then
                 RADEC(1,N) = RADEC(1,N) + PRcorr(1,N)
                 RADEC(2,N) = RADEC(2,N) + PRcorr(2,N)
                 PRcorr(1,N) = -PRcorr(1,N)
                 PRcorr(1,N) = -PRcorr(1,N)
                Endif
             Enddo
 250    Continue
        IF (KSTRC.eq.1) CALL PUT4 ('RADECADD      ', PRcorr, int2(2),
     .      Numstr, int2(1))
        IF (KSTRC.eq.2) CALL PUT4 ('STARPRMO      ', RADEC, int2(2),
     .      Numstr, int2(1))
       ENDIF                                        !Do proper motions
!
      DO 300  N = 1,3
        NN = N
        IF ( KERR(N) .EQ. 0 ) GO TO 300
           CALL TERMINATE_CALC ('STRI  ', NN, KERR(NN) )
  300 CONTINUE
!
!   Check KSTRD for debug output.
      IF ( KSTRD .EQ. 0 ) GO TO 500
      WRITE ( 6, 9)
    9 FORMAT (1X, "Debug output for subroutine STRI." )
      WRITE(6,8)' RADEC   ',RADEC
    8 FORMAT(A,4D25.16/(7X,5D25.16))
      WRITE(6,7)' NUMSTR  ',NUMSTR
    7 FORMAT(A6,15I8/(9X,15I8))
      WRITE ( 6, 9200 )  LNSTAR
 9200 FORMAT (1X, "LNSTAR = ", /, 1X, 10 ( 10 ( 4A2, 2X ), /,1X))
      If (KSTRC.eq.1 .or. KSTRC.eq.2)  WRITE(6,8)' P_motion ',
     .       ((P_motion(I,J), I=1,3), J=1,Numstr)
      If (KPLXC.eq.1)  WRITE(6,8)' D_Psec ', (D_Psec(J),J=1,Numstr)
!
!     Normal conclusion.
  500 RETURN
      END
!********************************************************************
      SUBROUTINE STRG (XJD, UTC, STAR)
      IMPLICIT None
!
! 4.1.1 STRG is the geometry section of the STAR Module. STRG computes the
!       J2000.0 unit vector in the direction of the source. Now will compute
!       proper motions in special situations.
!
! 4.2   STRG PROGRAM INTERFACE
!
! 4.2.1 CALLING SEQUENCE -
!           OUTPUT VARIABLES:
!             1. STAR(3)  -  THE J2000.0 SOURCE UNIT VECTOR. (UNITLESS)
!
! 4.2.2 COMMON BLOCKS USED -
!
      INCLUDE 'cmxsr.i'
!           VARIABLES 'FROM':
!             1. LNSTAR(4,MAX_ARC_SRC) - THE EIGHT ALPHANUMERIC CHARACTER NAMES
!                                        OF THE STARS IN THE STAR CATALOG.
!             2. NUMSTR                - THE NUMBER OF STARS IN THE STAR
!                                        CATALOG.
!             3. RADEC(2,MAX_ARC_SRC)  - THE RIGHT ASCENSIONS AND DECLINATIONS
!                                        OF THE STARS IN THE STAR CATALOG.
!                                        (RAD, RAD)
!           VARIABLES 'TO':
!             1. CD  - THE COSINE OF THE DECLINATION OF THE STAR BEING USED IN
!                      THE CURRENT OBSERVATION. (UNITLESS)
!             2. CRA - THE COSINE OF THE RIGHT ASCENSION OF THE STAR BEING
!                      USED IN THE CURRENT OBSERVATION. (UNITLESS)
!             3. SD  - THE SINE OF THE DECLINATION OF THE STAR BEING USED IN
!                      THE CURRENT OBSERVATION. (UNITLESS)
!             4. SRA - THE SINE OF THE RIGHT ASCENSION OF THE STAR BEING USED
!                      IN THE CURRENT OBSERVATION. (UNITLESS)
!
      INCLUDE 'ccon.i'
!           VARIABLES 'FROM':
!             1. KSTRC - THE STAR MODULE FLOW CONTROL FLAG.
!             2. KSTRD - THE STAR MODULE DEBUG OUTPUT FLAG.
!
      Real*8         PI, TWOPI, HALFPI, CONVD, CONVDS, CONVHS, SECDAY
      COMMON /CMATH/ PI, TWOPI, HALFPI, CONVD, CONVDS, CONVHS, SECDAY
!         CONVDS - THE CONVERSION FACTOR FROM ARCSECONDS TO RADIANS
!                 (RAD/ARCSECOND)
!
      Real*8 PR_RA, PR_DEC
      Common /Pmotn/ PR_RA, PR_DEC
!
      Real*8 Dparsec, t_prlx(2)
      Common /PRLX/ Dparsec, t_prlx
!
! 4.2.3 PROGRAM SPECIFICATIONS -
      Real*8  STAR(3), XJD, UTC
!
! 4.2.4 DATA BASE ACCESS -
!           'GET' VARIABLES:
!             1. LSTRNM(4) - THE EIGHT ALPHAMERIC CHARACTER STAR NAME FOR THE
!                            CURRENT OBSERVATION. (ALPHAMERIC)
!            ACCESS CODES:
!             1. 'STAR ID ' - THE DATA BASE ACCESS CODE FOR THE STAR NAME OF
!                             THE CURRENT OBSERVATION.
!
! 4.2.6 SUBROUTINE INTERFACE -
!             CALLER SUBROUTINES: DRIVG
!             CALLED SUBROUTINES: DCOS, DSIN, GETA, TERMINATE_CALC, JDY2K
!
! 4.2.7 CONSTANTS USED - NONE
!
! 4.2.8 PROGRAM VARIABLES -
      Integer*2 LSTRNM(4), NDO(3), KERR
      Integer*4 N, NN, IM, ID, Ieph
      REAL*8    RIGHT_ASC, DECLINATION, Xepoch, XJAN1, Xdays, Difyrs,
     .          JDepoch, JDY2K
!
!             1. KERR        - THE DATA BASE ERROR RETURN FLAG.
!             2. NDO(3)      - THE DATA BASE RETURN ARRAY INDICES.
!             3. RIGHT_ASC   - LOCAL VARIABLE FOR HOLDING THE RA.
!             4. DECLINATION - LOCAL VARIABLE FOR HOLDING THE DEC.
!
! 4.2.9 PROGRAMMER - DALE MARKHAM  01/13/77
!                    PETER DENATALE 07/14/77
!                    JIM RYAN      88.01.07
!                    Jim Ryan 89.07.09 Documentation simplied.
!                    Jim Ryan 89.12.12 UNIX-like database interface
!                          implimented.
!                    David Gordon 94.04.15 Converted to Implicit None
!                    B. Archinal  95.11.13 Max # sources set in cmxsr.i.
!                    David Gordon 98.09.08 Add /CMATH/ Common block.
!                    David Gordon 98.09.11 Add new /Pmotn/ and /PRLX/ common
!                          blocks to hold proper motion offsets and distance.
!                          Code added to compute and handle proper motions,
!                          if that option is turned on.
!                    David Gordon 98.11.25 Removed proper motion computations
!                          and moved them to the initialization section.
!                    Jim Ryan Sept 2002 Integer*2/4 mods.
!
!     STRG PROGRAM STRUCTURE
!
!     GET the star name of the current observation.
      CALL GETA ('STAR ID       ',LSTRNM,int2(4),int2(1),int2(1),NDO,
     . KERR)
      IF ( KERR .EQ. 0 )  GO TO 300
      CALL TERMINATE_CALC ( 'STRG  ', int2(1), KERR)
!
!     Construct the arrays which will hold the information for the
!     source being used in the current observation in order to pass
!     this information to the remainder of CALC.
!
!     Match the current star name against the names in the star catalog.
!     If no match, send a message and quit.
  300 DO 310  NN = 1, NUMSTR
           N = NN
           IF  ( ( LNSTAR(1,N) .EQ. LSTRNM(1) )
     .     .AND. ( LNSTAR(2,N) .EQ. LSTRNM(2) )
     .     .AND. ( LNSTAR(3,N) .EQ. LSTRNM(3) )
     .     .AND. ( LNSTAR(4,N) .EQ. LSTRNM(4) ) )  GO TO 320
  310 CONTINUE
      GO TO 600
!
!     Construct the arrays to hold the sine and cosine of the star
!     declination and right ascention.
  320 CONTINUE
      RIGHT_ASC   = RADEC(1,N)
      DECLINATION = RADEC(2,N)
!
!***********************************************************************
!  Check for proper motion computations
!     IF (KSTRC.eq.1 .or. KSTRC.eq.2) THEN
!  Compute proper motion offsets in RA and Dec
!   Proper motion epoch
!       Xepoch = P_motion(3,N)
!   Truncate to integer year
!       Ieph = Xepoch
!   Day of year
!       If (JMOD(Ieph,4) .eq. 0) Then
!        Xdays = (Xepoch - Ieph) / 366.D0
!       Else
!        Xdays = (Xepoch - Ieph) / 365.D0
!       Endif
!   Julian day an Jan. 1
!        IM = 1
!        ID = 1
!       XJAN1 = JDY2K(Ieph,IM,ID)
!   Julian day at proper motion epoch
!       JDepoch = XJAN1 - 1 + Xdays
!   Difference: (Observation time) - (Proper motion epoch), yrs
!       Difyrs = ((XJD+UTC) - JDepoch) / 365.25D0
!   Proper motion in RA (convert arc-seconds to time units in radians)
!       PR_RA = P_motion(1,N) / DCOS(DECLINATION) * CONVDS * Difyrs
!   Proper motion in Dec (convert arc-seconds to radians)
!       PR_DEC = P_motion(2,N) * CONVDS * Difyrs
!
!   Determine if proper motions should be added to the source vector. If so
!    the corrections should also be reversed for later use in STRP.
!       IF (KSTRC.eq.2) THEN
!         RIGHT_ASC   = RIGHT_ASC   + PR_RA
!         DECLINATION = DECLINATION + PR_DEC
!         PR_RA  = -PR_RA
!         PR_DEC = -PR_DEC
!       ENDIF
!      WRITE(6,8) ' Xepoch, Xdays, XJAN1, JDepoch, Difyrs ',
!    *              Xepoch, Xdays, XJAN1, JDepoch, Difyrs
!      WRITE(6,8) ' PR_RA, PR_DEC ', PR_RA, PR_DEC
!
!     ENDIF
!***********************************************************************
!
      SD = DSIN ( DECLINATION)
      CD = DCOS ( DECLINATION)
      SRA = DSIN ( RIGHT_ASC )
      CRA = DCOS ( RIGHT_ASC )
!
!     Compute the star position unit vector.
      STAR(1) = CD * CRA
      STAR(2) = CD * SRA
      STAR(3) = SD
!
      IF (KSTRC.eq.1 .or. KSTRC.eq.2) THEN
        PR_RA  = PRcorr(1,N)
        PR_DEC = PRcorr(2,N)
      ENDIF
!
!  Match source with distance, if parallax to be computed
      IF (KPLXC .eq. 1) Then
        Dparsec = D_psec(N)
        WRITE (6,8) ' Dparsec ', Dparsec
      ENDIF
!
!     Check KSTRD for debug output.
      IF ( KSTRD .EQ. 0 )  GO TO 500
      WRITE ( 6, 9)
    9 FORMAT (1X, "Debug output for subroutine STRG." )
!
      WRITE(6,8)' CD      ',CD
    8 FORMAT(A,4D25.16/(7X,5D25.16))
      WRITE(6,8)' CRA     ',CRA
      WRITE(6,8)' RADEC   ',RADEC
      WRITE(6,7)' NUMSTR  ',NUMSTR
    7 FORMAT(A,15I8/(9X,15I8))
      WRITE(6,8)' SD      ',SD
      WRITE(6,8)' SRA     ',SRA
!
      WRITE ( 6, 9200 )  STAR, LSTRNM, LNSTAR
 9200 FORMAT (1X, "STAR   = ", 3 ( D30.16, 10X ), /, 1X,
     .            "LSTRNM = ", 4A2, /, 1X,
     .            "LNSTAR = ", 10 ( 10 ( 4A2, 2X ), /, 1X ) )
!
      IF (KSTRC.eq.1 .or. KSTRC.eq.2) THEN
       WRITE(6,8) ' Xepoch, Xdays, XJAN1, JDepoch, Difyrs ',
     .              Xepoch, Xdays, XJAN1, JDepoch, Difyrs
       WRITE(6,8) ' PR_RA, PR_DEC ', PR_RA, PR_DEC
      ENDIF
      IF (KPLXC .eq. 1) WRITE (6,8) ' Dparsec ', Dparsec
!
!   5.    NORMAL PROGRAM CONCLUSION.
!
  500 RETURN
!
!   6.    ABNORMAL PROGRAM TERMINATION.
!
  600 WRITE ( 6, 9300 )
 9300 FORMAT (" CALC has terminated in subroutine STRG.  ",
     .        ' The source identification was not successful. ' )
!
      CALL TERMINATE_CALC ( 'STRG  ', int2(0), int2(0))
      END
!**********************************************************************
      SUBROUTINE STRP (EPBASE,STAR,EARTH,SITEV,DSTRP,CDX,CRAX,SDX,SRAX)
      IMPLICIT None
!
! 5.1.1 STRP is the partial derivatives section of the STAR module. It computes
!       the partial derivatives of the delay and rate with respect to the source
!       declination and right ascension.
!
! 5.2.1 CALLING SEQUENCE -
!           INPUT VARIABLES:
!             1. EPBASE(3,2) - THE J2000.0 GEOCENTRIC BASELINE VECTOR
!                              AND ITS CT TIME DERIVATIVE. (M, M/SEC)
!             2. STAR(3)     - THE J2000.0 SOURCE UNIT VECTOR. (UNITLESS)
!             3. EARTH(3,3)  - The ssbc position, velocity, and acceleration
!                               of the Earth. (m, m/s, m/s**2)
!           OUTPUT VARIABLES:
!             1. CDX -  THE COSINE OF THE DECLINATION OF THE SOURCE
!             2. CRAX - THE COSINE OF THE RIGHT ASCENSION OF THE SOURCE
!             3. SDX -  THE SINE OF THE DECLINATION OF THE SOURCE
!             4. SRAX - THE SINE OF THE RIGHT ASCENSION OF THE SOURCE
!             5. DSTRP(2,2)-THE PARTIAL DERIVATIVES OF THE DELAY AND THE DELAY
!                       RATE WITH RESPECT TO THE SOURCE RIGHT ASCENSION AND
!                       DECLINATION. (sec/rad, sec/sec-rad) THE FIRST INDEX
!                       RUNS OVER RA AND DEC, THE SECOND RUNS OVER DELAY AND
!                       DELAY RATE.
!
! 5.2.2 COMMON BLOCKS USED -
!
      INCLUDE 'cphys.i'
!           VARIABLES 'FROM':
!             1. VLIGHT  - THE VELOCITY OF LIGHT IN VACUUM.  (M/SEC)
!             2. VLIGHT2 - THE VELOCITY OF LIGHT IN VACUUM SQUARED.
!                          (M**2/SEC**2)
!
      INCLUDE 'cmxsr.i'
!           VARIABLES 'FROM':
!             1. CD  - THE COSINE OF THE DECLINATION OF THE STAR BEING
!                      USED IN THE CURRENT OBSERVATION.
!             2. CRA - THE COSINE OF THE RIGHT ASCENSION OF THE STAR BEING
!                      USED IN THE CURRENT OBSERVATION.
!             3. SD  - THE SINE OF THE DECLINATION OF THE STAR BEING USED
!                      IN THE CURRENT OBSERVATION.
!             4. SRA - THE SINE OF THE RIGHT ASCENSION OF THE STAR BEING
!                      USED IN THE CURRENT OBSERVATION.
!
      INCLUDE 'ccon.i'
!           VARIABLES 'FROM':
!             1.  KSTRC - THE STAR MODULE FLOW CONTROL FLAG.
!             2.  KSTRD - THE STAR MODULE DEBUG OUTPUT FLAG.
!
      Real*8 PR_RA, PR_DEC
      Common /Pmotn/ PR_RA, PR_DEC
!
! 5.2.3 PROGRAM SPECIFICATIONS -
!
       Real*8 DDEC(3), DRA(3), DSTRP(2,2), EPBASE(3,2), STAR(3), CDX,
     .        CRAX, SDX, SRAX, EARTH(3,3), SITEV(3,2), c1, c2, tt,
     .        vg(3), bp(3), bv(3), PMCONT(2), DOTP
       Integer*4 I
!
! 5.2.4 DATA BASE ACCESS -
!           'PUT' VARIABLES:
!             1. DSTRP(2,2) - THE PARTIAL DERIVATIVES OF THE DELAY AND OF THE
!                             DELAY RATE WITH RESPECT TO THE SOURCE RIGHT
!                             ASCENSION AND DECLINATION. (sec/rad, sec/sec-rad)
!                             THE FIRST INDEX RUNS OVER RIGHT ASCENSION AND
!                             DECLINATION, THE SECOND INDEX RUNS OVER THE DELAY
!                             AND THE DELAY RATE.
!           ACCESS CODES:
!             1. 'STR PART' - THE DATA BASE ACCESS CODE FOR THE STAR MODULE
!                             PARTIAL DERIVATIVES ARRAY.
!
! 5.2.6 SUBROUTINE INTERFACE -
!             CALLER SUBROUTINES: DRIVP
!             CALLED SUBROUTINES: PUT4
!
! 5.2.7 CONSTANTS USED - VLIGHT
!
! 5.2.8 PROGRAM VARIABLES -
!             1. DDEC(3) - THE PARTIAL DERIVATIVE OF THE J2000.0 SOURCE UNIT
!                          VECTOR WITH RESPECT TO SOURCE DECLINATION. (1/RAD)
!             2. DRA(3)  - THE PARTIAL DERIVATIVE OF THE J2000.0 SOURCE UNIT
!                          VECTOR WITH RESPECT TO THE SOURCE RIGHT ASCENSION.
!                          (1/RAD)
!             3. c1, c2, tt, vg(3), b(3) -  Dummy variables used in computation
!                          of the partials.
!
! 5.2.9 PROGRAMMER - 77.01.13 Dale Markham
!                    77.07.14 Peter Denatale
!                    88.11.10 Bruce Schupler
!                    88.01.07 Jim Ryan
!                    89.07.09 Jim Ryan Documentation simplied.
!                    89.10.05 Jim Ryan CPHYS common made an include file
!                    89.12.12 Jim Ryan UNIX-like database interface
!                             implimented.
!                    01:11:25 Jim Ryan Term 2 of Shapiro's model added
!                             to the partials.
!                    David Gordon 94.04.15 Converted to Implicit None
!                    David Gordon 95.05.02 DSTRP passed back to DRIVR for use
!                             in PLXP.
!                    B. Archinal  95.11.13 Max # sources set in cmxsr.i.
!                    David Gordon 98.09.08 Changed partials computation to
!                             use CONSENSUS model (Step 10B). Makes no
!                             significant difference.
!                    David Gordon 2000.05.15 Bug correction, variable c1
!                             redefined (had been commented out).
!                    Jim Ryan Sept 2002 Integer*2/4 mods.
!
!     STRP Program Structure
!
!  Compute the partial derivatives of the J2000.0 source unit vector with
!   respect to the source declination and with respect to the source R.A.
      DDEC(1) = - SD * CRA
      DDEC(2) = - SD * SRA
      DDEC(3) = + CD
!
      DRA(1) = - CD * SRA
      DRA(2) = + CD * CRA
      DRA(3) = 0.D0
!******************************************************************************
!   Complete the calculation of the partial derivatives.
!     c1 = 1.d0/VLIGHT
!     c2 = c1**2
!     Do i=1,3
!       vg(i) =  EARTH(I,2)
!       bp(i) = -EPBASE(I,1)
!       bv(i) = -EPBASE(I,2)
!     Enddo
!     tt = 1.d0 - c1*Dotp(STAR,vg)
!
!     DSTRP(1,1)=-c1*Dotp(bp,DRA )*tt+c2*Dotp(STAR,bp)*Dotp(vg,DRA)
!     DSTRP(1,2)=-c1*Dotp(bv,DRA )*tt+c2*Dotp(STAR,bv)*Dotp(vg,DRA)
!     DSTRP(2,1)=-c1*Dotp(bp,DDEC)*tt+c2*Dotp(STAR,bp)*Dotp(vg,DDEC)
!     DSTRP(2,2)=-c1*Dotp(bv,DDEC)*tt+c2*Dotp(STAR,bv)*Dotp(vg,DDEC)
!     WRITE(6,'(" Old DSTRP: ",4D22.14)') DSTRP
!******************************************************************************
!
!   Complete the calculation of the partial derivatives.
      Do I=1,3
        vg(I) =  EARTH(I,2) + SITEV(I,2)
        bp(I) = -EPBASE(I,1)
        bv(I) = -EPBASE(I,2)
      Enddo
      c1 = 1.d0/VLIGHT
      tt = 1.d0 + c1*Dotp(STAR,vg)
!   Changed to Consensus model formula
      DSTRP(1,1) = -Dotp(bp,DRA )/(Vlight*tt) +
     .              Dotp(STAR,bp)*Dotp(vg,DRA)/Vlight2
      DSTRP(1,2) = -Dotp(bv,DRA )/(Vlight*tt) +
     .              Dotp(STAR,bv)*Dotp(vg,DRA)/Vlight2
      DSTRP(2,1) = -Dotp(bp,DDEC)/(Vlight*tt) +
     .              Dotp(STAR,bp)*Dotp(vg,DDEC)/Vlight2
      DSTRP(2,2) = -Dotp(bv,DDEC)/(Vlight*tt) +
     .              Dotp(STAR,bv)*Dotp(vg,DDEC)/Vlight2
!     WRITE(6,'(" New DSTRP: ",4D22.14)') DSTRP
!
!   PUT the partials into the database.
      CALL PUT4 ('STR PART      ', DSTRP, int2(2), int2(2), int2(1))
!
!   Copy some values from STRCM into dummy variables for use elsewhere
      CDX = CD
      CRAX = CRA
      SDX = SD
      SRAX = SRA
!
!   Check KSTRD for debug output.
      IF ( KSTRD .EQ. 0 )  GO TO 600
      WRITE ( 6, 9)
    9 FORMAT (1X, "Debug output for subroutine STRP." )
      WRITE(6,8)' CD     ',CD
    8 FORMAT(A,4D25.16/(7X,5D25.16))
      WRITE(6,8)' CRA    ',CRA
      WRITE(6,8)' DDEC   ',DDEC
      WRITE(6,8)' DRA    ',DRA
      WRITE(6,8)' DSTRP  ',DSTRP
      WRITE(6,8)' SD     ',SD
      WRITE(6,8)' SRA    ',SRA
      WRITE(6,8)' VLIGHT ',VLIGHT
      WRITE(6,8)' c1     ',c1
      WRITE(6,8)' c2     ',c2
      WRITE(6,8)' tt     ',tt
      WRITE(6,8)' vg     ',vg
      WRITE(6,8)' bp     ',bp
      WRITE(6,8)' bv     ',bv
!
      WRITE ( 6, 9200 )  EPBASE, STAR,CDX,SDX,CRAX,SRAX
 9200 FORMAT (1X, "EPBASE = ", 2 ( 3 ( D30.16, 10X ), /, 1X ),
     .            "STAR   = ", 3 ( D30.16, 10X ),/,1X,
     .            "CDX    = ", D30.16,1X,
     .            "SDX    = ", D30.16,1X,
     .            "CRAX   = ", D30.16,/,1X,
     .            "SRAX   = ", D30.16)
!
! NORMAL PROGRAM CONCLUSION.
!
  600 RETURN
      END
!***********************************************************************
      SUBROUTINE STRC (DSTRP)
      IMPLICIT None
!
! 5.1.1 STRC is the contributions section of the STAR module. It computes
!       contributions to the delay and rate due to proper motions. Used
!       only when KSTRC = 1 or 2.
!
! 5.2.1 CALLING SEQUENCE -
!       INPUT VARIABLES:
!         1. DSTRP(2,2)-THE PARTIAL DERIVATIVES OF THE DELAY AND THE DELAY
!                       RATE WITH RESPECT TO THE SOURCE RIGHT ASCENSION AND
!                       DECLINATION. (sec/rad, sec/sec-rad) THE FIRST INDEX
!                       RUNS OVER RA AND DEC, THE SECOND RUNS OVER DELAY AND
!                       DELAY RATE.
!
! 5.2.2 COMMON BLOCKS USED -
!
      INCLUDE 'ccon.i'
!           VARIABLES 'FROM':
!             1.  KSTRC  -  THE STAR MODULE FLOW CONTROL FLAG.
!             2.  KSTRD  -  THE STAR MODULE DEBUG OUTPUT FLAG.
!
      Real*8 PR_RA, PR_DEC
      Common /Pmotn/ PR_RA, PR_DEC
!
! 5.2.3 PROGRAM SPECIFICATIONS -
       Real*8 DSTRP(2,2), PMCONT(2)
!
! 5.2.4 DATA BASE ACCESS -
!          'PUT' VARIABLES:
!            1. PMCONT(2) - If KSTRC = 1, these are the contributions to
!                           the delay and rate to correct for the effect
!                           of proper motion; add to theoreticals.
!                           If KSTRC = 2, these are the contributions to
!                           return the delay and rate to their non-proper
!                           motion values; add to theoreticals. (sec, sec/sec).
!          ACCESS CODES:
!            1. 'PMOTNCON' - The data base access code for the proper motion
!                            contribution.
!            2. 'PMOT2CON' - The data base access code for the proper motion
!                            removal contribution.
!
! 5.2.6 SUBROUTINE INTERFACE -
!             CALLER SUBROUTINES: DRIVP
!             CALLED SUBROUTINES: PUT4
!
! 5.2.9 PROGRAMMER - 98.09.15 D. Gordon - subroutine created
!                    Jim Ryan Sept 2002 Integer*2/4 mods.
!
!     STRC Program Structure
!
!  Check for proper motion computations
      IF (KSTRC.eq.1 .or. KSTRC.eq.2) THEN
        PMCONT(1) = PR_RA*DSTRP(1,1) + PR_DEC*DSTRP(2,1)
        PMCONT(2) = PR_RA*DSTRP(1,2) + PR_DEC*DSTRP(2,2)
        IF (KSTRC.eq.1) CALL PUT4 ('PMOTNCON      ', PMCONT, int2(2),
     .      int2(1), int2(1))
        IF (KSTRC.eq.2) CALL PUT4 ('PMOT2CON      ', PMCONT, int2(2),
     .      int2(1), int2(1))
      ELSE
       Return
      ENDIF
!
!** DEBUG *************************************************************
      WRITE(6,8)'STRP: DSTRP  ', DSTRP
      WRITE(6,8)'STRP: PMCONT ', PMCONT
!
!   Check KSTRD for debug output.
      IF (KSTRD .EQ. 0)  THEN
       WRITE ( 6, 9)
    9  FORMAT (1X, "Debug output for subroutine STRC." )
    8  FORMAT(A,4D25.16/(7X,5D25.16))
       WRITE(6,8)' DSTRP  ',DSTRP
       WRITE(6,8)'STRP: PMCONT ', PMCONT
      ENDIF
!
! NORMAL PROGRAM CONCLUSION.
!
  600 RETURN
      END
!***********************************************************************
      SUBROUTINE STRIN(Kerr)
      Implicit None
!
!    Subroutine to open external source catalog and get source a priori's.
!
!     98.03.19 D. Gordon/GSFC - Subroutine created
!     98.09.11 D. Gordon/GSFC - Expanded to read a non-blokq.dat file which
!                               may optionally contain proper motion rates,
!                               proper motion epochs, and source distances.
!     99.10.27 D. Gordon/GSFC - Extraneous printout removed.
!     Sept 2002 Jim Ryan      - Integer*2/4 mods.
!     2006.03.30 D. Gordon    - Improve missing star message.
!     2006.04.03 D. Gordon    - Improve missing star message again.
!
      INCLUDE 'cmxsr.i'
      INCLUDE 'inputs.i'
      INCLUDE 'ccon.i'
!            VARIABLES 'FROM':
!             1. KSTRC  -  THE STAR MODULE FLOW CONTROL FLAG.
!                          0 => Do not add proper motion constants to data
!                               base when in external file mode  (Lcode
!                               'PRMOTION'). Do not compute any proper
!                               motion contributions. Remove any previous
!                               proper motion contribution Lcodes.
!                          1 => Add proper motion constants ('PRMOTION') if
!                               in external file input mode. Compute proper
!                               motion contributions if proper motion
!                               constants are available and insert in data
!                               base under Lcode 'PMOTNCON'. Do NOT add
!                               these contributions to the theoretical.
!                               [They can be ADDED in SOLVE to get proper
!                               motion corrected delays and rates.]
!                          2 => Add proper motion constants ('PRMOTION') if
!                               in external file input mode. Also, ADD the
!                               proper motions to source vector and use the
!                               proper motion corrected source vector
!                               throughout all computations. Compute a
!                               contribution (ADD to theoretical) that will
!                               (approximately) return the delays and rates
!                               to their non-proper motion corrected values,
!                               and put in Lcode 'PMOT2CON'. For cases where
!                               there is a large accumulated proper motion
!                               (greater than ~1 arcsec). Intended for
!                               correlator useage only. USE WITH CAUTION!!
!             2. KSTRD  -  THE STAR MODULE DEBUG OUTPUT FLAG.
!             3. KPLXC  -  THE PARALLAX MODULE FLOW CONTROL FLAG.
!                          0 => Do not insert source distances into data
!                               base when in external file input mode, even
!                               if distances given in external source file.
!                          1 => Insert source distances into data base in
!                               external file input mode, even if all zeros,
!                               using Lcode 'DISTPSEC'.
!
      Real*8           PI, TWOPI, HALFPI, CONVD, CONVDS, CONVHS, SECDAY
      COMMON / CMATH / PI, TWOPI, HALFPI, CONVD, CONVDS, CONVHS, SECDAY
!
      Integer*2 Getunit, Kerr(3), Lstref(10,Max_arc_src)
      Integer*4 I, II, I2, Jstar(Max_arc_src), Iunit, ios,
     .          Iquit, Index, K, RA_hr, RA_min, Dec_deg, Dec_min,
     .          Itype
      Real*8    RA, Dec, RA_sec, Dec_sec, Dsign, PM_RA, PM_Dec, PM_ep,
     .       Psecs
      Character*1 Csign, dummy(3)
      Character*20 Cstref(Max_arc_src), Staref
      Equivalence ( Lstref, Cstref )
!
      Character*120 Inbuf
      Character*8  Dbstars(Max_arc_src), Star
      Equivalence (LNSTAR(1,1), Dbstars(1))
!
!   Initialize star counter
       Do I = 1, Max_arc_src
         Jstar(i) = 0
       Enddo
!
        Itype = 1
        Dpsec = 0
        Pmotion = 0
!
!  Open the Star catalog data file
       Iunit = getunit()
       Open (Unit=Iunit, File=Ex_stars, Status='old',
     .       Err=240, Iostat=ios)
!
      If (Index(Ex_stars,'blokq') .gt. 0) Then
!  Blokq.dat file, find the star catalog
        Itype = 2
        I2 = 0
  50   Continue
       Read(iunit,'(A120)') Inbuf
       If (Inbuf(1:2) .eq. '//') Then
        I2 = I2 + 1
        If (I2 .eq. 2) Go to 100
       Endif
       Go to 50
 100   Continue
      Endif
!
 130   Continue
       Read(iunit,'(A120)',end=200) Inbuf
!   Skip comments and illegal lines
       If (Inbuf(1:2) .eq. '//')   Go to 200
       If (Inbuf(1:2) .eq. '$$'  ) Go to 130
       If (Inbuf(1:4) .ne. '    ') Go to 130
!      If (Inbuf(13:13) .ne. ' ' ) Go to 130
!
       IF (Itype .eq. 2) THEN
!  blokq.dat file
        Read(Inbuf,1010,err=200,end=200) Star, RA_hr, RA_min, RA_sec,
     .       Csign, Dec_deg, Dec_min, Dec_sec, Staref
 1010   Format(4X,A8,2X,2(I2,1X),F13.10,1X,A1,2(I2,1X),F13.10,3X,A20)
!
       ELSE
!  Non-blokq.dat file
        Read(Inbuf,1015,err=200,end=200) Star, RA_hr, RA_min, RA_sec,
     .       Csign, Dec_deg, Dec_min, Dec_sec, PM_RA, PM_Dec, PM_ep,
     .       Psecs, Staref
 1015   Format(4X,A8,2X,2(I2,1X),F13.10,1X,A1,2(I2,1X),F13.10,3X,
     .         F8.7,2X,F8.7,2X,F8.3,2X,F8.1,3X,A20)
       ENDIF
!
! See if this station is in the database list
       Do I = 1, NUMSTR
         If (Star .eq. Dbstars(I)) Then
!          print *, 'Star matched: ', Dbstars(I)
           II = I
           Jstar(II) = II
!
           Dsign = 0.D0
           If(Csign .eq. '-') Dsign = -1.D0
           If(Csign .eq. '+') Dsign =  1.D0
           If(Csign .eq. ' ') Dsign =  1.D0
           If(Dsign .eq. 0.D0) Then
            WRITE ( 6, * ) '!!! No plus/minus sign for Declination !!! '
           Endif
!
!  [We use 2.D0*PI below instead of TWOPI for consistency with
!   Dbedit/Apriori/Skeleton programs. Difference is: TWOPI - 2.D0*PI = +1.D-16.
!   -Also, don't change or re-order these equations, or they may no longer
!    give identical results to Dbedit/Apriori/Skeleton.]
           RADEC(1,II) = (RA_hr/24.D0 + RA_min/1440.D0 + RA_sec/8.64D4)
     .           * 2.D0*PI
           RADEC(2,II) = ( Dec_deg + (Dec_min + Dec_sec/60.D0)/60.D0 )
     .           * Dsign * 2.D0*PI / 360.D0
!
           Cstref(II) = Staref
!           print *,'RA/Dec/ref ', RADEC(1,II), RADEC(2,II), Cstref(II)
!
           IF (Itype .eq. 1) THEN
            P_motion(1,II) = PM_RA
            P_motion(2,II) = PM_Dec
            P_motion(3,II) = PM_ep
            D_psec(II)     = Psecs
            If((P_motion(3,II).ge.1900.D0) .AND.
     .         (DABS(P_motion(1,II)) .gt. 1.D-12  .or.
     .          DABS(P_motion(2,II)) .gt. 1.D-12) )
     .                Pmotion = Pmotion + 1
            If(D_psec(II).ge.1.D0) Dpsec = Dpsec + 1
           ENDIF
!
         Endif
       Enddo
!
       Go to 130
!
 200   Continue
!
       Close(Iunit)
!
!   Verify that we have a priori's for all stars
        Iquit = 0
!
      DO I = 1, NUMSTR
        If (Jstar(i) .eq. 0) Then
            If (iquit.eq.0) Write(6,'(/)')
            Write(6,'(" STRIN: Missing source: ",A8)') Dbstars(I)
            Iquit = Iquit + 1
        Endif
         If (Iquit .gt. 0) Then
             Write(6,'(/,"!!! Missing sources!!! Update file ",/,10X,A80, 
     .        /,3X, " and rerun Calc!!!",/ )') Ex_stars
             Call TERMINATE_CALC( 'STRIN ',int2(0), int2(0))
         Endif
      ENDDO
!
       If (Iquit.eq.0) Kerr(3) = 0
!
!  Now we must replace the star a priori's in the data base
      CALL PUTR('STAR2000      ', RADEC, int2(2), NUMSTR, int2(1))
      CALL PUTA('STAR REF      ', LSTREF, int2(10), NUMSTR, int2(1))
      IF (KSTRC.eq.1 .or. KSTRC.eq.2) CALL PUTR('PRMOTION      ',
     .     P_motion, int2(3), NUMSTR, int2(1))
      IF (KPLXC.eq.1) CALL PUTR('DISTPSEC      ',D_psec,NUMSTR,
     .     int2(1), int2(1))
!
       Go to 270
!
!   Error on Read
 180  Continue
      WRITE ( 6, * )  'STRIN: Error on read of star catalog '
        Call TERMINATE_CALC('STRIN ', int2(0), int2(0))
!
!   Error on OPEN
 240  Continue
      WRITE ( 6, * )  'STRIN: Error on OPEN of star catalog '
        Call TERMINATE_CALC('STRIN ', int2(0), int2(0))
!
 270  Continue
      Return
      End
