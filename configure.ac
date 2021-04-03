AC_PREREQ(2.52)
AC_INIT([openorion2], [0.1], [next_ghost@quick.cz])
AC_CONFIG_SRCDIR([config.h.in])
AC_CANONICAL_SYSTEM

# cross-compiling support: we're cross compiling if the build system is
# different from the target one (assume host and target be always the same)
if test "$build" != "$host" ; then
    CC=$host_alias-gcc
    CXX=$host_alias-c++
    AR=$host_alias-ar
    RANLIB=$host_alias-ranlib
    DLLTOOL=$host_alias-dlltool
    RESCOMP=$host_alias-windres
    LD=$host_alias-ld
    NM=$host_alias-nm
    STRIP=$host_alias-strip
fi

AM_INIT_AUTOMAKE([subdir-objects])

AM_MAINTAINER_MODE

AC_ARG_ENABLE(debug, [  --enable-debug          Enable debugging information],
              USE_DEBUG="$enableval", USE_DEBUG="no")

# Checks for programs.
AC_PROG_INSTALL
AC_PROG_CC
AC_PROG_CXX
AC_PROG_CXXCPP

PKG_CHECK_MODULES([SDL2], [sdl2 SDL2_mixer])

case "$USE_DEBUG" in
yes)	DEBUG_FLAGS="-g -O0 -Wall"
    ;;    
*)	DEBUG_FLAGS="-O3"
    ;;
esac

CXXFLAGS="$SDL2_CFLAGS $CXXFLAGS $GCC_FLAGS $DEBUG_FLAGS"

AC_CANONICAL_HOST

case "${host_os}" in
linux*|darwin*)
	moo_system=unix ;;
cygwin*|mingw*)
	moo_system=windows ;;
*)
	AC_MSG_ERROR(["$host_os is not supported (yet)"])
esac

AM_CONDITIONAL([SYSTEM_UNIX], [test "x$moo_system" = "xunix"])
AM_CONDITIONAL([SYSTEM_WINDOWS], [test "x$moo_system" = "xwindows"])

AC_SUBST(LIBS)
AC_SUBST(LDFLAGS)
AC_SUBST(CXXFLAGS)

AM_CONFIG_HEADER([config.h])

AC_OUTPUT([
         Makefile
         src/Makefile
         ])

