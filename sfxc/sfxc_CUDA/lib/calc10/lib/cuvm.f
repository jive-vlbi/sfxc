      SUBROUTINE UVA()
      IMPLICIT None
!
!       UVA adds entries to the table of contents for the U, V
!       coordinates Lcode.
!
      INCLUDE 'ccon.i'
!        VARIABLES 'FROM'
!          1. KASTC - The Dave Shaffer switch to turn on the computation
!                     of (U,V) coordinates. (Logic reversed 2001.01.12)
!                      = 0 ==> Switched ON. (default)
!                      = 1 ==> Switched OFF
!
      INCLUDE 'cuser.i'
!        VARIABLES 'FROM'
!          1. Calc_user - 'A' for analysis centers, 'C' for correlators.
!
!       DATA BASE ACCESS -
!         ACCESS CODES ADDED:
!          1. 'UVF/ASEC' -  The data base access code for the
!                           U, V coordinates.
!          2. 'UVF/MHz ' -  The correlator code for the
!                           U, V coordinates in fringes per arcsec,
!                           per MHz.
!   [new]  3. 'UVW/m   ' -  The correlator code for UVW & UVWdot
!                           in [m,m/s] - rmc 26okt-6nov2006
!
! 1.2.6 SUBROUTINE INTERFACE -
!             CALLER SUBROUTINES: TOCUP
!             CALLED SUBROUTINES: ADDR
!
!       PROGRAMMER:
!             David Gordon 1998.11.17 Subroutine created
!             David Gordon 2001.01.12 Logic reversed, default is to
!                          compute and add U/V coordinates, since the
!                          Mark IV correlators are not computing them.
!                          New access code ('UVF/MHz ', U/V coordinates in
!                          fringes per arcsec per MHz) added for Mark IV
!                          correlator usage.
!             Jim Ryan Sept 2002 Integer*2/4 mods.
!             Jim Ryan     03.03.10 Kill replaced with terminate_solve
!
!     UVA PROGRAM STRUCTURE
!
!   ADD for U,V coordinates
!     modified for Calc_user.eq.C for UVW[m,m/s] output - rmc 26okt06
!     passed int2(k) numbers not actually used by our ../C/addr.c (?)
      If (KASTC .eq. 0) Then
       If (Calc_user .eq. 'A') CALL ADDR (int2(2),'UVF/ASEC',
     .  'U,V in FR per arcsec, from CALC ',int2(2),int2(1),int2(1))
       If (Calc_user .eq. 'C') CALL ADDR (int2(2),'UVF/MHz ',
     .  'U,V in FR per arcsec per MHz    ',int2(2),int2(1),int2(1))
       If (Calc_user .eq. 'C') CALL ADDR (int2(2),'UVW/m   ',
     .  'U,V,W in m                      ',int2(3),int2(2),int2(1))
      Endif
!
      RETURN
      END
!*************************************************************************
      SUBROUTINE UVG (STAR, EPBASE )
      IMPLICIT None
!
!       UVG computes the (U,V) coordinates of the baseline.
!
!      CALLING SEQUENCE -
!        INPUT VARIABLES:
!          1. STAR(3)     - The J2000.0 Source unit vector.
!          2. EPBASE(3,2) - The J2000.0 Geocentric baseline position and
!                           velocity vectors. (m, m/sec)
!
!   COMMON BLOCKS USED -
!
      INCLUDE 'ccon.i'
!        VARIABLES 'FROM':
!          1. KASTC - The Dave Shaffer switch to turn on the computation
!                     of (U,V) coordinates. (Logic reversed 2001.01.12)
!                      = 0 ==> Switched ON. (default)
!                      = 1 ==> Switched OFF
!
      INCLUDE 'cphys.i'
!        VARIABLES 'FROM':
!          1. VLIGHT - The velocity of light in a vacuum (m/sec).
!
      INCLUDE 'cuser.i'
!        VARIABLES 'FROM'
!          1. Calc_user - 'A' for analysis centers, 'C' for correlators.
!
!       DATA BASE ACCESS -
!         ACCESS CODES ADDED:
!          1. 'UVF/ASEC' -  The data base access code for the
!                           U, V coordinates in fringes per arcsec.
!          2. 'UVF/MHz ' -  The correlator code for the
!                           U, V coordinates in fringes per arcsec,
!                           per MHz.
!   [new]  3. 'UVW/m   ' -  The correlator code for UVW & UVWdot
!                           in [m, m/s] - rmc 26okt-6nov2006
!
      Real*8         PI,TWOPI,HALFPI,CONVD,CONVDS,CONVHS,SECDAY
      COMMON /CMATH/ PI,TWOPI,HALFPI,CONVD,CONVDS,CONVHS,SECDAY
!        VARIABLES 'FROM':
!          1. CONVDS - THE CONVERSION FACTOR FROM ARCSECONDS TO RADIANS
!                      (RAD/ARCSECOND)
!
      Real*8 STAR(3), EPBASE(3,2)
      Real*8 DOTP, REF_FREQ, B(3), NCP(3), vectr(3), Bpr(3), NCPpr(3),
     .       U_V(2), VECMG, UVW(3,2), BX(3),BXpr(3),BV(3),BVpr(3)
      Integer*2 KERR, NDO(3)
!
!        Program variables:
!          1. U_V(2) -   Baseline coordinates in the (U,V) plane, in
!                        units of fringes per arcsec. [Multiply by
!                        206264.81 to get the more conventional values.]
!                        Scaled by the value of REF_FREQ.
!   [new]  1. UVW(3,2) - Baseline coordinates in (U,V,W) & (U,V,W)dot, in 
!                        units of [m, m/s] - rmc 26okt-6nov2006
!          2. REF_FREQ - For databases, should be the correct reference
!                        frequency. For correlator usage, set to 1.0 MHz,
!                        should be multiplied later by the correct
!                        frequency or frequencies.
!
!       PROGRAMMER:
!             David Gordon 1998.11.17 Subroutine created
!             David Gordon 2001.01.12 Logic reversed, default is to
!                          compute and add U/V coordinates, since the
!                          Mark IV correlators are not computing them.
!                          New access code ('UVF/MHz ', U/V coordinates in
!                          fringes per arcsec per MHz) added for Mark IV
!                          correlator usage.
!             Jim Ryan Sept 2002 Integer*2/4 mods.
!
!       UVG PROGRAM STRUCTURE
!
      IF (KASTC .ne. 0) Go to 800
!
      If (Calc_user .eq. 'A') Then
!   Get the reference frequency.
       CALL GET4('REF FREQ      ', REF_FREQ, int2(1), int2(1),
     .      int2(1), NDO, KERR)
       IF(KERR.NE.0) then
         write(6,'("UVG: Failure to obtain ref frequency.")')
         CALL TERMINATE_CALC( 'UVG   ', int2(1), KERR)
       Endif
!    Convert from MHz to Hz.
       REF_FREQ = REF_FREQ*1.D6
      Endif
!
!  Set frequency to 1 MHz for correlators.
      If (Calc_user .eq. 'C') Then
       REF_FREQ = 1.D6
      Endif
!
!   Baseline vector 
!     keep BX in [m] & BV in [m/s] -- rmc 26okt-6nov2006
       B(1) = EPBASE(1,1) * REF_FREQ/VLIGHT*CONVDS
       B(2) = EPBASE(2,1) * REF_FREQ/VLIGHT*CONVDS
       B(3) = EPBASE(3,1) * REF_FREQ/VLIGHT*CONVDS
       BX(1) = EPBASE(1,1)
       BX(2) = EPBASE(2,1)
       BX(3) = EPBASE(3,1)
       BV(1) = EPBASE(1,2)
       BV(2) = EPBASE(2,2)
       BV(3) = EPBASE(3,2)
!   NCP unit vector
       NCP(1) = 0.D0
       NCP(2) = 0.D0
       NCP(3) = 1.D0
!
! Get component of baseline pos/vel vectors projected into the plane
!   perpendicular to the STAR vector:
        CALL CROSP (STAR,      B, vectr)
        CALL CROSP (vectr,  STAR,   Bpr)
        CALL CROSP (STAR,     BX, vectr)
        CALL CROSP (vectr,  STAR,  BXpr)
        CALL CROSP (STAR,     BV, vectr)
        CALL CROSP (vectr,  STAR,  BVpr)
! Get component of NCP vector projected into the plane perpendicular
!   to the STAR vector:
        CALL CROSP (STAR,   NCP, vectr)
        CALL CROSP (vectr, STAR, NCPpr)
! Convert to a unit vector
        CALL VUNIT (NCPpr, NCP)
!
        U_V(2) = DOTP (Bpr, NCP)
        UVW(2,1) = DOTP (BXpr,NCP)
        UVW(2,2) = DOTP (BVpr,NCP)
!
        CALL CROSP (Bpr, NCP, vectr)
        U_V(1) = VECMG (vectr)
        If (DOTP (STAR,vectr) .lt. 0.d0) U_V(1) = -U_V(1)
        CALL CROSP (BXpr, NCP, vectr)
        UVW(1,1) = VECMG (vectr)
        If (DOTP (STAR,vectr) .lt. 0.d0) UVW(1,1) = -UVW(1,1)
        CALL CROSP (BVpr, NCP, vectr)
        UVW(1,2) = VECMG (vectr)
        If (DOTP (STAR,vectr) .lt. 0.d0) UVW(1,2) = -UVW(1,2)
!
!  new for W (sign tested empirically to match corr.output) - rmc 26okt06
        UVW(3,1) = DOTP (BX,STAR)
        UVW(3,2) = DOTP (BV,STAR)
!
!  PUT the U,V coordinates in the database.
      If (Calc_user .eq. 'A') CALL PUT4 ('UVF/ASEC      ', U_V,
     .    int2(2), int2(1), int2(1))
      If (Calc_user .eq. 'C') CALL PUT4 ('UVF/MHz       ', U_V,
     .    int2(2), int2(1), int2(1))
!  PUT the U,V,W coordinates in the database. - rmc 6nov06
      If (Calc_user .eq. 'C') CALL PUT4 ('UVW/m         ', UVW,
     .    int2(3), int2(2), int2(1))
!
!     Normal conclusion.
  800 CONTINUE

!  Add debug output - rmc 26okt2006
  999 FORMAT (1x,'Debug output for subroutine UVG.')
  998 FORMAT (A,1p,3d25.16)
  997 FORMAT (1p,3d25.16,/,3d25.16)
      IF (KASTD .ne. 0) THEN
!        WRITE (6,999)
!        WRITE (6,998) 'UVW     ', U_V
        WRITE (6,997)  UVW
!        WRITE (6,998) 'Bsln L  ', VECMG(B)
!        WRITE (6,998) 'UVW L   ', VECMG(U_V)
      END IF

      RETURN
      END
