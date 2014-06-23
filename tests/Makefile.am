if ENABLE_TESTS
bin_PROGRAMS = \
$(top_builddir)/bin/test-png \
$(top_builddir)/bin/img2c
#$(top_builddir)/bin/test-imageio

__top_builddir__bin_test_png_LDFLAGS  = $(top_builddir)/lib/.libs/libimageio.a
__top_builddir__bin_test_png_SOURCES  = test-png.c

__top_builddir__bin_img2c_LDFLAGS  = $(top_builddir)/lib/.libs/libimageio.a
__top_builddir__bin_img2c_SOURCES  = img2c.c

#__top_builddir__bin_test_imageio_LDFLAGS  = -framework OpenGL -lglut $(top_builddir)/lib/.libs/libimageio.a
#__top_builddir__bin_test_imageio_SOURCES  = test-imageio.c

endif
