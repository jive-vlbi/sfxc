
! >>>>> Include block for package VTD ( VLBI Theoretical Delay )
! >>>>> 2004.01.26  (c)  L. Petrov  v 1.11  26-APR-2006 09:15:42
!
      INCLUDE    'de_eph.i'
      INCLUDE    'ueop.i'
      INCLUDE    'sotid_type.i'
      INCLUDE    'sotid_data.i'
      INCLUDE    'harpos.i'
      INCLUDE    'bindisp.i'
      INCLUDE    'vtd_sou_map.i'
      INCLUDE    'heo.i'
      INCLUDE    'eec.i'
      INCLUDE    'eec_peta.i'
      INTEGER*4  VTD__M_STA, VTD__M_BAS, VTD__M_SOU, VTD__MSES_STA,
     .           VTD__MSES_SOU, VTD__M_EPC, VTD__M_ECC, VTD__M_PSF, VTD__M_SDI,
     .           VTD__M_LPS, VTD__M_STR, VTD__M_BND, VTD__M_SCC,
     .           VTD__M_NZO, VTD__M_SPL, VTD__M_NOD, VTD__M_ERM,
     .           VTD__M_ERD, VTD__MEM
      INTEGER*4  VTD__M_CNF 
      PARAMETER  ( VTD__M_STA =    32 ) ! Maximal number of stations
      PARAMETER  ( VTD__M_BAS =   (VTD__M_STA*(VTD__M_STA+1))/2 ) !
      PARAMETER  ( VTD__M_SOU =  1024 ) ! Maximal number of sources
      PARAMETER  ( VTD__M_EPC =     4 ) ! The maximum number of breaks in coordinates
      PARAMETER  ( VTD__M_ECC =    64 ) ! The maximum number of eccentricites
      PARAMETER  ( VTD__M_PSF =     8 ) ! The maximum number of position 
!                                      ! variation files
      PARAMETER  ( VTD__M_SDI =    32 ) ! The number of knots for interpolation
      PARAMETER  ( VTD__M_CNF =    43 ) ! The number of parameters in control file
      PARAMETER  ( VTD__M_LPS =    64 ) ! Maximum number of entries for leap-sec
      PARAMETER  ( VTD__M_STR =    64 ) ! Maximum number of source structure entries
      PARAMETER  ( VTD__M_BND =     2 ) ! Maximum number of bands
      PARAMETER  ( VTD__M_SCC =     4 ) ! Maximum number of source catalogues
      PARAMETER  ( VTD__M_NZO =     4 ) ! Maximum number of near zone objects
      PARAMETER  ( VTD__M_SPL =     5 ) ! Maximum spline degree
      PARAMETER  ( VTD__M_NOD =   128 ) ! Maximum number of nodes for B-spline
      PARAMETER  ( VTD__M_ERM = 16384 ) ! Maximum number of knots of ERM B-spline
      PARAMETER  ( VTD__M_ERD =     3 ) ! Maximum degree of ERM B-spline
      PARAMETER  ( VTD__MEM   =   128 ) ! Maxumum number of chunks memory 
!                                        explicitly allocated by VTD
!
      CHARACTER  VTD__LABEL*42
      PARAMETER  ( VTD__LABEL = 'VTD  version  1.7  Revision of 2005.12.31 ' )
!
! --- NB: VTD__M_SDI should be the same as M__PSV from bindisp.i
!
      INTEGER*4    VTD__M_PLA
      PARAMETER  ( VTD__M_PLA = 10 )
      INTEGER*4    VTD__COO, VTD__VEL, VTD__ACC
      PARAMETER  ( VTD__COO = 1 )
      PARAMETER  ( VTD__VEL = 2 )
      PARAMETER  ( VTD__ACC = 3 )
      TYPE       VTD_STA__TYPE
            CHARACTER   IVS_NAME*8   
	    INTEGER*4   MJD
	    REAL*8      TAI
!
	    INTEGER*4   N_EPC       ! The number of epochs for site positions
	    INTEGER*4   MJD_REF     ! Reference 
	    REAL*8      TAI_REF     ! epoch
	    INTEGER*4   MJD_EPC(VTD__M_EPC)  ! Date of the 
	    REAL*8      TAI_EPC(VTD__M_EPC)  ! epoch start
	    REAL*8      COO_TRS(3,VTD__M_EPC)  ! Station coordinate per epoch
	    REAL*8      VEL_TRS(3)             ! Station global velocity
	    REAL*8      AXIS_OFFSET
	    CHARACTER   MOUNT_TYPE*4
!
	    INTEGER*4   N_ECC
	    INTEGER*4   ECC_MJD_BEG(VTD__M_ECC)
	    INTEGER*4   ECC_MJD_END(VTD__M_ECC)
	    REAL*8      ECC_TAI_BEG(VTD__M_ECC)
	    REAL*8      ECC_TAI_END(VTD__M_ECC)
	    REAL*8      ECC_TRS(3,VTD__M_ECC)
	    INTEGER*4   CDP_NUMBER
	    CHARACTER   IERS_DOME_NAME*12
!
            REAL*8      BEG_TRS(3)
	    REAL*8      HEI_ELL
	    REAL*8      GAC_ELL
	    REAL*8      AXIS_TRS(3)
	    REAL*8      LONG
	    REAL*8      LAT_GCN
	    REAL*8      LAT_GDT
	    REAL*8      RAD
	    REAL*8      REN_TO_TRS(3,3)
	    REAL*8      UEN_TO_TRS(3,3)
	    REAL*8      MOM_COO_TRS(3)
	    REAL*8      MOM_VEL_TRS(3)
	    REAL*8      MOM_ACC_TRS(3)
	    REAL*8      COO_CRS(3)
	    REAL*8      VEL_CRS(3)
	    REAL*8      ACC_CRS(3)
	    REAL*8      ELEV
	    REAL*8      AZ
	    REAL*8      PARANG
	    REAL*8      ELEV_DER
	    REAL*8      AZ_DER
!
	    REAL*8      ATM_PRES
	    REAL*8      AIR_TEMP
!
	    REAL*8      PSV_TIM_BEG(VTD__M_PSF)
	    REAL*8      PSV_TIM(VTD__M_SDI,VTD__M_PSF)
	    REAL*8      PSV_POS(VTD__M_SDI,3,VTD__M_PSF)
	    REAL*8      PSV_SPL(VTD__M_SDI,3,VTD__M_PSF)
!
	    REAL*8      TROP_HZD
	    REAL*8      TROP_WZD
	    REAL*8      TROP_DEL
	    REAL*8      TROP_DEL_RATE
!
	    REAL*8      TROP_ZEN_DER
	    REAL*8      TROP_TILT_N_DER
	    REAL*8      TROP_TILT_E_DER
!
	    REAL*8      TROP_AXOF_TAU
	    REAL*8      TROP_AXOF_RAT
	    REAL*8      DELAY_AXOF_VTD
	    REAL*8      RATE_AXOF_VTD
	    REAL*8      DELAY_AXOF_CALC
	    REAL*8      RATE_AXOF_CALC
!
	    INTEGER*4   STATUS
      END TYPE   VTD_STA__TYPE
!
      TYPE       VTD_SOU__TYPE
            CHARACTER   IVS_NAME*8
            CHARACTER   J2000_NAME*10
	    CHARACTER   FILLER_1*6
!
	    REAL*8      ALPHA
	    REAL*8      DELTA
	    REAL*8      DIST
	    REAL*8      SOU_CRS(3) ! Full vector
	    REAL*8      S_CRS(3)   ! Unit vector
!
	    REAL*8      ALPHA_RATE
	    REAL*8      DELTA_RATE
	    REAL*8      DIST_RATE
	    REAL*8      SOU_CRS_RATE(3)
	    REAL*8      S_CRS_RATE(3)
!
	    REAL*8      TAI_REF
	    INTEGER*4   MJD_REF
	    INTEGER*4   IND_NZO
	    CHARACTER   NZO_NAME*8
	    CHARACTER   OBJ_TYPE*1
	    CHARACTER   FILLER_2*3
      END TYPE   VTD_SOU__TYPE
!
      TYPE       VTD_NZO__TYPE
	    CHARACTER  NAME*8
       	    INTEGER*4  MJD_BEG
	    INTEGER*4  TIM_CODE
	    REAL*8     TIM_BEG
	    INTEGER*4  DEG_SPL              ! Degree of the spline
	    INTEGER*4  NOD_SPL              ! Number of nodes for spline
	    REAL*8     TIM_ARR(1-VTD__M_SPL:VTD__M_NOD)
	    REAL*8     SPL_ARR(1-VTD__M_SPL:VTD__M_NOD,3)
	    CHARACTER  OBJ_TYPE*1
	    CHARACTER  FILLER_1*7
      END TYPE   VTD_NZO__TYPE
!
      TYPE       VTD_CONF__TYPE
	    INTEGER*4  MJD_START
	    INTEGER*4  MJD_STOP
	    REAL*8     TAI_START
	    REAL*8     TAI_STOP
!
	    CHARACTER  FINAM_STADESC*256
	    CHARACTER  FINAM_STACOO*256
	    CHARACTER  FINAM_STAVEL*256
	    CHARACTER  FINAM_STAECC*256
	    CHARACTER  FINAM_SOUCOO(VTD__M_SCC)*256
	    CHARACTER  FINAM_DE_EPH*256
	    CHARACTER  FINAM_LEAPSEC*256
	    CHARACTER  FINAM_EOP*256
	    CHARACTER  FINAM_ERM*256
            CHARACTER  FINAM_HEO*256
	    INTEGER*4  EOP_TIME_SCALE
	    INTEGER*4  UZT_MODEL
	    INTEGER*4  UZT_USE
            INTEGER*4  PREC_EXP
            INTEGER*4  NUT_EXP
	    INTEGER*4  NUT_GDS
!
	    INTEGER*4  STD_2ND_MODEL
	    INTEGER*4  STD_3RD_MODEL 
	    INTEGER*4  STD_ZF_MODEL 
	    INTEGER*4  PTD_MODEL
	    CHARACTER  MPL_FILE*256
	    REAL*8     XPOL_REF_SEC ! Reference epoch for X pole model in sec from J2000.0
	    REAL*8     XPOL_MEAN    ! Modeled value of mean X pole  in rad
	    REAL*8     XPOL_DRIFT   ! Modeled value of X pole drift in rad/sec
	    REAL*8     YPOL_REF_SEC ! Reference epoch for Y pole model in sec from J2000.0
	    REAL*8     YPOL_MEAN    ! Modeled value of mean Y pole  in rad
	    REAL*8     YPOL_DRIFT   ! Modeled value of Y pole drift in rad/sec
	    CHARACTER  POSVAR_FIL(VTD__M_PSF)*256
	    INTEGER*4  POSVAR_MOD(VTD__M_PSF)
	    INTEGER*4  POSVAR_INT(VTD__M_PSF)
	    INTEGER*4  POSVAR_USE(VTD__M_PSF)
	    INTEGER*4  EROT_COMPAT
!
	    INTEGER*4  METEO_DEF
	    INTEGER*4  HMF_MODEL
	    INTEGER*4  WMF_MODEL
	    INTEGER*4  HZD_MODEL
	    INTEGER*4  WZD_MODEL
	    INTEGER*4  ATD_PARTIALS
	    INTEGER*4  GRS_METRIC
	    INTEGER*4  AXOF_MODEL
	    INTEGER*4  TROP_AXOF_COUPL
	    INTEGER*4  TROP_GEOM_COUPL
	    INTEGER*4  GEOM_EXPR_FAR_ZONE
	    INTEGER*4  GEOM_EXPR_NEAR_ZONE
!
	    CHARACTER  FINAM_STRUC*256
	    LOGICAL*4  FL_RATE
!
	    INTEGER*4  IVRB     !  Verbosity level
	    LOGICAL*4  FL_WARN  !  Warning flag
!
	    CHARACTER  CONFIG_FINAM*256
	    INTEGER*4  STATUS
      END TYPE   VTD_CONF__TYPE
!
      TYPE       VTD_POSVAR__TYPE
            INTEGER*4  N_PSVHAR   ! Number of harmonics
            INTEGER*4  N_PSVSTA   ! Number of sites
!
            INTEGER*4  LEN_NAMSIT
            INTEGER*4  LEN_HARVAL
            INTEGER*4  LEN_HARDSP
            INTEGER*4  LEN_STACOO
            INTEGER*4  LEN_BDSFIL
            INTEGER*4  LEN_BDSSAM
            INTEGER*4  LEN_BDSFMJ
            INTEGER*4  LEN_BDSFSC
            INTEGER*4  LEN_BDSLMJ
            INTEGER*4  LEN_BDSLSC
            INTEGER*4  LEN_BDSNSA
!
	    INTEGER*4  ADR_NAMSIT
            INTEGER*4  ADR_HARVAL
            INTEGER*4  ADR_HARDSP
	    INTEGER*4  ADR_STACOO
	    INTEGER*4  ADR_BDSFIL
	    INTEGER*4  ADR_BDSSAM
	    INTEGER*4  ADR_BDSFMJ
	    INTEGER*4  ADR_BDSFSC
	    INTEGER*4  ADR_BDSLMJ
	    INTEGER*4  ADR_BDSLSC
	    INTEGER*4  ADR_BDSNSA
!
	    INTEGER*4  STS_NAMSIT
            INTEGER*4  STS_HARVAL
            INTEGER*4  STS_HARDSP
	    INTEGER*4  STS_STACOO
	    INTEGER*4  STS_BDSFIL
	    INTEGER*4  STS_BDSSAM
	    INTEGER*4  STS_BDSFMJ
	    INTEGER*4  STS_BDSFSC
	    INTEGER*4  STS_BDSLMJ
	    INTEGER*4  STS_BDSLSC
	    INTEGER*4  STS_BDSNSA
	    REAL*8     RD_AREA
! 
            INTEGER*4  TIM_PSVFIL  ! Date of last modification of the file 
!                                  ! with position variations stored in the 
!                                  ! native UNIX format
	    INTEGER*4  MEM_LEN
	    INTEGER*4  MEM_ADR
	    INTEGER*4  STATUS
            CHARACTER  BDS_ENDIAN*1
            CHARACTER  BDS_FLOAT*1
      END TYPE   VTD_POSVAR__TYPE
!
      TYPE       VTD_MOMENT__TYPE
	    INTEGER*4  MJD
	    REAL*8     TAI
	    REAL*8     TDB
	    REAL*8     TDT
	    CHARACTER  SOU_NAM*8
!
	    REAL*8     XPL
	    REAL*8     YPL
	    REAL*8     UT1_M_TAI
	    REAL*8     XPL_RATE
	    REAL*8     YPL_RATE
	    REAL*8     UT1_RATE
	    REAL*8     S_ANG
	    REAL*8     S_ANG_RATE
	    REAL*8     TRS_TO_CRS(3,3)
	    REAL*8     TRS_TO_CRS_DER1(3,3)
	    REAL*8     TRS_TO_CRS_DER2(3,3)
	    REAL*8     DTRS_TO_CRS_DEOP(3,3,3)
	    REAL*8     DTRS_TO_CRS_DER1_DEOP(3,3,3)
	    REAL*8     PLAN(3,3,VTD__M_PLA)
	    TYPE     ( SOUMAP__TYPE ) :: MAP(VTD__M_BND)
	    LOGICAL*4  FL_STRUC(VTD__M_BND)
	    CHARACTER  IMAGE_BAND(VTD__M_BND)*1
	    INTEGER*4  STATUS
      END TYPE   VTD_MOMENT__TYPE
!
      TYPE VTD_LEAPS__TYPE
	   INTEGER*4  L_LPS
	   INTEGER*4  MJD_LPS(VTD__M_LPS)
	   REAL*8     TAI_LPS(VTD__M_LPS)
	   REAL*8     UTC_M_TAI(VTD__M_LPS)
	   INTEGER*4  STATUS
      END TYPE VTD_LEAPS__TYPE
!
      TYPE VTD_ERM__TYPE
           SEQUENCE
           CHARACTER  FINAM*128
	   INTEGER*4  STATUS
           INTEGER*4  DEGREE(3)
           INTEGER*4  MJD_BEG
           INTEGER*4  MJD_END
           REAL*8     TAI_BEG
           REAL*8     TAI_END
	   REAL*8     TIME_SPAN(3)
!
	   INTEGER*4  NKNOTS(3)
           REAL*8     TIM(1-VTD__M_ERD:VTD__M_ERM,3)
           REAL*8     VAL(1-VTD__M_ERD:VTD__M_ERM,3)
           REAL*8     ERR(1-VTD__M_ERD:VTD__M_ERM,3)
	   REAL*8     COV(1-VTD__M_ERD:VTD__M_ERM,VTD__M_ERD,3)
      END  TYPE VTD_ERM__TYPE
!
      INTEGER*4  STRUC__MAP, STRUC__CLC
      PARAMETER  ( STRUC__MAP = 59341 )
      PARAMETER  ( STRUC__CLC = 59347 )
!
      TYPE VTD_STRUC__TYPE
           CHARACTER  SOU_NAME*8
	   CHARACTER  BAND*1
	   INTEGER*4  MJD_BEG
	   INTEGER*4  MJD_END
	   REAL*8     SEC_BEG
	   REAL*8     SEC_END
           REAL*8     PHASE_CENTER_PIXEL(2)
	   INTEGER*4  USAGE_CODE
	   CHARACTER  MAP_FILE*128
      END TYPE VTD_STRUC__TYPE
!
      TYPE       VTD__TYPE
            TYPE ( VTD_CONF__TYPE   ) :: CONF
            TYPE ( VTD_STA__TYPE    ) :: STA(VTD__M_STA)
            TYPE ( VTD_SOU__TYPE    ) :: SOU(VTD__M_SOU)
            TYPE ( UEOP__TYPE       ) :: UEOP
            TYPE ( DE_EPH__TYPE     ) :: DE_EPH
            TYPE ( DE_EPH_HEA__TYPE ) :: DE_EPH_HEA
            TYPE ( STATID__STRU     ) :: STATID(VTD__M_STA)
            TYPE ( TIDCNF__STRU     ) :: TIDCNF_STD
            TYPE ( TIDCNF__STRU     ) :: TIDCNF_PTD
            TYPE ( VTD_MOMENT__TYPE ) :: MOM
            TYPE ( TIMTID__STRU     ) :: TIMTID
            TYPE ( VTD_POSVAR__TYPE ) :: POSVAR(VTD__M_PSF)
	    TYPE ( VTD_LEAPS__TYPE  ) :: LEAPSEC
	    TYPE ( VTD_STRUC__TYPE  ) :: STRUC(VTD__M_SOU)
	    TYPE ( HEO__STRUC       ) :: HEO(M__HEO)
	    TYPE ( VTD_ERM__TYPE    ) :: ERM
	    TYPE ( VTD_NZO__TYPE    ) :: NZO(VTD__M_NZO)
!
	    INTEGER*4  L_STA
	    INTEGER*4  L_SOU
	    INTEGER*4  L_NZO
	    INTEGER*4  L_STR
	    INTEGER*4  L_HEO
!
	    CHARACTER  HEO_NAME*128
	    REAL*8     HEO_EPOCH_SEC
!
	    INTEGER*4  MJD_BEG
	    INTEGER*4  MJD_END
	    REAL*8     TAI_BEG
	    REAL*8     TAI_END
!
	    INTEGER*4  MEM_CHN
	    INTEGER*4  MEM_LEN(VTD__MEM)
	    INTEGER*4  MEM_ADR(VTD__MEM)
!
	    INTEGER*4  STATUS_STA
	    INTEGER*4  STATUS_SOU
	    INTEGER*4  STATUS_EPH
	    INTEGER*4  STATUS_EOP
	    INTEGER*4  STATUS_NUT
	    INTEGER*4  STATUS
!
	    INTEGER*4  LAST_FIELD ! Because of a bug in Sun compiler we need this field
      END TYPE   VTD__TYPE
!
      INTEGER*4  VTD__UNDF, VTD__INIT, VTD__DESC, VTD__COOR, VTD__STRT,
     .           VTD__LOAD, VTD__NOAV, VTD__ALLC
      PARAMETER  ( VTD__UNDF = 0 )
      PARAMETER  ( VTD__INIT = 1205938721 )
      PARAMETER  ( VTD__DESC = 1783254063 )
      PARAMETER  ( VTD__COOR = 2036492761 )
      PARAMETER  ( VTD__STRT = 1301857342 )
      PARAMETER  ( VTD__LOAD = 1419304723 )
      PARAMETER  ( VTD__ALLC = 1983218327 )
      PARAMETER  ( VTD__NOAV = 1029487522 )
      REAL*8       VTD__TIME_TOL
      PARAMETER  ( VTD__TIME_TOL = 1.D-4 ) ! Time tolerance
!
      INTEGER*4  VTD__EARLIEST_MJD
      REAL*8     VTD__EARLIEST_TAI
      PARAMETER  ( VTD__EARLIEST_MJD = 40000, VTD__EARLIEST_TAI = 0.0 )
      REAL*8       VTD__REA, VTD__FE, VTD__EXC_SQ, VTD__ACC_EQU,
     .             VTD__GRV_H, VTD__GRV_LAT
      PARAMETER  ( VTD__REA     = 6378136.7D0 )       ! Earth's radius
      PARAMETER  ( VTD__FE      = 1.D0/298.257D0 )    ! Earth's flattening
      PARAMETER  ( VTD__EXC_SQ  = 2.D0*VTD__FE - VTD__FE**2 )   ! Earth's eccentricity
      PARAMETER  ( VTD__ACC_EQU = 9.7803184558D0 )    ! Equatorial gravity acc.
      PARAMETER  ( VTD__GRV_H   = -2.D0*VTD__ACC_EQU/VTD__REA ) ! D(ACC_EQU)/DH
      PARAMETER  ( VTD__GRV_LAT = 0.001931663  )      ! D(ACC_EQU)/D(phi)
!
      REAL*8       VTD__HEIGHT_TOL
      PARAMETER  ( VTD__HEIGHT_TOL   =   50000.0D0 )
!
      INTEGER*4    UZT__NONE, UZT__DICKMAN1993, UZT__ADD, UZT__SUBTRACT,
     .             UZT__INTERPOLATE
      PARAMETER  ( UZT__NONE        =    0 )
      PARAMETER  ( UZT__DICKMAN1993 = 6001 )
      PARAMETER  ( UZT__ADD         = 6011 )
      PARAMETER  ( UZT__INTERPOLATE = 6012 )
      PARAMETER  ( UZT__SUBTRACT    = 6013 )
!
      INTEGER*4  VTD__TAI, VTD__TDT, VTD__TDB, VTD__UTC, VTD__UT1
      PARAMETER  ( VTD__TAI =  7001 )
      PARAMETER  ( VTD__TDT =  7002 )
      PARAMETER  ( VTD__TDB =  7003 )
      PARAMETER  ( VTD__UTC =  7004 )
      PARAMETER  ( VTD__UT1 =  7005 )
!
      INTEGER*4    VTD__BRS, VTD__IERS1992, VTD__GRS
      PARAMETER  ( VTD__BRS      = 7101 ) 
      PARAMETER  ( VTD__IERS1992 = 7102 )
      PARAMETER  ( VTD__GRS      = 7103 )
!
! --- Constants for nutation models
!
      INTEGER*4    NUT__GDS_YES, NUT__GDS_NO
      PARAMETER  ( NUT__GDS_YES = 5101 )
      PARAMETER  ( NUT__GDS_NO  = 5102 )
!
! --- Constants for precession model
!
      INTEGER*4    PREC__LISKE1976
      PARAMETER  ( PREC__LISKE1976 = 6001 )
!
      INTEGER*4    VTD__NONE, VTD__YES, VTD__IMA, VTD__CALC, VTD__CALC9,
     .             VTD__CALC10
      PARAMETER  ( VTD__NONE   =    0 ) 
      PARAMETER  ( VTD__YES    =    1 ) 
      PARAMETER  ( VTD__IMA    = 4801 ) 
      PARAMETER  ( VTD__CALC   = 4802 ) 
      PARAMETER  ( VTD__CALC9  = 4803 ) 
      PARAMETER  ( VTD__CALC10 = 4804 ) 
!
! --- Constants for tropospheric delay models/mapping function codes
!
      INTEGER*4    VTD__NMFH, VTD__NMFW, VTD__SAA, VTD__MM95, VTD__TNMFH,
     .             VTD__TNMFW
      PARAMETER  ( VTD__NMFH  = 9001 )
      PARAMETER  ( VTD__NMFW  = 9002 )
      PARAMETER  ( VTD__SAA   = 9003 )
      PARAMETER  ( VTD__MM95  = 9004 )
      PARAMETER  ( VTD__TNMFH = 9005 )
      PARAMETER  ( VTD__TNMFW = 9006 )
!
! --- Constants for VLBI theoretical delay expression
!
      INTEGER*4    VTD__KS1999, VTD__PK2001, VTD__LT, VTD__SF2004
      PARAMETER  ( VTD__KS1999 =  9991 )
      PARAMETER  ( VTD__PK2001 =  9992 )
      PARAMETER  ( VTD__LT     =  9993 )
      PARAMETER  ( VTD__SF2004 =  9994 )
!
      INTEGER*4    VTD__NZO_ITER 
      PARAMETER  ( VTD__NZO_ITER = 3 )
      INTEGER*4    VTD__I92_TO_BRS, VTD__I92_TO_GRS, VTD__GRS_TO_I92,
     .             VTD__GRS_TO_BRS, VTD__BRS_TO_GRS, VTD__BRS_TO_I92
      PARAMETER  ( VTD__I92_TO_BRS = 8401 )
      PARAMETER  ( VTD__I92_TO_GRS = 8402 )
      PARAMETER  ( VTD__GRS_TO_I92 = 8403 )
      PARAMETER  ( VTD__GRS_TO_BRS = 8404 )
      PARAMETER  ( VTD__BRS_TO_GRS = 8405 )
      PARAMETER  ( VTD__BRS_TO_I92 = 8406 )
!
! --- Constants for TRS metric
!
      INTEGER*4    VTD__METRIC_ITRF2000, VTD__METRIC_IAU2000
      PARAMETER  ( VTD__METRIC_ITRF2000 = 8001 )
      PARAMETER  ( VTD__METRIC_IAU2000  = 8002 )
! 
      CHARACTER  VTD__PLANAM(VTD__M_PLA)*8
      REAL*8     VTD__GM(VTD__M_PLA)
      INTEGER*4  VTD__$INC1
!
! --- Planet names and their GM extracted from the header of DE_EPH ephemerides
!
      DATA ( VTD__PLANAM(VTD__$INC1), VTD__GM(VTD__$INC1), 
     .       VTD__$INC1=1,VTD__M_PLA )                     
     .       /                                             
     .       'SUN     ',    1.327124400179869D+20,         
     .       'MERCURY ',    2.203208048641792D+13,         
     .       'VENUS   ',    3.248585988264596D+14,         
     .       'EARTH   ',    3.986004356081032D+14,         
     .       'MARS    ',    4.282831425806711D+13,         
     .       'JUPITER ',    1.267127678577959D+17,         
     .       'SATURN  ',    3.794062606113728D+16,         
     .       'URANUS  ',    5.794549007071871D+15,         
     .       'NEPTUNE ',    6.836534063879259D+15,         
     .       'MOON    ',    4.902799107879761D+12          
     .       /
!
      INTEGER*4  VTD__SUN,  VTD__MERC, VTD__VENU, VTD__EART, VTD__MARS,
     .           VTD__JUPT, VTD__SATU, VTD__URAN, VTD__NEPT, VTD__MOON
!
      PARAMETER  ( VTD__SUN =   1  )
      PARAMETER  ( VTD__MERC =  2  )
      PARAMETER  ( VTD__VENU =  3  )
      PARAMETER  ( VTD__EART =  4  )
      PARAMETER  ( VTD__MARS =  5  )
      PARAMETER  ( VTD__JUPT =  6  )
      PARAMETER  ( VTD__SATU =  7  )
      PARAMETER  ( VTD__URAN =  8  )
      PARAMETER  ( VTD__NEPT =  9  )
      PARAMETER  ( VTD__MOON = 10  )
!  
      REAL*8     VTD__C, VTD__RFR, VTD__LB, VTD__LG
      PARAMETER  ( VTD__C   = 299792458.0D0 )
      PARAMETER  ( VTD__RFR = 3.13D-4 ) ! Refractivity parameter
      PARAMETER  ( VTD__LB  = 1.55051976772D-8 )
      PARAMETER  ( VTD__LG  = 6.969290134D-10  )
!
      INTEGER*4  VTD__NDER
      PARAMETER ( VTD__NDER = 32 )
      INTEGER*4  VTD__DER_E1,   VTD__DER_E2,   VTD__DER_E3,  
     .           VTD__DER_ST1X, VTD__DER_ST1Y, VTD__DER_ST1Z,
     .           VTD__DER_ST2X, VTD__DER_ST2Y, VTD__DER_ST2Z,
     .           VTD__DER_RA,   VTD__DER_DL,                 
     .           VTD__DER_AT1,  VTD__DER_AT2,                
     .           VTD__DER_ATN1, VTD__DER_ATE1,               
     .           VTD__DER_ATN2, VTD__DER_ATE2,               
     .           VTD__DER_POS1, VTD__DER_POS2, VTD__DER_POS3,
     .           VTD__DER_VEL1, VTD__DER_VEL2, VTD__DER_VEL3,
     .           VTD__DER_AXOF
      INTEGER*4  VTD__DER_EOP(3), VTD__DER_STA1(3), VTD__DER_STA2(3),
     .           VTD__DER_POS(3), VTD__DER_VEL(3)
!
      PARAMETER ( VTD__DER_E1   =  1 ) ! Earth rotation Euler angle 1
      PARAMETER ( VTD__DER_E2   =  2 ) ! Earth rotation Euler angle 2
      PARAMETER ( VTD__DER_E3   =  3 ) ! Earth rotation Euler angle 3
      PARAMETER ( VTD__DER_ST1X =  4 ) ! Station 1, X coordinate
      PARAMETER ( VTD__DER_ST1Y =  5 ) ! Station 1, Y coordinate
      PARAMETER ( VTD__DER_ST1Z =  6 ) ! Station 1, Z coordinate
      PARAMETER ( VTD__DER_ST2X =  7 ) ! Station 2, X coordinate
      PARAMETER ( VTD__DER_ST2Y =  8 ) ! Station 2, Y coordinate
      PARAMETER ( VTD__DER_ST2Z =  9 ) ! Station 2, Z coordinate
      PARAMETER ( VTD__DER_RA   = 10 ) ! Right ascension
      PARAMETER ( VTD__DER_DL   = 11 ) ! Declination
      PARAMETER ( VTD__DER_AT1  = 12 ) ! Station 1, atmospheric path delay
      PARAMETER ( VTD__DER_AT2  = 13 ) ! Station 2, atmospheric path delay
      PARAMETER ( VTD__DER_ATN1 = 14 ) ! Station 1, atmospheric north tilt
      PARAMETER ( VTD__DER_ATE1 = 15 ) ! Station 1, atmospheric east  tilt
      PARAMETER ( VTD__DER_ATN2 = 16 ) ! Station 2, atmospheric north tilt
      PARAMETER ( VTD__DER_ATE2 = 17 ) ! Station 2, atmospheric east  tilt
      PARAMETER ( VTD__DER_POS1 = 18 ) ! Position of the object, X coordinate
      PARAMETER ( VTD__DER_POS2 = 19 ) ! Position of the object, Y coordinate
      PARAMETER ( VTD__DER_POS3 = 20 ) ! Position of the object, Z coordinate
      PARAMETER ( VTD__DER_VEL1 = 21 ) ! Velocity of the object, X coordinate
      PARAMETER ( VTD__DER_VEL2 = 22 ) ! Velocity of the object, Y coordinate
      PARAMETER ( VTD__DER_VEL3 = 23 ) ! Velocity of the object, Z coordinate
      PARAMETER ( VTD__DER_AXOF = 24 ) ! Antenna axis offset length
!
! --- Derivatvies with indexes 25-32 are reserved for future used
!
      DATA         VTD__DER_EOP  
     .           /
     .             VTD__DER_E1,  
     .             VTD__DER_E2,  
     .             VTD__DER_E3   
     .           / 
      DATA         VTD__DER_STA1 
     .           /
     .             VTD__DER_ST1X,
     .             VTD__DER_ST1Y,
     .             VTD__DER_ST1Z 
     .           / 
      DATA         VTD__DER_STA2 
     .           /
     .             VTD__DER_ST2X,
     .             VTD__DER_ST2Y,
     .             VTD__DER_ST2Z 
     .           / 
      DATA         VTD__DER_POS  
     .           /
     .             VTD__DER_POS1,
     .             VTD__DER_POS2,
     .             VTD__DER_POS3 
     .           / 
      DATA         VTD__DER_VEL  
     .           /
     .             VTD__DER_VEL1,
     .             VTD__DER_VEL2,
     .             VTD__DER_VEL3 
     .           / 
!
      CHARACTER  VTD__MG*1, VTD__GAL*1, VTD__SS*1, VTD__ES*1
      PARAMETER  ( VTD__MG  = 'M' ) ! Extra-galactic object
      PARAMETER  ( VTD__GAL = 'G' ) ! Galactic object
      PARAMETER  ( VTD__SS  = 'S' ) ! Near zone object in Solar system
      PARAMETER  ( VTD__ES  = 'E' ) ! Near zone object near Earth
!
      CHARACTER  MPL__LABEL*36
      PARAMETER  ( MPL__LABEL = 'MEAN_POLE file. Format of 2004.06.15' )
      INTEGER*4  M_MPL
      PARAMETER  ( M_MPL = 6 ) ! Number of data records in MPL file
!
! >>>>> End of include block for package VTD 
!
