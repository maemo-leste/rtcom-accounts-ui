librtcom_accounts_ui_la_LTLIBRARIES = librtcom-accounts-ui.la
librtcom_accounts_ui_ladir = $(cplpluginlibdir)

librtcom_accounts_ui_la_CFLAGS = $(ACCOUNTS_UI_CFLAGS) $(DGETTEXT)	\
		-DPLUGINLIBDIR=\"@pluginlibdir@\"

librtcom_accounts_ui_la_LDFLAGS = -Wl,--as-needed $(ACCOUNTS_UI_LIBS)	\
		-Wl,--no-undefined -module -avoid-version

librtcom_accounts_ui_la_SOURCES =					\
		main.c							\
		accounts-ui.c						\
		accounts-wizard-dialog.c

librtcom_accounts_ui_includedir = $(includedir)/@PACKAGE_NAME@-ui
librtcom_accounts_ui_include_HEADERS = accounts-wizard-dialog.h

cplplugindesktopentry_DATA = rtcom-accounts-ui.desktop

MAINTAINERCLEANFILES = Makefile.in
