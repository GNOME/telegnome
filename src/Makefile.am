AM_CPPFLAGS = -I. -I$(srcdir)/ -include $(top_builddir)/config.h
AM_CFLAGS = $(GLIB_CFLAGS) $(TELEGNOME_CFLAGS) $(VALA_CFLAGS)
AM_VALAFLAGS = \
	--pkg posix \
	--pkg gtk+-3.0 \
	--pkg cairo \
	--pkg dconf

bin_PROGRAMS = telegnome

telegnome_SOURCES = \
	app.vala \
	channel.vala \
	gui.vala \
	http.vala \
	legacy-config.vala \
	main.vala \
	paths.vala \
	pixpack.vala \
	prefs.vala \
	uuid.vapi

telegnome_built = \
	resources.c \
	resources.h
BUILT_SOURCES = $(telegnome_built)
nodist_telegnome_SOURCES = $(telegnome_built)
CLEANFILES = \
	$(telegnome_built) \
	app.c \
	channel.c \
	gui.c \
	http.c \
	legacy-config.c \
	main.c \
	pixpack.c \
	prefs.c

do_subst = sed \
	-e 's,[@]sysconfdir[@],$(sysconfdir),g' \
	-e 's,[@]localedir[@],$(localedir),g'

paths.vala: paths.vala.in Makefile
	$(do_subst) < $(srcdir)/paths.vala.in > $@

resources.c resources.h: telegnome.gresource.xml Makefile $(shell $(GLIB_COMPILE_RESOURCES) --generate-dependencies --sourcedir $(srcdir) $(srcdir)/telegnome.gresource.xml)
	$(AM_V_GEN) XMLLINT=$(XMLLINT) $(GLIB_COMPILE_RESOURCES) --target $@ --sourcedir $(srcdir) --generate --c-name tg $<

LINK_FLAGS= $(GNOME_LIBDIR) $(INTLLIBS) $(GHTTP_LIB)
telegnome_LDADD=$(LINK_FLAGS) $(GNOME_LIBDIR) $(GLIB_LIBS) $(TELEGNOME_LIBS)

EXTRA_DIST = \
	menus.ui \
	prefs.ui \
	telegnome.gresource.xml \
	telegnome.ui
