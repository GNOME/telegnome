SUBDIRS = data src po pixmaps doc

EXTRA_DIST = \
	ChangeLog-pre-2-0 \
	MAINTAINERS \
	gnome-doc-utils.make \
	intltool-extract.in \
	intltool-merge.in \
	intltool-update.in \
	telegnome.doap

DISTCLEANFILES = intltool-extract intltool-merge intltool-update

MAINTAINERCLEANFILES = gnome-doc-utils.make

appsdir=$(datadir)/applications
apps_in_files = telegnome.desktop.in
apps_DATA = $(apps_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

appstreamdir = $(datadir)/appdata
appstream_in_files = telegnome.appdata.xml.in
appstream_DATA = $(appstream_in_files:.xml.in=.xml)
@INTLTOOL_XML_RULE@

EXTRA_DIST += $(apps_in_files) $(appstream_in_files)
CLEANFILES = $(apps_DATA) $(appstream_DATA)

ACLOCAL_AMFLAGS = -I m4

# Build ChangeLog from git history
ChangeLog:
	$(AM_V_GEN) if test -d "$(top_srcdir)/.git"; then \
		GIT_DIR="$(top_srcdir)/.git" git log f9312743cfed0e512e7870a42fcb2a1b376aaf3c.. --stat > $@; \
	fi

dist: ChangeLog

.PHONY: ChangeLog
