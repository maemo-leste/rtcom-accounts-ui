lib_LTLIBRARIES = librtcom-accounts-widgets.la
librtcom_accounts_widgets_la_CFLAGS = $(ACCOUNTS_WIDGETS_CFLAGS)	\
		$(DGETTEXT)
librtcom_accounts_widgets_la_LDFLAGS = -Wl,--as-needed			\
		$(ACCOUNTS_WIDGETS_LIBS) -Wl,--no-undefined
#librtcom_accounts_widgets_la_LIBADD =					\
#		$(top_builddir)/client/librtcom-accounts-ui-client.la
librtcom_accounts_widgets_la_SOURCES =					\
		rtcom-account-marshal.c					\
		rtcom-account-item.c \
		rtcom-account-plugin.c \
		rtcom-dialog-context.c \
		rtcom-page.c \
		rtcom-widget.c \
		rtcom-account-service.c

#librtcom_accounts_widgets_includedir = $(includedir)/@PACKAGE_NAME@
#librtcom_accounts_widgets_include_HEADERS = accounts-wizard-dialog.h

rtcom-account-marshal.c: rtcom-account-marshal.list
	$(GLIB_GENMARSHAL) --prefix=rtcom_account_marshal $< --header	\
	--body --internal > xgen-$(@F)	&& ( cmp -s xgen-$(@F) $@ ||	\
	cp xgen-$(@F) $@ ) && rm -f xgen-$(@F)

rtcom-account-marshal.h: rtcom-account-marshal.list
	$(GLIB_GENMARSHAL) --prefix=rtcom_account_marshal $< --header	\
	--internal > xgen-$(@F) && ( cmp -s xgen-$(@F) $@ ||		\
	cp xgen-$(@F) $@ ) && rm -f xgen-$(@F)

BUILT_SOURCES =								\
		rtcom-account-marshal.c					\
		rtcom-account-marshal.h

CLEANFILES = $(BUILT_SOURCES)
MAINTAINERCLEANFILES = Makefile.in
