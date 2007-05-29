      SUBROUTINE CALC
      IMPLICIT None
!
!     Main Program specification
!
!     The CALC main merely serves as an executive which calls the subroutines
!     which do the real work. These are  OBSNT, AND DRIVR.
!     These routines are:
!
!     START - handles initialization of CALC.
!     TOCUP - provides for the entries to the header table of contents of the
!             text messages from the model modules and from the necessary
!             utility routines, places the frequency and PEP Tape flags into
!             the CALC mainstream, modifies the observation table of contents.
!     INITL - Obtains the math and physical constants, initializes the model
!             modules and the necessary utility routines, writes the header
!             record.
!     OBSNT - moves to and fetches the next observation.
!     DRIVR - performs the program calculations.
!     WRIDR - data base call which actually writes each record.
!
!     The program loops infinitely until START detects an END OF DATA in the
!     control data set. START, TOCUP, AND INITL are called once per database,
!     while OBSNT, DRIVR, and WRIDR are called once for each observation.
!
!     RESTRICTIONS - NONE
!
!     REFERENCES - NONE
!
!     MAIN PROGRAM INTERFACE
!
!       CALLING SEQUENCE - NONE
!
!     COMMON BLOCKS USED -
      INCLUDE 'inputs.i'
!            Variables 'to':
!              1. External_inputs   - Character*80 string containing the name
!                                     of the file which contains the external
!                                     file inputs (source position, site
!                                     position, etc. files)
!              2. External_aprioris - Logical variable controlling whether
!                                     external a priori inputs will be looked
!                                     for. If .FALSE., will not look for them.
!                                     If .TRUE., will look for them.
!
!     PROGRAM SPECIFICATIONS -
!
      CHARACTER*128 CALCON_NAME,luout_chr
      Integer*4 KOUNT, KEND, Iar
      Integer*2 ILUOUT, ILU
!
!     DATA BASE ACCESS - NONE
!
!     EXTERNAL INPUT/OUTPUT -
!
!     Run String variables:
!        1 - No current function.
!        2 - Terminal output control. 0 for normal, -1 to suppress output.
!        3 - The full path and name of the CALC input control file.
!
!     SUBROUTINE INTERFACE -
!             CALLER SUBROUTINES: NONE
!             CALLED SUBROUTINES: DRIVR, INITL, OBSNT, START,
!                                 TOCUP, WRIDR, RMPAR, RCPAR
!
!     CONSTANTS USED - NONE
!
! 1.2.8 PROGRAM VARIABLES -
!           1.  KEND   -  THE END OF DATA FLAG.  KEND = 0 IF THERE IS
!                         MORE DATA TO BE PROCESSED, KEND = 1 IF THE
!                         END OF THE DATA HAS BEEN REACHED.  (UNITLESS)
!           2.  KOUNT  -  THE FLAG WHICH INITIALIZES THE COUNTING OF
!                         THE OBSERVATION ITEMS.  (UNITLESS)
!           3. IMPAR(5)-  THE RMPAR PARAMETERS
!           4. ILUIN   -  THE USER'S LU. [Not used anywhere]
!           5. ILU     -  The message LU, set to 6. [Not used anywhere?]
!
! 1.2.9 PROGRAMMER - DALE MARKHAM   01/12/77
!                    PETER DENATALE 07/05/77
!                    BRUCE SCHUPLER 05/11/78
!                    BRUCE SCHUPLER 12/05/78
!                    BRUCE SCHUPLER 01/07/80
!                    DAVID GORDON   01/08/85  (ADDED IDISC)
!                    SAVITA GOEL    06/02/87  (CDS FOR A900)
!                    GREGG COOKE    12/21/88
!                    Jim Ryan  89.07.25 Documentation simplified.
!                    Jim Ryan  90.02.08 CALCON file logic improved and
!                                       input and crt units harded coded
!                                       to 1, except for suppressed output.
!      91.06.16  Jim Ryan - Bug in suppressing output fixed.
!      92.06.22  jwr - Use of rmpar replaced by rcpar for iluout.
!      93.10.13  DG - Pass ILU instead of 6 to subroutine START.
!      94.04.14  DG - Converted to Implicit none
!      98.03.05  DG - Removed RCPAR's and replaced with IGETARG's and IARGC.
!                     Adding external file input capabilities.
!      99.10.27  DG - Removing extraneous printouts.
!      2005.02.28 DG -Replaced IGETARG's with GETARG's. IGETARG not 
!                     supported by HP-UX Fortran 95.
!
!     MAIN Program Structure
!
        External_aprioris = .false.
!
!   Find number of parameters on the command line
       Iar = IARGC()
!     Find out the LU'S.
!      Call IGETARG(2,luout_chr,128)
       Call  GETARG(2,luout_chr    )
      read(luout_chr,*,end=10) iluout
  10  continue
!      Call IGETARG(3,CALCON_NAME,128)
       Call  GETARG(3,CALCON_NAME    )
      ILU = 6
!   Get control file name for list of external input files
       If(Iar .ge. 4) Then
!       Call IGETARG(4,External_inputs,80)
        Call  GETARG(4,External_inputs   )
        External_aprioris = .true.
       write(6,'("External_inputs ",1x,A80)') External_inputs
       Endif
!
!!      CALL DATSV()
!
!   Initialize CALC and obtain the frequency and database start/stop
!   information by peeking into the database.
   50 CONTINUE
      CALL START(ILU, CALCON_NAME, ILUOUT)
!
!   Provide for the header entries and for placing the frequency PEP Tape
!   information into the mainstream of CALC. Also provide for the observations.
      CALL TOCUP()
!
!   Provide for the initialization of the model modules and the necessary
!   utilities.
      CALL INITL(KOUNT)
!
!   Move to an observation and set the flag KEND if the end-of-file
!   has been reached.
  500 CALL OBSNT (KOUNT, KEND)
!
!   Conclude CALC normally if the end-of-data flag KEND has been set. Otherwise
!   subroutine DRIVR is called for the processing of another observation.
      IF ( KEND .EQ. 1 )  GO TO 800
!
!   Calculate geometry, partial derivatives,contributions, and the theoretical
!   delay and rate.
      CALL DRIVR()
!
!   Write the observation to the database.
      CALL WRIDR()
!
!   Return to subroutine OBSNT to get another observation.
      GO TO 500
!
!   The databases are closed in OBSNT so we continue without a FINIS.
  800 CONTINUE
!
!   Normal CALC termination.
      IF (ILUOUT.NE.-1) WRITE(6,"(' All data for this experiment has ',
     .          'been processed. ')")
!
!   Go to 50 to see if we want another database.
!   We use this funny GOTO to keep the compiler happy.
!99999 IF(1 .EQ. 1) GO TO 50
!      STOP
      RETURN
      END
