#!/bin/sh

Xdialog --title "INPUT BOX" \
        --inputbox "Hi, this is an input dialog box. You can use\n\
this to ask questions that require the user\n\
to input a string as the answer. You can\n\
input strings of length longer than the\n\
width of the input box, in that case, the\n\
input field will be automatically scrolled.\n\
You can use BACKSPACE to correct errors.\n\n\
Try inputing your name below:" 18 45 2> /tmp/inputbox.tmp.$$

retval=$?
input=`cat /tmp/inputbox.tmp.$$`
rm -f /tmp/inputbox.tmp.$$

case $retval in
  0)
    echo "Input string is '$input'";;
  1)
    echo "Cancel pressed.";;
  255)
    echo "Box closed.";;
esac
