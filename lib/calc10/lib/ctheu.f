      SUBROUTINE THERA()
      IMPLICIT None
!
!     THERA ADDs entries to the table of contents for the Theory routine text
!     message and the theoretical delay and delay rate arrays. Also deletes
!     obsolete entries of the old (Calc 8.x and earlier) relativity values.
!
!     THERA Program Interface:
!
!     Common blocks used -
       INCLUDE 'ccon.i'
!           Variables 'from':
!             1.  KTHEC  -  The Theory module flow control flag.
!                           (No longer has any function.)
!             2.  KTHED  -  The Theory routine debug output flag.
!             3.  KRELC  -  The relativity module flow control flag.
!                           0     --> Gravitational bending used in Shapiro,
!                                     Hellings, and Consensus models.
!                           NOT 0 --> Gravitational bending not used.
!
!     Database access -
!           Access codes:
!             1. 'THE MESS' - The database access code for the Theory routine
!                             text message.
!             2. 'CONSNDEL' - The database access code for the VLBI delay
!                             using the Eubanks' Consensus model.
!             3. 'CONSNRAT' - The database access code for the VLBI delay
!                             rate using the Eubanks' Consensus model.
!             4. 'CONSPART' - Database access code for partial derivatives
!                             of the Consensus model delays and rates with
!                             respect to Gamma.
!             5. 'CON CONT' - Database access code for the TOTAL relativistic
!                             bending contributions based on the Consensus
!                             model.
!             6. 'SUN CONT' - Database access code for the Sun's relativistic
!                             bending contributions based on the Consensus
!                             model.
!             7. 'BENDPART' - Database access code for partial derivatives
!                             of the Consensus gravitational bending portion
!                             of the delays and rates with respect to Gamma.
!
!     Subroutine interface -
!             Caller subroutines: TOCUP
!             Called subroutines: ADDA, ADDR, DELR
!
!     Programmer - DALE MARKHAM  01/17/77
!                  PETER DENATALE 07/20/77
!                  CHOPO MA / DAVID GORDON 04/12/84
!                  DAVID GORDON  07/31/84  ('RGD CONT' ADDED)
!                  SAVITA GOEL   06/03/87  (CDS FOR A900)
!                  GREC COOKE AND JIM RYAN 06/06/89 (Mods for Shipiro)
!                  Jim Ryan 89/09/13 Robertson algorithm removed.
!                  Jim Ryan 89/09/25 Hellings delay algorithm added.
!                  Jim Ryan 89/10/08 Relativity module code moved here.
!                  Jim Ryan 89.12.12 UNIX-like database interface
!                           implimented.
!                  Jim Ryan 89:12:14 Additional Shapiro and Helings
!                           information stored in database.
!                  Jim Ryan 90:11:20 Debug statements and some
!                           comments cleaned up.
!                  David Gordon 07/06/93 Access codes CONSNDEL and CONSNRAT
!                               added for Eubank's Consensus relativity model.
!                  David Gordon 12/27/93 Added access code CONSCONT, correction
!                               to convert Hellings to Consensus theoreticals.
!                  David Gordon 01/21/94 Added access code SHAPCONT, correction
!                               to convert Hellings to Shapiro theoreticals.
!                  David Gordon 94/04/18 Convert to Implicit None.
!                  David Gordon 94.10.05 Changed 'REL CFLG' from 31 to 30 words.
!                  David Gordon 96.01.30 Removed 'CON PART' L-code. Added
!                               'CONSPART' and 'SUN CONT'.
!                  David Gordon 98.08.18 DELR's to delete Shapiro and Hellings
!                               model Lcodes.
!                  David Gordon 98.11.16 ADDR for 'SUN2CONT', higher order
!                               solar bending contributions.
!                  David Gordon 98.12.16 ADDR for 'BENDPART', partials of the
!                               gravitional bending contributions with respect
!                               to Gamma.
!                  Jim Ryan     Sept 2002 Integer*2/4 mods.
!
!   THERA program structure
!
!   Do ADD for THEORY routine text message.
      CALL ADDA (int2(1),'THE MESS','Theory module identification    ',
     . int2(40), int2(1), int2(1))
!
!   Remove old Shapiro model Lcodes.
      CALL DELR (int2(2), 'SHAP DEL')
      CALL DELR (int2(2), 'SHAP RAT')
      CALL DELR (int2(2), 'SHAP T62')
      CALL DELR (int2(2), 'SHAPCONT')
      CALL DELR (int2(2), 'REL PART')
      CALL DELR (int2(2), 'REL CONT')
!
!   Remove old Hellings model Lcodes.
      CALL DELR (int2(2), 'HELL DEL')
      CALL DELR (int2(2), 'HELL RAT')
      CALL DELR (int2(2), 'HELL EMS')
!
!   Do ADD's for the Eubanks' Consensus model information. 07/06/93
      CALL ADDR (int2(2),'CONSNDEL','Consensus theo. delay (microsec)  '
     .     ,int2(2), int2(1), int2(1))
      CALL ADDR (int2(2),'CONSNRAT','Consensus theo. rate (sec/sec)    '
     .     ,int2(1), int2(1), int2(1))
!   Remove old Consensus correction to Hellings
      CALL DELR (int2(2), 'CONSCONT')
!
!   Do DEL's to delete any vestige of the old (incorrect) Shapiro relativistic
!   delay correction. Also delete the old Robertson delay and rate theoretical
!   lcodes. Also delete 'CON PART' from Calc 8.1 databases.
      CALL DELA (int2(1), 'REL MESS')
      CALL DELA (int2(1), 'RGD MESS')
      CALL DELR (int2(2), 'RGD CONT')
      CALL DELR (int2(2), 'THODELAY')
      CALL DELR (int2(2), 'THODRATE')
      CALL DELR (int2(2), 'CON PART')
!
!   Do the Add's for the relativity partials, contributions, and
!   relativity application status.
      CALL ADDA (int2(1),'REL CFLG','Relativisitc bending use status ',
     . int2(30), int2(1), int2(1))
      CALL ADDR (int2(2),'CONSPART','Consensus partial w.r.t. Gamma  ',
     . int2(2), int2(1), int2(1))
      CALL ADDR (int2(2),'CON CONT','Consensus bending contrib. (sec)',
     . int2(2), int2(1), int2(1))
      CALL ADDR (int2(2),'SUN CONT','Consensus bending contrib. (sec)',
     . int2(2), int2(1), int2(1))
      CALL ADDR (int2(2),'SUN2CONT','High order bending contrib.(sec)',
     . int2(2), int2(1), int2(1))
      CALL ADDR (int2(2),'BENDPART','Grav. bend. partial w.r.t. Gamma',
     . int2(2), int2(1), int2(1))
!
  500 RETURN
      END
!***********************************************************************
      SUBROUTINE THERI()
      IMPLICIT None
!
!     THERI is the Theory input and initialization section.
!
!     THERI program interface:
!
!     Common blocks used -
       INCLUDE 'ccon.i'
!          Variables 'from':
!            1.  KTHEC  -  The Theory routine flow control flag.
!                          (No longer has any function.)
!            2.  KTHED  -  The Theory routine debug control flag.
!            3.  KRELC  -  Relativity module flow control flag.
!
!     Program specifications -
!
      INTEGER*2      LTHEU(40)
      CHARACTER*40 C_LTHEU(2)
      INTEGER*2      L_on(30),  L_off(30)
      CHARACTER*30 C_L_on(2), C_L_off(2)
      EQUIVALENCE (LTHEU, C_LTHEU), (L_on, C_L_on), (L_off, C_L_off)
!
      DATA C_LTHEU /
     . 'THEORY ROUTINE: Eubanks Consensus Relati',
     . 'vity Model, Last mod 98DEC16, D. Gordon '/
!
      DATA C_L_on  /
     . 'Relativistic bending turned ON',
     . ' in theoreticals computation. '/
!
      DATA C_L_off /
     . 'Relativistic bending turned OF',
     . 'F in theoreticals computation.'/
!
!    Database access -
!          'PUT' variables:
!            1. LTHEU(40)  - The theory routine text message.
!          Access codes:
!            1. 'THE MESS' - The database access code for the Theory routine
!                            text message.
!
!    Subroutine interface -
!          Caller subroutines: INITL
!          Called subroutines: PUTA
!
!    Programmer - DALE MARKHAM  01/17/77
!                 PETER DENATALE 07/20/77
!                 CHOPO MA /DAVID GORDON 04/12/84
!                 JIM RYAN 89/06/06 Some character string used.
!                 Jim Ryan 89/10/08 Relativity module code moved here.
!                 Jim Ryan 89.12.12 UNIX-like database interface
!                          implimented.
!                 Jim Ryan 90:11:20 Comments cleaned up and debug fixed.
!                 David Gordon 93AUG02 Data base message revised for Consensus
!                              model.
!                 David Gordon 93DEC23 Fixed up debug output.
!                 David Gordon 94.04.18 Converted to Implicit None.
!                 David Gordon 94.10.05 Updated database text message. Added
!                              character variables for 'REL CFLG' message.
!                 David Gordon 94.12.15 Corrected dimensioning of C_L_on and
!                              C_L_off.
!                 David Gordon 96.01.30 Changed text message date.
!                 David Gordon 98.08.18 Changed text message.
!                 Jim Ryan     Sept 2002 Integer*2/4 mods.
!
!     THERI program structure
!
!     PUT the Theory Module text message into the database.
      CALL PUTA ('THE MESS      ', LTHEU, int2(40), int2(1), int2(1))
!
!     PUT the relativistic bending status into the database.
      IF(KRELC .eq. 0)  then
        CALL PUTA ('REL CFLG      ', L_on, int2(30), int2(1), int2(1))
      else
        CALL PUTA ('REL CFLG      ', L_off, int2(30), int2(1), int2(1))
      Endif
!
      RETURN
      END
!************************************************************************
      SUBROUTINE THERY ( DATMC, DAXOC, DIONC, DLPGR, EARTH, EPBASE,
     .           SITEP, SITEV, SITEA, SUN, STAR, XMOON, AT )
      Implicit none
!
!     Routine THERY computes the theoretical values of delay and delay rate
!     using the information generated by the modules. Subroutine CONSEN is
!     called to perform the computations using the Eubanks "Consensus"
!     relativity model. The Shapiro and Hellings relativity model computations
!     have been removed with Calc 9.0 and are no longer used here.
!
!     References: None
!
!     Program Interface
!       Input variables:
!         1. DATMC(2,2)  - The contributions to the delay and delay rate due to
!                          tropospheric refraction at each site. (sec, sec/sec)
!         2. DAXOC(2,2)  - The contributions to the delay and rate due to the
!                          antenna axis offsets. First index runs over sites,
!                          the second runs over delay and rate (sec,sec/sec).
!         3. DIONC(2)    - The contributions to the delay and rate due to
!                          ionospheric refraction at each site. (sec, sec/sec)
!         4. DLPGR       - The CT time derivative of the long period terms in
!                          'AT MINUS CT' offset. (sec/sec) (Not used)
!         5. EARTH(3,3)  - The solar system barycentric Earth position
!                          velocity, and acceleration vectors. The first index
!                          runs over the vector components and the second runs
!                          over the time derivatives. (m, m/sec, m/sec**2)
!         6. EPBASE(3,2) - The J2000.0 baseline position and velocity vectors.
!                          (m, m/sec)
!         7. SITEA(3,2)  - The J2000.0 geocentric acceleration vectors of each
!                          observing site. (m/sec**2)
!         8. SITEP(3,2)  - The J2000.0 geocentric position vectors of each
!                          observing site. (m)
!         9. SITEV(3,2)  - The J2000.0 geocentric velocity vectors of each
!                          observing site. (m/sec)
!        10. SUN(3,2)    - The J2000.0 geocentric Sun position and velocity
!                          vectors. (m, m/sec)
!        11. XMOON(3,2)  - The J2000.0 geocentric Moon position and velocity
!                          vectors. (m, m/sec)
!        12. STAR(3)     - The J2000.0 source unit vector. (unitless)
!        13. AT          - The Atomic Time fraction of the Atomic Time Day
!                          (TAI). (days) (Not used)
!
!     Common blocks used -
!
      INCLUDE 'ccon.i'
!       Variables 'from':
!         1.  KTHEC - The Theory routine flow control flag.
!                     (No longer has any function.)
!         2.  KTHED - The theory routine debug control flag.
!         3.  KRELC - The relativity module flow control flag.
!                       0     --> Gravitational bending used.
!                       NOT 0 --> Gravitational bending not used.
!
!   Program specifications -
      INTEGER*4 I
      REAL*8 DATMC(2,2), DAXOC(2,2), DIONC(2), SUN(3,2), EARTH(3,3),
     .       EPBASE(3,2), SITEA(3,2), STAR(3), SITEP(3,2), SITEV(3,2),
     .       XMOON(3,2), AT, SUNHAT(3), DLPGR
      Real*8 CONDEL(2), CONRAT, delta_t_grav, d_delta_t_grav,
     .       tg2_tg1, dtg2_tg1, xtg2_tg1, CON_CNTRB(2), CON_PART(2),
     .       delta_t_grav_Sun, d_delta_t_grav_Sun, CONSENSUS(2),
     .       con_cont(2), Sun_cntrb(2), Bend_par(2), Sunplus(2)
!
! 4.2.4 DATA BASE ACCESS -
!
!        'PUT' VARIABLES:
!          1. CONDEL(2)    - The theoretical delay from the Consensus
!                            model in two pieces in units of MICROSECONDS.
!                            The 1st is the integer microseconds and the
!                            2nd is the submicroseconds portion.
!          2. CONRAT       - The theoretical delay rate from the Consensus
!                            model/ (sec/sec).
!          3. SUN_CNTRB(2) - The solar gravitational bending delay and rate
!                            terms from the Consensus model. (sec, sec/sec)
!          4. CON_CNTRB(2) - The total gravitational bending delay and rate
!                            terms from the Consensus model. (sec, sec/sec)
!          5. CON_PART(2)  - The total Consensus delay and rate partials
!                            with respect to Gamma. (sec, sec/sec)
!          6. Sunplus(2)   - Higher order solar bending delay and rate
!                            contributions, as defined in IERS Conventions
!                            (1996), page 91, eqn. 14. (sec, sec/sec) It
!                            has also been added to the total solar bending
!                            delay and rate contibutions.
!          7. Bend_par(2)  - The relativistic bending delay and rate partials
!                            with respect to Gamma. (sec, sec/sec)
!
!        ACCESS CODES:
!          1. 'CONSNDEL' - The database access code for the theoretical
!                          delay using the Eubanks' Consensus model.
!          2. 'CONSNRAT' - The database access code for the theoretical
!                          delay rate using the Eubanks' Consensus model.
!          3. 'SUN CONT' - The database access code for the Consensus model
!                          Solar gravitational bending delay and rate terms.
!          4. 'CON CONT' - The database access code for the Consensus model
!                          TOTAL gravitational bending delay and rate terms.
!          5. 'CONSPART' - Database access code for the partial derivatives
!                          of the Consensus model delays and rates with
!                          respect to Gamma.
!          6. 'SUN2CONT' - Database access code for the additional solar
!                          bending due to higher order relativistic effects.
!                          This term is already in the theoretical, so to
!                          remove it, SUBTRACT the values in this access code.
!          7. 'BENDPART' - Database access code for the partial derivatives
!                          of the Consensus model gravitational bending
!                          delay and rate contributions with respect to Gamma.
!
!    Subroutine interface -
!          Caller subroutine:  DRIVR
!          Called subroutines:  PUT4, CONSEN
!
!    Program variables -
!           1. tg2_tg1      - The geometric delay corrected for relativistic
!                             effects using the Consensus model.
!           2. dtg2_tg1     - The time derivative of the geometric delay
!                             corrected for relativistic effects using the
!                             Consensus model.
!           3. delta_t_grav - The total differential gravitational time delay,
!                             or "bending delay" from the Consensus model.
!           4. d_delta_t_grav-The time derivative of the total differential
!                             gravitational time delay, or "bending delay,"
!                             from the Consensus model.
!           5. SUN_CNTRB(2) - The solar gravitational bending delay and rate
!                             contributions from the Consensus model. (s, s/s).
!           6. CON_CNTRB(2) - The total gravitational bending delay and rate
!                             contributions from the Consensus model. (s, s/s).
!           7. CON_PART(2)  - The partial derivatives of the Consensus model
!                             delay and rate with respect to Gamma (s and s/s).
!           8. Sunplus(2)  -  Higher order solar bending delay and rate
!                             contributions, as defined in IERS Conventions
!                             (1996), page 91, eqn. 14 (sec, sec/sec). It
!                             has also been added to the total solar bending
!                             delay and rate contibutions.
!           9. Bend_par(2)  - The relativistic bending delay and rate partials
!                             with respect to Gamma. (sec, sec/sec)
!
! 4.2.9 PROGRAMMER - DALE MARKHAM 01/17/77
!                  PETER DENATALE 07/20/77
!                  CHOPO MA / DAVID GORDON 04/12/84
!                  DAVID GORDON 06/19/84 REMOVED ATMOSPHERE.
!                  DAVID GORDON 07/31/84 RELATIVISTIC CORRECTIONS.
!                  DAVID GORDON 01/03/85 ADDED ATMOSPHERE AFTER MODS TO
!                               ATMOSPHERE FLAGS.
!                  SAVITA GOEL  06/03/87 CDS FOR A900.
!                  GREGG COOKE  05/01/89 NEW MODEL FROM I.SHAPIRO.
!                  JIM RYAN     06/06/89 Some bugs fixed and documentation
!                               modified.
!                  Jim Ryan     09/13/89 Robertson code removed and Hellings
!                               added.
!                  Jim Ryan     89.10.05 CPHYS common made an include file
!                  Jim Ryan     89.10.09 Relativity partials and contributions
!                               moved here.
!                  Jim Ryan     89.11.20 Shapiro algorithm modified to make
!                               it reflect Ryan's memo
!                  Jim Ryan     89.12.12 UNIX-like database interface
!                               implemented.
!                  Jim Ryan     89.12.14 Addional delay information stored
!                               and Helling rate implemented.
!                  C Ma         90.08.10 Corrections to documentation.
!                  Jim Ryan     90.11.20 Debug statement fixed and comments
!                               cleaned up.
!                  T. Marshall Eubanks & Brent Archinal 91.05.10 HELL EMS
!                               fixed, debug statements and comments modified.
!                               A900 and HP-UX versions consolidated except for
!                               dbh calls and ILUOUT.
!                  Jim Ryan     91.05.28 A few, mostly cosmetic changes made.
!                  David Gordon 93.04.27 GMEARTH put into cphys.i and
!                               defined in cinit.f; definition removed here.
!                  David Gordon 93.08.02 Thery modified to call subroutine
!                               Consen (previously called by DRIVR), and to
!                               do the puts for the Consensus delay and rate.
!                  David Gordon 93.12.22 Fixed up debug output.
!                  David Gordon 93.12.27 Added access code CONSCONT, correction
!                               to convert Hellings to Comsensus theoreticals.
!                  David Gordon 94.01.21 Added access code SHAPCONT, correction
!                               to convert Hellings to Shapiro theoreticals.
!                  David Gordon 94 Feb/March - Modified axis offset correction
!                               for use here.
!                  David Gordon 94.10.05 Corrected error in computing second
!                               half of Lcode for Shapiro delay contribution.
!                               Many unused variables removed.
!                  David Gordon 95.10.11 Minor correction to Hellings model
!                               when gravitational bending turned OFF (KRELC=1).
!                  David Gordon 96.01.30 Added Consensus model solar bending
!                               term (Sun_cntrb(2) and the L-code 'SUN CONT').
!                               Removed 'CON PART' L-code (Solar bending
!                               partials w.r.t. Gamma) and replaced it with
!                               'CONSPART' (Total Consensus delay and rate
!                               partials w.r.t. Gamma).
!                  David Gordon 98.08.18 Shapiro and Hellings models removed.
!                  David Gordon 98.11.16 Added Sunplus to CALL CONSEN argument
!                               list. Contains the delay and rate contributions
!                               due to higher order solar bending, as defined
!                               in the 1996 IERS Conventions, p. 91, eqn. 14.
!                               Added PUTR of 'SUN2CONT' to put it in the
!                               data bases. It is already included in the
!                               theoretical, therefore to remove its effects,
!                               you must SUBTRACT it from the theoretical.
!                  David Gordon 98.12.16 Added Bend_par(2), the partials of the
!                               gravitional bending contributions with respect
!                               to Gamma.
!                  Jim Ryan     Sept 2002 Integer*2/4 mods.
!
!     THERY program structure
!--------------------------------------------------------------------------
!
!   Call subroutine CONSEN to compute the delay and delay rate based on the
!   Eubanks' Consensus relativity model. We put it in its own subroutine
!   because it is a very long and detailed set of computations. 93JUL29, DG
!
      Call  CONSEN ( DATMC, EARTH, EPBASE, SITEP, SITEV,
     .      SITEA, SUN, XMOON, STAR, tg2_tg1, dtg2_tg1,
     .      delta_t_grav, d_delta_t_grav, delta_t_grav_Sun,
     .      d_delta_t_grav_Sun, Con_part, Bend_par, Sunplus )
!
!      print *,' Delay,Rate: ', tg2_tg1*1.D12, dtg2_tg1*1.D12
!   Add the axis offset corrections
      tg2_tg1  = tg2_tg1  + DAXOC(1,1) + DAXOC(2,1)
      dtg2_tg1 = dtg2_tg1 + DAXOC(1,2) + DAXOC(2,2)
!      print *,' Delay,Rate: ', tg2_tg1*1.D12, dtg2_tg1*1.D12
!
      CONSENSUS(1) =  tg2_tg1      ! Consensus delay
      CONSENSUS(2) = dtg2_tg1      ! Consensus rate
!
!      print *,' THERY/Delay (psec): ', CONSENSUS(1)*1.D12
!      print *,' THERY/Rate (psec/sec): ', CONSENSUS(2)*1.D12
!
!   Convert the theoretical delay to microseconds and split the double
!   precision results into an integer microseconds portion and a
!   submicroseconds portion.
      xtg2_tg1  = tg2_tg1  * 1.D6
      CONDEL(1) = IDINT(xtg2_tg1 )
      CONDEL(2) = xtg2_tg1 - CONDEL(1)
      CONRAT = dtg2_tg1
!
!   Put the Consensus model theoretical delays and rates into the database.
      CALL PUT4 ('CONSNDEL      ', CONDEL, int2(2), int2(1), int2(1))
      CALL PUT4 ('CONSNRAT      ', CONRAT, int2(1), int2(1), int2(1))
!
!  Compute and store the total gravitational light bending terms
      CON_CNTRB(1) = delta_t_grav
      CON_CNTRB(2) = d_delta_t_grav
!  Compute and store the solar gravitational light bending terms
      Sun_cntrb(1) = delta_t_grav_Sun
      Sun_cntrb(2) = d_delta_t_grav_Sun
!
!   Note: The Lcode 'CON CONT' will hold the TOTAL gravitational bending delay
!         and rate from the Sun, Moon, Earth, and planets. 'SUN CONT' will hold
!         the gravitational bending delay from the Sun only.
!         'CONSPART' will hold the TOTAL Consensus delay and rate partials with
!         respect to Gamma.
!
!   Put into the data base
      CALL PUT4 ('CON CONT      ', CON_CNTRB, int2(2), int2(1),int2(1))
      CALL PUT4 ('SUN CONT      ', SUN_CNTRB, int2(2), int2(1),int2(1))
      CALL PUT4 ('CONSPART      ', CON_PART, int2(2), int2(1), int2(1))
      CALL PUT4 ('BENDPART      ', BEND_PAR, int2(2), int2(1), int2(1))
!  The following is the higher order solar bending from the 1996 Conventions.
!   It is already included in the total theoreticals ('CONSNDEL' and
!   'CONSNRAT') and the total solar bending contribution ('SUN CONT').
      CALL PUT4 ('SUN2CONT      ', Sunplus, int2(2), int2(1), int2(1))
!
!--------------------------------------------------------------------------
!     Check KTHED to determine if debug output is neccessary.
      IF ( KTHED .ne. 0 )  Then
       WRITE (6, 9100 )
 9100  FORMAT (1X, "Debug output for subroutine THERY." )
    8  FORMAT(A,4D25.16/(7X,5D25.16))
       write(6,8)' tg2_tg1, dtg2_tg1 ',tg2_tg1, dtg2_tg1
       write(6,8)' CONDEL, CONRAT ', CONDEL, CONRAT
       write(6,8)' CON_CNTRB ',CON_CNTRB
       write(6,8)' SUN_CNTRB ',SUN_CNTRB
       write(6,8)' CON_PART  ',CON_PART
       write(6,8)' CONSENSUS ',CONSENSUS
       write(6,8)' Sunplus   ',Sunplus
      Endif
!
      Return
      END
!**********************************************************************
      SUBROUTINE CONSEN ( DATMC, EARTH, EPBASE, SITEP, SITEV,
     .           SITEA, SUN, XMOON, STAR, tg2_tg1, dtg2_tg1,
     .           delta_t_grav, d_delta_t_grav, delta_t_grav_Sun,
     .           d_delta_t_grav_Sun, Con_part, Bend_par, Sunplus )
      IMPLICIT NONE
!
!     Routine CONSEN computes the theoretical values of delay and delay rate
!     using the Eubanks' Consensus relativity model.
!
!     References:
!       - Eubanks, T.M., "A Consensus Model for Relativistic Effects in
!          Geodetic VLBI," in "Proceedings of the U.S. Naval Observatory
!          Workshop on Relativistic Models for Use in Space Geodesy,",
!          T.M. Eubanks (editor), USNO, Washington, DC, June, 1991.
!
!      Input Variables:
!         1.  DATMC(2,2)   -  The contributions to the delay and delay rate due
!                             to tropospheric refraction at each site. (s, s/s)
!         2.  EARTH(3,3)   -  The solar system barycentric Earth position,
!                             velocity, and acceleration vectors. The first
!                             index runs over the vector components and the
!                             second runs over the time derivatives.
!                             (m, m/sec, m/sec**2)
!         3.  EPBASE(3,2)  -  The J2000.0 baseline position and velocity
!                             vectors.  (m, m/sec)
!         4.  SITEP(3,2)   -  The J2000.0 geocentric position vectors of each
!                             observing site. (m)
!         5.  SITEV(3,2)   -  The J2000.0 geocentric velocity vectors of each
!                             observing site. (m/sec)
!         6.  SITEA(3,2)   -  The J2000.0 geocentric acceleration vectors of
!                             each observing site. (m/sec**2)
!         7.  SUN(3,2)     -  The J2000.0 geocentric Sun position and velocity
!                             vectors.  (m, m/sec)
!         8.  XMOON(3,2)   -  The J2000.0 geocentric Moon position and velocity
!                             vectors.  (m, m/sec)
!         9.  STAR(3)      -  The J2000.0 source unit vector. (unitless)
!
!      Output Variables:
!         1. tg2_tg1       -  The geometric delay corrected for relativistic
!                             effects (but not atmospheric refraction) using
!                             the Consensus model.
!         2. dtg2_tg1      -  The time derivative of the geometric delay
!                             corrected for relativistic effects using the
!                             Consensus model.
!         3. delta_t_grav  -  The total differential gravitational time delay,
!                             or "bending delay" from the Consensus model.
!         4. d_delta_t_grav-  The time derivative of the total differential
!                             gravitational time delay, or "bending delay" from
!                             the Consensus model.
!         5. delta_t_grav_Sun-The Sun's contribution to the "bending delay"
!                             from the Consensus model. Now includes
!                             Sunplus(1).
!         6. d_delta_t_grav_Sun-The time derivative of the Sun's contribution
!                             to the "bending delay" from the Consensus model.
!                             Now includes Sunplus(2).
!         7. Con_part      -  Partial derivatives of the Consensus delays and
!                             rates with respect to Gamma.
!         8. Sunplus(2)    -  Higher order solar bending delay and rate
!                             contributions, as defined in IERS Conventions
!                             (1996), page 91, eqn. 14. (sec, sec/sec) It
!                             has also been added to the total solar bending
!                             delay and rate contibutions.
!         9. Bend_par(2)   -  The relativistic bending delay and rate partials
!                             with respect to Gamma. (sec, sec/sec)
!
!     Common blocks used -
!
       INCLUDE 'cphys.i'
!          Variables 'from':
!            1. VLIGHT  - The velocity of light in vacuum.  (m/sec)
!            2. VLIGHT2 - The velocity of light squared. (m/sec)**2
!            3. VLIGHT3 - The velocity of light cubed. (m/sec)**3
!            4. GAMMA   - The post Newtonian expansion parameter which
!                         affects light bending. (1.0 used here). (unitless)
!            5. GMSUN, GMMOON, GMEARTH, GMPLANET(7) - Gravitational constant
!                         times the masses of the Sun, Moon, Earth, and the
!                         other planets except Pluto.
!            6. REARTH  - The equatorial radius of the Earth. (meters)
!
      INCLUDE 'csolsys.i'
!       Variables 'from':
!         1. SPLANET(3,2,7) - The J2000.0 Solar System Barycentric positions
!                             and velocities of all planets except the Earth
!                             and Pluto. (meters, meters/sec) The first index
!                             runs over X,Y, and Z, the second runs over
!                             position and velocity, and the third runs over
!                             the planets, where
!                                    1 = Mercury
!                                    2 = Venus
!                                    3 = Mars
!                                    4 = Jupiter
!                                    5 = Saturn
!                                    6 = Uranus
!                                    7 = Neptune
!
!         2. GPLANET(3,2,7) - The J2000.0 Geocentric positions and velocities
!                             of all planets except the Earth and Pluto.
!                             (meters, meters/sec) The first index runs over
!                             X,Y, and Z, the second runs over position and
!                             velocity, and the third runs over the planets,
!                             where
!                                    1 = Mercury
!                                    2 = Venus
!                                    3 = Mars
!                                    4 = Jupiter
!                                    5 = Saturn
!                                    6 = Uranus
!                                    7 = Neptune
!
      Real*8           PI, TWOPI, HALFPI, CONVD, CONVDS, CONVHS, SECDAY
      COMMON / CMATH / PI, TWOPI, HALFPI, CONVD, CONVDS, CONVHS, SECDAY
!
       INCLUDE 'ccon.i'
!            Variables 'from':
!              1.  KTHEC  -  The Theory routine flow control flag.
!                            (No longer has any function.)
!              2.  KTHED  -  The theory routine debug control flag.
!              3.  KRELC  -  The relativity module flow control flag.
!                            0     --> Gravitational bending used.
!                            NOT 0 --> Gravitational bending not used.
!
      INCLUDE 'cobsn.i'
!          Variables from:
!            1. Nzero  -  Set to 1 or 2 if station 1 or 2 is at the geocenter,
!                         otherwise equals zero. For correlator usage.
!
!   Program Specifications -
      INTEGER*4 l,k
      REAL*8 DATMC(2,2), SUN(3,2), EARTH(3,3), EPBASE(3,2), SITEA(3,2),
     .       STAR(3), SITEP(3,2), SITEV(3,2), XMOON(3,2), CON_CNTRB(2),
     .       SUN_CNTRB(2), DOTP, VECMG
      Real*8 XsubEarth(3), VsubEarth(3), AsubEarth(3)
      Real*8 x_sub1t1(3), x_sub2t1(3), w_sub1(3), w_sub2(3)
      Real*8 a_sub1(3), a_sub2(3)
      Real*8 unit_K(3), b_nought(3),db_nought(3)
      Real*8 x_subSun(3), x_subMoon(3), v_subSun(3), v_subMoon(3)
      Real*8 XsubSun(3), XsubMoon(3), VsubSun(3), VsubMoon(3)
      Real*8 R_Earth_Sun(3), R_Earth_Moon(3)
      Real*8 V_Earth_Sun(3), V_Earth_Moon(3)
      Real*8 Xsub1(3), Xsub2(3), t_sub1, t_sub2
      Real*8 dXsub1(3), dXsub2(3)
      Real*8 x1Sun(3), x1Moon(3), x1Planet(3,7)
      Real*8 del_t_Sun, del_t_Moon, del_t_Planet(7)
      Real*8 XSunt1J(3), XMoont1J(3), XPlant1J(3,7)
      Real*8 dXSunt1J(3), dXMoont1J(3), dXPlant1J(3,7)
      Real*8 R1Sunt1(3), R1Moont1(3), R1Plant1(3,7)
      Real*8 R2Sunt1(3), R2Moont1(3), R2Plant1(3,7)
      Real*8 dR1Sunt1(3), dR1Moont1(3), dR1Plant1(3,7)
      Real*8 dR2Sunt1(3), dR2Moont1(3), dR2Plant1(3,7)
      Real*8 delta_t_grav, delta_t_grav_Sun, delta_t_grav_Moon,
     .       delta_t_grav_Earth, delta_t_grav_Plan(7),
     .       delta_t_grav_Planets
      Real*8 d_delta_t_grav, d_delta_t_grav_Sun, d_delta_t_grav_Moon,
     .       d_delta_t_grav_Earth, d_delta_t_grav_Plan(7),
     .       d_delta_t_grav_Planets
      Real*8 U, U_Sun, dU, absVEarth
      Real*8 C_Earth, C_Sun, C_Moon, C_Plan(7)
      Real*8 term_a, term_b, term_c, term_d, term_e, term_f,
     .       term_g, term_h
      Real*8 dterm_a, dterm_b, dterm_c, dterm_d, dterm_e, dterm_f,
     .       dterm_g, dterm_h
      Real*8 term1, term2a, term2b, term2c, term2d, term2,
     .       term2bcd, term3a, term3b, term3, term4,
     .       term123, vec_sum(3), tv2_tv1
      Real*8 dterm1, dterm2a, dterm2b, dterm2c, dterm2d, dterm2,
     .       dterm2bcd, dterm3a, dterm3b,  dterm3, dterm4,
     .       dterm123, dvec_sum(3), dtv2_tv1
      Real*8 V_w1(3), V_w2(3), K_V_w1, K_V_w2           !!
      Real*8 tg2_tg1, dtg2_tg1, Con_part(2), Bend_par(2)
      Real*8 w2_w1(3), a2_a1(3), K_dotw2w1, dK_dotw2w1
!     Real*8 k_sub1(3), k_sub2(3)
      Real*8 KdotB, dKdotB, VdotB, dVdotB, KdotV, dKdotV
      Real*8 delta_t2_t1, d_delta_t2_t1, term10a, term10b, term10c,
     .       dterm10a, dterm10b, dterm10c, del_del_t(2,2)
      Real*8 x_delta_t2_t1, dx_delta_t2_t1
      Real*8 vecmg1,vecmg2
      Real*8 N_hat(3), dN_hat(3), Vmag_S, CSun1, NplusK(3), V1, dV1,
     .       V2, dV2, Sunplus(2)
!
!     Subroutines used:  DOTP, IDINT, PUT4, VECAD, VECMG
!
!     Constants used - VLIGHT      - Speed of light (m/s)
!                      VLIGHT2     - Speed of light squared
!                      VLIGHT3     - Speed of light cubed
!                      GMMOON      - GM of the Moon.
!                      GMEARTH     - GM of the Earth.
!                      GMSUN       - GM of the Sun.
!                      GMPlanet(7) - GM's of all planets except Earth and Pluto
!
!   Program variables -
!
!    1.  XsubEarth(3)  =  Barycentric radius vector of the geocenter (meters).
!    2.  VsubEarth(3)  =  Barycentric velocity vector of the geocenter (m/sec).
!    3.  AsubEarth(3)  =  Barycentric acceleration of the geocenter (m/sec**2).
!
!    4.  x_sub1t1(3)   =  Geocentric radius vector of receivers 1 and 2 at the
!    5.  x_sub2t1(3)        geocentric time t_sub1 (meters).
!
!    6.  w_sub1(3)     =  Geocentric velocity of receiver 1 (m/sec).
!    7.  w_sub2(3)     =  Geocentric velocity of receiver 2 (m/sec).
!
!    8.  a_sub1(3)     =  Geocentric acceleration of receiver 1 (m/sec).
!    9.  a_sub2(3)     =  Geocentric acceleration of receiver 2 (m/sec).
!
!   10.  x_subSun(3)   =  Geocentric radius vector of the Sun (meters).
!   11.  x_subMoon(3)  =  Geocentric radius vector of the Moon (meters).
!   12.  GPLANET(l,1,k)=  Geocentric radius vectors of the planets (meters).
!                          (l = X,Y,Z; k = planet #)
!
!   13.  v_subSun(3)   =  Geocentric velocity vector of the Sun (m/sec).
!   14.  v_subMoon(3)  =  Geocentric velocity vector of the Moon (m/sec).
!   15.  GPLANET(l,2,k)=  Geocentric velocity vector of the planets (m/sec).
!                          (l = X,Y,Z; k = planet #)
!
!   16.  Xsub1(3)      =  Barycentric radius vector of receiver 1 (meters).
!   17.  Xsub2(3)      =  Barycentric radius vector of receiver 2 (meters).
!   18.  dXsub1(3)     =  Barycentric velocity vector of receiver 1 (m/sec).
!   19.  dXsub2(3)     =  Barycentric velocity vector of receiver 2 (m/sec).
!
!   20.  x1Sun(3)      =  Vector from receiver 1 to the Sun at time t_sub1
!   21.  x1Moon(3)     =  Vector from receiver 1 to the Moon at time t_sub1
!   22.  x1Planet(3,7) =  Vector from receiver 1 to the planets at time t_sub1
!
!   23.  XsubSun(3)    =  Barycentric radius vector of the Sun/Moon/planets
!   24.  XsubMoon(3)
!   25.  SPLANET(l,1,k)
!
!   26.  VsubSun(3)    =  Barycentric velocity vector of the Sun/Moon/planets
!   27.  VsubMoon(3)
!   28.  SPLANET(l,2,k)
!
!   29.  XSunt1J(3)    =  SSBC radius vector to the Sun/Moon/planets
!   30.  XMoont1J(3)         at the time of closest approach.
!   31.  XPlant1J(3,7)              (meters)
!
!   32.  R_Earth_Sun(3) = Vector from the Sun to the geocenter
!   33.  R_Earth_Moon(3)= Vector from the Moon to the geocenter
!   34.  -GPLANET(l,1,k)= Vector from planets to the geocenter
!
!   35.  V_Earth_Sun(3) = Velocity Vector of the geocenter from the Sun
!
!   36.  R1Sunt1(3)    =  Vector from the Sun/Moon/planets to receiver
!   37.  R1Moont1(3)   =      1 at the time of closest approach.
!   38.  R1Plant1(3,7) =                (meters)
!
!   39.  R2Sunt1(3)    =  Vector from the Sun/Moon/planets to receiver
!   40.  R2Moont1(3)   =      2 at the time of closest approach.
!   41.  R2Plant1(3,7) =              (meters)
!
!   42.  t_sub1        =  Time of arrival of the signal at receivers 1 and 2.
!   43.  t_sub2            [Note: not needed here so we don't actually define
!                          them.]
!
!   44.  del_t_Sun     =  Difference between arrival time at receiver 1, t_sub1,
!   45.  del_t_Moon         and time of closest approach to Sun/Moon/planets
!   46.  del_t_Planet(7)      ( > 0 if closest approach is before arrival)
!
!   47.  unit_K(3)     =  Barycentric unit vector to the source (in the absense
!                         of gravitational or aberrational bending).
!
!   48.  k_sub1(3)     =  Aberrated unit vector from station 1 to the source.
!   49.  k_sub2(3)     =  Aberrated unit vector from station 2 to the source.
!                          [Note: not computed here, see atmosphere module.]
!
!   50.  b_nought(3)   =  A priori geocentric baseline vector at time t_sub1
!                         (Baseline vector and it's velocity. Defined from
!                         site #1 to site #2 (M,M/S).)
!
!   51.  U             =  Sun's potential
!
!   52.  delta_t_grav_Sun
!   53.  delta_t_grav_Moon =  The differential gravitational time delay for
!   54.  delta_t_grav_Plan(7)     the Sun/Moon/planets/Earth.
!   55.  delta_t_grav_Earth
!
!   56.  delta_t_grav_Planets = Sum of bending delays for the planets
!
!   57.  delta_t_grav  =  The total differential gravitational time delay,
!                         or "bending delay."
!
!   58.  tv2_tv1       =  The theoretical vacuum delay computed using the
!                         Consensus model. (sec)
!
!   59.  tg2_tg1       =  The total theoretical geometric delay (doesn't
!                         include tropospheric refraction effects which are
!                         usually added in Solve) computed using the Consensus
!                         model. (sec)
!
! 4.2.9 PROGRAMMER - DAVID GORDON 04/22/93 thru 08/02/93 - Written and debuged
!                    D. Gordon Nov. 1993 - Modified to use all planets
!                              except Pluto.
!                    D. Gordon Jan-Mar 1994 - Modified for axis offset
!                              correction. Not using Eubank's Step 10.
!                    D. Gordon 10.05.94 Many unused variable and much unused
!                              code removed.
!                    D. Gordon 96.01.30 Added section to compute partial
!                              derivatives of delay and rate w.r.t. Gamma.
!                    D. Gordon 96.02.09 Corrected typo in Step 2 debug printout.
!                    D. Gordon 98.08.18 Mods for geocenter station.
!                    D. Gordon 98.11.16 Added computation of higher order
!                              solar bending term, from IERS Conventions
!                              (1996), page 91, eqn. 14. This term is now
!                              added to the total solar bending term. It
!                              only becomes significant within about 2
!                              degrees of the Sun.
!                    D. Gordon 98.12.16 Added computation of Bend_par(2), the
!                              partial derivatives of the gravitational
!                              bending contributions with respect to Gamma.
!
!     CONSEN program structure:
!
!     Compute the theoretical delay and delay rate using Eubanks's Consensus
!     Relativity model.
!_____________________________________________________________________________
!
!     Copy CALC variables into variables with names which mimic the variables
!     in the consensus paper. VLIGHT is used for the velocity of light because
!     'C' is a poor variable name that could be easily lost.
!
      Do l=1,3
!
       XsubEarth(l) = EARTH(l,1)     ! SSBC Earth position
       VsubEarth(l) = EARTH(l,2)     !   ditto    velocity
       AsubEarth(l) = EARTH(l,3)     !   ditto    acceleration
!
       x_sub1t1(l) = SITEP(l,1)      ! Site 1 geocentric position
       w_sub1(l)   = SITEV(l,1)      !      ditto        velocity
       a_sub1(l)   = SITEA(l,1)      !      ditto        acceleration
!
       x_sub2t1(l) = SITEP(l,2)      ! Site 2 geocentric position
       w_sub2(l)   = SITEV(l,2)      !      ditto        velocity
       a_sub2(l)   = SITEA(l,2)      !      ditto        acceleration
!
       unit_K(l)    = STAR(l)        ! J2000.0 unit source vector
!
       x_subSun(l)  = SUN(l,1)       ! Geocentric Sun position
       v_subSun(l)  = SUN(l,2)       !       ditto    velocity
!
       x_subMoon(l) = XMOON(l,1)     ! Geocentric Moon position
       v_subMoon(l) = XMOON(l,2)     !       ditto     velocity
!
       b_nought(l)  = -EPBASE(l,1)   ! Baseline vector from site 1 to site 2
       db_nought(l) = -EPBASE(l,2)   ! Time derivative of baseline vector
!
       R_Earth_Sun(l)  = -SUN(l,1)
       V_Earth_Sun(l)  = -SUN(l,2)    ! = -v_subSun(l) also
       R_Earth_Moon(l) = -XMOON(l,1)
!
!   SSBC radius vectors of Sun and Moon:
       XsubSun(l)  = XsubEarth(l) + x_subSun(l)
       XsubMoon(l) = XsubEarth(l) + x_subMoon(l)
!
!   SSBC velocity vectors of Sun and Moon:
       VsubSun(l)  = VsubEarth(l) + v_subSun(l)
       VsubMoon(l) = VsubEarth(l) + v_subMoon(l)
!
      Enddo
!
!    Atomic time, TAI, at receiver #1:
!      t_sub1 =   ! Not needed here
!
      If (KTHED .ne. 0) Then
       write(6,'(/,15x,"  Debug output for subroutine CONSEN",/)')
       write(6,8)' GMSUN  ', GMSUN
       write(6,8)' GMMOON ', GMMOON
       write(6,8)' GMEARTH', GMEARTH
       write(6,'("GM-Planets = ",3d25.16,/,5x,4d25.16)') GMPLANET
!      write(6,8)' GAMMA  ', GAMMA
       write(6,'(/,"XsubEarth:",3D23.14)') XsubEarth
       write(6,'("VsubEarth:",3D23.14)') VsubEarth
       write(6,'("AsubEarth:",3D23.14)') AsubEarth
       write(6,'("x_sub1t1:",3D23.14)') x_sub1t1
       write(6,'("w_sub1:",3D23.14)') w_sub1
       write(6,'("a_sub1:",3D23.14)') a_sub1
       write(6,'("x_sub2t1:",3D23.14)') x_sub2t1
       write(6,'("w_sub2:",3D23.14)') w_sub2
       write(6,'("a_sub2:",3D23.14)') a_sub2
       write(6,'("unit_K:",3D23.14)') unit_K
       write(6,'("x_subSun:",3D23.14)') x_subSun
       write(6,'("x_subMoon:",3D23.14)') x_subMoon
       write(6,'("v_subSun:",3D23.14)') v_subSun
       write(6,'("v_subMoon:",3D23.14)') v_subMoon
       write(6,'("b_nought:",3D23.14)') b_nought
       write(6,'("db_nought:",3D23.14)') db_nought
       write(6,'("R_Earth_Sun:",3D23.14)') R_Earth_Sun
       write(6,'("V_Earth_Sun:",3D23.14)') V_Earth_Sun
       write(6,'("R_Earth_Moon:",3D23.14)') R_Earth_Moon
       write(6,'("XsubSun:",3D23.14)') XsubSun
       write(6,'("XsubMoon:",3D23.14)') XsubMoon
       write(6,'("VsubSun:",3D23.14)') VsubSun
       write(6,'("VsubMoon:",3D23.14)') VsubMoon
      Endif
!
!******************************************************************************
!  Step 1: Estimate barycentric radius and velocity vectors for stations 1
!          and 2 at time t_sub1 (Equation 6 and time derivative of).
!
      Do l=1,3
       Xsub1(l) = XsubEarth(l) + x_sub1t1(l)
       Xsub2(l) = XsubEarth(l) + x_sub2t1(l)
       dXsub1(l) = VsubEarth(l) + w_sub1(l)     !derivative
       dXsub2(l) = VsubEarth(l) + w_sub2(l)     !derivative
      Enddo
!
      If (KTHED .ne. 0) Then
       write(6,'(/,15x,"Step 1 dump:")')
       write(6,8)' Xsub1 ',Xsub1
       write(6,8)' Xsub2 ',Xsub2
       write(6,8)' dXsub1 ',dXsub1
       write(6,8)' dXsub2 ',dXsub2
      Endif
!
!******************************************************************************
!  Step 2: Estimate the vectors from the Sun, the Moon, and each planet (except
!          Earth and Pluto) to receiver 1.
!
!   Eq. 5a  -  Find time of closet approach to the gravitating body. [Actually
!              we find only how much earlier (or later) the quasar's rays
!              passed closest to the gravitating body. Then, if earlier, we
!              extrapolate the body's position back to that earlier time. If
!              not earlier we just keep the current position.]
!
!  J2000.0 vector from receiver #1 to the Sun/Moon/planets:
      Do l=1,3
       x1Sun(l)  = XsubSun(l)  - Xsub1(l)
       x1Moon(l) = XsubMoon(l) - Xsub1(l)
      Enddo
      Do k=1,7
       do l=1,3
        x1Planet(l,k)  = SPLANET(l,1,k)  - Xsub1(l)
       enddo
      Enddo
!
!   more good stuff
       KdotB  = DOTP(unit_K,b_nought)
       dKdotB = DOTP(unit_K,db_nought)     !derivative
!
! * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
!  Sun:
       del_t_Sun  = DOTP(unit_K,x1Sun) / Vlight
       If(del_t_Sun .lt. 0.D0) del_t_Sun = 0.0D0
!
        Do l=1,3
!    SSBC vector to Sun at time of closest approach and its time derivative:
         XSunt1J(l) = Xsubsun(l) - VsubSun(l)*del_t_Sun
         dXSunt1J(l) = Vsubsun(l)   !Derivative (approx. - no acceleration)
!
!         equation 5b: Vector from the Sun to receiver 1
         R1Sunt1(l) = Xsub1(l) - XSunt1J(l)
         dR1Sunt1(l) = dXsub1(l) - dXSunt1J(l)   !Derivative
!
!         equation 5c: Vector from the Sun to receiver 2
         R2Sunt1(l) = Xsub2(l) - VsubEarth(l)*KdotB/VLIGHT - XSunt1J(l)
         dR2Sunt1(l) = dXsub2(l) -  AsubEarth(l)*KdotB/VLIGHT -
     .                 VsubEarth(l)*dKdotB/VLIGHT - dXSunt1J(l)   !Derivative
        Enddo
!
!  98NOV18 addition, unit vector from Sun to receiver #1
          Call VUNIT (R1Sunt1, N_hat)
          Vmag_S = VECMG(R1Sunt1)
          Do l=1,3
           dN_hat(l) = dR1Sunt1(l)/Vmag_S -
     .        R1Sunt1(l)*(DOTP(dR1Sunt1,R1Sunt1))/Vmag_S**3
          Enddo
!
! * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
!  Moon:
       del_t_Moon = DOTP(unit_K,x1Moon) / Vlight
       If(del_t_Moon .lt. 0.D0) del_t_Moon = 0.D0
!
        Do l=1,3
!    SSBC vector to Moon at time of closest approach and its time derivative:
         XMoont1J(l) = XsubMoon(l) - VsubMoon(l)*del_t_Moon
         dXMoont1J(l) = VsubMoon(l)    !Derivative (approx. - no acceleration)
!
!        equation 5b: Vector from the Moon to receiver 1
         R1Moont1(l) = Xsub1(l) - XMoont1J(l)
         dR1Moont1(l) = dXsub1(l) - dXMoont1J(l)   !Derivative
!
!        equation 5c: Vector from the Moon to receiver 2
         R2Moont1(l) = Xsub2(l) - VsubEarth(l)* KdotB/VLIGHT -
     .                 XMoont1J(l)
         dR2Moont1(l) = dXsub2(l) - AsubEarth(l)*KdotB/VLIGHT -
     .                VsubEarth(l)*dKdotB/VLIGHT - dXMoont1J(l)  !Derivative
        Enddo
!
! * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
!  Planets:
      Do k=1,7      !Planet loop
       del_t_Planet(k)  = DOTP(unit_K,x1Planet(1,k)) / Vlight
       If(del_t_Planet(k) .lt. 0.D0) del_t_Planet(k) = 0.D0
!
        Do l=1,3
!    SSBC vector to Planet at time of closest approach and its time derivative:
         XPlant1J(l,k) = SPLANET(l,1,k) - SPLANET(l,2,k)*del_t_Planet(k)
         dXPlant1J(l,k) = SPLANET(l,2,k)    ! Derivative  (approximate)
!
!        equation 5b: Vector from Planet to receiver 1
         R1Plant1(l,k) = Xsub1(l) - XPlant1J(l,k)
         dR1Plant1(l,k) = dXsub1(l) - dXPlant1J(l,k)   !Derivative
!
!        equation 5c: Vector from Planet to receiver 2
         R2Plant1(l,k) = Xsub2(l) - VsubEarth(l)*KdotB/VLIGHT -
     .                XPlant1J(l,k)
         dR2Plant1(l,k) = dXsub2(l) - AsubEarth(l)*KdotB/VLIGHT -
     .               VsubEarth(l)*dKdotB/VLIGHT - dXPlant1J(l,k)   !Derivative
        Enddo
      Enddo      !Planet loop
!
!******************* Debug ********************
      IF(KTHED .ne. 0) Then               ! Debug
       write(6,'(/,15x,"Step 2 dump:")')
       write(6,8)' x1Sun ',x1Sun
       write(6,8)' x1Moon ',x1Moon
      Do k=1,7
         write(6,'("x1Planet(",i1,") = ",3d25.16)') k,x1Planet(1,k),
     .            x1Planet(2,k),x1Planet(3,k)
         write(6,'("SPLANET(",i1,") = ",3d25.16)') k,SPLANET(1,1,k),
     .            SPLANET(2,1,k),SPLANET(3,1,k)
      Enddo
!
       write(6,8)' KdotB, dKdotB ', KdotB, dKdotB
       write(6,8)' del_t_Sun ',del_t_Sun
       write(6,8)' XSunt1J ',XSunt1J
       write(6,8)' dXSunt1J ',dXSunt1J
       write(6,8)' R1Sunt1 ',R1Sunt1
       write(6,8)' dR1Sunt1 ',dR1Sunt1
       write(6,8)' R2Sunt1 ',R2Sunt1
       write(6,8)' dR2Sunt1 ',dR2Sunt1
       write(6,8)' del_t_Moon ',del_t_Moon
       write(6,8)' XMoont1J ',XMoont1J
       write(6,8)' dXMoont1J ',dXMoont1J
       write(6,8)' R1Moont1 ',R1Moont1
       write(6,8)' dR1Moont1 ',dR1Moont1
       write(6,8)' R2Moont1 ',R2Moont1
       write(6,8)' dR2Moont1 ',dR2Moont1
       write(6,8)' Vmag_S ',  Vmag_S
       write(6,8)' N_hat  ',  N_hat
       write(6,8)' dN_hat ', dN_hat
!
      Do k=1,7
         write(6,'("del_t_Planet(",i1,") = ",d25.16)')k,del_t_Planet(k)
         write(6,'("XPlant1J(",i1,") = ",3d25.16)') k,XPlant1J(1,k),
     .            XPlant1J(2,k),XPlant1J(3,k)
         write(6,'("dXPlant1J(",i1,") = ",3d25.16)') k,dXPlant1J(1,k),
     .            dXPlant1J(2,k),dXPlant1J(3,k)
         write(6,'("R1Plant1(",i1,") = ",3d25.16)') k,R1Plant1(1,k),
     .            R1Plant1(2,k),R1Plant1(3,k)
         write(6,'("dR1Plant1(",i1,") = ",3d25.16)') k,dR1Plant1(1,k),
     .            dR1Plant1(2,k),dR1Plant1(3,k)
         write(6,'("R2Plant1(",i1,") = ",3d25.16)') k,R2Plant1(1,k),
     .            R2Plant1(2,k),R2Plant1(3,k)
         write(6,'("dR2Plant1(",i1,") = ",3d25.16)') k,dR2Plant1(1,k),
     .            dR2Plant1(2,k),dR2Plant1(3,k)
      Enddo
!
      Endif                               ! Debug
!
!******************************************************************************
!  Step 3: Use Equation 2 to estimate the differential gravitational delay for
!          the Sun, the Moon, and the Planets.
!
       C_Sun = (1.0D0 + gamma) * GMSUN/VLIGHT3
       vecmg1 = VECMG(R1Sunt1)
       term_a = vecmg1 + DOTP(unit_K,R1Sunt1)
       vecmg2 = VECMG(R2Sunt1)
       term_b = vecmg2 + DOTP(unit_K,R2Sunt1)
!    Derivatives:
       dterm_a = Dotp(R1Sunt1,dR1Sunt1)/vecmg1 + DOTP(unit_K,dR1Sunt1)
       dterm_b = Dotp(R2Sunt1,dR2Sunt1)/vecmg2 + DOTP(unit_K,dR2Sunt1)
!
       delta_t_grav_Sun = C_Sun * DLOG(term_a / term_b)
       d_delta_t_grav_Sun = C_Sun * ( dterm_a/term_a -
     .                      dterm_b/term_b )                 !derivative
!
!  98NOV18 addition: Additional solar gravitional delay term, for observations
!   close to the Sun. IERS Conventions (1996), paged 91, equation 14.
       CSun1 = C_Sun**2 * VLIGHT
        Call VECAD (N_hat, unit_K, NplusK)
        V1 = DOTP(b_nought, NplusK)
       dV1 = DOTP(db_nought, NplusK) + DOTP(b_nought, dN_hat)
        V2 = term_a**2
       dV2 = 2.D0 * term_a * dterm_a
        Sunplus(1) = CSun1 * V1 / V2
        Sunplus(2) = CSun1*dV1/V2 - CSun1*V1*DV2/V2**2
!  Don't add thess to the solar bending yet, or they will mess up the Gamma
!   partials.
!
! * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
       C_Moon = (1.0D0 + gamma) * GMMoon/VLIGHT3
       vecmg1 = VECMG(R1Moont1)
       term_c = vecmg1 + DOTP(unit_K,R1Moont1)
       vecmg2 = VECMG(R2Moont1)
       term_d = vecmg2 + DOTP(unit_K,R2Moont1)
!    Derivatives:
       dterm_c = Dotp(R1Moont1,dR1Moont1)/vecmg1 +
     .           DOTP(unit_K,dR1Moont1)
       dterm_d = Dotp(R2Moont1,dR2Moont1)/vecmg2 +
     .           DOTP(unit_K,dR2Moont1)
!
       delta_t_grav_Moon = C_Moon * DLOG(term_c / term_d)
       d_delta_t_grav_Moon = C_Moon* ( dterm_c/term_c -
     .                      dterm_d/term_d )                   !derivative
!
      IF(KTHED .ne. 0) Then               ! Debug
       write(6,'(/,15x,"Step 3 dump:")')
       write(6,8)' C_Sun    ',C_Sun
       write(6,8)' term_a, dterm_a ',term_a, dterm_a
       write(6,8)' term_b, dterm_b ',term_b, dterm_b
       write(6,8)' delta_t_grav_Sun, d_delta_t_grav_Sun,  ',
     .             delta_t_grav_Sun, d_delta_t_grav_Sun
       write(6,8)' C_Moon   ',C_Moon
       write(6,8)' term_c, dterm_c ',term_c, dterm_c
       write(6,8)' term_d, dterm_d ',term_d, dterm_d
       write(6,8)' delta_t_grav_Moon, d_delta_t_grav_Moon,  ',
     .             delta_t_grav_Moon, d_delta_t_grav_Moon
       write(6,8)' CSun1    ', CSun1
       write(6,8)' NplusK   ', NplusK
       write(6,8)' V1, dV1  ', V1, dV1
       write(6,8)' V2, dV2  ', V2, dV2
       write(6,8)' Sunplus  ', Sunplus
      Endif                               ! Debug
! * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
      Do k=1,7       ! Planet loop
!
       C_Plan(k) = (1.0D0 + gamma) * GMPlanet(k)/VLIGHT3
       vecmg1 = VECMG(R1Plant1(1,k))
       term_e = vecmg1 + DOTP(unit_K,R1Plant1(1,k))
       vecmg2 = VECMG(R2Plant1(1,k))
       term_f = vecmg2 + DOTP(unit_K,R2Plant1(1,k))
!    Derivatives:
       dterm_e = Dotp(R1Plant1(1,k),dR1Plant1(1,k))/vecmg1 +
     .           Dotp(unit_K,dR1Plant1(1,k))
       dterm_f = Dotp(R2Plant1(1,k),dR2Plant1(1,k))/vecmg2 +
     .           Dotp(unit_K,dR2Plant1(1,k))
!
       delta_t_grav_Plan(k) = C_Plan(k) * DLOG(term_e / term_f)
       d_delta_t_grav_Plan(k) = C_Plan(k) * ( dterm_e/term_e -
     .                      dterm_f/term_f )                 !derivative
!
      IF(KTHED .ne. 0) Then               ! Debug
       write(6,13) k, C_Plan(k), vecmg1,term_e, dterm_e, vecmg2,
     .             term_f, dterm_f, delta_t_grav_Plan(k),
     .             d_delta_t_grav_Plan(k)
  13   format(" k,C_Plan: ",i5,d25.16,/," vecmg1,term_e,dterm_e: ",
     .        3d25.16,/," vecmg2,term_f,dterm_f: ",3d25.16,/,
     .        " delta_t_grav_Plan, d_delta_t_grav_Plan: ",2d25.16,/)
      Endif                               ! Debug
!
      Enddo           ! Planet loop
!
!******************************************************************************
!  Step 4: Use Equation 4 to find the differential delay due to the Earth.
!
       C_Earth = (1.0D0 + gamma) * GMEarth/VLIGHT3
!
       IF(Nzero .ne. 1) THEN
        vecmg1 = VECMG(x_sub1t1)
        term_g = vecmg1 + DOTP(unit_K,x_sub1t1)
        dterm_g = Dotp(x_sub1t1,w_sub1)/vecmg1 + DOTP(unit_K,w_sub1)
       ELSE
        vecmg1 =  0.0D0
        term_g =  2.0D0 * REARTH
        dterm_g = 0.0D0
       ENDIF
!
       IF(Nzero .ne. 2) THEN
        vecmg2 = VECMG(x_sub2t1)
        term_h = vecmg2 + DOTP(unit_K,x_sub2t1)
        dterm_h = Dotp(x_sub2t1,w_sub2)/vecmg2 + DOTP(unit_K,w_sub2)
       ELSE
        vecmg2 =  0.0D0
        term_h =  2.0D0 * REARTH
        dterm_h = 0.0D0
       ENDIF
!
       delta_t_grav_Earth = C_Earth * DLOG(term_g / term_h)
       d_delta_t_grav_Earth = C_Earth*( dterm_g/term_g -
     .                        dterm_h/term_h )               !derivative
!
      IF(KTHED .ne. 0) Then               ! Debug
       write(6,'(/,15x,"Step 4 dump:")')
       write(6,8)' C_Earth  ',C_Earth
       write(6,8)' term_g, dterm_g  ', term_g, dterm_g
       write(6,8)' term_h, dterm_h  ', term_h, dterm_h
       write(6,8)' delta_t_grav_Earth, d_delta_t_grav_Earth ',
     .             delta_t_grav_Earth,d_delta_t_grav_Earth
      Endif                               ! Debug
!
!******************************************************************************
!  Step 5: Add up all components from steps 3 and 4 to get the total
!          differential gravitational delay (equation 7).
!          [Does not include the term for observations close to the Sun.]
!
        delta_t_grav_Planets = 0.D0
        d_delta_t_grav_Planets = 0.D0
       do k=1,7
        delta_t_grav_Planets = delta_t_grav_Planets +
     .                          delta_t_grav_Plan(k)
        d_delta_t_grav_Planets = d_delta_t_grav_Planets +
     .                         d_delta_t_grav_Plan(k)
       enddo
!
       delta_t_grav = delta_t_grav_Sun + delta_t_grav_Moon
     .              + delta_t_grav_Planets + delta_t_grav_Earth
!  derivative
       d_delta_t_grav = d_delta_t_grav_Sun + d_delta_t_grav_Moon
     .              + d_delta_t_grav_Planets + d_delta_t_grav_Earth
!
      IF(KTHED .ne. 0) Then               ! Debug
       write(6,'(/,15x,"Step 5 dump:")')
       write(6,8)' delta_t_grav_Planets, d_delta_t_grav_Planets ',
     .             delta_t_grav_Planets, d_delta_t_grav_Planets
       write(6,8)' delta_t_grav, d_delta_t_grav ',
     .             delta_t_grav, d_delta_t_grav
      Endif                               ! Debug
!
!******************************************************************************
!  Step 6: Add the total differential gravitational delay to the rest of the
!          a priori vacuum delay, Equation 9.
!
!  Find Sun's potential:
!
       U_Sun  = GMSUN/VLIGHT2
       vecmg1 = VECMG(R_Earth_Sun)
       U      = U_Sun/vecmg1
!   Derivative:
       dU = -U_Sun * Dotp(R_Earth_Sun,V_Earth_Sun) / vecmg1**3
!
!   Compute individual terms of Eqn. 9
!
!    Check to see if the user wants to use gravitational bending.
      If(KRELC .eq. 0) Then      ! Default, use bending
!       Also add in term for observations close to the Sun
       term1 = delta_t_grav + Sunplus(1)
       dterm1 = d_delta_t_grav + Sunplus(2)
      Else                       ! Don't use bending
       term1 = 0.D0
       dterm1 = 0.D0
      Endif
!
       term2a = KdotB/VLIGHT
       dterm2a = dKdotB/VLIGHT                  ! derivative
!
       term2b = 1.D0 - ((1.D0 + gamma) * U)
       dterm2b = -(1.D0 + gamma) * dU           ! derivative
!
       absVEarth = VECMG(VsubEarth)
       term2c = (absVEarth)**2 / (2.D0*VLIGHT2)
       dterm2c = Dotp(VsubEarth,AsubEarth) /  VLIGHT2        ! derivative
!
       term2d = DOTP(VsubEarth,w_sub2) / VLIGHT2
       dterm2d = (DOTP(AsubEarth,w_sub2) + DOTP(VsubEarth,a_sub2))
     .            / VLIGHT2                                  ! derivative
!
!  Combine terms 2b,2c,2d
       term2bcd = term2b - term2c - term2d
       dterm2bcd = dterm2b - dterm2c - dterm2d               ! derivative
!
       term2  = term2a * term2bcd
       dterm2  = term2a * dterm2bcd + dterm2a * term2bcd     ! derivative
!
       VdotB  = DOTP(VsubEarth,b_nought)
       dVdotB = DOTP(AsubEarth,b_nought) + DOTP(VsubEarth,db_nought) !derivative
!
       KdotV  = DOTP(unit_K,VsubEarth)
       dKdotV = DOTP(unit_K,AsubEarth)                       ! derivative
!
       term3a = VdotB/VLIGHT2
       term3b = 1.D0 + KdotV/(2.D0*VLIGHT)
       term3  = term3a * term3b
!
       dterm3a = dVdotB/VLIGHT2                              ! derivative
       dterm3b = dKdotV/(2.D0*VLIGHT)                        ! derivative
       dterm3  = dterm3a*term3b +  term3a*dterm3b            ! derivative
!
       call VECAD(VsubEarth,w_sub2,vec_sum)
       call VECAD(AsubEarth,a_sub2,dvec_sum)                 ! derivative
       term4  = 1.D0 + DOTP(unit_K,vec_sum)/VLIGHT
       dterm4  = DOTP(unit_K,dvec_sum)/VLIGHT                ! derivative
!
       term123  = term1  - term2  - term3
       dterm123 = dterm1 - dterm2 - dterm3                   ! derivative
!
       tv2_tv1 = term123 / term4
       dtv2_tv1 = dterm123 / term4  -
     .            term123 * dterm4 / term4**2                ! derivative
!
      IF(KTHED .ne. 0) Then               ! Debug
       write(6,'(/,15x,"Step 6 dump:")')
       write(6,8)' U_Sun, U, dU ',U_Sun, U, dU
       write(6,8)' term1, dterm1  ',term1, dterm1
       write(6,8)' term2a, dterm2a  ',term2a, dterm2a
       write(6,8)' term2b, dterm2b  ',term2b, dterm2b
       write(6,8)' absVEarth',absVEarth
       write(6,8)' term2c, dterm2c  ',term2c, dterm2c
       write(6,8)' term2d, dterm2d  ',term2d, dterm2d
       write(6,8)' term2bcd, dterm2bcd  ',term2bcd, dterm2bcd
       write(6,8)' term2, dterm2  ',term2, dterm2
       write(6,8)' VdotB, dVdotB  ',VdotB, dVdotB
       write(6,8)' KdotV, dKdotV  ',KdotV, dKdotV
       write(6,8)' term3a, dterm3a  ',term3a, dterm3a
       write(6,8)' term3b, dterm3b  ',term3b, dterm3b
       write(6,8)' term3, dterm3  ',term3, dterm3
       write(6,8)' vec_sum, dvec_sum ',vec_sum, dvec_sum
       write(6,8)' term4, dterm4  ',term4, dterm4
       write(6,8)' term123, dterm123  ',term123, dterm123
       write(6,8)' tv2_tv1, dtv2_tv1 ',tv2_tv1, dtv2_tv1
      Endif                               ! Debug
!
!******************************************************************************
!  Step 7: Calculate the aberrated source vectors for use in the tropospheric
!          propogation delay calculation.
!
!      [Note: These calculations are in the atmosphere module, subroutine
!      ATMG, and do not need to be done here. The aberrated source vectors are
!      used in the atmosphere module to compute the topocentric azimuths and
!      elevations, and in the axis offset module (along with refraction) to
!      compute the vector axis offset.]
!
!     call vecad(VsubEarth,w_sub1,V_w1)
!     K_V_w1 = Dotp(unit_K,V_w1)
!
!     call vecad(VsubEarth,w_sub2,V_w2)
!     K_V_w2 = Dotp(unit_K,V_w2)
!
!     Do l=1,3
!       k_sub1(l) = unit_K(l) + (VsubEarth(l) + w_sub1(l)) / VLIGHT
!    .            - unit_K(l) * K_V_w1 /VLIGHT
!       k_sub2(l) = unit_K(l) + (VsubEarth(l) + w_sub2(l)) / VLIGHT
!    .            - unit_K(l) * K_V_w2 /VLIGHT
!     Enddo
!
!     IF(KTHED .ne. 0) Then               ! Debug
!      write(6,'(/,15x,"Step 7 dump:")')
!      write(6,8)' V_w1, K_V_w1  ',V_w1, K_V_w1
!      write(6,8)' V_w2, K_V_w2  ',V_w2, K_V_w2
!      write(6,8)' k_sub1, k_sub2  ',k_sub1, k_sub2
!     Endif                               ! Debug
!
!******************************************************************************
!  Step 8: Add the geometric part of the tropospheric propogation delay to the
!          vacuum delay.
!          [Geocenter station: DATMC(Nzero,1) and DATMC(Nzero,2) should
!          already be zero.]
!
      call vecsb(w_sub2,w_sub1,w2_w1)
      call vecsb(a_sub2,a_sub1,a2_a1)
      K_dotw2w1 = Dotp(unit_K,w2_w1)
      dK_dotw2w1 = Dotp(unit_K,a2_a1)           ! derivative
!
!  Apparent error in earlier versions of following code because station 1
!   atmosphere is negative. Error only at the .01 picosecond level though.
      tg2_tg1 = tv2_tv1 -  DATMC(1,1) * K_dotw2w1/VLIGHT
      dtg2_tg1 = dtv2_tv1 -  DATMC(1,2) * K_dotw2w1/VLIGHT
     .                    -  DATMC(1,1) * dK_dotw2w1/VLIGHT       ! derivative
!
!
      IF(KTHED .ne. 0) Then               ! Debug
       write(6,'(/,15x,"Step 8 dump:")')
       write(6,8)' w2_w1, a2_a1   ',w2_w1, a2_a1
       write(6,8)' K_dotw2w1, dK_dotw2w1 ', K_dotw2w1, dK_dotw2w1
       write(6,8)' DATMC  ', DATMC
       write(6,8)' tg2_tg1, dtg2_tg1 ', tg2_tg1, dtg2_tg1
      Endif                               ! Debug
!
!******************************************************************************
!  Step 9: Step 9 is to compute the total delay by adding in the "Best"
!          estimate of the troposphere propogation delay for each site.
!          Normally this will be done 'on the fly' in program SOLVE.
!          However, we add the Niell atmosphere term here if the atmosphere
!          flag, KATMC is 1. The default is not to add it.
!           [Geocenter station: DATMC(Nzero,1) and DATMC(Nzero,2) should
!            already be zero.]
!
      IF(KATMC.eq.1) then
         tg2_tg1  =  tg2_tg1 + DATMC(1,1) + DATMC(2,1)
         dtg2_tg1 = dtg2_tg1 + DATMC(1,2) + DATMC(2,2)
!
       IF(KTHED .ne. 0) Then               ! Debug
        write(6,'(/,15x,"Step 9 dump:")')
        write(6,8)'(KATMC=1:) tg2_tg1, dtg2_tg1 ', tg2_tg1, dtg2_tg1
       Endif                               ! Debug
!
      Endif
!
!******************************************************************************
!  Step 10: Correct for axis offset:
!
!      [ These computations are made in the axis offset module. ]
!
!******************************************************************************
!  Step Gamma: Take the partial derivatives of the delay (tg2_tg1) and the
!              delay rate (dtg2_tg1) with respect to Gamma.
!              [Does not include the term for observing close to the Sun.]
!
      Con_part(1) = ( delta_t_grav/(1.d0+Gamma) + U*term2a ) / term4
      Con_part(2) =
     .   ( ( d_delta_t_grav/(1.d0+Gamma) + U*dterm2a + term2a*dU )
     .     / term4 ) -
     .      (dterm4/term4**2) *
     .     ( delta_t_grav/(1.d0+Gamma) + U*term2a )
!
!  Take partial derivative of the gravitational bending part w.r.t. Gamma.
!    [Does not include the term for observing close to the Sun.]
      Bend_par(1) = ( delta_t_grav/(1.d0+Gamma) ) / term4
      Bend_par(2) = ( d_delta_t_grav/(1.d0+Gamma) ) / term4  -
     .      (dterm4/term4**2) * ( delta_t_grav/(1.d0+Gamma) )
!
      IF(KTHED .ne. 0) Then               ! Debug
       write(6,'(/,15x,"Step Gamma dump:")')
       write(6,8)' Con_part   ', Con_part
       write(6,8)' Bend_par  ', Bend_par
      Endif                               ! Debug
!
!  Cleanup Step: Finish computation of components for PUT's into data base
!
!   Total light bending portion:
      CON_CNTRB(1) = (   delta_t_grav + Sunplus(1) ) / term4
      CON_CNTRB(2) = ( d_delta_t_grav + Sunplus(2) ) / term4 -
     .        dterm4 * ( delta_t_grav + Sunplus(1) ) / term4**2
!      write(6,8)' delta_t_grav, d_delta_t_grav ',
!    .             delta_t_grav, d_delta_t_grav
!      write(6,8)' CON_CNTRB ', CON_CNTRB
!
!   Solar light bending portion:
      Sun_cntrb(1) = (   delta_t_grav_Sun + Sunplus(1) ) / term4
      Sun_cntrb(2) = ( d_delta_t_grav_Sun + Sunplus(2) ) / term4 -
     .        dterm4 * ( delta_t_grav_Sun + Sunplus(1) ) / term4**2
!      write(6,8)' delta_t_grav_Sun, d_delta_t_grav_Sun ',
!    .             delta_t_grav_Sun, d_delta_t_grav_Sun
!      write(6,8)' SUN_CNTRB ', SUN_CNTRB
!
!   Close to the Sun additional light bending portion:
!      write(6,8)' Sunplus   ',Sunplus
      Sunplus(1) =  Sunplus(1)/term4
      Sunplus(2) =  Sunplus(2)/term4 - dterm4*Sunplus(1)/term4**2
!      write(6,8)' Sunplus   ',Sunplus
!
!--------------------------------------------------------------------------
!
    8 FORMAT(A,4D25.16/(7X,5D25.16))
!
      RETURN
      END
