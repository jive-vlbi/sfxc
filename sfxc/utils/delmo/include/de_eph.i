!
! >>>>> Include block for epehermirdes DE403
! >>>>> 2004.01.24   (c)  L. Petrov  v 1.3  21-SEP-2004 19:50:00
!
      INTEGER*4  DE_EPH__RECLEN, DE_EPH__RECSH, DE_EPH__RECLEN_R8, DE_EPH__NREC
      PARAMETER  ( DE_EPH__RECLEN = 8144, DE_EPH__RECLEN_R8 = DE_EPH__RECLEN/8 )
      PARAMETER  ( DE_EPH__RECSH  = 7184 )
      PARAMETER  ( DE_EPH__NREC   = 1200 )
      TYPE       DE_EPH__TYPE
            REAL*8      BUF(DE_EPH__RECLEN_R8,DE_EPH__NREC)
            CHARACTER   TIT(3)*84    
            CHARACTER   HEA_NAM(400)*6  
            REAL*8      HEA_VAL(400) 
            REAL*8      DATE_BEG_JD
            REAL*8      DATE_END_JD
            REAL*8      STEP_DAY
            INTEGER*4   NCON
            INTEGER*4   NUMDE      
            REAL*8      AU         
            REAL*8      EMRAT      
            INTEGER*4   IPT(3,13)
            INTEGER*4   STATUS
      END TYPE   DE_EPH__TYPE
!
      TYPE       DE_EPH_HEA__TYPE
            CHARACTER   TIT(3)*84    !  252
            CHARACTER   CNAM(400)*6  ! 2400
            INTEGER*1   ARR(5492)    ! 5492
      END TYPE   DE_EPH_HEA__TYPE
      INTEGER*4  DE_EPH__LOADED
      PARAMETER  ( DE_EPH__LOADED = 1 ) 
!
! >>>>> End of include block for epehermirdes DE_EPH
!
