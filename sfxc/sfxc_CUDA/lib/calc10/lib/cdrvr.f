      SUBROUTINE DRIVR()
      IMPLICIT None
!
! 1.    DRIVR
!
! 1.1   DRIVR PROGRAM SPECIFICATION
!
! 1.1.1 DRIVR is the main calculation subroutine. It calculates the theoretical
!       delays and delay rates, the contributions of each model module to the
!       delays and delay rates, the partials of the delays and delay rates with
!       respect to model module parameters, and the coordinate time at site #1.
!
! 1.1.2 RESTRICTIONS - NONE
!
! 1.1.3 REFERENCES - PEP MANUAL, GREENBOOK, D. ROBERTSON'S THESIS,
!                    P. McCLURES X-DOCUMENT.
!
! 1.2   DRIVR PROGRAM INTERFACE
!
! 1.2.1 CALLING SEQUENCE - CALL DRIVR
!       INPUT VARIABLES  -NONE
!       OUTPUT VARIABLES - NONE
!
! 1.2.2 COMMON BLOCKS USED - NONE
!
! 1.2.3 PROGRAM SPECIFICATIONS -
      REAL*8 EPSMD, CENT, DUT1P(2,2)
      Real*8 AXOFF(2), CFBASE(3), CFLAT(3,2), CFLON(3,2), PANGL(2),
     .       CFSITE(3,2), CFSITN(3,2), DIONC(2),
     .       EARTH(3,3), EPBASE(3,2), SUN(3,2),
     .       EPLATP(3,2), EPLATV(3,2), EPLONP(3,2), EPLONV(3,2),
     .       EPS(2), EPSITN(3,2), OCEAMP(11,3,2),
     .       OCEPHS(11,3,2), SITEA(3,2),SITEP(3,2),
     .       SITEV(3,2), SITLAT(2), SITLON(2), SITRAD(2), STAR(3),
     .       SUNCU(3), TCTOCF(3,3,2), DATMC(2,2), ZPATH(2),
     .       TIDEP(3,2), TIDEV(3,2), USITEP(3,2), USITEV(3,2),
     .       XLOADP(3,2), XLOADV(3,2), XMOON(3,2), DAXOC(2,2),
     .       POLTDP(3,2), POLTDV(3,2), AZ(2,2), ELEV(2,2),
     .       SITHEIGHT(2), DSTRP(2,2), RTTOCF(3,3,2), GEOLAT(2),
     .       AXTILT(2,2), ROTAXIS(3,3,2)
      Real*8 AXIS2000(3,2), DAXIS2000(3,2), STAR_ABERRATED(3,2),
     .       dATMCdh(2,2)
!    .       dATMCdh(2,2), CITEA(3,2), UCITEP(3,2), UCITEV(3,2),
!    .       ,D2003P(3,2), D2003V(3,2), D2003A(3,2)
      Real*8 UTC, XJD, AT, DUTCAT, CT, DATDCT, DLPGR, DUT1AT, UT1,
     .       EPSMNR, DIURNV, WOBXR, WOBYR, CD, CRA, SD, SRA,
     .       NUTDIF(2,2), SJD, TJD, OBSDIF
      Real*8 WOBXD,WOBYD,THETA,RPOM(3,3),RBPN(3,3),RT2C(3,3),
     .       RBPNC(3,3),RT2CC(3,3)
      Real*8 DAS2R, APC2R, RPNKK(3,3,2), DR1(3,3), DR2(3,3)
      Real*8 RPN2K(3,3,2),X,Y,S, ERA2K,DERA2K,RS2K(3,3,3),
     .       SP,DSP,RW2K(3,3,2),R2K(3,3,3), RPC2K(3,3,2), FA2K(14),
     .       FAD2K(14), GAST2K(2), RSC2K(3,3,3), RNC2K(3,3,2),
     .       RC2K (3,3,3), DPSI2K(2), DEPS2K(2), EPSA(2), GMST2K(2),
     .       RFR2K(3,3), pERA2K, DNUpe(2,2), Xn(2), Yn(2), Sn(2)
      Real*8 Xti, Yti, UT1t, dXti, dYti, dUT1t
!     Real*8 UT1td
!     Real*8 R2Kdif(3,3)
      Integer*2 KAXIS(2)
      Integer*4 TSKIP, I, J
!
      DATA SJD /-999.D6/
!
! 1.2.3.1   SAVE BLOCK -
       SAVE SITEA, SITEP,
     .      SITEV, SITLAT, STAR, SUNCU, TCTOCF, TIDEP, SITRAD,
     .      TIDEV, XLOADP, XLOADV, ZPATH, EPS, DSTRP,
     .      POLTDP, POLTDV, SUN, AXOFF, CFBASE, DIURNV, DLPGR, EPBASE,
     .      CFSITE, CFSITN, CFLON, CFLAT, SITLON, OCEAMP, OCEPHS,
     .      PANGL, AZ, ELEV, KAXIS, EARTH, EPSMNR, STAR_ABERRATED,
     .      EPSMD, DUT1P, XMOON, SITHEIGHT, FA2K, FAD2K,
     .      NUTDIF, XJD, CT, SJD, TJD, OBSDIF, CENT, UT1, DUT1AT,
     .      RTTOCF, GEOLAT, WOBXR, WOBYR, UTC, AT, DUTCAT, DATDCT,
     .      RPN2K, Xn, Yn, Sn, ERA2K, RS2K, SP, RW2K,R2K,
     .      WOBXD, WOBYD, DERA2K, RPC2K, GAST2K, RSC2K,
     .      RNC2K, RC2K, RFR2K, DPSI2K, DEPS2K, GMST2K, pERA2K, EPSA,
     .      AXTILT, ROTAXIS, DNUpe, Xti, Yti, UT1t, dXti, dYti,
     .      dUT1t
!
! 1.2.4 DATA BASE ACCESS - NONE
!
! 1.2.5 EXTERNAL INPUT/OUTPUT - NONE
!
! 1.2.6 SUBROUTINE INTERFACE -
!             CALLER SUBROUTINES: MAIN
!             CALLED SUBROUTINES: INITL, OBSNT, START, TOCUP, WRIDR,
!                                 ATIME, ATMG, AXOG, CTIMG, RMPAR,
!                                 DIURNL, ETDG, M2000, NUTG, OCEG,
!                                 PEP, PREG, PTDG, RELG, ROSIT, SITG,
!                                 SITCOR, STRCOR, STRG, SUNCOR, UT1G,
!                                 UTCTME, WOBG, PLXG, ATMP, AXOP, ETDP,
!                                 NUTP, OCEP, PREP, RELP, SITP, STRP,
!                                 UT1P, WOBP, PLXP, PTDP, ATMC, AXOC,
!                                 ETDC, OCEC, PTDC, RELC, CSTAR, WOBC
!
! 1.2.7 CONSTANTS USED - NONE
!
! 1.2.8 PROGRAM VARIABLES -
!             1. AXOFF(2)      - THE ANTENNA AXIS OFFSETS AT EACH OBSERVATION
!                                SITE.  (M)
!             2. CFBASE(3)     - THE GEOCENTRIC CRUST FIXED BASELINE VECTOR. (M)
!             3. DATMC(2,2)    - THE CONTRIBUTIONS TO THE DELAY AND DELAY
!                                RATE DUE TO TROPOSPHERIC REFRACTION AT EACH
!                                OBSERVATION SITE. (SEC, SEC/SEC)
!             4. DAXOC(2,2)    - THE CONTRIBUTIONS TO THE DELAY AND DELAY
!                                RATE DUE TO THE ANTENNA AXIS OFFSETS AT EACH
!                                OBSERVATION SITE. (SEC, SEC/SEC)
!             5. DEPS2K(2)     - THE NUTATION IN OBLIQUITY AND ITS CT TIME
!                                DERIVATIVE COMPUTED FROM THE IAU2000A MODEL.
!                                (RAD, RAD/SEC)
!             6. DPSI2K(2)     - THE NUTATION IN LONGITUDE AND ITS CT TIME
!                                DERIVATIVE COMPUTED FROM THE IAU2000A MODEL.
!                                (RAD, RAD/SEC)
!             7. DIONC(2)      - THE CONTRIBUTIONS TO THE DELAY AND DELAY
!                                RATE DUE TO IONOSPHERE EFFECTS. (SEC, SEC/SEC)
!                                Dummy variables!
!             8. DIURNV        - THE DIURNAL ANGULAR VELOCITY OF THE EARTH.
!                                (RAD/SEC)
!                                --- No longer computed !!!!
!             9. DLPGR         - THE CT TIME DERIVATIVE OF THE LONG PERIOD
!                                TERMS IN THE 'AT MINUS CT' OFFSET. (SEC/SEC)
!            10. EARTH(3,3)    - THE SOLAR SYSTEM BARYCENTRIC EARTH POSITION,
!                                VELOCITY, AND ACCELERATION VECTORS.
!                                (M, M/SEC, M/SEC**2)
!            11. EPBASE(3,2)   - THE J2000.0 GEOCENTRIC BASELINE POSITION AND
!                                VELOCITY VECTORS. (M, M/SEC)
!            12. EPS(2)        - THE TRUE OBLIQUITY OF THE ECLIPTIC AND ITS CT
!                                TIME DERIVATIVE. (RAD, RAD/SEC)
!            13. EPSMNR        - MEAN OBLIQUITY AT EPOCH J2000.0. (RAD)
!            14. EPSMD         - Mean obliquity of date (radians)
!            15. FA2K(14)      - The Fundamental arguments (5 Luni-solar,
!                                8 planetary, accumulated precession)
!                                (Radians)
!            16. FAD2K(14)     - Time derivative of the fundamental arguments
!                                (Radians/second)
!            17. CENT          - Number of Julian centuries elapsed since the
!                                epoch January 1.5, 2000. (centuries)
!            18. KEND          - THE 'END OF DATA' FLAG. KEND = 0 IF THERE IS
!                                MORE DATA TO BE PROCESSED. KEND = 1 IF THE END
!                                OF THE DATA HAS BEEN REACHED.
!            19. KOUNT         - THE FLAG WHICH INITIALIZES THE COUNTING OF THE
!                                OBSERVATION ITEMS.
!            20. PANGL(2)      - THE PARALLACTIC ANGLE DUE TO FEED BOX ROTATION
!                                AT EACH OBSERVATION SITE. (RAD)
!            21. POLTDP(3,2)   - GEOCENTRIC J2000.0 SITE POSITION CORRECTION FOR
!                                THE EFFECTS OF THE POLE TIDE. (M)
!            22. POLTDV(3,2)   - GEOCENTRIC J2000.0 SITE VELOCITY CORRECTION FOR
!                                THE EFFECTS OF THE POLE TIDE. (M/SEC)
!            23. RPC2K(3,3,2)  - THE PRECESSION PORTION OF THE COMPLETE CRUST
!                                FIXED TO J2000.0 ROTATION MATRIX AND ITS CT
!                                TIME DERIVATIVE, consistent with the IERS
!                                Conventions (2003). (UNITLESS, 1/SEC)
!            24. RS2K(3,3,3)   - THE DIURNAL SPIN PORTION OF THE COMPLETE CRUST
!                                FIXED TO J2000.0 ROTATION MATRIX AND ITS FIRST
!                                TWO CT TIME DERIVATIVES, consistent with the
!                                IERS Conventions (2003) - CEO based version.
!                                (UNITLESS, 1/SEC, 1/SEC**2)
!            25. RSC2K(3,3,3)  - THE DIURNAL SPIN PORTION OF THE COMPLETE CRUST
!                                FIXED TO J2000.0 ROTATION MATRIX AND ITS FIRST
!                                TWO CT TIME DERIVATIVES, consistent with the
!                                IERS Conventions (2003) - Classical version.
!                                (UNITLESS, 1/SEC, 1/SEC**2)
!            26. RW2K(3,3,2)   - THE WOBBLE PORTION OF THE COMPLETE CRUST FIXED
!                                TO J2000.0 ROTATION MATRIX and its time
!                                derivative, consistent with the IERS
!                                Conventions (2003). (unitless, 1/sec)
!            27. SITEA(3,2)    - THE J2000.0 GEOCENTRIC ACCELERATION VECTORS OF
!                                EACH OBSERVATION SITE. (M/SEC**2)
!            28. SITEP(3,2)    - THE J2000.0 GEOCENTRIC POSITION VECTORS OF EACH
!                                OBSERVATION SITE. (M)
!            29. SITEV(3,2)    - THE J2000.0 GEOCENTRIC VELOCITY VECTORS OF EACH
!                                OBSERVATION SITE. (M/SEC)
!            30. SITLAT(2)     - THE SITE GEODETIC LATITUDES. (RAD)
!            31. SITLON(2)     - The site East longitudes. (rad)
!            32. SITHEIGHT(2)  - The site heights above the geoid. (m)
!            33. STAR(3)       - THE J2000.0 SOURCE UNIT VECTOR. (UNITLESS)
!            34. SUN(3,2)      - THE J2000.0 GEOCENTRIC SUN POSITION AND
!                                VELOCITY VECTORS. (M, M/SEC)
!            35. TCTOCF(3,3,2) - THE ROTATION MATRIX WHICH ROTATES THE
!                                TOPOCENTRIC REFERENCE SYSTEM TO THE CRUST FIXED
!                                REFERENCE SYSTEM AT EACH OBSERVATION SITE.
!            36. TIDEP(3,2)    - THE CORRECTIONS TO THE J2000.0 GEOCENTRIC SITE
!                                POSITION VECTORS DUE TO EARTH TIDE EFFECTS. (M)
!            37. TIDEV(3,2)    - THE CORRECTIONS TO THE J2000.0 GEOCENTRIC SITE
!                                VELOCITY VECTORS DUE TO EARTH TIDES. (M/SEC)
!            38. WOBX          - THE LONG PERIOD WOBBLE X-OFFSET. (RAD)
!            39. WOBY          - THE LONG PERIOD WOBBLE Y-OFFSET. (RAD)
!                                (NOTE: WOBY IS LEFT HANDED.)
!            40. XLOADP(3,2)   - THE CORRECTIONS TO THE J2000.0 GEOCENTRIC SITE
!                                POSITION VECTORS DUE TO OCEAN LOADING. (M)
!            41. XLOADV(3,2)   - THE CORRECTIONS TO THE J2000.0 GEOCENTRIC SITE
!                                VELOCTY VECTORS DUE TO OCEAN LOADING. (M/SEC)
!            42. ZPATH(2)      - THE ZENITH ELECTRICAL PATH LENGTH AT EACH
!                                OBSERVATION SITE. (SEC)
!            43. STAR_ABERRATED(3,2) - THE J2000.0 SOURCE UNIT VECTOR AT EACH
!                                SITE CORRECTED FOR ABERRATION. (UNITLESS)
!            44. axis2000(3,2) - Vector axis offset of antenna in the J2000.0
!                                frame (effect on baseline). First index is
!                                X,Y,Z (meters), second runs over sites.
!            45. daxis2000(3,2) -Time derivative of axis2000, rate of change
!                                of vector axis offset of antenna in the
!                                J2000.0 frame (effect on baseline). First
!                                index is velocity, second runs over sites.
!            46. ELEV(2,2)     - The elevation angle of the source corrrected
!                                for aberration and its CT time derivative at
!                                each site (rad,rad/sec)
!            47. AZ(2,2)       - The azimuth angle of the source corrrected
!                                for aberration and its CT time derivative
!                                at each site (rad,rad/sec)
!            48. DSTRP(2,2)    - Partial derivatives of the delay and delay
!                                rate with respect to source RA and Dec. First
!                                runs over RA and Dec, second runs over delay
!                                and delay rate. (sec/rad, sec/sec-rad
!            49. NUTDIF(2,2)   - Nutation difference: IAU1980 minus IAU2000A,
!                                with frame bias and rate difference.
!                                First index over psi and epsilon; second
!                                index over difference and derivative of
!                                difference. (radians, radians/sec)
!            50. DNUTP(2,2)    - PARTIAL DERIVATIVES OF THE DELAY AND THE DELAY
!                                RATE W.R.T DPSI2K AND DEPS2K. (SEC/RAD,
!                                SEC/SEC/RAD)
!            51. RTTOCF(3,3,2) - The rotation matrix which rotates the
!                                'radial-transverse' reference system to the
!                                crust fixed reference system at each site.
!            52. GEOLAT(2)     - The geocentric latitude at each site. (rad)
!            53. SJD           - Time of the previous observation.
!            54. XJD           - The Julian Date at zero hours UTC of the
!                                observation.
!            55. UTC           - UTC time fraction of the UTC day.
!            56. RPN2K(3,3,2)  - The Bias Precession Nutation portion of
!                                the complete Fixed to J2000.0 rotation
!                                matrix and its CT time derivative,
!                                consistent with the IERS Conventions
!                                (2003). (unitless, 1/sec)
!            57. RNC2K(3,3,2)  - The IAU200A Nutation portion of
!                                the complete Fixed to J2000.0 rotation
!                                matrix and its CT time derivative,
!                                consistent with the IERS Conventions
!                                (2003). (unitless, 1/sec)
!            58. Xn(2)         - X-component of the CIP (Celestial
!                                Intermediate Pole) in the GCRS (Geocentric
!                                Celestial Reference System), and its time
!                                derivative. (Radians, Radians/sec)
!            59. Yn(2)         - Y-component of the CIP (Celestial
!                                Intermediate Pole) in the GCRS (Geocentric
!                                Celestial Reference System), and its time
!                                derivative. (Radians, Radians/sec)
!            60. Sn(2)         - Position of the CEO (Celestial Ephemeris
!                                Origin) on the equator of the CIP, and its
!                                time derivative. (Radians, Radians/sec)
!            61. ERA2K         - The Earth Rotation Angle, angle between
!                                the CEO Celestial Ephemeris Origin) and
!                                the TEO (Terrestrial Ephemeris Origin)
!                                on the equator of the the CIP at the
!                                observation epoch. (Radians)
!            62. DERA2K        - Time derivative of ERA2K (Radians/sec)
!            63. SP            - S-prime, position of the TEO (Terrestrial
!                                Ephemeris Origin) on the equator of the
!                                CIP. (Radians)
!            64. DSP           - Time derivative of SP. (Radians/sec)
!            65. R2K(3,3,3)    - THE COMPLETE CRUST FIXED TO J2000.0 ROTATION
!                                MATRIX AND ITS FIRST TWO CT TIME DERIVATIVES.
!                                CEO-based version. (UNITLESS, 1/SEC, 1/SEC**2)
!            66. RC2K(3,3,3)   - THE COMPLETE CRUST FIXED TO J2000.0 ROTATION
!                                MATRIX AND ITS FIRST TWO CT TIME DERIVATIVES.
!                                Classical version. (UNITLESS, 1/SEC, 1/SEC**2)
!            67. RSC2K(3,3,3)  - THE DIURNAL SPIN PORTION OF THE COMPLETE CRUST
!                                FIXED TO J2000.0 ROTATION MATRIX AND ITS FIRST
!                                TWO CT TIME DERIVATIVES. (UNITLESS, 1/SEC,
!                                1/SEC**2)
!            68. GAST2K(2)     - THE GREENWICH APPARENT SIDEREAL TIME AND
!                                ITS CT TIME DERIVATIVE. (RAD, RAD/SEC)
!            69. AXTILT(2,2)   - Antenna fixed axis tilts (arc-seconds).
!                                First index runs over the two orthogonal
!                                tilt directions (Alt-Az: 1 => East,
!                                2 => North; (X-Y (N-S or E-W fixed) and
!                                Equatorial: 1 => Az error, 2 => Elev error).
!                                Second index runs over the two stations.
!            70. ROTAXIS(3,3,2)- Topocentric rotation matrices representing
!                                the fixed axis tilts for station 1 and
!                                station 2 of the current observation.
!            71. RFR2K(3,3)   -  The frame bias rotation matrix
!
! 1.2.9 PROGRAMMER - DALE MARKHAM  01/12/77
!                    DALE MARKHAM  02/16/77
!                    KATHY WATTS   03/28/77
!                    PETER DENATALE 07/07/77
!                    BRUCE SCHUPLER 05/11/78
!                    BRUCE SCHUPLER 12/05/78
!                    BRUCE SCHUPLER 02/01/79
!                    BRUCE SCHUPLER 01/07/80
!                    BRUCE SCHUPLER 08/26/80
!                    CHOPO MA 08/03/81
!                    HAROLD M. SCHUH 10/08/83
!                    GEORGE KAPLAN   ????????
!                    CHOPO MA / DAVID GORDON 04/09/84
!                    DAVID GORDON 05/15/84
!                    DAVID GORDON 06/11/84
!                    JIM RYAN  06/20/84     (OCEAN LOADING)
!                    DAVID GORDON 07/12/84  (POLE TIDE)
!                    DAVID GORDON 07/18/84  (K1 DISPLACEMENT TIDE)
!                    DAVID GORDON 08/30/84  (CHANGED CALL TO OCEG)
!                    DAVID GORDON 01/03/85  (REMOVED POLTDP & POLTDV
!                                            FROM CALL PTDC)
!                    DAVID GORDON 01/08/85  (ADDED IDISC)
!                    SAVITA GOEL  06/04/87  (CDS FOR A900)
!                    GREGG COOKE  12/21/88  (CONSOLIDATED DRIVERS)
!                    LOTHAR MOHLMANN 03/23/89 (CHANGED SITG, OCEG)
!                    GREGG COOKE  05/22/89    (ADDED PANC)
!                    89.07.25  Jim Ryan Documentation simplified
!                    89.10.08  All code relating to computing a perturbed
!                              source positon deleted.  Logic changed for
!                              Shapiro (89) algorithm.
!                    91.10.05  Jim Ryan Arrary EARTH passed to ATMG for
!                              aberration computation.
!                    91.11.25  jwr  The array EARTH added to the call to
!                              SITP, STRP, UT1P, and WOBP.
!                    93MAY     CONSEN added, AXOG & ATMG calls modified, etc.
!                    93.10.07  NZ/DG, added call to DIRNC, new equation of
!                              equinox's contribution (IERS note 13)
!                    94.01.07  D. Gordon XMOON added to SAVE block, needed
!                              (along with EARTH and SUN) in PEP to allow
!                              reusing solar system info if obs. time doesn't
!                              change.
!                    94.04.13  D. Gordon Converted to Implicit None.
!                    94.06.08  D. Gordon Removed unused variable 'TRHOHF' from
!                              Save block
!                    94.09.21  D. Gordon Added SITHEIGHT(2) to AXOG argument
!                              list.
!                    94.10.05  D. Gordon Removed unused arguments from call to
!                              THERY
!                    94.10.24  D. Gordon Removed unused arguments from calls to
!                              ATMG and AXOG.
!                    95.05.02  D. Gordon  DSTRP(2,2) added, put in SAVE block;
!                              added SUN and DSTRP to subroutine PLXP argument
!                              list; added DSTRP to subroutine STRP argument
!                              list.
!                    95.12.04  D. Gordon: Variable CT passed to WOBG for X,Y
!                              interpolation.
!                    95.12.11  D. Gordon  Changing RW(3,3) to RW(3,3,2). Time
!                              derivative of wobble rotation matrix computed
!                              in WOBG.
!                    98.02.04  D. Gordon: Added DNUTP to subroutine NUTP call.
!                              Added call to subroutine NUTC for Wahr
!                              contribution. Removed DEPSD and DPSID (database
!                              nutation values) from call to PEP. Logic added
!                              to allow skipping repeat time-dependent
!                              computations.
!                    98.06.26  D. Gordon: Removed obsolete arguments from call
!                              to ETDG and added rotation matrix RTTOCF(3,3,2)
!                    98.09.08  D. Gordon: Added SITEV to subroutine STRP
!                              argument list.
!                    98.09.10  D. Gordon: Move 'CALL UTCTM' ahead of 'CALL
!                              STRG' so that the time can be used for proper
!                              motion corrections (optional) in the Star
!                              geometry computations. Add XJD and UTC to
!                              STRG and STRP argument list.
!                    98.10.15  D. Gordon: Added SITEV to subroutine SITP
!                              argument list.
!                    98.10.16  D. Gordon: Added 'CALL PLXC' for optional
!                              computation of parallax contributions.
!                    98.11.05  D. Gordon: Added WOBXR, WOBYR, UTC, AT, DUTCAT,
!                              DATDCT, GMST to SAVE block.
!                    98.11.12  D. Gordon: Removed PANG subroutine call. The
!                              feedbox rotation (parallactic angle) module
!                              has been removed and its functions have been
!                              merged into the axis offset module.
!                              Added variable FUKU(2) to NUTG and NUTC
!                              argument lists. Used to compute effect of
!                              geodesic nutation.
!                    98.11.19  D. Gordon: Added EL(2,2) to ATMP argument
!                              list to calculate the Niell atmosphere gadient
!                              partials.
!                    98.11.24  D. Gordon: Added STAR to PTDP argument list to
!                              calculate pole tide partials w.r.t. X and Y.
!                    98.12.17  D. Gordon: Added CENT to PTDP argument list to
!                              compute and remove a secular mean value for
!                              X-pole and Y-pole.
!                    99.01.14  D. Gordon: Put TSKIP in Subroutine PEP
!                              argument list to check new/repeat time in PEP.
!                              PEP now does PUT's of Earth, Moon, and Sun
!                              coordinates.
!                  2002.09     J. Ryan: Integer*4 conversion.
!                  2003-2004   D. Gordon: Update for IERS Conventions 2003.
!
! PROGRAM STRUCTURE
!
!  Perform the geometry and time calculations.
!     The basic coordinate system is referenced to the Epoch of 2000.0 and is a
!     right-handed Cartesian system oriented to the mean celestial pole and mean
!     equator of that epoch. The nominal origin is the solar system barycenter.
!     There is also an Earth fixed coordinate system which is a right-handed
!     Cartesian system oriented to the mean geographic pole of 1900-1906 and the
!     Greenwich Meridian. The nominal origin is the Earth's center of mass. The
!     basic unit of time is the coordinate second as used by the PEP Tape. UTC,
!     AT, AND UT1 are also used. The geometry of the observation is calculated
!     with an accuracy goal of 0.1 picoseconds of delay. In doing the
!     calculations for the geometry, much of the work neccesary for the
!     computation of model contributions to delay and delay rate and partials of
!     delay and delay rate with respect to model parameters is also done.
!     Matrices which represent coordinate rotations (precession, nutation,
!     diurnal spin, diurnal polar motion, and wobble) and their CT time
!     derivatives are stored as (3,3,N) arrays, where N indixes the N-1'th time
!     derivative. The subroutines suffixed G are sections of model modules. The
!     other subroutines may be considered utilities and either superseed or
!     incorporate many present PEP routines.
!
!     Call SITG for the geographical site data. SITG provides the following
!     geocentric information for each observing site: the antenna axis offsets
!     (AXOFF), the antenna types (KAXIS), the crust fixed site vectors (CFSITE),
!     the crust fixed baseline vector (CFBASE), the crust fixed site normal unit
!     vectors (CFSITN), the geodetic latitudes (SITLAT), the site east
!     longitudes (SITLON), the spherical Earth radii, the partial derivatives of
!     the crust fixed site vector components with respect to the geodetic
!     latitudes (CFLAT) and east longitudes (CFLON), the rotation matrices which
!     rotate the topocentric site reference system to the geocentric system at
!     each site (TCTOCF), and the zenith tropospheric path delays at each
!     observing site. SITG is the only routine which 'knows' which two sites are
!     involved in the observation. All other routines merely work with site #1
!     and site #2.
!
      CALL SITG (AXOFF, CFBASE, CFLAT, CFLON, CFSITE, CFSITN, KAXIS,
     .     OCEAMP, OCEPHS, SITLAT, SITLON, SITRAD, TCTOCF, RTTOCF,
     .     ZPATH, SITHEIGHT, GEOLAT, AXTILT, ROTAXIS )
!
!     Call UTCTM for the UTC time fraction of the UTC day (UTC) and for
!     the Julian Date at zero hours UTC of the date in question (XJD).
      CALL UTCTM (UTC, XJD)
!
!     Call STRG for the J2000.0 unit vector in the direction of the
!     radio source. (STAR)
      CALL STRG (XJD, UTC, STAR)
!
!     Call ATIME for the atomic time fraction of the atomic time day (AT) and
!     for the partial derivative of the UTC time with respect to the atomic
!     time (DUTCAT).
      CALL ATIME (UTC, XJD, AT, DUTCAT)
!
!     Call CTIMG for the coordinate time fraction of the coordinate time day at
!     site #1 (CT), the partial derivative of the atomic time with respect to
!     the coordinate time (DATDCT), and the partial derivative of the long
!     period terms in the 'AT minus CT' offset with respect to the coordinate
!     time (DLPGR).
      CALL CTIMG (AT, CFSITE, SITLON, UTC, XJD, CT, DATDCT, DLPGR)
!
!     Compute epoch and compare with previous observation. If same, set
!     TSKIP=1, otherwise TSKIP=0. If TSKIP=1, then we can skip many steps in
!     the geometry subroutines.
       TJD = XJD + CT
       OBSDIF = DABS(TJD - SJD)
       IF (OBSDIF .lt. 1.D-10) THEN
          TSKIP = 1
       ELSE
          TSKIP = 0
          SJD = TJD
       ENDIF
!
!     Call PEP for the J2000.0 geocentric Sun (SUN) and Moon (XMOON) position
!     and velocity vectors; the J2000.0 solar system barycentric Earth
!     position, velocity, and acceleration vectors (EARTH); the other planets'
!     (except Pluto) barycentric and geocentric positions and velocities.
!     The solar system info comes from the DE/LE405 JPL Ephemeris by default.
      CALL PEP (XJD, CT, TSKIP, EARTH, SUN, XMOON)
!
!     Call NUTFA before NUTG and before UT1G to get epoch in centuries and
!     the fundamental arguments for the nutation series.
      IF (TSKIP .NE. 1)
     .    CALL NUTFA (XJD, CT, CENT, FA2K, FAD2K)
!
!     Call UT1G for the UT1 fraction of the UT1 day (UT1) and for the partial
!     derivative of the UT1 time with respect to the atomic time (DUT1AT).
      CALL UT1G (AT, DUTCAT, UTC, XJD, CT, FA2K, FAD2K,
     .     CENT, TSKIP, DUT1AT, UT1, Xti, Yti, UT1t,
     .     dXti, dYti, dUT1t)
!
!     Call NUTG for the nutation portion of the complete crust fixed to
!     J2000.0 rotation matrices and their CT time derivatives (RPN2K and
!     RNC2K), the true obliquity of the ecliptic and its CT time derivative,
!     (EPS), the mean obliquity at J2000.0 (EPSMNR), the CEO-based
!     nutation offsets (Xn,Yn), and the classical nutation offsets
!     (DEPS2K, DPSI2K).
      CALL NUTG (CENT, FA2K, FAD2K, XJD, CT, TSKIP, EPS,
     .          EPSMNR, Xn, Yn, Sn, RPN2K,
     .          DEPS2K, DPSI2K, EPSA, RNC2K, NUTDIF )
!
!     Call PREG for the precession portion of the classical crust fixed to
!     J2000.0 rotation matrix and its CT time derivative (RPC2K).
      IF (TSKIP .NE. 1) CALL PREG (CT, CENT, EPSMNR, XJD, RPC2K)
!
!     Call DIRNL for the diurnal spin portion of the complete crust fixed to
!     J2000.0 rotation matrices and their first two CT time derivatives (RS2K
!     and RSC2K), the Earth rotation angle and its CT time derivative
!     (ERA2K and DERA2K), the Greenwich apparent siderial time and its CT
!     time derivative (GAST2K), the Greenwich mean siderial time (GMST2K),
!     and the diurnal rotational velocity of the Earth (DIURNV).
!            ---> DIURNV removed, not used
!
      IF (TSKIP .NE. 1)
     .  CALL DIRNL (DATDCT, DUT1AT, EPS, FA2K, FAD2K, UT1,
     .       XJD, CT, DUTCAT, CENT, DEPS2K, DPSI2K, EPSA,
     .       ERA2K, DERA2K, pERA2K, RS2K, GAST2K, GMST2K, RSC2K)
!
!     Call WOBG for the wobble portion of the complete crust fixed to J2000.0
!     rotation matrix and its first time derivative (RW2K), and the long period
!     wobble X and Y OFFSETS. (NOTE: Right-handed coordinate system.)
      CALL WOBG (CENT, CT, UTC, XJD, GMST2K, TSKIP, FA2K, FAD2K,
     .           UT1, DUT1AT, Xti, Yti, dXti, dYti,
     .           WOBXR, WOBYR, WOBXD, WOBYD, SP, DSP, RW2K)
!
!     Call M2K to complete the new IERS 2003 CEO-based TRF ==> CRF
!      tranformation matrix and its first two time derivatives.
      CALL M2K (RPN2K, RS2K, RW2K, TSKIP, R2K )
!
!     Call MC2K to compute the new IERS 2003 classical TRF ==> CRF
!     tranformation matrix and its first two time derivatives.
      CALL MC2K (RNC2K, RPC2K, RSC2K, RW2K, RFR2K, TSKIP, RC2K )
!
!xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
!      Write(6,1014) RPN2K
!1014  Format(1x,'DRIVR/RPN2K:',(6(/,3E25.15)))
!      Write(6,1016) RNC2K
!1016  Format(1x,'DRIVR/RNC2K:',(6(/,3E25.15)))
!      Write(6,1028) RS2K
!1028  Format(1x,' DRIVR/RS2K  ',(9(/,3E25.15)))
!      Write(6,1030) RSC2K
!1030  Format(1x,' DRIVR/RSC2K  ',(9(/,3E25.15)))
!      Write(6,1029) RPC2K
!1029  Format(1x,'DRIVR/RPC2K, New Classical Prec: ',(6(/,3E25.15)))
!      Write(6,1034) RW2K
!1034  Format(1x,'DRIVR/RW2K, New Wobble: ',(6(/,3E25.15)))
!
!      Write(6,1027) R2K
!1027  Format(1x,'DRIVR/R2K: ',(9(/,3E25.15)))
!      Write(6,1031) RC2K
!1031  Format(1x,'DRIVR/RC2K: ',(9(/,3E25.15)))
!
!   Difference between transformations, CEO - Equinox
!        Do I = 1,3
!        Do J = 1,3
!         R2Kdif(I,J) = R2K(I,J,1) - RC2K(I,J,1)
!        Enddo
!        Enddo
!        Write(6,1043) R2Kdif
!1043    Format(1x,'DRIVR/R2Kdif: ',(3(/,3E25.15)))
!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!
!     Call ROT2K to rotate the crust fixed site data into the J2000.0 inertial
!     reference system. The following variables are output for each observing
!     site in J2000.0 coordinates: the site position vectors (USITEP) and
!     velocity vectors (USITEV) uncorrected for Earth tidal and ocean loading
!     effects; the site accelerations (SITEA); the site normal unit vectors
!     (EPSITN); and the partial derivatives of the site position and velocity
!     vector components with respect to the site geodetic latitudes (EPLATP
!     and EPLATV) and east longitudes (EPLONP and EPLONV).
!
!     CALL ROTC2K(CFLAT, CFLON, CFSITE, CFSITN, RC2K, EPLATP, EPLATV,
!    .     EPLONP, EPLONV, EPSITN, CITEA, UCITEP, UCITEV)
!     write(6,235) UCITEP, UCITEV, CITEA
!235  format(' DRIVR/RC2K: UCITEP, UCITEV, CITEA ',/,6(3D30.16,/))
!
      CALL ROT2K (CFLAT, CFLON, CFSITE, CFSITN, R2K, EPLATP, EPLATV,
     .     EPLONP, EPLONV, EPSITN, SITEA, USITEP, USITEV)
!     write(6,236) USITEP, USITEV, SITEA
!236  format(' DRIVR/R2K: USITEP, USITEV, SITEA ',/,6(3D30.16,/))
!
!  Compute difference vectors between the two transformations
!   EQ - CEO
!     DO J = 1,2
!      DO I = 1,3
!       D2003P(I,J) = UCITEP(I,J) - USITEP(I,J)
!       D2003V(I,J) = UCITEV(I,J) - USITEV(I,J)
!       D2003A(I,J) =  CITEA(I,J) -  SITEA(I,J)
!      ENDDO
!     ENDDO
!     write(6,238) D2003P, D2003V, D2003A
!238  format(' DRIVR/D2003P, D2003V, D2003A ',/,6(3D30.16,/))
!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!
!     Call ETDG for the corrections to the J2000 site position vectors (TIDEP)
!     and velocity vectors (TIDEV) due to Earth tide effects.
      CALL ETDG ( R2K, SITLAT, SITLON, SUN, TCTOCF, RTTOCF,
     .            USITEP, USITEV, XMOON, EARTH, GAST2K, STAR,
     .                FA2K, FAD2K, CENT, GEOLAT, TIDEP, TIDEV)
!
!     Call 'PTDG' for the corrections to the J2000.0 site positions
!      and site velocity vectors due to the solid Earth pole tide.
      CALL PTDG (SITLAT, SITLON, SITRAD, WOBXR, WOBYR,
     .     TCTOCF, R2K, CENT, POLTDP, POLTDV)
!
!     Call OCEG for the corrections to the J2000.0 site position vectors
!      (XLOADP) and velocity vectors (XLOADV) due to ocean loading effects.
      CALL OCEG (CFSITE, UT1, OCEAMP, OCEPHS, R2K  , XJD, TCTOCF, TSKIP,
     .     XLOADP, XLOADV)
!
!     Call SITCR to apply the Earth tide, ocean loading, and pole tide
!      corrections to the J2000.0 site position vectors (SITEP), site
!      velocity vectors (SITEV), and the J2000.0 baseline position and
!      velocity vectors (EPBASE).
      CALL SITCR (TIDEP, TIDEV, USITEP, USITEV, XLOADP,
     .     XLOADV, EPBASE, SITEP, SITEV, POLTDP, POLTDV)
!
!     Call UVG to compute the (U,V) coordinates of the baseline, depending
!      on the value of KASTC.
      CALL UVG (STAR, EPBASE)
!
!     Call ATMG for the aberrated elevation and azimuth angles of the
!      source and their CT time derivatives, and the aberrated source
!      unit vector.
      CALL ATMG (R2K, STAR, EARTH, TCTOCF, SITEV, AZ, ELEV,
     .     STAR_ABERRATED)
!
!     Call AXOG for the J2000.0 vector axis offsets of the antennas and
!      their time derivatives at each site.
      CALL AXOG (KAXIS, R2K, SITLAT, STAR, TCTOCF, SITEV, AXOFF,
     .     EARTH, AZ, ELEV, STAR_ABERRATED, SITHEIGHT, AXTILT,
     .     ROTAXIS, AXIS2000, DAXIS2000)
!
!     Call PLXG to compute the parallax goemetry.
      CALL PLXG()
!
!   Perform the partial derivatives calculations.
!     The partials are calculated using exact geometry wherever practical. Each
!     section PUT's its partials into the observation item using the database
!     handler. The calling sequence for each module has the form: Call
!     <MOD>P(...) where ... are the variables passed from the geometry section
!     of DRIVR needed to calculate the partials. All of the subroutines are
!     parts of model modules. Note that the relativity partials are now in the
!     THERY subroutine.
!
!     Compute the atmosphere partials.
      CALL ATMP (ELEV, AZ, SITLAT, SITHEIGHT, XJD, CT, dATMCdh)
!
!     Compute the axis offset partials.
      CALL AXOP (AXOFF, STAR, EARTH, SITEV)
!
!     Compute the Earth tide partials.
      CALL ETDP (R2K, SITLAT, STAR, TCTOCF)
!
!     Compute the pole tide partials.
      CALL PTDP (STAR)
!
!     Compute the nutation partials.
      CALL NUTP (CFBASE, EPS, Xn,Yn, Sn, DEPS2K, DPSI2K, EPSA, GAST2K,
     .      STAR,  RPN2K, RS2K, RW2K, RNC2K, RPC2K, RSC2K, RFR2K,
     .      TSKIP, DNUpe)
!
!     Compute the ocean loading partials.
      CALL OCEP()
!
!     Compute the precession partials.
      CALL PREP (CENT, CFBASE, EPSMNR, RFR2K, RNC2K, RSC2K, RW2K,
     .           STAR)
!
!     Compute the site partials.
      CALL SITP (R2K, STAR, EARTH, SITEV)
!
!     Compute the star partials.
      CALL STRP (EPBASE, STAR, EARTH, SITEV, DSTRP, CD, CRA, SD, SRA)
!
!     Compute the UT1 partials.
      CALL UT1P (CFBASE, STAR,EARTH, RPN2K, RW2K, ERA2K, dERA2K,
     .           pERA2K, SITEV, DUT1P)
!
!     Compute the wobble partials.
      CALL WOBP (CFBASE, STAR, EARTH, RPN2K, RS2K, SITEV)
!
!     Compute the parallax partials.
      CALL PLXP (SUN, DSTRP, CD, CRA, SD, SRA, EARTH, STAR, EPBASE,
     .           SITEV)
!
!  Perform the contributions calculations.
!     The individual module contributions are calcualted so that they may be
!     removed from the theoretical in the program 'SOLVE' if desired. Some of
!     the routines are essentially dummies returning values passed from the
!     observation item. However, they are included to retain the capability of
!     using other models. In several cases the contributions can be calculated
!     to first order with sufficient accuracy using the partial derivatives of
!     the dalays and rates with respect to the model module parameters. Each
!     section will PUT its contributions into the observation item using the
!     database handler. Note that the relativity contributions are now in
!     subroutine THERY.
!
!     Compute the atmosphere contributions.
      CALL ATMC (ZPATH, DATMC)
!
!     Compute the axis offset contributions.
      CALL AXOC (AXOFF, DAXOC)
!
!     Compute the earth tide contributions.
      CALL ETDC (TIDEP, TIDEV, STAR)
!
!     Compute the pole tide contributions.
      CALL PTDC (STAR)
!
!     Zero out the ionosphere contribution.
      DIONC(1) = 0.D0
      DIONC(2) = 0.D0
!
!     Compute the ocean loading contributions.
      CALL OCEC (STAR)
!
!     Compute the Wahr nutation contribution
!       (Does nothing)
      CALL NUTC (NUTDIF, DNUpe)
!
!     Compute proper motion contributions.
      CALL STRC (DSTRP)
!
!     Compute the UT1 contributions.
      CALL UT1C (DUT1P, UT1t, dUT1t)
!
!     Compute the wobble contributions.
      CALL WOBC(Xti,Yti,dXti,dYti)
!
!     Compute the parallax contributions.
      CALL PLXC()
!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!   Test - Add axis offset to the baseline and carry through the
!          Thery module
!       print *,' Test: Axis offset added to site position!!'
!     Do J=1,2
!      Do I=1,3
!       SITEP(I,J) = SITEP(I,J) +  AXIS2000(I,J)
!       SITEV(I,J) = SITEV(I,J) + DAXIS2000(I,J)
!      Enddo
!     Enddo
!      Do I=1,3
!       EPBASE(I,1) = SITEP(I,1) - SITEP(I,2)
!       EPBASE(I,2) = SITEV(I,1) - SITEV(I,2)
!      Enddo
!       Daxoc(1,1) = 0.D0
!       Daxoc(1,2) = 0.D0
!       Daxoc(2,1) = 0.D0
!       Daxoc(2,2) = 0.D0
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!     Perform the calculation for the complete theoretical delay and rate.
!     Also do all the work of all elements of the Relativity Module,
!     including the contributions and partials. This now includes only
!     the Consensus relativity model computations:
      CALL THERY (DATMC, DAXOC, DIONC, DLPGR, EARTH, EPBASE,
     .     SITEP, SITEV, SITEA, SUN, STAR, XMOON, AT)
!
!     Go back to the main.
      RETURN
      END
