!
! >>>>> INCLUDE-BLOCK with description of data structures used by
!       programs dclient
!
!       dclient.i  30-SEP-99 13:57:30 v 2.3  Leonid Petrov  16-JAN-2001 18:51:42
!
        CHARACTER  DCLIENT__LABEL*25, DCLIENT__HELP*80
        PARAMETER ( DCLIENT__LABEL = 'dclient v 2.3  2001.01.16' )
        PARAMETER ( DCLIENT__HELP  = 'dclient_01.hlp' )
        INTEGER*4  M_PAR_DBS, M_PAR_EOS, M_BUF
        PARAMETER  ( M_PAR_DBS =  11 )
        PARAMETER  ( M_PAR_EOS =  11 )
        PARAMETER  ( M_BUF = 128 )
!
        TYPE      DCLIENT__STRU
           INTEGER*4      FIRST_FIELD
!
           CHARACTER*256  DATA_CENTER
           CHARACTER*256  DSERVER_EMAIL
           CHARACTER*256  FTP_DIR
           CHARACTER*256  URL_PREFIX
           CHARACTER*256  MAIL_COMMAND
           CHARACTER*256  CONFIRM_EMAIL
           CHARACTER*256  SUCCESS_EMAIL
           CHARACTER*256  SUBMIT_LOG
           CHARACTER*256  TMP_DIR
           CHARACTER*256  EOP_SUFFIX
           CHARACTER*8    DO_LINK
           CHARACTER*8    DO_PURGE
!
           CHARACTER*256  CONFIG_FILE
           INTEGER*4      LAST_FIELD
        END TYPE  DCLIENT__STRU  !  DCLIENT__STRU  !
!
! <<<<< end of INCLUDE-BLOCK  dclient.i
!
