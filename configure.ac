AC_PREREQ(2.67)
AC_INIT([libimageio], [0.6.0], [manvscode@gmail.com], [libimageio], [http://www.joemarrero.com/])
AM_INIT_AUTOMAKE([1.13 subdir-objects foreign dist-zip silent-rules -Wall -Werror])

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


AM_PROG_AR
LT_INIT([static])

AC_PROG_LIBTOOL
AM_PROG_LIBTOOL

AC_PROG_INSTALL


AC_ARG_ENABLE([tests],
	[AS_HELP_STRING([--disable-tests], [Enable test programs.])],
	[:],
	[enable_tests=no])

AM_CONDITIONAL([ENABLE_TESTS], [test "$enable_tests" = "yes"])

if test "x$enable_tests" = "xyes"; then
	PKG_CHECK_MODULES([UTILITY], [utility],
		[AC_MSG_NOTICE([Found libutility])],
		[AC_MSG_ERROR([Missing libutility])],
	)
fi


echo "------------------------------------------------------------------------------"
if [test -z "$CFLAGS"]; then
	echo "  CFLAGS: Not set"
else
	echo "  CFLAGS: $CFLAGS"
fi
if [test -z "$LDFLAGS"]; then
	echo " LDFLAGS: Not set"
else
	echo " LDFLAGS: $LDFLAGS"
fi
echo "------------------------------------------------------------------------------"

AC_CONFIG_FILES([
	Makefile
	src/Makefile
	tests/Makefile
	imageio.pc
])

AC_OUTPUT
