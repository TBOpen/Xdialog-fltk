#!/bin/sh

cat << EOF > /tmp/textbox.tmp.$$
Hi, this is a text dialog box. It can be used to display text from a file.

It's like a simple text file viewer, with these keys implemented:

PGDN	- Move down one page
PGUP	- Move up one page
DOWN	- Move down one line
UP	- Move up one line

The following is a sample text file:

EOF

cat /etc/bashrc | expand >> /tmp/textbox.tmp.$$

Xdialog --title "TEXT BOX" --textbox "/tmp/textbox.tmp.$$" 22 77

case $? in
  0)
    echo "OK";;
  1)
    echo "Cancel pressed.";;
  255)
    echo "Box closed.";;
esac

rm -f /tmp/textbox.tmp.$$
