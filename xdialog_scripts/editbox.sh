#!/bin/sh

cat << EOF > /tmp/editbox.tmp.$$
Hi, this is an edit box. It can be used to edit text from a file.

It's like a simple text editor, with these keys implemented:

PGDN	- Move down one page
PGUP	- Move up one page
DOWN	- Move down one line
UP	- Move up one line
DELETE	- Delete the current character
BACKSPC	- Delete the previous character
CTRL C	- Copy text
CTRL V	- Paste text

Mouse selection pasting (selecting text to be copied with left mouse
button held down and pasting by clicking the middle button mouse) is
also available.

Try to imput some text below:

EOF

Xdialog --title "EDIT BOX" --fixed-font --print "" \
	--editbox /tmp/editbox.tmp.$$ 24 75 2>/tmp/editbox.txt.$$

case $? in
  0)
    cat /tmp/editbox.txt.$$
    echo "OK"
    ;;
  1)
    echo "Cancel pressed."
    ;;
  255)
    echo "Box closed."
    ;;
esac

rm -f /tmp/editbox.tmp.$$
rm -f /tmp/editbox.txt.$$
