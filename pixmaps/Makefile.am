icons = \
	telegnome-icon-48x48.png \
	telegnome-icon-64x64.png

install-icons:
	set -e; for icon in $(icons); do \
		size=$$(echo "$$icon" | cut -d- -f3 | cut -d. -f1); \
		mkdir -p $(DESTDIR)$(datadir)/icons/hicolor/$$size/apps; \
		$(INSTALL_DATA) $(srcdir)/$$icon \
			$(DESTDIR)$(datadir)/icons/hicolor/$$size/apps/telegnome.png; \
	done

uninstall-icons:
	set -e; for icon in $(icons); do \
		size=$$(echo "$$icon" | cut -d- -f3 | cut -d. -f1); \
		rm -f $(DESTDIR)$(datadir)/icons/hicolor/$$size/apps/telegnome.png; \
	done

GTK_UPDATE_ICON_CACHE = gtk-update-icon-cache -f -t

update-icon-cache:
	@-if test -z "$(DESTDIR)"; then \
		echo "Updating Gtk icon cache."; \
		$(GTK_UPDATE_ICON_CACHE) $(datadir)/icons/hicolor; \
	else \
		echo "*** Icon cache not updated.  After (un)install, run this:"; \
		echo "***   $(GTK_UPDATE_ICON_CACHE) $(datadir)/icons/hicolor"; \
	fi

install-data-local: install-icons update-icon-cache

uninstall-local: uninstall-icons update-icon-cache

EXTRA_DIST = \
	$(icons) \
	telegnome-logo.png \
	testbeeld.png
