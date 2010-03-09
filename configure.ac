#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT(TOONLOOP, 1.3.0 )
AC_CONFIG_HEADER([src/config.h])
AC_CONFIG_MACRO_DIR([m4]) # check for m4 macros in directory m4
AC_PROG_LIBTOOL
AM_INIT_AUTOMAKE()

# Check for C and C++ compilers
AC_PROG_CXX
AC_PROG_CC

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([arpa/inet.h stdlib.h string.h sys/ioctl.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
PKG_PROG_PKG_CONFIG

AC_PROG_GCC_TRADITIONAL
AC_CHECK_FUNCS([inet_ntoa memset socket])

AC_CHECK_LIB([pthread],[pthread_exit],[AC_SUBST([LIBTHREAD_CXXFLAGS],["-lpthread"])])
AC_CHECK_LIB([m],[sin],[AC_SUBST([LIBM_CXXFLAGS],["-lm"])])
AC_CHECK_LIB([lo],[lo_send_internal],[AC_SUBST([LIBLO_CXXFLAGS],["-llo"])])

# GTK and GST
PKG_CHECK_MODULES([GST], [gstreamer-gl-0.10 >= 0.10.0 gstreamer-0.10 >= 0.10.0   gstreamer-interfaces-0.10 >= 0.10.24 gtk+-2.0 >= 2.12.9 glib-2.0 >= 2.16.6])
PKG_CHECK_MODULES([GTK], [gtk+-2.0])
PKG_CHECK_MODULES([GDK], [gdk-2.0])

PKG_CHECK_MODULES([GTKGLEXT], [gtkglext-1.0 >= 1.0])
AC_SUBST([GST_LIBS])
AC_SUBST([GST_CFLAGS])

# BOOST
AX_BOOST_BASE([1.35])

AX_BOOST_PROGRAM_OPTIONS
if test "x${ax_cv_boost_program_options}" = "xno" ; then
	AC_MSG_ERROR([missing package: libboost-program-options-dev])
fi

AX_BOOST_FILESYSTEM
if test "x${ax_cv_boost_filesystem}" = "xno" ; then
	AC_MSG_ERROR([missing package: libboost-filesystem-dev])
fi

AX_BOOST_THREAD
if test "x${ax_cv_boost_thread}" = "xno" ; then
    AC_MSG_ERROR([missing package: libboost-thread-dev])
fi

AX_CHECK_GL
AX_CHECK_GLU
AC_CHECK_LIB([GLEW], [glewInit],[AC_SUBST([GLEW_LIBS],["-lGLEW"])])

# SDL
PKG_CHECK_MODULES(SDL, sdl >= 1.2.0)
AC_SUBST(SDL_LIBS)
AC_SUBST(SDL_CFLAGS)

# PORTMIDI
AC_CHECK_LIB([asound],[snd_seq_open],[AC_SUBST([ASOUND_CXXFLAGS],["-lasound"])])
AC_CHECK_LIB([portmidi],[Pm_Initialize],[AC_SUBST([PORTMIDI_CXXFLAGS],["-lportmidi -lporttime"])],[echo "Could not find portmidi !"], ["-lporttime"])

# external tools
AC_CHECK_PROG(MENCODER, mencoder, yes, no)
if test "$MENCODER" = no ; then
    AC_MSG_ERROR([Cannot find mencoder.])
fi


AC_CONFIG_FILES([
    Makefile
    src/Makefile
    tests/Makefile
    ])
AC_OUTPUT
