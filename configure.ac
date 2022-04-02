AC_INIT([rtcom-accounts-ui], [4.140])
AC_CANONICAL_SYSTEM

AM_CONFIG_HEADER(config.h)

AM_MAINTAINER_MODE

AM_INIT_AUTOMAKE

AC_PROG_CC
AC_PROG_INSTALL
AM_PROG_LIBTOOL

AC_HEADER_STDC

AC_PATH_X
AC_PATH_XTRA
AC_SUBST(X_CFLAGS)

PKG_PROG_PKG_CONFIG

PKG_CHECK_MODULES(ACCOUNTS_UI,
                  [hildon-1 libosso telepathy-glib dbus-glib-1 dnl
                  hildon-control-panel libaccounts xproto])
PKG_CHECK_MODULES(ACCOUNTS_WIDGETS,
                  [dbus-glib-1 hildon-1 telepathy-glib libaccounts xproto])

#+++++++++++++++
# Misc programs 
#+++++++++++++++

AC_PATH_PROG(DBUS_BINDING_TOOL, dbus-binding-tool)
AC_PATH_PROG(GLIB_GENMARSHAL, glib-genmarshal)

#+++++++++++++++++++
# Directories setup
#+++++++++++++++++++

cplpluginlibdir=`pkg-config hildon-control-panel --variable=pluginlibdir`
AC_SUBST(cplpluginlibdir)

cplplugindesktopentrydir=`pkg-config hildon-control-panel --variable=plugindesktopentrydir`
AC_SUBST(cplplugindesktopentrydir)

#+++++++++++++++++++
# Misc stuff
#+++++++++++++++++++

AC_DEFINE_UNQUOTED([G_LOG_DOMAIN], "$PACKAGE_NAME", [Default logging facility])

dnl Localization
GETTEXT_PACKAGE=osso-applet-accounts
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "${GETTEXT_PACKAGE}", [gettext package])

DGETTEXT="-D'_(msgid)'='g_dgettext(GETTEXT_PACKAGE, msgid)'"
AC_SUBST(DGETTEXT)

AC_OUTPUT([
	Makefile
	client/Makefile
	lib/Makefile
	service/Makefile
	widgets/Makefile
	com.nokia.AccountsUI.service
	rtcom-accounts-ui.pc
	rtcom-accounts-ui-client.pc
])

