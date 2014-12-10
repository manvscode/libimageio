AC_PREREQ(2.67)
AC_INIT([libimageio], [0.6], [manvscode@manvscode.com], [libimageio], [http://www.manvscode.com/])
AM_INIT_AUTOMAKE([1.13 subdir-objects foreign silent-rules -Wall -Werror])

AM_SILENT_RULES([yes])

AC_PROG_CC
AC_PROG_CC_STDC
AC_C_INLINE
AC_HEADER_STDC

AH_TOP([
#ifndef _LIBIMAGEIO_H_
#define _LIBIMAGEIO_H_
])
AH_BOTTOM([
#endif /* _LIBIMAGEIO_H_ */
])

# If big endian define WORDS_BIGENDIAN
AC_C_BIGENDIAN

#DFLAGS="-D DEBUG"

#CFLAGS="$CFLAGS -g -ggdb -O0"
CFLAGS="$CFLAGS -O2"
LDFLAGS="-lm $LDFLAGS"



AC_ARG_ENABLE([tests],
	[AS_HELP_STRING([--disable-tests], [Enable test programs.])],
	[:],
	[enable_tests=no])

AM_CONDITIONAL([ENABLE_TESTS], [test "$enable_tests" = "yes"])

AM_PROG_AR
LT_INIT([shared static])

AC_PROG_LIBTOOL
AM_PROG_LIBTOOL

AC_PROG_INSTALL

echo " ------------------------------ "
echo ""
echo "   CFLAGS = $CFLAGS"
echo "  LDFLAGS = $LDFLAGS"
echo ""
echo " ------------------------------ "

AC_CONFIG_FILES([
	Makefile
	src/Makefile
	tests/Makefile
])

AC_OUTPUT
