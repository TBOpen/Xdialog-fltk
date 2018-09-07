#!/bin/sh

(echo "10" ; sleep 2

echo "XXX"
echo "The new"
echo "\\n"
echo "message"
echo "XXX"
echo "20" ; sleep 2

echo "75" ; sleep 2) |
Xdialog --title "GAUGE" --gauge "Hi, this is a gauge widget" 10 40

if [ "$?" = 255 ] ; then
	echo ""
	echo "Box closed !"
fi
