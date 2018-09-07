#!/bin/sh

CHOICE=`Xdialog --title "TREE VIEW BOX" \
		--treeview "treeview box demo" 18 32 4 \
			   tag1 one off 0 \
			   tag2 two off 1 \
			   tag3 three on 2 \
			   tag4 four off 1 2>&1`

case $? in
	0)
		echo "$CHOICE";;
	1)
		echo "Cancel pressed.";;
	255)
		echo "Box closed.";;
esac
