      SUBROUTINE TOCUP()
      IMPLICIT None
!
! 1.1.1 TOCUP calls the 'A' sections of the CALC modules which provide
!       for entries into the database table of contents. TOCUP is called
!       once per data base.
!
! 1.2   TOCUP PROGRAM INTERFACE
!
! 1.2.1 CALLING SEQUENCE -
!             INPUT VARIABLES: NONE
!             OUTPUT VARIABLES: NONE
!
! 1.2.2 COMMON BLOCKS USED:
      INCLUDE 'cuser.i'
!           1. Calc_user - Analysis center or Correlator user
!
! 1.2.6 SUBROUTINE INTERFACE -
!             CALLER SUBROUTINES: MAIN
!             CALLED SUBROUTINES: ATMA, AXOA, ETDA, PTDA, NUTA, OCEA,
!                                 PREA, SITA, STRA, UT1A, WOBA, ATIMA,
!                                 CTIMA, PEPA, THERA, STAA, PLXA,
!                                 DIRNA, M2000A,
!
! 1.2.7 CONSTANTS USED - NONE
!
! 1.2.8 PROGRAM VARIABLES - None
!
! 1.2.9 PROGRAMMER - DALE MARKHAM  01/12/77
!                    PETER DENATALE 07/26/77
!                    BRUCE SCHUPLER 09/14/78
!                    BRUCE SCHUPLER 01/08/80
!                    BRUCE SCHUPLER 08/26/80
!                    CHOPO MA AND DAVID GORDON 04/09/84
!                    DAVID GORDON   07/12/84  (POLE TIDE)
!                    SAVITA GOEL    06/03/87  (CDS FOR A900)
!                    GREGG COOKE    12/21/88
!                    GREGG COOKE    05/22/89
!                    Jim Ryan       89.07.26 Documentation simplified.
!                    Jim Ryan       89.10.08 Call to RELA removed.
!                    N.Zacharias/D.Gordon 93.10.07 Call to DIRNA
!                                          (equation of equinox)
!                    D. Gordon 94.04.14 Changed to Implicit None
!                    D. Gordon 96.04.02 Added 'Call M2000A', subroutine to do
!                              the ADD for the crust fixed to J2000 rotation
!                              matrix L-code, 'CF2J2000'.
!                    D. Gordon 98.07.23 Removed ASK for 'PEP TAPE'; removed
!                              unnecessary variables, corrected documentation.
!                    D. Gordon 98.11.12 Removed 'CALL PANA'. Feedbox
!                              rotation module merged into axis offset module.
!                    D. Gordon 98.11.17 Added subroutine UVA, for
!                              calculation of missing U,V coordinates.
!                    Jim Ryan  Sept 2002 Integer*2 mods.
!                    D. Gordon 2003. Added subroutine DELXX, to put all
!                              database access code deletes.
!
!     TOCUP Program Structure.
!
!   Provide for the entries to the table of contents for the text messages from
!   the model modules and for the necessary utility routines. Also pass the
!   frequency and the PEP information to the necessary programs.
!
      CALL ATMA()
      CALL AXOA()
      CALL ETDA()
      CALL PTDA()
      CALL NUTA()
      CALL OCEA()
      CALL PREA()
      CALL SITA()
      CALL STRA()
      CALL UT1A()
      CALL WOBA()
      CALL ATIMA()
      CALL CTIMA()
      CALL PEPA()
      CALL THERA()
      CALL STAA()
      CALL PLXA()
      CALL DIRNA()
      CALL M2000A()
      CALL UVA()
!
      IF (Calc_user .eq. 'A') THEN
       CALL DELXX()
      ENDIF
!
      END
!*******************************************************************
      SUBROUTINE DELXX()
      Implicit None
!
!     DELXX was created to consolidate most of the access code
!      deletions.
!
!    Programmer:
!      2004.04.30 D. Gordon  Subroutine created.
!
!  Deletes for old Chau Lcodes, from ATMA).
      CALL DELR ( int2(2), 'ATM PART')
      Call DELR ( int2(2), 'WET PART')
      CALL DELR ( int2(2), 'ATM CONT')
!
!   Delete for old axis offset correction.
      CALL DELR ( int2(2), 'AXIS OLD')
!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!   Temporary addition. Remove some Lcodes.
!      CALL DELI( int2(1), 'PRT FLAG')
!      CALL DELI( int2(1), 'OBCLFLGS')
!      CALL DELI( int2(1), 'OBCLLIST')
!
       CALL DELR( int2(2), 'CFA22DRY')
       CALL DELR( int2(2), 'CFA22WET')
       CALL DELR( int2(2), 'CFA PART')
       CALL DELR( int2(2), 'LANYIDRY')
       CALL DELR( int2(2), 'LANYIWET')
       CALL DELR( int2(2), 'LANYINFO')
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!
      RETURN
      END
