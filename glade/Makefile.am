librtcom_accounts_widgets_la_LTLIBRARIES = librtcom-accounts-widgets.la
librtcom_accounts_widgets_ladir = $(glademoduledir)

librtcom_accounts_widgets_la_CFLAGS =					\
		$(ACCOUNTS_GLADE_CFLAGS) -I$(top_srcdir)/widgets/

librtcom_accounts_widgets_la_LIBADD =					\
		$(top_builddir)/widgets/librtcom-accounts-widgets.la

librtcom_accounts_widgets_la_LDFLAGS =					\
		-Wl,--as-needed $(ACCOUNTS_GLADE_LIBS)			\
		-Wl,--no-undefined  -module -avoid-version

librtcom_accounts_widgets_la_SOURCES =					\
		widgets.c

librtcom_hildon_la_LTLIBRARIES = librtcom-hildon.la

librtcom_hildon_ladir = $(glademoduledir)

librtcom_hildon_la_CFLAGS =						\
		$(ACCOUNTS_GLADE_CFLAGS)

librtcom_hildon_la_LDFLAGS =						\
		-Wl,--as-needed $(ACCOUNTS_GLADE_LIBS)			\
		-Wl,--no-undefined  -module -avoid-version

librtcom_hildon_la_SOURCES =						\
		hildon.c

MAINTAINERCLEANFILES = Makefile.in
