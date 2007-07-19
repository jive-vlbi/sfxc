!
! --- This file was generated automatically by nut_empexp
! --- written by Leonid Petrov ( Leonid.Petrov@lpetrov.net )
! --- File was created at 2006.02.15-16:57:44
! --- It contains nutation expansion of the model heo_08d
! --- Phases, frequencies, accelerations of the arguments as well as amplitudes
! --- of cos and sin components of nutation in longitude and in obliquity
! --- are transformed to the form suitable for computations.
! --- Nutation is considered as a a harmonic variations in polar motion 
! --- Convention: angles of a small rotation are computed as :
! ---             E1 = sum_i {  PMC_i*cos(ARG_i) + PMS_i*sin(ARG_i) }
! ---             E2 = sum_i { -PMC_i*sin(ARG_i) + PMS_i*cos(ARG_i) }
! ---             E3 = 0 
! --- Truncation limit for amplitudes:  0.5d-7 rad
! --- Units are picoradians for amplitudes 
! --- Frequency range -9.500D-05, -5.000D-05 rad/sec
!
      INTEGER*4   N_NUT_PETA, I_NUT_PETA
      PARAMETER ( N_NUT_PETA =  3 )
      CHARACTER  MODEL_NUT_PETA*40
      REAL*8     PHAS_NUT_PETA(N_NUT_PETA), FREQ_NUT_PETA(N_NUT_PETA), ACCL_NUT_PETA(N_NUT_PETA)
      REAL*8     PMC_NUT_PETA(N_NUT_PETA),      PMS_NUT_PETA(N_NUT_PETA) 
      REAL*8     EPSILON_0_NUT_PETA
      PARAMETER  ( EPSILON_0_NUT_PETA = 0.4090928041D0 ) ! rad
      PARAMETER  ( MODEL_NUT_PETA = 'heo_nut_peta                            ' )
      DATA       ( PHAS_NUT_PETA(I_NUT_PETA), FREQ_NUT_PETA(I_NUT_PETA),  
     .             ACCL_NUT_PETA(I_NUT_PETA),                             
     .             PMC_NUT_PETA(I_NUT_PETA), PMS_NUT_PETA(I_NUT_PETA),    
     .             I_NUT_PETA=1,N_NUT_PETA )
     .             /           
     . 2.1824392000D0, -7.293185551531D-05,  7.2752D-24,  & !   1 
     .   38904910.0D0,         0.0D0,                     & !
     . 4.1007461000D0, -7.291046159118D-05, -7.2752D-24,  & !   2 
     .    5722295.9D0,         0.0D0,                     & !
     . 3.5069407000D0, -7.252294578275D-05,  2.1256D-24,  & !   3 
     .    2658993.2D0,         0.0D0                      & !
     . /
