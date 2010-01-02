SUBDIRS=src po pixmaps doc

EXTRA_DIST = $(srcdir)/telegnome.desktop.in \
	BUGS \
	MAINTAINERS \
	gnome-doc-utils.make \
	intltool-extract.in \
	intltool-merge.in \
	intltool-update.in \
	telegnome.doap

CLEANFILES = telegnome.desktop

DISTCLEANFILES = intltool-extract intltool-merge intltool-update

MAINTAINERCLEANFILES = gnome-doc-utils.make

appsdir=$(datadir)/applications
apps_in_files = telegnome.desktop.in
apps_DATA = $(apps_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

ACLOCAL_AMFLAGS = -I m4
