gnome_hello_helpdir = $(datadir)/gnome/help/telegnome/C

gnome_hello_help_DATA =         \
        telegnome.html        \
        topic.dat

SGML_FILES =                    \
        telegnome.sgml

# files that aren't in a binary/data/library target have to be listed here
# to be included in the tarball when you 'make dist'
EXTRA_DIST = \
        topic.dat               \
        $(SGML_FILES)


## The - before the command means to ignore it if it fails.  that way
## people can still build the software without the docbook tools

all: 

telegnome.html: telegnome/telegnome.html
	-cp telegnome/telegnome.html .

telegnome/telegnome.html: $(SGML_FILES)
	-db2html telegnome.sgml

## when we make dist, we include the generated HTML so people don't
## have to have the docbook tools
dist-hook:
	mkdir $(distdir)/telegnome
	-cp telegnome/*.html telegnome/*.css $(distdir)/telegnome
	-cp telegnome.html $(distdir)

install-data-local: telegnome.html
	$(mkinstalldirs) $(gnome_hello_helpdir)/images
	-for file in $(srcdir)/telegnome/*.html $(srcdir)/telegnome/*.css; do \
	basefile=`basename $$file`; \
	$(INSTALL_DATA) $(srcdir)/$$file $(gnome_hello_helpdir)/$$basefile; \
	done

telegnome.ps: telegnome.sgml
	-db2ps $<

telegnome.rtf: telegnome.sgml       
	-db2rtf $<

