INCLUDES= -I. -I$(srcdir)/ $(GNOME_INCLUDEDIR) \
	  -DGNOMELOCALEDIR=\""$(datadir)/locale"\"

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

LINK_FLAGS= $(GNOME_LIBDIR) $(INTLLIBS) $(GHTTP_LIB) -lgdk_pixbuf -lgnomecanvaspixbuf
telegnome_LDADD=$(LINK_FLAGS) $(GNOME_LIBDIR) $(GNOMEUI_LIBS)
