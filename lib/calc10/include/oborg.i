!@This is the start of file &OBORG
!
!   IF you update this file, you must also update obors.i
!
!***** mwh 940503  Expanded from 1024 to 1536 words to make room for
!*****              more contributions
!      kdb 960311  Add sun and moon geocentric coordinates.
!      pet 980309  Added missed stuff the S-band
!      pet 980325  Added TAU_ACM and RATE_ACM. Made them equivalent with
!                  some field of IFILL
!      pet 980330  Added NSCA
!      pet 1999.11.09  Added parameter OBORG_IFILL_LEN
!                      Added variables OBORG_CALCV, AGRAD_PART,
!                            DPSI_APRIORI, DEPS_APRIORI, DPSI_IAU1980,
!                            DEPS_IAU1980, PART_DIST, CALIBM, CALIBM_AVAILABLE,
!                            CALIBM_APPLY
!                      Added common block OBORG_EXT
!      pet 1999.11.11  Corrected an old bug: OBORG_IFILL_LEN  should be 92,
!                      not 95
!      pet 2000.06.13  Massive update:
!
!                      removed: TIDP(3,2,2), NPPRIN(2,2,2), NPSEMA(2,2,2),
!                               NPSEMM(2,2,2), NPDECA(2,2,2), NPANNU(2,2,2),
!                               NP122D(2,2,2)
!
!                      added:   SCAN_NAME,
!                               FRINGE_ROOT_FINAM,
!                               FRINGE_X_FINAM,
!                               FRINGE_S_FINAM
!
!                      legalized: SNR_S, UACSUP, SUPSTAT, AMPL, TOTPH_S,
!                                 DPH_ORIG_S, PHAMI8_S, DNB_S, RATE_ACM,
!                                 FEED_HORN, DOBS_ORIG_S, TAU_ACM, AMPL_S,
!                                 FAMB_S, AGRAD_PART, DPSI_APRIORI,
!                                 DEPS_APRIORI, DPSI_IAU1980, DEPS_IAU1980,
!                                 PART_DIST, CALIBM
!
!      pet 2002.03.05  Added array ATM_ZENDEL which will keep atmoopshere
!                      path delay
!      pet 2005.11.29  Added UT1_RATE, XP_RATE, YP_RATE which keep EOP rate
!                      computed by Calc.
!      pet 2005.12.01  Added Post-NOV2005 bit fields for suppression status:
!                      AUTO_SUP, USER_SUP, USER_REC
!
!CCCCCC
!
!     BEGIN 'OBSFIL' SPECIFICATION  01-DEC-2005 18:44:21
!
      INTEGER*2    OBORG_IFILL_LEN
      PARAMETER  ( OBORG_IFILL_LEN = 69 ) ! Length of secondary space in 2-byte
!                                         ! words
      REAL*8
     .         FJD,
     .         FRACT,
     .         FRACTC,
     .         DT,
     .         DOBS,
     .         DOBS_S,
     .         RT,
     .         ROBS,
     .         ROBS_S,
     .         TOTPH,
     .         TOTPH_S, &      ! Total phase (DEGREES!!!)
     .         DPH,
     .         DPH_S,
     .         DNB,
     .         DNB_S, &        ! Narrow-band delay for S-band (MICROSECONDS!!!)
     .         PHION,
     .         PHAMI8,
     .         PHAMI8_S, &     ! Phase ambiguity spacing (MICROSECONDS!!!)
     .         UT1_M_TAI,
!
     .         FREQ_SKY,
     .         ELEV(2),
     .         AZ(2),
     .         FAMB,
     .         FAMB_S, &       ! Group ambiguity spacing for S-band (sec)
     .         DERR,
     .         DERR_S,
     .         RERR,
     .         RERR_S,
     .         BP(3,2,2),
     .         SP(2,2),
     .         AP(2,2),
     .         AXOFP(2,2),
     .         ROTP(3,2),
     .         RELP(2),
     .         PRCP(2),
     .         NUTP(2,2),
     .         SNR,
     .         SNR_S, &        ! at S-band
     .         TEMPC(2),
     .         ATMPR(2),
     .         RELHU(2),
     .         GIONSG(2),
     .         PHIONS,
     .         EFFREQ,
     .         EFFREQ_S,
     .         SECTAG,
     .         PCDLY(2),
     .         DNBER,
     .         DNBER_S,
     .         DPHER,
     .         DPHER_S,
     .         GION(2),
     .         CALIBS(2,2,MAX_CAL),
     .         CALIBZ(2,2,MAX_CLZ),
     .         CALIBB(2,MAX_CONT),
     .         X_POLE,
     .         Y_POLE,
     .         UT1_M_UT1R,
     .         PHEFFREQ,
     .         PHEFFREQ_S,
     .         REFFREQ,
     .         REFFREQ_S,
     .         AP_NMF_H(2,2),
     .         AP_NMF_W(2,2),
     .         DPH_ORIG,
     .         DPH_ORIG_S, &   ! Measured group delay for S-band (MICROSEC!!!)
     .         DOBS_ORIG,
     .         DOBS_ORIG_S, &  ! Measured group delay for S-band (MICROSEC!!!)
     .         SUN_GEOC(3,2),
     .         MOON_GEOC(3,2),
     .         TAU_ACM, &      ! Correction to theoretical delay due to
!                           ! a priori clock model which HAS BEEN added
!                           ! to theoretical delay (in sec)!!
     .         RATE_ACM, &     ! Correction to theoretical delay rate due to
!                           ! a priori clock model which HAS BEEN added
!                           ! to theoretical delay rate (dimensionless)!!
     .         AMPL, &         ! Fringe amplitude for X-band  (in range [0,1])
     .         AMPL_S, &       ! Fringe amplitude for S-band
     .         FEED_HORN, &    ! Feed_horn correction (rad)
     .         AGRAD_PART(2,2,2), &  ! Atmosphere gradient partials
     .         DPSI_APRIORI, &       ! Nutation PSI a priori value
     .         DEPS_APRIORI, &       ! Nutation EPS a priori value
     .         DPSI_IAU1980, &       ! Nutation PSI IAU 1980 theory value
     .         DEPS_IAU1980, &       ! Nutation EPS IAU 1980 theory value
     .         PART_DIST(2), &       ! Parial derivatives on distance
     .         CALIBM(6,M_CLM), &    ! Mode calibration
     .         ATM_ZENDEL(2),   &    ! Zenith troposphere path delay
     .         UT1_RATE,    &   ! UT1 rate from Calc   sec/sec
     .         XP_RATE,     &   ! Xpole rate from Calc rad/sec
     .         YP_RATE          ! Ypole rate from Calc rad/sec
!
        CHARACTER  SCAN_NAME*10         ! Scan name
        CHARACTER  FRINGE_ROOT_FINAM*16 ! Filename of the root fringe file
        CHARACTER  FRINGE_X_FINAM*16    ! Filename of the fringe file X-band
        CHARACTER  FRINGE_S_FINAM*16    ! Filename of the fringe file S-band
!
        INTEGER*4  NPHAM4,   &    ! Number of phase delay ambiguities S-band
     .             NPHAM4_S, &    ! Number of phase delay ambiguities S-band
     .             AUTO_SUP, &    ! Automatic suppression bit field
     .             USER_SUP, &    ! User suppression bit field
     .             USER_REC       ! User recovery bit field
!
        INTEGER*2
     .             IFIRST_OBORG_I2,
     .             ISITE(2),
     .             ISTAR,
     .             IUNW,
     .             IUNWP,
     .             ICNCL,
     .             NUMAMB,
     .             NUMAMB_S, &     ! Number of group delay ambiguities for S-band
     .             IPNTR,
     .             IONFLG,
     .             NUMDB,
     .             IWVCOD(2),
     .             LQUAL,
     .             LQUAL_S,
     .             ICORR,
     .             IWVBIT1(2),
     .             IWVBIT2(2),
     .             NSCA, &         ! Index of the scan for this observation
     .             SUPSTAT(2), &   ! Suppression status
     .             UACSUP, &       ! User action for suppression
     .             OBORG_CALCV, &  ! Version of CALC multiplued by 1000
!
     .             CALIBM_AVAILABLE, &   ! Availability status of modal calibration
     .             CALIBM_APPLY, &       ! Applying status of modal calibration
!
     .             IFILL_OBORG(OBORG_IFILL_LEN),
     .             ILAST_OBORG_I2
!
!
      INTEGER*2    IOBSFIL(JOBSREC_BYTES) ! Copy of oborg record
      CHARACTER
     .             LQUAL_CHR*2,
     .             LQUAL_S_CHR*2
!
      EQUIVALENCE ( IOBSFIL, FJD )
      EQUIVALENCE ( IOBSFIL, IFIRST_OBORG_I2 )
      EQUIVALENCE ( LQUAL,   LQUAL_CHR )
      EQUIVALENCE ( LQUAL_S, LQUAL_S_CHR )
!
      SAVE   / OBORG  /
      COMMON / OBORG /
!     REAL*8
     .           FJD,        FRACT,       FRACTC,
     .           DT,         DOBS,        DOBS_S,
     .           RT,         ROBS,        ROBS_S,
     .           TOTPH,      TOTPH_S,     DPH,       DPH_S,    DNB,    DNB_S,
     .           PHION,      PHAMI8,      PHAMI8_S,
     .           PHEFFREQ,   PHEFFREQ_S,  REFFREQ,   REFFREQ_S,
     .           FREQ_SKY,   ELEV,        AZ,
     .           FAMB,       FAMB_S,
     .           DERR,       DERR_S,      RERR,      RERR_S,
     .           BP,         SP,          AP,
     .           AXOFP,      ROTP,        RELP,      PRCP,     NUTP,
     .           SNR,        SNR_S,
     .           TEMPC,      ATMPR,       RELHU,
     .           GIONSG,     PHIONS,
     .           EFFREQ,     EFFREQ_S,
     .           SECTAG,     PCDLY,
     .           DNBER,      DNBER_S,     DPHER,     DPHER_S,
     .           UT1_M_UT1R, GION,
     .           CALIBS,     CALIBZ,      CALIBB,
     .           AP_NMF_H,   AP_NMF_W,
     .           SUN_GEOC,   MOON_GEOC,
     .           UACSUP,     SUPSTAT,
     .           AMPL,       AMPL_S,
     .           RATE_ACM,   FEED_HORN,
     .           TAU_ACM,    AGRAD_PART,
     .           DPSI_APRIORI,            DEPS_APRIORI,
     .           DPSI_IAU1980,            DEPS_IAU1980,
     .           PART_DIST,               CALIBM,
!
!     INTEGER*4
!
     .           NPHAM4,      NPHAM4_S,
!
!     INTEGER*2
!
     .           ISITE,       ISTAR,       IUNW,        IUNWP,    ICNCL,
     .           NUMAMB,      NUMAMB_S,
     .           IPNTR,       IONFLG,      NUMDB,
     .           IWVCOD,      LQUAL,       LQUAL_S,
     .           ICORR,       IWVBIT1,     IWVBIT2,
     .           UT1_M_TAI,   X_POLE,      Y_POLE,
     .           DPH_ORIG,    DPH_ORIG_S,  DOBS_ORIG,
     .           DOBS_ORIG_S, OBORG_CALCV, NSCA,
     .           CALIBM_AVAILABLE,         CALIBM_APPLY,
!
!       CHARACTER
!
     .           SCAN_NAME,
     .           FRINGE_ROOT_FINAM,    FRINGE_X_FINAM,   FRINGE_S_FINAM,
!
! -------------- tail variables
!
     .           ATM_ZENDEL, &       ! Real*8
     .           UT1_RATE,    XP_RATE,     YP_RATE,
     .           AUTO_SUP,   USER_SUP,  USER_REC,  
     .           IFILL_OBORG,
     .           ILAST_OBORG_I2
!
! ______ End of oborg common block ______
!
!
! --- Synonymos of archaic names
!
      REAL*8
     .          EFFREQ_XS,
     .          PHEFFREQ_XS,
     .          REFFREQ_XS,
     .          DOBSXS,
     .          DPHXS,
     .          DERRXS,
     .          DPHERXS,
     .          ROBSXS,
     .          RERRXS
      INTEGER*2
     .           LQUALXS
      CHARACTER  LQUALXS_CHR*2
!
      EQUIVALENCE ( EFFREQ_S,   EFFREQ_XS   )
      EQUIVALENCE ( PHEFFREQ_S, PHEFFREQ_XS )
      EQUIVALENCE ( REFFREQ_S,  REFFREQ_XS  )
      EQUIVALENCE ( DOBS_S,     DOBSXS      )
      EQUIVALENCE ( DPH_S,      DPHXS       )
      EQUIVALENCE ( DERR_S,     DERRXS      )
      EQUIVALENCE ( DPHER_S,    DPHERXS     )
      EQUIVALENCE ( ROBS_S,     ROBSXS      )
      EQUIVALENCE ( RERR_S,     RERRXS      )
      EQUIVALENCE ( LQUAL_S,    LQUALXS     )
      EQUIVALENCE ( LQUAL_S,    LQUALXS_CHR )
!
!     END 'OBSFIL' SPECIFICATIONS
!
!     Oborg variable definition:
!     FJD      - Julian date of midnight prior (or at) this obs. (days)
!     FRACT    - UTC fraction of day for this obs. (days)
!     FRACTC   - Coordinate time fraction of day for this obs. (days)
!     DT       - Delay theoretical. (MICROSECONDS!!!)
!     DOBS     - Delay observation. (MICROSECONDS!!!)
!     RT       - Rate theoretical. (s/s)
!     ROBS     - Rate observation. (s/s)
!     TOTPH    - Total phase observaton (????)
!     DPH      - Phase delay observation (microseconds)
!     DNB      - Narrow band delay (microseconds)
!     PION     - Phase delay ionosphere correction (s).
!     PHAMI8   - Phase delay ambiguitiy spacing (MICROSECONDS!!!).
!     DPHSB    - Single band phase delay (not currently used.)
!     DOBSXS   - S or X matching delay to DOBS.
!     DPHXS    - S or X matching delay to DPH.
!     FREQ_SKY - Reference frequency (MHz).
!     ELEV(2)  - Source elevations and rates at two sites (radians).
!     AZ(2)    - Source azimuths and rates at two sites (radians).
!     FAMB     - Group delay ambiguity (s).
!     DERR     - Group delay formal error from correlator (s).
!     RERR     - Phase delay rate formal error from correlator (s/s).
!     BP(3,2,2)        - Site partials.
!     SP(2,2)          - Source partials.
!     AP(2,2)          - Troposhere partials.
!     AXOFP(2,2)       - Axis offset partials.
!     ROTP(3,2)        - Earth rotation partials.
!     RELP(2)          - General relativity (gamma) partials.
!     PRCP(2)          - Precession partials.
!     NUTP(2,2)        - Nutation offset partials.
!     SNR              - Signal to noise ratio.
!     TEMPC(2)         - Temperature at two sites (K)
!     ATMPR(2)         - Atmosphere pressure at two sites (millibars)
!     RELHU(2)         - Relative humidity at two sites (%)
!     GIONSG(2)        - Group-based ionospheric correction errors (del,rate)
!     PHIONS           - Phase delay ionosphere sigma.
!     EFFREQ           - The effective freq for computing the ion corr. (MHz)
!     SECTAG           - Seconds part of time tag.
!     PCDLY(2)
!     DNBER            - Narrow band delay formal error (s).
!     DPHER            - Phase delay formal sigma (s).
!     DPHSER           - (No longer in list. ut1_m_ut1r replaced it.)
!     GION(2)          - Group-based ionospheric correction (delay and rate).
!     DERRXS           - Formal error in DOBSXS
!     DPHERSX          - Formal error in DPHXS.
!     CALIBS(2,2,10)   - Used by CORFIL system.
!     CALIBZ(2,2,10)     - Same
!     CALIBB(2,max_cont) - Same
!     CALIBM(6,5)      - Same
!     NPHAM4           - Phase delay ambiguity count.
!     ISITE(2)         - Solve Intermal site numbers.
!     ISTAR            - Solve internal source number
!     IUNW             - Observation weight flag.
!     ICNCL
!     NUMAMB           - Group delay ambiguity counter.
!     IPNTR            - Pointer for difference processing (obsolete).
!     IONFLG           - Old ion status flag.
!     NUMDB            - Solve internal database number for this obs.
!     IWVCOD           - WVR code.
!     LQUAL            - Correlator quality code.
!     ICORR            - New ionosphere flag array. Repalces IONFLG.
!     IWVBIT1          - wvr stuff.
!     IWVBIT2          - wvr stuff.
!     ut1_m_tai        - Interpolated value of 'ut1-tai'. (s)
!     x_pole           - Interpolated value of the x-pole (0".001)
!     y_pole           - Interpolated value of the y-pole (0".001)
!     ut1_m_ut1r       - 'ut1 - ut1r' (s)
!     sun_geoc         - geocentric Sun coordinates
!     moon_geoc        - geocentric Moon coordinates
!     ATM_ZENDEL       - atmopshere zenith path delay in secs
!     UT1_RATE         - Calc UT1 rate (sec/sec)
!     XP_RATE          - Calc X pole rate (rad/sec)
!     YP_RATE          - Calc Y pole rate (rad/sec)
!     AUTO_SUP         - Bit field with status of automatic suppression 
!     USER_SUP         - Bit field with status of user defined suppression flags
!     USER_REC         - Bit field with status of user defined recovery flags
