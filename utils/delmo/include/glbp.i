!@This is the start of file &glbp.i
!
!   Flags and their meanings:
!
!   Last update: 04-MAR-99 17:40:56
!
!   NB: when change this block one should trace its actual size. It
!       should correspond to JGLBP_BLOCKS from solve.i
!
!     modifications
!
!  pet 01-JAN-99  -- created
!
      INTEGER*4  L__GPA
      PARAMETER  ( L__GPA = 20 ) ! length of one string in C_GPA (bytes)
!
      TYPE      GLB_MEM__STRU
          INTEGER*4     FIRST_FIELD
!C
          INTEGER*4     NPAR_CGM      ! max expected number of global parameters
          INTEGER*4     ADR_CGM_BLO   ! Address of CGM block
          INTEGER*4     ADR_CGV       ! Address of CGV
          INTEGER*4     ADR_CGM       ! Address of CGM
          INTEGER*4     ADR_C_GPA     ! Address of the list of paramters
          INTEGER*4     ADR_GLO_SOCOM ! Address of global socom area
          INTEGER*4     ADR_GLO_PRFIL ! Address of global parfil area
          INTEGER*4     ADR_GLO       ! Address of the block of memory for the
!                                     !   global parts of arrays
          INTEGER*4     LEN_GLO       ! Number of bytes alloted for global
!                                     !   parts of arrays
!
          INTEGER*4     ADR_NORMARR2  ! Address of the arrays for normal eq.-2
          INTEGER*4     ADR_NORMARR   ! Address of the arrays for normal eq.-1
          INTEGER*4     ADR_LOC       ! Address of the block of memory for the
!                                     !   local parts of arrays
          INTEGER*4     LEN_LOC       ! Number of bytes alloted for local
!                                     !   parts of arrays
!
          INTEGER*4     L_GPA         ! Current number of parameters in CGM
          INTEGER*4     L_ARC         ! Total number of local + global
!                                     ! parameters in this session (arc)
!C
          INTEGER*4     LAST_FIELD
      END TYPE  GLB_MEM__STRU  ! GLB_MEM__STRU  !
!
