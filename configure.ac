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

#+++++++++++++++
# Misc programs 
#+++++++++++++++

AC_PATH_PROG(DBUS_BINDING_TOOL, dbus-binding-tool)

PKG_CHECK_MODULES(ACCOUNTS_UI,
                  [glib-2.0 hildon-1 libosso telepathy-glib dbus-glib-1 dnl
                  hildon-control-panel libaccounts xproto])

#+++++++++++++++++++
# Directories setup
#+++++++++++++++++++

cplpluginlibdir=`pkg-config hildon-control-panel --variable=pluginlibdir`
AC_SUBST(cplpluginlibdir)

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
	lib/Makefile
	service/Makefile
	com.nokia.AccountsUI.service
	rtcom-accounts-ui.pc
])

