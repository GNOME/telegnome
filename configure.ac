AC_INIT([TeleGNOME], [0.3.7], [cjwatson@debian.org], [telegnome])
AC_PREREQ(2.50)
AC_CONFIG_SRCDIR([src/main.vala])
AC_CONFIG_MACRO_DIRS([m4])

AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([1.13 dist-xz no-dist-gzip])

AM_MAINTAINER_MODE
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AX_REQUIRE_DEFINED([YELP_HELP_INIT])

AX_IS_RELEASE([git-directory])
AX_CHECK_ENABLE_DEBUG([yes], [GNOME_ENABLE_DEBUG])
AX_COMPILER_FLAGS
dnl https://gitlab.gnome.org/GNOME/vala/-/issues/1408
AX_APPEND_COMPILE_FLAGS([-Wno-error=incompatible-pointer-types])
YELP_HELP_INIT
GLIB_GSETTINGS

AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL

AC_DEFINE_UNQUOTED([SYSCONFDIR], "$sysconfdir", [System configuration directory.])

AM_PROG_VALAC

AM_PATH_GLIB_2_0([2.44.0], [], [AC_MSG_ERROR([GLib >= 2.44.0 is required])], [gobject gio])
PKG_CHECK_MODULES([TELEGNOME], [gtk+-3.0 >= 3.22 gdk-pixbuf-2.0 >= 2.26 cairo >= 1.10 uuid])

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
AM_GNU_GETTEXT_VERSION([0.19.7])
AM_GNU_GETTEXT([external])

AC_CONFIG_FILES([Makefile 
	   data/Makefile
	   pixmaps/Makefile 
	   src/Makefile
	   po/Makefile.in
	   doc/Makefile
	 ])
AC_OUTPUT
