!@This is the start of file &BATCM
!
      INTEGER DCBBLK
      PARAMETER (DCBBLK=10)
      CHARACTER INTERN*8192
      INTEGER DCB(DCBBLK*128+16),LENRD
      INTEGER*4 SAVREC,SAVPOS
      LOGICAL KGOT,KEOF
!
      COMMON /BATCM/ KGOT, KEOF, INTERN, LENRD, SAVREC, SAVPOS, DCB
!