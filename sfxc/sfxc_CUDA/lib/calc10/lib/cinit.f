      SUBROUTINE INITL ( KOUNT )
      IMPLICIT None
!
! 1.    INITL
!
! 1.1   INITL PROGRAM SPECIFICATION
!
! 1.1.1 INITL obtains the mathematical and physical constants from the database
!       and elsewhere and loads them into the common block 'cphys' for their
!       use throughout the program. INITL also calls the input and
!       initialization sections of the model modules and the necessary utility
!       routines. Each section will obtain internally the model module parameter
!       values from the database and initialize all such variables in the local
!       common block. Each section will also put into the header a text message
!       for each model module and necessary utility routine. INITL also
!       initializes a counter which counts the observation number and writes a
!       header text for the observation number, time tag, baseline
!       identification, and source identification.
!       SUBROUTINE INITL IS CALLED ONLY ONCE PER DATA BASE.
!
! 1.2   INITL PROGRAM INTERFACE
!
! 1.2.1 CALLING SEQUENCE -
!             OUTPUT VARIABLES:
!               1. KOUNT - THE VARIABLE WHICH INITIALIZES THE COUNTER
!                          OF THE OBSERVATION ITEMS TO ZERO. (UNITLESS)
!
! 1.2.2 COMMON BLOCKS USED -
!
      INCLUDE 'cphys.i'
!            VARIABLES 'TO':
!              2. EFLAT   - THE FLATTENNING OF THE ELLIPSOID WHICH APPROXIMATES
!                           THE SHAPE OF THE EARTH. (UNITLESS)  (Site module)
!              3. GMMOON  - THE MASS OF THE MOON MULTIPLIED BY THE NEWTONIAN
!                           GRAVITATIONAL CONSTANT. (M**3/SEC**2)
!              4. GMSUN   - THE MASS OF THE SUN MULTIPLIED BY THE NEWTONIAN
!                           GRAVITATIONAL CONSTANT. (M**3/SEC**2)
!              5. GMEARTH - THE MASS OF THE EARTH MULTIPLIED BY THE NEWTONIAN
!                           GRAVITATIONAL CONSTANT. (M**3/SEC**2)
!              6. REARTH  - THE EQUATORIAL RADIUS OF THE EARTH. (M)
!              7. SECPAU  - THE NUMBER OF LIGHT-SECONDS PER ASTRONOMICAL UNIT.
!                           (SEC/A.U.)
!              8. GAMMA   - THE POST NEWTONIAN EXPANSION PARAMETER WHICH AFFECTS
!                           LIGHT BENDING. (1.0 FOR EINSTEIN).
!              9. VLIGHT  - THE VELOCITY OF LIGHT IN VACUUM. (M/SEC)
!             10. VLIGHT2 - THE VELOCITY OF LIGHT SQUARED. ((M/SEC)**2)
!             11. VLIGHT3 - THE VELOCITY OF LIGHT CUBED. ((M/SEC)**3)
!             12. GMPLANET(7)-The masses of 7 of the planets multiplied by the
!                           gravitational constant. (1=Mercury, 2=Venus, 3=Mars,
!                           4=Jupiter, 5=Saturn, 6=Uranus, and 7=Neptune)
!             13. AU_meters-The Astronomical unit. (meters)
!
      INCLUDE 'ccon.i'
!       Variables 'from':
!              1. ILUOUT  - Output control flag.
!
! 1.2.3 PROGRAM SPECIFICATIONS -
      Integer*2  KERR(12), NDO(3), N, NN, idm1
      Integer*4  KOUNT
!
! 1.2.4 DATA BASE ACCESS -
!
!            'GET' VARIABLES:
!              1. EFLAT, GMMOON, GMSUN, REARTH, SECPAU, VLIGHT, GAMMA
!                               - THE PHYSICAL CONSTANTS.
!
!            ACCESS CODES:
!              1. 'VLIGHT  ' - THE DATABASE ACCESS CODE FOR THE VELOCITY OF
!                              LIGHT IN A VACUUM.
!              3. 'GMSUN   ' - THE DATABASE ACCESS CODE FOR THE MASS OF THE
!                              SUN MULTIPLIED BY THE NEWTONIAN GRAVITATIONAL
!                              CONSTANT.
!              4. 'GMMOON  ' - THE DATABASE ACCESS CODE FOR THE MASS OF THE
!                              MOON MULTIPLIED BY THE NEWTONIAN GRAVITATIONAL
!                              CONSTANT.
!              5. 'GMEARTH ' - THE DATABASE ACCESS CODE FOR THE MASS OF THE
!                              EARTH MULTIPLIED BY THE NEWTONIAN GRAVITATIONAL
!                              CONSTANT.
!              6. 'TSEC/AU ' - THE DATABASE ACCESS CODE FOR THE NUMBER OF
!                              LIGHT-SECONDS PER ASTRONOMICAL UNIT.
!              7. 'EARTHRAD' - THE DATABASE ACCESS CODE FOR THE EQUATORIAL
!                              RADIUS OF THE EARTH.
!              8. 'E-FLAT  ' - THE DATABASE ACCESS CODE FOR THE SQUARE OF THE
!                              ECCENTRICITY OF THE ELLIPSOID WHICH APPROXIMATES
!                              THE SHAPE OF THE EARTH.
!              9. 'REL DATA' - THE DATABASE ACCESS CODE FOR THE POST NEWTONIAN
!                              EXPANSION PARAMETER.
!
! 1.2.5 EXTERNAL INPUT/OUTPUT -
!            OUTPUT VARIABLES:
!              1. THE HEADER TEXT FOR THE OBSERVATION ITEM NUMBER, TIME TAG,
!                 BASELINE IDENTIFICATION, AND SOURCE IDENTIFICATION.
!              2. POSSIBLE ERROR OUTPUT
!
! 1.2.6 SUBROUTINE INTERFACE -
!             CALLER SUBROUTINES: MAIN
!             CALLED SUBROUTINES: ATIMI, ATMI, AXOI, ETDI, PTDI,
!                                 GET4, TERMINATE_CALC, MVREC, NUTI, OCEI,
!                                 PEPI, PREI, RELI, SITI, STRI, THERI,
!                                 UT1I, WOBI, WRIDR, CTIMI, STAI, PLXI
!
! 1.2.8 PROGRAM VARIABLES -
!           1. KERR(3) - THE DATA BASE ERROR RETURN FLAGS.
!           2. NDO(3)  - THE DATA BASE RETURN ARRAY INDICES.
!
! 1.2.9 PROGRAMMER - DALE MARKHAM  01/12/77
!                    PETER DENATALE 07/27/77
!                    BRUCE SCHUPLER 03/08/78
!                    BRUCE SCHUPLER 09/18/78
!                    BRUCE SCHUPLER 08/26/80
!                    CHOPO MA AND DAVID GORDON 04/09/84
!                    DAVID GORDON   07/13/84   (POLE TIDE)
!                    SAVITA GOEL    06/03/87   (CDS FOR A900)
!                    GREGG COOKE    12/21/88
!                    GREGG COOKE    05/22/89
!                    Jim Ryan 89.07.25 Documentation simplified.
!                    Jim Ryan 89.10.06 CPHYS common made an include file and
!                             GAMMA added to the list. GAMMA now pulled here.
!                    Jim Ryan 89.10.08 Call to RELI removed.
!                    Jim Ryan 89.12.12 UNIX-like database interface
!                             implimented.
!                    David Gordon 93/04/27 GMEARTH added to cphys.i and
!                             defined here.
!                    David Gordon 93/05/06 VLIGHT2 and VLIGHT3 added to
!                             cphys.i and defined here.
!                    David Gordon 93/10/19 GMPLANET(7) added (G times mass of
!                             each planet except Earth and Pluto).
!                    David Gordon 94/04/12 Removed GET4'ing of GAUSS, EMS/MMS,
!                             and U-GRV-CN.
!                    David Gordon 94.04.14 Changed to Implicit None.
!                    David Gordon 94.06.08 Corrected format statement, single
!                                 and double quotes in wrong order.
!                    David Gordon 95.05.02 Adding AU_meters, Astronomical unit.
!                    David Gordon 96.01.23 Change to take GMEARTH from the
!                                 database. Removal of old L-codes. Cleanup of
!                                 documentation.
!                    David Gordon 98.10.13 Adjusted observation header line
!                                 for change to a 4-digit year.
!                    David Gordon 98.10.14 Removed ACCGRV and its 'GET'
!                                 Also removed ACCGRV from cphys.i.
!                    David Gordon 98.11.12 Removed 'CALL PANI'. Feedbox
!                                 rotation module merged into axis offset
!                                 module.
!                    Jim Ryan Sept2002 Integer*4 mods.
!                    Jim Ryan 03.03.10 Kill replaced with terminate_solve
!                    David Gordon 2004.07.28 Updated Saturn and Uranus masses.
!
!   INITL Program Structure.
!
!   Call MVREC to make the header record available and check for db error.
      CALL MVREC ( int2(1), int2(1), int2(1), KERR (1))
      IF (KERR(1) .NE. 0) CALL TERMINATE_CALC('INITL ',int2(1),KERR(1))
!
! Physical constants from IERS Conventions (2003)
          VLIGHT  = 299792458.0D0
          GMSUN   = 1.32712442076D20
          SECPAU  = 499.0047838061D0
          REARTH  = 6378136.6D0
          GMEARTH = 3.986004418D14
          GMMOON  = GMEARTH * .0123000383D0
          EFLAT   = 1.D0/298.25642D0
          GAMMA   = 1.0D0
          AU_meters = 149597870691.D0
!
!  Compute GM of planets. Reciprocal solar mass units from IERS Technical
!   Note 13. 93OCT19, D. Gordon
!   Saturn and Uranus updated to agree with DE405, 2004JUL28, D. Gordon
      GMPLANET(1) = GMSUN / 6023600.D0          ! Mercury
      GMPLANET(2) = GMSUN /  408523.71D0        ! Venus
      GMPLANET(3) = GMSUN / 3098708.D0          ! Mars
      GMPLANET(4) = GMSUN /    1047.3486D0      ! Jupiter
      GMPLANET(5) = GMSUN /    3497.898D0       ! Saturn
      GMPLANET(6) = GMSUN /   22902.98D0        ! Uranus
      GMPLANET(7) = GMSUN /   19412.24D0        ! Neptune
!  Compute square and cube of velocity of light. 93MAY06, D. Gordon
      VLIGHT2 = VLIGHT * VLIGHT
      VLIGHT3 = VLIGHT2 * VLIGHT
!
!  Provide for the input and initializations of the model modules and of the
!  necessary utility routines and for the adding to the header of the
!  corresponding text messages.
!
      CALL STAI()
      CALL ATMI()
      CALL AXOI()
      CALL ETDI()
      CALL PTDI()
      CALL NUTI()
      CALL OCEI()
      CALL PREI()
      CALL SITI()
      CALL STRI()
      CALL UT1I()
      CALL WOBI()
      CALL ATIMI()
      CALL CTIMI()
      CALL PEPI()
      CALL THERI()
      CALL PLXI()
!
!  Write the header record to the output database.
      CALL WRIDR()
!
!  Initialize the observation counter to 0.
      KOUNT = 0
!
!  Write the observation banner.
      If(ILUOUT.ne.-1)
     .  WRITE(6,'(/,12X,13("*"),"The observation header",
     .         12("*"),/" Number Year Mn Dy Hr Mn  Sec  ",
     .         "   Baseline       Source  ")')
!
!     Normal conclusion.
      RETURN
      END
