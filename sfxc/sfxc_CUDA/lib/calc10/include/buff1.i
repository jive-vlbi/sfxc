!This is the start of file &BUFF1
!
      INTEGER*2 BUFF1_WORDS
!      PARAMETER (BUFF1_WORDS=(3*OLD_MAX_STA_CMP+1)*INTS_WORDS
!     &                       +3*OLD_MAX_STA*REALL_WORDS)
      PARAMETER (BUFF1_WORDS=(3*MAX_STA_CMP+1)*INTS_WORDS
     .                       +(3*MAX_STA+1)*REALL_WORDS)
      INTEGER*2 ISCNT(3,MAX_STA_CMP),NSCNT1
      REAL*8 VSITE1(3,MAX_STA),prepoch
      integer*2 bufbuf(buff1_words)
      equivalence (bufbuf,vsite1)
      COMMON/BUFF1/VSITE1,prepoch, ISCNT, NSCNT1
