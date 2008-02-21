AM_CPPFLAGS = -I. -I$(srcdir)/ $(TELEGNOME_CFLAGS)

bin_PROGRAMS=telegnome
telegnome_SOURCES= \
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
	channel.h

LINK_FLAGS= $(GNOME_LIBDIR) $(INTLLIBS) $(GHTTP_LIB)
telegnome_LDADD=$(LINK_FLAGS) $(GNOME_LIBDIR) $(TELEGNOME_LIBS)
