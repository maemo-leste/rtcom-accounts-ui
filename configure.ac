AC_INIT([rtcom-accounts], [4.141])
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
                  hildon-control-panel libaccounts xproto dnl
                  rtcom-accounts-ui-client])
PKG_CHECK_MODULES(ACCOUNTS_WIDGETS,
                  [dbus-glib-1 hildon-1 telepathy-glib libaccounts dnl
                  xproto conic libhildonmime libosso-abook-1.0])

PKG_CHECK_MODULES(ACCOUNTS_GLADE, [libglade-2.0 hildon-1 telepathy-glib])

#+++++++++++++++
# Misc programs 
#+++++++++++++++

AC_PATH_PROG(DBUS_BINDING_TOOL, dbus-binding-tool)
AC_PATH_PROG(GLIB_GENMARSHAL, glib-genmarshal)

#+++++++++++++++++++
# Directories setup
#+++++++++++++++++++

cplpluginlibdir=`$PKG_CONFIG hildon-control-panel --variable=pluginlibdir`
AC_SUBST(cplpluginlibdir)

cplplugindesktopentrydir=`$PKG_CONFIG hildon-control-panel --variable=plugindesktopentrydir`
AC_SUBST(cplplugindesktopentrydir)

pluginlibdir="`$PKG_CONFIG --variable=pluginlibdir libaccounts`"
AC_SUBST(pluginlibdir)

dbusinterfacedir="`$PKG_CONFIG --variable=dbusinterfacedir rtcom-accounts-ui-client`"
AC_SUBST(dbusinterfacedir)

glademoduledir="`$PKG_CONFIG --variable=moduledir libglade-2.0`"
AC_SUBST(glademoduledir)

#+++++++++++++++++++
# Misc stuff
#+++++++++++++++++++

AC_ARG_ENABLE(cast-checks,  [  --disable-cast-checks   compile with GLIB cast checks disabled],[cchecks=${enableval}],cchecks=yes)
if test "x$cchecks" = "xno"; then
    CFLAGS="$CFLAGS -DG_DISABLE_CAST_CHECKS"
fi

AC_ARG_ENABLE(asserts,      [  --disable-asserts       compile with GLIB assertions disabled],[asserts=${enableval}],asserts=yes)
if test "x$asserts" = "xno"; then
    CFLAGS="$CFLAGS -DG_DISABLE_ASSERTS"
fi

AC_ARG_ENABLE(checks,       [  --disable-checks        compile with GLIB checks disabled],[checks=${enableval}],checks=yes)
if test "x$checks" = "xno"; then
    CFLAGS="$CFLAGS -DG_DISABLE_CHECKS"
fi

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
	widgets/Makefile
	glade/Makefile
	com.nokia.AccountsUI.service
	rtcom-accounts-ui.pc
	rtcom-accounts-widgets.pc
])

