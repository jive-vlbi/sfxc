! Note: If any changes are made here, changes may be necessary in
!       ../sdbh/namblk.f.
!
! Modifications:
! BA 98.08.26  Added above note.
!
      CHARACTER*70 KBUF
      CHARACTER*63 NNAME
      CHARACTER*4 LHOLD
      INTEGER*2 ISTATUS, IREC, IACTSEC, KSTATUS
      INTEGER*2 IFIRST(14), IQQ(16), LBUF(35), IDBS, ILAST
!
      COMMON/NAMHOL/ISTATUS,IREC,IQQ,IACTSEC,KSTATUS,LBUF
      SAVE /NAMHOL/
      COMMON/namchr/NNAME,LHOLD
      SAVE /namchr/
!
      EQUIVALENCE (IQQ,IDBS),(IQQ(2),ILAST),(IQQ(3),IFIRST)
      EQUIVALENCE (LBUF,KBUF)
!
