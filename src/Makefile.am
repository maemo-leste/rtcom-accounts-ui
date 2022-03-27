bin_PROGRAMS = rtcom-accounts-ui

rtcom_accounts_ui_CFLAGS = $(ACCOUNTS_UI_CFLAGS)
rtcom_accounts_ui_LDFLAGS = $(ACCOUNTS_UI_LIBS)
rtcom_accounts_ui_LDADD = librtcom-accounts-ui.la

BUILT_SOURCES =						\
		dbus-glib-marshal-aui-service.h		\
		dbus-glib-marshal-aui-instance.h

dbus-glib-marshal-aui-service.h: aui-service.xml
	$(DBUS_BINDING_TOOL) --prefix=aui_service	\
		--mode=glib-server $< > xgen-$(@F)	\
	&& ( cmp -s xgen-$(@F) $@ || cp xgen-$(@F) $@ )	\
	&& rm -f xgen-$(@F)

dbus-glib-marshal-aui-instance.h: aui-instance.xml
	$(DBUS_BINDING_TOOL) --prefix=aui_instance	\
		--mode=glib-server $< > xgen-$(@F)	\
	&& ( cmp -s xgen-$(@F) $@ || cp xgen-$(@F) $@ )	\
	&& rm -f xgen-$(@F)

rtcom_accounts_ui_SOURCES =				\
			main.c				\
			aui-service.c			\
			aui-instance.c

librtcom_accounts_ui_la_LTLIBRARIES = librtcom-accounts-ui.la
librtcom_accounts_ui_ladir = $(cplpluginlibdir)
librtcom_accounts_ui_la_CFLAGS = $(ACCOUNTS_UI_CFLAGS)
librtcom_accounts_ui_la_LDFLAGS = $(ACCOUNTS_UI_LIBS)
librtcom_accounts_ui_la_SOURCES =			\
			accounts-ui.c

CLEANFILES = $(BUILT_SOURCES)

MAINTAINERCLEANFILES = Makefile.in
