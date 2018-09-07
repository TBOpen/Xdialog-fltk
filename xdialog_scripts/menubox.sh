#!/bin/sh

Xdialog --title "MENU BOX" \
        --menu "Hi, this is a menu box. You can use this to\n\
present a list of choices for the user to\n\
choose. If there are more items than can fit\n\
on the screen, the menu will be scrolled.\n\
Try it now!\n\n\
Choose the OS you like:" 24 51 7 \
        "Linux"  "The Great Unix Clone for 386/486" \
        "NetBSD" "Another free Unix Clone for 386/486" \
        "OS/2" "IBM OS/2" \
        "WIN NT" "Microsoft Windows NT" \
        "WINXNT" "Microsoft Windows NT" \
        "WINYNT" "Microsoft Windows NT whatever" \
        "WINZNT" "Microsoft Windows NT" \
         "PCDOS"  "IBM PC DOS" \
        "MSDOS"  "Microsoft DOS" 2> /tmp/menu.tmp.$$

retval=$?
choice=`cat /tmp/menu.tmp.$$`
rm -f /tmp/menu.tmp.$$

case $retval in
  0)
    echo "'$choice' chosen.";;
  1)
    echo "Cancel pressed.";;
  255)
    echo "Box closed.";;
esac
