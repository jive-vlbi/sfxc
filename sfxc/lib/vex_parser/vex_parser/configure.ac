dnl Process this file with autoconf to produce a configure script.
AC_INIT(Vex parser, 0.0.3)
AC_CONFIG_MACRO_DIR(m4)
AM_INIT_AUTOMAKE()

dnl Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AC_PROG_LIBTOOL
AC_PROG_YACC
AM_PROG_LEX

AC_LANG_CPLUSPLUS

AC_ARG_ENABLE(32-bit,
              [  --enable-32-bit        Enable 32-bit code [default=no]],
              DO_32BIT="$enableval", DO_32BIT="no")
AM_CONDITIONAL(DO_32BIT, [test "$DO_32BIT" = "yes"])

pythonlib_default="no"
AC_ARG_ENABLE(pythonlib, 
              [  --enable-pythonlib     [no/yes] compile the python library],
              , 
              enable_pythonlib=$pythonlib_default)
AM_CONDITIONAL(BUILD_PYTHONLIB, test "x$enable_pythonlib" = "xyes")
if test "x$enable_pythonlib" = "xyes"; then
  echo ================ ENABLE PYTHONLIB
  dnl Check for python
  AM_PATH_PYTHON(2.7)
  dnl Check for boost.python
  AC_CHECK_LIB(python2.7, main)
  AC_CHECK_LIB(boost_python-py27, main)
  dnl AC_CHECK_LIB(boost_python-mt, main)
  echo ================ /ENABLE PYTHONLIB
fi

dnl export these variable (so Makefile substitutions can be made). 
CXXFLAGS+=" -std=gnu++03"
AC_SUBST(CXXFLAGS)
AC_SUBST(LDFLAGS)

AC_OUTPUT(vex_parser.pc
          Makefile
          include/Makefile
          include/vex/Makefile
          lib/Makefile
          src/Makefile
          python/Makefile
          test/Makefile
         )

