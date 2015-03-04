if ENABLE_TESTS
bin_PROGRAMS = \
$(top_builddir)/bin/test-png \
$(top_builddir)/bin/img2c \
$(top_builddir)/bin/rgb2bgr \
$(top_builddir)/bin/blend \
$(top_builddir)/bin/drawing \
$(top_builddir)/bin/charts
#$(top_builddir)/bin/test-imageio

__top_builddir__bin_test_png_CFLAGS   = -I /usr/local/include
__top_builddir__bin_test_png_LDFLAGS  = $(top_builddir)/lib/libimageio.la -lutility
__top_builddir__bin_test_png_SOURCES  = test-png.c

__top_builddir__bin_img2c_LDFLAGS  = $(top_builddir)/lib/libimageio.la
__top_builddir__bin_img2c_SOURCES  = img2c.c

__top_builddir__bin_rgb2bgr_LDFLAGS  = $(top_builddir)/lib/libimageio.la
__top_builddir__bin_rgb2bgr_SOURCES  = rgb2bgr.c

__top_builddir__bin_blend_LDFLAGS  = $(top_builddir)/lib/libimageio.la
__top_builddir__bin_blend_SOURCES  = blend.c

__top_builddir__bin_drawing_LDFLAGS  = $(top_builddir)/lib/libimageio.la
__top_builddir__bin_drawing_SOURCES  = drawing.c

__top_builddir__bin_charts_LDFLAGS  = $(top_builddir)/lib/libimageio.la
__top_builddir__bin_charts_SOURCES  = charts.c

#__top_builddir__bin_test_imageio_LDFLAGS  = -framework OpenGL -lglut $(top_builddir)/lib/libimageio.la
#__top_builddir__bin_test_imageio_SOURCES  = test-imageio.c

endif
