!@This is the start of file &GLBC3
!
!   modifications:
!
!   kdb 950720 Change declaration of velohoriz from logical*2 to integer*2
!              to accomodate splitting of velocity_origin horizontal keyword
!              into horizontal and vertical choices.
!   kdb 961125 Accomodate new batch suppression options,
!              no net rotation sources and
!              no net rotation and translation positions and velocities
!   kdb 970312 Implement the ability to use weighted or uniform no_net
!              constraints via the batopt file, using JMG's algorithm.
!              Also implement in batopt the ability to use the horizontal
!              projection matrix vs. just the identity matrix for the
!              no net constraints, using JMG's algorithm.
!   pet 970929 Renamed IFILL to IFILL_GLBC3 to avoid intereference with oborg.i
!   pet 2000.08.01  Added variables BEGMARK_GLBC3_I2, ENDMARK_GLBC3_I2
!                   and constant LEN_GLBC3_FILL_I2
!   pet 2002.03.13  Added variables NNT_POS_GLO, NNT_POS_LOC, NNR_POS_GLO,
!                   NNR_POS_LOC, NNR_SRC_GLO, NNR_SRC_LOC -- they keep flags
!                   whether constraints should be imposed on local or global
!                   paramters or even both
!   pet 2002.09.23  Added variables DTOSTA_CHR
!   pet 2003.09.01  Changed dimension of from SOUSUP(SRC_BIT_WORDS,7) to
!                   SOUSUP(SRC_BIT_WORDS,8) 
!
!   Flags and their meanings:
!
      INTEGER*2 LEN_GLBC3_FILL_I2, BEGMARK_GLBC3_I2, ENDMARK_GLBC3_I2
      PARAMETER ( LEN_GLBC3_FILL_I2 = 93 )     ! Length of unused space in
      INTEGER*2 IFILL_GLBC3(LEN_GLBC3_FILL_I2) ! Integer*2 words
!
      LOGICAL*2 PRESUP, RELSUP, TIDSUP
      INTEGER*2 VELOHORIZ, NUTSUP(116), DEFVEL, DEFCMP, DEFSRC
      INTEGER*2 VELSUP(STA_BIT_WORDS,6), CMPSUP(STA_BIT_WORDS,6)
      INTEGER*2 ISTASP, STASUP(4,MAX_STA), DATSTA(4), DTOSTA(4)
      INTEGER*2 SOUSUP(SRC_BIT_WORDS,8), ISRCSP, SRCSUP(4,MAX_SRC)
      INTEGER*2 IND_NNR_SRC(2), IND_NNR_PRP(2) 
      INTEGER*2 NUMVELGRP, VELTIES(MAX_STA)
      INTEGER*2 NUMSTAGRP, STATIES(MAX_STA)
      LOGICAL*2 NNT_POS_GLO, NNT_POS_LOC, NNR_POS_GLO, NNR_POS_LOC
      LOGICAL*2 NNR_SRC_GLO, NNR_SRC_LOC
      CHARACTER FIXED_PLATE*4
      REAL*8    NUVEL_WT
      CHARACTER*2 KMATRIX_NNTP, KMATRIX_NNTV,
     .            KSIG_SCALE_NNTP, KSIG_SCALE_NNTV,
     .            KSIG_SCALE_NNRP, KSIG_SCALE_NNRV, KSIG_SCALE_NNRS,
     .            KSIG_SCALE_NNRQ
!
      CHARACTER  STASUP_CHR(MAX_STA)*8
      CHARACTER  SRCSUP_CHR(MAX_SRC)*8
      CHARACTER  DTOSTA_CHR*8
      EQUIVALENCE ( STASUP, STASUP_CHR )
      EQUIVALENCE ( SRCSUP, SRCSUP_CHR )
      EQUIVALENCE ( DTOSTA, DTOSTA_CHR )
!
      COMMON / GLBC3 /
     .         BEGMARK_GLBC3_I2,
     .         PRESUP, RELSUP, TIDSUP, NUTSUP, DEFVEL, VELSUP, CMPSUP,
     .         ISTASP, STASUP, DATSTA, DTOSTA, SOUSUP, ISRCSP, SRCSUP,
     .         DEFCMP, DEFSRC, NUMVELGRP, VELTIES, VELOHORIZ,
     .         NUMSTAGRP, STATIES, FIXED_PLATE, NUVEL_WT,
     .         KMATRIX_NNTP, KMATRIX_NNTV,
     .         KSIG_SCALE_NNTP, KSIG_SCALE_NNTV,
     .         KSIG_SCALE_NNRP, KSIG_SCALE_NNRV, KSIG_SCALE_NNRS,
     .         KSIG_SCALE_NNRQ, NNT_POS_GLO, NNT_POS_LOC, NNR_POS_GLO,
     .         NNR_POS_LOC, NNR_SRC_GLO, NNR_SRC_LOC,
     .         IND_NNR_SRC, IND_NNR_PRP,
     .         IFILL_GLBC3,
     .         ENDMARK_GLBC3_I2
