!
! >>>>> INCLUDE-BLOCK with description of data structures used for saving
!       equations of conditions in memory.
!
!       equmem.i 27-MAR-2000 09:48:16  --  28-MAR-2000 17:01:08
!
        TYPE      EQUMEM__STRU
            INTEGER*4  FIRST_FIELD
            INTEGER*4  NOBS
            INTEGER*4  LEN_TOTAL
            LOGICAL*4  USE_FLAG
!
            INTEGER*4  ADR_MEMOBS(MAX_OBS)
            INTEGER*4  LEN_MEMOBS(MAX_OBS)
!
            INTEGER*4  N_GLO(MAX_OBS)
            INTEGER*4  N_LOC(MAX_OBS)
            INTEGER*4  N_SG1(MAX_OBS)
            INTEGER*4  N_SG2(MAX_OBS)
!
            INTEGER*4  CLO_SEG(MAX_OBS)
            INTEGER*4  ATM_SEG(MAX_OBS)
            INTEGER*4  EOP_SEG(MAX_OBS)
!
            INTEGER*4  STATUS
            INTEGER*4  LAST_FIELD
        END TYPE  EQUMEM__STRU  ! EQUMEM_STRU  !
!
! <<<<< end of INCLUDE-BLOCK  equmem.i
!
