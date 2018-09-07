#!/bin/sh

USERTIME=`Xdialog --stdout --title "TIMEBOX" --timebox "Please set the time..." 0 0`

case $? in
  0)
    echo "Time entered: $USERTIME.";;
  1)
    echo "Cancel pressed.";;
  255)
    echo "Box closed.";;
esac
