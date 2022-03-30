librtcom_accounts_ui_la_LTLIBRARIES = librtcom-accounts-ui.la
librtcom_accounts_ui_ladir = $(cplpluginlibdir)
librtcom_accounts_ui_la_CFLAGS = $(ACCOUNTS_UI_CFLAGS) $(DGETTEXT)	\
		-I$(top_srcdir)/client/
librtcom_accounts_ui_la_LDFLAGS = $(ACCOUNTS_UI_LIBS) -Wl,--no-undefined
librtcom_accounts_ui_la_LIBADD =						\
		$(top_builddir)/client/librtcom-accounts-ui-client.la
librtcom_accounts_ui_la_SOURCES = main.c accounts-ui.c			\
		accounts-wizard-dialog.c

librtcom_accounts_ui_includedir = $(includedir)/@PACKAGE_NAME@
librtcom_accounts_ui_include_HEADERS = accounts-wizard-dialog.h

cplplugindesktopentry_DATA = rtcom-accounts-ui.desktop

MAINTAINERCLEANFILES = Makefile.in
