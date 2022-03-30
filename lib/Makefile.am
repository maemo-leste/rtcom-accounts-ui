librtcom_accounts_ui_la_LTLIBRARIES = librtcom-accounts-ui.la
librtcom_accounts_ui_ladir = $(cplpluginlibdir)
librtcom_accounts_ui_la_CFLAGS = $(ACCOUNTS_UI_CFLAGS) $(DGETTEXT)	\
		-I$(top_srcdir)/client/
librtcom_accounts_ui_la_LDFLAGS = $(ACCOUNTS_UI_LIBS)
librtcom_accounts_ui_la_SOURCES = main.c accounts-ui.c			\
		accounts-wizard-dialog.c

CLEANFILES = $(BUILT_SOURCES)

MAINTAINERCLEANFILES = Makefile.in
