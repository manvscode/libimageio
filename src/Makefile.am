
# Add new files in alphabetical order. Thanks.
libimageio_src = imageio.c \
				 charts.c \
../extern/libpng-1.6.15/png.c \
../extern/libpng-1.6.15/pngerror.c \
../extern/libpng-1.6.15/pngget.c \
../extern/libpng-1.6.15/pngmem.c \
../extern/libpng-1.6.15/pngpread.c \
../extern/libpng-1.6.15/pngread.c \
../extern/libpng-1.6.15/pngrio.c \
../extern/libpng-1.6.15/pngrtran.c \
../extern/libpng-1.6.15/pngrutil.c \
../extern/libpng-1.6.15/pngset.c \
../extern/libpng-1.6.15/pngtrans.c \
../extern/libpng-1.6.15/pngwio.c \
../extern/libpng-1.6.15/pngwrite.c \
../extern/libpng-1.6.15/pngwtran.c \
../extern/libpng-1.6.15/pngwutil.c \
../extern/zlib-1.2.8/adler32.c \
../extern/zlib-1.2.8/compress.c \
../extern/zlib-1.2.8/crc32.c \
../extern/zlib-1.2.8/deflate.c \
../extern/zlib-1.2.8/gzclose.c \
../extern/zlib-1.2.8/gzlib.c \
../extern/zlib-1.2.8/gzread.c \
../extern/zlib-1.2.8/gzwrite.c \
../extern/zlib-1.2.8/infback.c \
../extern/zlib-1.2.8/inffast.c \
../extern/zlib-1.2.8/inflate.c \
../extern/zlib-1.2.8/inftrees.c \
../extern/zlib-1.2.8/trees.c \
../extern/zlib-1.2.8/uncompr.c \
../extern/zlib-1.2.8/zutil.c


# Add new files in alphabetical order. Thanks.
libimageio_headers = imageio.h blending.h charts.h

library_includedir      = $(includedir)/libimageio/
library_include_HEADERS = $(libimageio_headers)

# Library
lib_LTLIBRARIES                           = $(top_builddir)/lib/libimageio.la
__top_builddir__lib_libimageio_la_SOURCES = $(libimageio_src)
__top_builddir__lib_libimageio_la_CFLAGS  = -fPIC -I $(top_builddir)/extern/libpng-1.6.15/ -I $(top_builddir)/extern/zlib-1.2.8/
__top_builddir__lib_libimageio_la_LDFLAGS = -lm

#__top_builddir__lib_libimageio_la_LIBADD  = -lpng

