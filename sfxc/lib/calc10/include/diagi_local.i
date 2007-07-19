!
! This file is generated automatically by use_local.f from
!      template_file: /mk5/include/diagi_local.templ
!      local customization file: /mk5/local/solve.lcl
!
!      PLEASE DON'T EDIT THIS FILE!
!      ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
!
!
! >>>>> INCLUDE-BLOCK with local preferences for DiaGI
!       (Dialogue Graphic Interface) utility
!
! Created: 2000.05.24 by Leonid Petrov
!
! NOTE FOR PROGRAMMERS:
!
!      Lines which starts from & are meta-definitions and they are tied with
!            local customization file.
!      The value of the parameter is an installation-specific.
!      Format of meta-commands:
!      a) &V&  <param_name> -- definition of the name of non-character type
!      b) &C0& <param_name> -- definition of the name of character type.
!                               No restrictions on the length of the line
!                               (but the line should not be empty)
!      c) &C<max_length>& <param_name> -- definition of the name of character
!                                         type. But the value should not have
!                                         length more than <max_length>
!                                         characters.
!      Actual values are are taken from the local customization file and
!      the output include file is generated.
!
!CCCCCCCCCCCCCCC
!
! ---- IXS__DEF -- Default type of XS-device for DiaGI:
! ----             1 -- huge screen   1600x1200 pixels, 395x280mm
! ----             2 -- big screen    1280x1024 pixels, 330x215mm
! ----             3 -- small screen: 1024x768  pixels, 290x180mm.
! ----             Environment variable DIAGI_SCREEN overrides this parameter.
!
      INTEGER*4  IXS__DEF
      PARAMETER ( IXS__DEF = 1 ) ! Local customization
!
! --- PAPER_SIZE:  Default paper_size
!                  A4
!                  Letter
!
      CHARACTER PAPER_SIZE*6
      PARAMETER ( PAPER_SIZE = 'Letter' ) ! Local customization
