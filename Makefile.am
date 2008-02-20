SUBDIRS=macros src po pixmaps doc

EXTRA_DIST = config.rpath m4/ChangeLog \
	$(srcdir)/telegnome.desktop \
	BUGS

Internetdir=$(datadir)/gnome/apps/Internet
Internet_DATA= telegnome.desktop

## to automatically rebuild aclocal.m4 if any of the macros in
## `macros/' change
@MAINT@include macros/macros.dep
@MAINT@macros/macros.dep: macros/Makefile.am
@MAINT@ cd macros && $(MAKE) macros.dep

ACLOCAL_AMFLAGS = -I m4
