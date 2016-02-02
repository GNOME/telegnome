AM_CPPFLAGS = -I. -I$(srcdir)/ $(GLIB_CFLAGS) $(TELEGNOME_CFLAGS)

bin_PROGRAMS = telegnome

telegnome_SOURCES = \
	main.h \
	main.c \
	http.h \
	http.c \
	gui.h \
	gui.c \
	prefs.h \
	prefs.c \
	menu.h \
	view.h \
	view.c \
	pixpack.c \
	pixpack.h \
	channel.c \
	channel.h \
	legacy-config.c \
	legacy-config.h

telegnome_built = \
	resources.c \
	resources.h
BUILT_SOURCES = $(telegnome_built)
nodist_telegnome_SOURCES = $(telegnome_built)

resources.c resources.h: telegnome.gresource.xml Makefile $(shell $(GLIB_COMPILE_RESOURCES) --generate-dependencies --sourcedir $(srcdir) $(srcdir)/telegnome.gresource.xml)
	$(AM_V_GEN) XMLLINT=$(XMLLINT) $(GLIB_COMPILE_RESOURCES) --target $@ --sourcedir $(srcdir) --generate --c-name tg $<

LINK_FLAGS= $(GNOME_LIBDIR) $(INTLLIBS) $(GHTTP_LIB)
telegnome_LDADD=$(LINK_FLAGS) $(GNOME_LIBDIR) $(GLIB_LIBS) $(TELEGNOME_LIBS)
