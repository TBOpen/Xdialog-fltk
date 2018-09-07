#!/bin/sh

Xdialog --title "INFO BOX" \
        --infobox "Hi, this is an information box. It is
different from a message box in that it will
not pause waiting for input after displaying
the message.

You have 20 seconds to read this..." 13 45 20000

case $? in
  0)
    echo "OK";;
  255)
    echo "Box closed.";;
esac
