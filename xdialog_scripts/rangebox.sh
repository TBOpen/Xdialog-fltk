#!/bin/sh

VOLUME=`Xdialog 2>&1 --title "RANGE BOX" --rangebox "Please set the volume..." 8 24 0 12 5`

case $? in
  0)
    echo "Volume set to $VOLUME.";;
  1)
    echo "Cancel pressed.";;
  255)
    echo "Box closed.";;
esac