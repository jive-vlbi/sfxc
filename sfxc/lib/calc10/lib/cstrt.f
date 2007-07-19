      REAL*8 FUNCTION JDY2K (IYEAR, IMONTH, IDAY)
      Implicit None
!
!     Function JDY2K: Function to convert year, month, day to full Julian
!     day. The year can be either a four-digit year or a two-digit year.
!
!     If a 4-digit year, this function is good from 1 March 1900 to 31
!     December 2099.
!
!     If a 2-digit year, this function is good from 1 January 1970 to
!     31 December 2069. If year is 70 - 99, 1900 is added. If year is
!     00 - 69, 2000 is added.
!
!     Programmer:
!      98.07.23  D. Gordon  Function written from code in cutcu.f
!
      Integer*4 IYEAR, IMONTH, IDAY, IY, IM, ID
!
       IY = IYEAR
       IM = IMONTH
       ID = IDAY
!
       If (IY .ge. 70 .and. IY .le. 99) Then
        IY = IY + 1900
        Go To 100
       Endif
!
       If (IY .ge. 0 .and. IY .le. 69) Then
        IY = IY + 2000
        Go To 100
       Endif
!
       If (IY .gt.1900 .and. IY .le. 2099) Then
        Go To 100
       Endif
!
!     Year out of range if we get here
       Print *, ' JDY2K, Year out of Range, Stopping! ', IY
       Stop
!
 100   Continue
!
        JDY2K = 367.D0*IY - (7 * ( IY + (IM+9)/12) )/4 +
     .          (275*IM)/9 + ID + 1721013.5D0
!
!      Write(6,1000) IYEAR, IMONTH, IDAY, IY, IM, ID, JDY2k
 1000  Format(/,'Function JDY2K: ',/,' Input, Modified Y,M,D: ',
     .        2x,3I5,5x,3I5,/,' JDY2K ', F15.2)
      Return
      End
