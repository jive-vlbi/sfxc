!@This is the start of file &OBORS
!
!  OBORG is defined as a clone of oborg wih S_ prepended.
!
!     BEGIN 'OBSFIL' SPECIFICATION  01-DEC-2005 18:46:34
!
       INTEGER*2    S_OBORG_IFILL_LEN
       PARAMETER  ( S_OBORG_IFILL_LEN = 69 ) ! Length of secondary space
!
      REAL*8
     .         S_FJD,
     .         S_FRACT,
     .         S_FRACTC,
     .         S_DT,
     .         S_DOBS,
     .         S_DOBS_S,
     .         S_RT,
     .         S_ROBS,
     .         S_ROBS_S,
     .         S_TOTPH,
     .         S_TOTPH_S, &      ! Total phase (DEGREES!!!)
     .         S_DPH,
     .         S_DPH_S,
     .         S_DNB,
     .         S_DNB_S, &        ! Narrow-band delay for S-band (MICROSECONDS!!!)
     .         S_PHION,
     .         S_PHAMI8,
     .         S_PHAMI8_S, &     ! Phase ambiguity spacing (MICROSECONDS!!!)
     .         S_UT1_M_TAI,
!
     .         S_FREQ_SKY,
     .         S_ELEV(2),
     .         S_AZ(2),
     .         S_FAMB,
     .         S_FAMB_S, &       ! Group ambiguity spaing for S-band (sec)
     .         S_DERR,
     .         S_DERR_S,
     .         S_RERR,
     .         S_RERR_S,
     .         S_BP(3,2,2),
     .         S_SP(2,2),
     .         S_AP(2,2),
     .         S_AXOFP(2,2),
     .         S_ROTP(3,2),
     .         S_RELP(2),
     .         S_PRCP(2),
     .         S_NUTP(2,2),
     .         S_SNR,
     .         S_SNR_S, &        ! SNR
     .         S_TEMPC(2),
     .         S_ATMPR(2),
     .         S_RELHU(2),
     .         S_GIONSG(2),
     .         S_PHIONS,
     .         S_EFFREQ,
     .         S_EFFREQ_S,
     .         S_SECTAG,
     .         S_PCDLY(2),
     .         S_DNBER,
     .         S_DNBER_S,
     .         S_DPHER,
     .         S_DPHER_S,
     .         S_GION(2),
     .         S_CALIBS(2,2,MAX_CAL),
     .         S_CALIBZ(2,2,MAX_CLZ),
     .         S_CALIBB(2,MAX_CONT),
     .         S_X_POLE,
     .         S_Y_POLE,
     .         S_UT1_M_UT1R,
     .         S_PHEFFREQ,
     .         S_PHEFFREQ_S,
     .         S_REFFREQ,
     .         S_REFFREQ_S,
     .         S_AP_NMF_H(2,2),
     .         S_AP_NMF_W(2,2),
     .         S_DPH_ORIG,
     .         S_DPH_ORIG_S, &   ! Measured group delay for S-band (MICROSEC!!!)
     .         S_DOBS_ORIG,
     .         S_DOBS_ORIG_S, &  ! Measured group delay for S-band (MICROSEC!!!)
     .         S_SUN_GEOC(3,2),
     .         S_MOON_GEOC(3,2),
     .         S_TAU_ACM, &      ! Correction to theoretical delay due to
!                           ! a priori clock model which HAS BEEN added
!                           ! to theoretical delay (in sec)!!
     .         S_RATE_ACM, &     ! Correction to theoretical delay rate due to
!                           ! a priori clock model which HAS BEEN added
!                           ! to theoretical delay rate (dimensionless)!!
     .         S_AMPL, &         ! Fringe amplitude for X-band  (in range [0,1])
     .         S_AMPL_S, &       ! Fringe amplitude for S-band
     .         S_FEED_HORN, &    ! Feed_horn correction (rad)
     .         S_AGRAD_PART(2,2,2), &  ! Atmosphere gradient partials
     .         S_DPSI_APRIORI, &       ! Nutation PSI a priori value
     .         S_DEPS_APRIORI, &       ! Nutation EPS a priori value
     .         S_DPSI_IAU1980, &       ! Nutation PSI IAU 1980 theory value
     .         S_DEPS_IAU1980, &       ! Nutation EPS IAU 1980 theory value
     .         S_PART_DIST(2), &       ! Parial derivatives on distance
     .         S_CALIBM(6,M_CLM), &    ! Mode calibration
     .         S_ATM_ZENDEL(2),   &    ! Atmosphere zenith path delay
     .         S_UT1_RATE,    &   ! UT1 rate from Calc   sec/sec
     .         S_XP_RATE,     &   ! Xpole rate from Calc rad/sec
     .         S_YP_RATE          ! Ypole rate from Calc rad/sec
!
        CHARACTER  S_SCAN_NAME*10         ! Scan name
        CHARACTER  S_FRINGE_ROOT_FINAM*16 ! Filename of the root fringe file
        CHARACTER  S_FRINGE_X_FINAM*16    ! Filename of the fringe file X-band
        CHARACTER  S_FRINGE_S_FINAM*16    ! Filename of the fringe file S-band
!
        INTEGER*4  S_NPHAM4,   &  ! Number of phase delay ambiguities S-band
     .             S_NPHAM4_S, &  ! Number of phase delay ambiguities S-band
     .             S_AUTO_SUP, &  ! Automatic suppression bit field
     .             S_USER_SUP, &  ! User suppression bit field
     .             S_USER_REC     ! User recovery bit field
!
        INTEGER*2
     .             S_IFIRST_OBORG_I2,
     .             S_ISITE(2),
     .             S_ISTAR,
     .             S_IUNW,
     .             S_IUNWP,
     .             S_ICNCL,
     .             S_NUMAMB,
     .             S_NUMAMB_S, &     ! Number of group delay ambiguities for S-band
     .             S_IPNTR,
     .             S_IONFLG,
     .             S_NUMDB,
     .             S_IWVCOD(2),
     .             S_LQUAL,
     .             S_LQUAL_S,
     .             S_ICORR,
     .             S_IWVBIT1(2),
     .             S_IWVBIT2(2),
     .             S_NSCA, &         ! Index of the scan for this observation
     .             S_SUPSTAT(2), &   ! Suppression status
     .             S_UACSUP, &       ! User action for suppression
     .             S_OBORG_CALCV, &  ! Version of CALC multiplued by 1000
!
     .             S_CALIBM_AVAILABLE, &   ! Availability status of modal calibration
     .             S_CALIBM_APPLY, &       ! Applying status of modal calibration
     .             S_IFILL_OBORG(S_OBORG_IFILL_LEN),
     .             S_ILAST_OBORG_I2
!
!
      INTEGER*2    S_IOBSFIL(JOBSREC_BYTES) ! Copy of oborg record
      CHARACTER
     .             S_LQUAL_CHR*2,
     .             S_LQUAL_S_CHR*2
!
      EQUIVALENCE ( S_IOBSFIL, S_FJD )
      EQUIVALENCE ( S_IOBSFIL, S_IFIRST_OBORG_I2 )
      EQUIVALENCE ( S_LQUAL,   S_LQUAL_CHR )
      EQUIVALENCE ( S_LQUAL_S, S_LQUAL_S_CHR )
!
      COMMON / OBORG /
!     REAL*8
     .           S_FJD,        S_FRACT,       S_FRACTC,
     .           S_DT,         S_DOBS,        S_DOBS_S,
     .           S_RT,         S_ROBS,        S_ROBS_S,
     .           S_TOTPH,      S_TOTPH_S,     S_DPH,   S_DPH_S,  S_DNB, S_DNB_S,
     .           S_PHION,      S_PHAMI8,      S_PHAMI8_S,
     .           S_PHEFFREQ,   S_PHEFFREQ_S,  S_REFFREQ,   S_REFFREQ_S,
     .           S_FREQ_SKY,   S_ELEV,        S_AZ,
     .           S_FAMB,       S_FAMB_S,
     .           S_DERR,       S_DERR_S,      S_RERR,      S_RERR_S,
     .           S_BP,         S_SP,          S_AP,
     .           S_AXOFP,      S_ROTP,        S_RELP,      S_PRCP,      S_NUTP,
     .           S_SNR,        S_SNR_S,
     .           S_TEMPC,      S_ATMPR,       S_RELHU,
     .           S_GIONSG,     S_PHIONS,
     .           S_EFFREQ,     S_EFFREQ_S,
     .           S_SECTAG,     S_PCDLY,
     .           S_DNBER,      S_DNBER_S,     S_DPHER,     S_DPHER_S,
     .           S_UT1_M_UT1R, S_GION,
     .           S_CALIBS,     S_CALIBZ,      S_CALIBB,
     .           S_AP_NMF_H,   S_AP_NMF_W,
     .           S_SUN_GEOC,   S_MOON_GEOC,
     .           S_UACSUP,     S_SUPSTAT,
     .           S_AMPL,       S_AMPL_S,
     .           S_RATE_ACM,   S_FEED_HORN,
     .           S_TAU_ACM,    S_AGRAD_PART,
     .           S_DPSI_APRIORI,            S_DEPS_APRIORI,
     .           S_DPSI_IAU1980,            S_DEPS_IAU1980,
     .           S_PART_DIST,               S_CALIBM,
!
!     INTEGER*4
!
     .           S_NPHAM4,     S_NPHAM4_S,
!
!     INTEGER*2
!
     .           S_ISITE,       S_ISTAR,       S_IUNW,    S_IUNWP,     S_ICNCL,
     .           S_NUMAMB,      S_NUMAMB_S,
     .           S_IPNTR,       S_IONFLG,      S_NUMDB,
     .           S_IWVCOD,      S_LQUAL,       S_LQUAL_S,
     .           S_ICORR,       S_IWVBIT1,     S_IWVBIT2,
     .           S_UT1_M_TAI,   S_X_POLE,      S_Y_POLE,
     .           S_DPH_ORIG,    S_DPH_ORIG_S,  S_DOBS_ORIG,
     .           S_DOBS_ORIG_S, S_OBORG_CALCV, S_NSCA,
     .           S_CALIBM_AVAILABLE,           S_CALIBM_APPLY,
!
!       CHARACTER
!
     .           S_SCAN_NAME,
     .           S_FRINGE_ROOT_FINAM,  S_FRINGE_X_FINAM,  S_FRINGE_S_FINAM,
!
! -------------- tail variables
!
     .           S_ATM_ZENDEL, &          ! Real*8
     .           S_UT1_RATE,   S_XP_RATE,   S_YP_RATE,
     .           S_AUTO_SUP,   S_USER_SUP,  S_USER_REC,  
     .           S_IFILL_OBORG,
     .           S_ILAST_OBORG_I2
!
! ______ End of oborg common block ______
!
!
! --- Synomymos of archaic names
!
      REAL*8
     .          S_EFFREQ_XS,
     .          S_PHEFFREQ_XS,
     .          S_REFFREQ_XS,
     .          S_DOBSXS,
     .          S_DPHXS,
     .          S_DERRXS,
     .          S_DPHERXS,
     .          S_ROBSXS,
     .          S_RERRXS
      INTEGER*2
     .           S_LQUALXS
      CHARACTER  S_LQUALXS_CHR*2
!
      EQUIVALENCE ( S_EFFREQ_S,   S_EFFREQ_XS   )
      EQUIVALENCE ( S_PHEFFREQ_S, S_PHEFFREQ_XS )
      EQUIVALENCE ( S_REFFREQ_S,  S_REFFREQ_XS  )
      EQUIVALENCE ( S_DOBS_S,     S_DOBSXS      )
      EQUIVALENCE ( S_DPH_S,      S_DPHXS       )
      EQUIVALENCE ( S_DERR_S,     S_DERRXS      )
      EQUIVALENCE ( S_DPHER_S,    S_DPHERXS     )
      EQUIVALENCE ( S_ROBS_S,     S_ROBSXS      )
      EQUIVALENCE ( S_RERR_S,     S_RERRXS      )
      EQUIVALENCE ( S_LQUAL_S,    S_LQUALXS     )
      EQUIVALENCE ( S_LQUAL_S,    S_LQUALXS_CHR )
!
!     END 'OBSFIL' SPECIFICATIONS
