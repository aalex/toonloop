#!/bin/sh
if test ! -e NEWS ; then
touch NEWS
fi

if test ! -e INSTALL ; then
touch INSTALL
fi

if test ! -e AUTHORS ; then
touch AUTHORS
fi

if test ! -e README ; then
touch README
fi

if test ! -e ChangeLog ; then
touch ChangeLog
fi

# could be replaced with autoreconf -fivI m4 (verbose, force rebuild of ltmain, .in files, etc.)
libtoolize --force
aclocal -I m4
autoheader
automake -a -f --add-missing
autoconf -f
#./configure $@
