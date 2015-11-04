AC_INIT([jpeg_wc_streamer], [1.0], [kyle.zhou@live.com])
AC_CONFIG_SRCDIR([src/WebcamStreamer.cpp])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])
AC_PROG_CXX
PKG_CHECK_MODULES([LIVE555], [live555 >= 0.26])
AC_CHECK_HEADERS([fcntl.h unistd.h errno.h sys/stat.h sys/types.h sys/time.h sys/mman.h sys/ioctl.h])
AC_CHECK_HEADER([linux/videodev2.h], [has_videodev2=yes])
AM_CONDITIONAL([HAVE_VIDEODEV2], [test “$has_videodev2” = yes])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT
