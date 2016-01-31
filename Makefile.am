SUBDIRS = data src po pixmaps doc

EXTRA_DIST = $(srcdir)/telegnome.desktop.in \
	BUGS \
	ChangeLog-pre-2-0 \
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

# Build ChangeLog from git history
ChangeLog:
	$(AM_V_GEN) if test -d "$(top_srcdir)/.git"; then \
		GIT_DIR="$(top_srcdir)/.git" git log f9312743cfed0e512e7870a42fcb2a1b376aaf3c.. --stat > $@; \
	fi

dist: ChangeLog

.PHONY: ChangeLog
