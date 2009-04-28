AC_INIT
AC_PREREQ(2.50)
AC_CONFIG_SRCDIR([src/main.c])
AC_CONFIG_MACRO_DIR([m4])

AM_CONFIG_HEADER(config.h)
AM_INIT_AUTOMAKE(telegnome, 0.1.1)

AM_MAINTAINER_MODE

GNOME_COMMON_INIT
GNOME_DEBUG_CHECK
GNOME_COMPILE_WARNINGS([maximum])
GNOME_DOC_INIT

AC_ISC_POSIX
AC_PROG_CC
AC_PROG_CPP
AC_HEADER_STDC
AC_PROG_INSTALL

PKG_CHECK_MODULES([TELEGNOME], [glib-2.0 gtk+-2.0 >= 2.12 gdk-pixbuf-2.0 libgnomeui-2.0 gnome-vfs-2.0])

PROGRAMS_TELEGNOME="telegnome"
AC_SUBST(PROGRAMS_TELEGNOME)

GETTEXT_PACKAGE=telegnome
AC_SUBST([GETTEXT_PACKAGE])
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE], ["$GETTEXT_PACKAGE"], [Gettext Package])
AM_GLIB_GNU_GETTEXT
AM_GLIB_DEFINE_LOCALEDIR([GNOMELOCALEDIR])

AC_CONFIG_FILES([Makefile 
	   pixmaps/Makefile 
	   src/Makefile
	   po/Makefile.in
	   doc/Makefile
	 ])
AC_OUTPUT