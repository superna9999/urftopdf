#!/bin/sh

[ ! -d $DESTDIR/usr/share/cups/mime ] && mkdir -p $DESTDIR/usr/share/cups/mime
cp urftopdf.types  $DESTDIR/usr/share/cups/mime
cp urftopdf.convs  $DESTDIR/usr/share/cups/mime
[ ! -d $DESTDIR/usr/lib/cups/filter ] && mkdir -p $DESTDIR/usr/lib/cups/filter
cp urftopdf $DESTDIR/usr/lib/cups/filter
