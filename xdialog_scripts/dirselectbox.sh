#!/bin/sh

DIR=`Xdialog --title "Please choose a directory" --dselect /etc 0 0 2>&1`

case $? in
	0)
		echo "\"$DIR\" chosen";;
	1)
		echo "Cancel pressed.";;
	255)
		echo "Box closed.";;
esac
