#!/bin/sh

Xdialog --title "MESSAGE BOX" \
        --msgbox "Hi, this is a simple message box. You can use this to
display any message you like. The box will remain until
you press the ENTER key." 10 41

case $? in
  0)
    echo "OK";;
  255)
    echo "Box closed.";;
esac