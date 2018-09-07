#!/bin/sh

FILE=`Xdialog --title "Please choose a file" --fselect /home 28 48 2>&1`

case $? in
	0)
		echo "\"$FILE\" chosen";;
	1)
		echo "Cancel pressed.";;
	255)
		echo "Box closed.";;
esac
