SUBDIRS = . data src po pixmaps doc

EXTRA_DIST = \
	ChangeLog-pre-2-0 \
	MAINTAINERS \
	telegnome.doap

appsdir=$(datadir)/applications
apps_in_files = telegnome.desktop.in
apps_DATA = telegnome.desktop

telegnome.desktop: telegnome.desktop.in
	$(AM_V_GEN)$(MSGFMT) --desktop --template $< -d $(top_srcdir)/po -o $@

appstreamdir = $(datadir)/metainfo
appstream_in_files = telegnome.appdata.xml.in
appstream_DATA = telegnome.appdata.xml

telegnome.appdata.xml: telegnome.appdata.xml.in
	$(AM_V_GEN)$(MSGFMT) --xml --template $< -d $(top_srcdir)/po -o $@

EXTRA_DIST += $(apps_in_files) $(appstream_in_files)
CLEANFILES = $(apps_DATA) $(appstream_DATA)

# Build ChangeLog from git history
ChangeLog:
	$(AM_V_GEN) if test -d "$(top_srcdir)/.git"; then \
		GIT_DIR="$(top_srcdir)/.git" git log f9312743cfed0e512e7870a42fcb2a1b376aaf3c.. --stat > $@; \
	fi

dist: ChangeLog

.PHONY: ChangeLog
