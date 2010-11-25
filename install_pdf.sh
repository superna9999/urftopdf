#!/bin/sh

cp urftopdf.types  $DESTDIR/usr/share/cups/mime
cp urftopdf.convs  $DESTDIR/usr/share/cups/mime
cp urftopdf $DESTDIR/usr/lib/cups/filter/
