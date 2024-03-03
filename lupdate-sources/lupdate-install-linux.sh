#!/usr/bin/bash

NAME_JS='lupdate-js'
NAME_PHP='lupdate-php'
VERSION='1.0.0'
ARCHITECTURE='amd64'
MAINTAINER='Gustav Lindberg <95423695+GustavLindberg99@users.noreply.github.com>'
DESCRIPTION='Creates Qt .ts files from %s files. Part of QtLinguistWeb, see https://github.com/GustavLindberg99/QtLinguistWeb'

LUPDATE_JS_BUILDDIR=/tmp/${NAME_JS}_${VERSION}_${ARCHITECTURE}
LUPDATE_PHP_BUILDDIR=/tmp/${NAME_PHP}_${VERSION}_${ARCHITECTURE}

LUPDATE_JS_BINARY=../build-lupdate-js/linux/lupdate-js
LUPDATE_PHP_BINARY=../build-lupdate-php/linux/lupdate-php

cd $(dirname $0)

# lupdate-js binary
rm -rf $LUPDATE_JS_BUILDDIR 2> /dev/null
mkdir -p $LUPDATE_JS_BUILDDIR/usr/bin
cp $LUPDATE_JS_BINARY $LUPDATE_JS_BUILDDIR/usr/bin

# lupdate-js metadata
mkdir $LUPDATE_JS_BUILDDIR/DEBIAN
echo "Package: ${NAME_JS}
Version: ${VERSION}
Architecture: ${ARCHITECTURE}
Maintainer: ${MAINTAINER}
Description: $(printf $DESCRIPTION 'JavaScript')
" > $LUPDATE_JS_BUILDDIR/DEBIAN/control

# Create lupdate-js deb file
dpkg-deb --build --root-owner-group $LUPDATE_JS_BUILDDIR
mv $LUPDATE_JS_BUILDDIR.deb ../lupdate-js.deb
rm -rf $LUPDATE_JS_BUILDDIR


# lupdate-php binary
rm -rf $LUPDATE_PHP_BUILDDIR 2> /dev/null
mkdir -p $LUPDATE_PHP_BUILDDIR/usr/bin
cp $LUPDATE_PHP_BINARY $LUPDATE_PHP_BUILDDIR/usr/bin

# lupdate-php metadata
mkdir $LUPDATE_PHP_BUILDDIR/DEBIAN
echo "Package: ${NAME_PHP}
Version: ${VERSION}
Architecture: ${ARCHITECTURE}
Maintainer: ${MAINTAINER}
Description: $(printf $DESCRIPTION 'PHP')
" > $LUPDATE_PHP_BUILDDIR/DEBIAN/control

# Create lupdate-php deb file
dpkg-deb --build --root-owner-group $LUPDATE_PHP_BUILDDIR
mv $LUPDATE_PHP_BUILDDIR.deb ../lupdate-php.deb
rm -rf $LUPDATE_PHP_BUILDDIR
