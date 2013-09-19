AC_INIT([libimageio], [0.4], [manvscode@manvscode.com], [libimageio], [http://www.manvscode.com/])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])

AM_SILENT_RULES([yes])
#AC_CONFIG_HEADERS([src/libimageio-config.h])

AC_PROG_CC
AC_PROG_CC_C99
AC_PROG_CXX
AC_C_INLINE
AC_HEADER_STDC
#AC_LANG([C++])
#AX_CXX_COMPILE_STDCXX_11 # C++11 support


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

#CFLAGS="-Wall -pedantic -g -ggdb -O0 $DFLAGS -I/usr/local/include -I/opt/X11/include/libpng15 -L/opt/X11/lib"
CFLAGS="-Wall -pedantic -O3 -I/usr/local/include"
LDFLAGS="-lm -lpng"


AM_PROG_AR
LT_INIT([shared static])
AC_PROG_LIBTOOL
AM_PROG_LIBTOOL
#AC_PROG_RANLIB


AC_CHECK_FUNCS([floor])
AC_CHECK_FUNCS([memset])
AC_CHECK_FUNCS([sqrt])
AC_FUNC_MALLOC
AC_HEADER_STDBOOL
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T
AC_PREREQ
AC_PROG_AWK
AC_PROG_LN_S
AC_PROG_MAKE_SET


AC_PROG_INSTALL

echo " ------------------------------ "
echo ""
echo "   CFLAGS = $CFLAGS"
echo " CXXFLAGS = $CFLAGS"
echo "  LDFLAGS = $LDFLAGS"
echo ""
echo " ------------------------------ "

AC_CONFIG_FILES([
	Makefile
	src/Makefile
	tests/Makefile
])

AC_OUTPUT
