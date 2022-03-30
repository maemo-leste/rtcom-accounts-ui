bin_PROGRAMS = rtcom-accounts-ui

rtcom_accounts_ui_CFLAGS = -I$(top_srcdir)/lib/			\
		$(ACCOUNTS_UI_CFLAGS) $(DGETTEXT)
rtcom_accounts_ui_LDFLAGS = $(ACCOUNTS_UI_LIBS)
rtcom_accounts_ui_LDADD =					\
		$(top_builddir)/lib/librtcom-accounts-ui.la

BUILT_SOURCES =							\
		dbus-glib-marshal-aui-service.h			\
		dbus-glib-marshal-aui-instance.h

dbus-glib-marshal-aui-service.h: aui-service.xml
	$(DBUS_BINDING_TOOL) --prefix=aui_service		\
		--mode=glib-server $< > xgen-$(@F)		\
	&& ( cmp -s xgen-$(@F) $@ || cp xgen-$(@F) $@ )		\
	&& rm -f xgen-$(@F)

dbus-glib-marshal-aui-instance.h: aui-instance.xml
	$(DBUS_BINDING_TOOL) --prefix=aui_instance		\
		--mode=glib-server $< > xgen-$(@F)		\
	&& ( cmp -s xgen-$(@F) $@ || cp xgen-$(@F) $@ )		\
	&& rm -f xgen-$(@F)

rtcom_accounts_ui_SOURCES =					\
			main.c					\
			aui-service.c				\
			aui-instance.c

CLEANFILES = $(BUILT_SOURCES)

MAINTAINERCLEANFILES = Makefile.in
