AC_INIT([TeleGNOME], [0.3.0], [cjwatson@debian.org], [telegnome])
AC_PREREQ(2.50)
AC_CONFIG_SRCDIR([src/main.vala])
AC_CONFIG_MACRO_DIR([m4])

AM_CONFIG_HEADER(config.h)
AM_INIT_AUTOMAKE([dist-xz no-dist-gzip])

AM_MAINTAINER_MODE
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

GNOME_COMMON_INIT
GNOME_DEBUG_CHECK
GNOME_COMPILE_WARNINGS([maximum])
GNOME_DOC_INIT
GLIB_GSETTINGS

AC_ISC_POSIX
AC_PROG_CC
AC_PROG_CPP
AC_HEADER_STDC
AC_PROG_INSTALL

AC_DEFINE_UNQUOTED([SYSCONFDIR], "$sysconfdir", [System configuration directory.])

AM_PROG_VALAC

AM_PATH_GLIB_2_0([2.44.0], [], [AC_MSG_ERROR([GLib >= 2.44.0 is required])], [gobject gio])
PKG_CHECK_MODULES([TELEGNOME], [gtk+-3.0 >= 3.8 gdk-pixbuf-2.0 >= 2.26 cairo >= 1.10 dconf uuid])

AC_ARG_VAR([XMLLINT], [the xmllint program])
AC_PATH_PROG([XMLLINT], [xmllint])
if test -z "$XMLLINT"; then
  AC_MSG_ERROR([xmllint not found])
fi

PROGRAMS_TELEGNOME="telegnome"
AC_SUBST(PROGRAMS_TELEGNOME)

GETTEXT_PACKAGE=telegnome
AC_SUBST([GETTEXT_PACKAGE])
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE], ["$GETTEXT_PACKAGE"], [Gettext Package])
AM_GLIB_GNU_GETTEXT
AM_GLIB_DEFINE_LOCALEDIR([GNOMELOCALEDIR])
IT_PROG_INTLTOOL([0.35.0], [no-xml])

AC_CONFIG_FILES([Makefile 
	   data/Makefile
	   pixmaps/Makefile 
	   src/Makefile
	   po/Makefile.in
	   doc/Makefile
	 ])
AC_OUTPUT
