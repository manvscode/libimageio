AC_PREREQ(2.67)
AC_INIT([libimageio], [0.6], [manvscode@manvscode.com], [libimageio], [http://www.manvscode.com/])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])

AM_SILENT_RULES([yes])

AC_PROG_CC
AC_PROG_CC_STDC
AC_PROG_CXX
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

#CFLAGS="-Wall -pedantic -g -ggdb -O0 $DFLAGS -I/usr/local/include -I/opt/X11/include/libpng15 -L/opt/X11/lib $CFLAGS"
CFLAGS="$CFLAGS"
LDFLAGS="-lm -lpng $LDFLAGS"


AM_PROG_AR
LT_INIT([shared static])
AC_PROG_LIBTOOL
AM_PROG_LIBTOOL


AC_CHECK_FUNCS([floor])
AC_CHECK_FUNCS([memset])
AC_CHECK_FUNCS([sqrt])
AC_HEADER_STDBOOL

AC_ARG_ENABLE([tests],
	[AS_HELP_STRING([--disable-tests], [Enable test programs.])],
	[:],
	[enable_tests=no])

AM_CONDITIONAL([ENABLE_TESTS], [test "$enable_tests" = "yes"])


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
