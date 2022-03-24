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

AM_GCONF_SOURCE_2
AC_PATH_PROG(GCONFTOOL, gconftool-2)

PKG_PROG_PKG_CONFIG

#+++++++++++++++
# Misc programs 
#+++++++++++++++

AC_PATH_PROG(DBUS_BINDING_TOOL, dbus-binding-tool)

PKG_CHECK_MODULES(ACCOUNTS_UI_SERVICE,
			[glib-2.0 dnl
			hildon-1 dnl
			libosso dnl
			dbus-glib-1])

#+++++++++++++++++++
# Directories setup
#+++++++++++++++++++

hildondesktoplibdir=`pkg-config libhildondesktop-1 --variable=hildondesktoplibdir`
AC_SUBST(hildondesktoplibdir)

AC_OUTPUT([
Makefile
src/Makefile
com.nokia.AccountsUI.service
])

