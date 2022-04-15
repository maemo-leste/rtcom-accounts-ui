lib_LTLIBRARIES = librtcom-accounts-widgets.la

librtcom_accounts_widgets_la_CFLAGS =					\
		$(ACCOUNTS_WIDGETS_CFLAGS) $(DGETTEXT)

librtcom_accounts_widgets_la_LDFLAGS =					\
		-Wl,--as-needed $(ACCOUNTS_WIDGETS_LIBS)		\
		-Wl,--no-undefined

librtcom_accounts_widgets_la_SOURCES =					\
		rtcom-account-marshal.c					\
		rtcom-account-item.c					\
		rtcom-account-plugin.c					\
		rtcom-dialog-context.c					\
		rtcom-page.c						\
		rtcom-widget.c						\
		rtcom-username.c					\
		rtcom-login.c						\
		rtcom-param-string.c					\
		rtcom-param-int.c					\
		rtcom-param-bool.c					\
		rtcom-enabled.c						\
		rtcom-edit.c						\
		rtcom-alias.c						\
		rtcom-avatar.c						\
		rtcom-displayname.c					\
		rtcom-entry-validation.c				\
		rtcom-account-service.c

librtcom_accounts_widgets_includedir =					\
		$(includedir)/lib@PACKAGE_NAME@-widgets

librtcom_accounts_widgets_include_HEADERS =				\
		rtcom-account-item.h					\
		rtcom-account-plugin.h					\
		rtcom-account-service.h					\
		rtcom-accounts.h					\
		rtcom-alias.h						\
		rtcom-avatar.h						\
		rtcom-context.h						\
		rtcom-dialog-context.h					\
		rtcom-displayname.h					\
		rtcom-edit.h						\
		rtcom-enabled.h						\
		rtcom-entry-validation.h				\
		rtcom-login.h						\
		rtcom-page.h						\
		rtcom-param-bool.h					\
		rtcom-param-int.h					\
		rtcom-param-string.h					\
		rtcom-username.h					\
		rtcom-widget.h

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
