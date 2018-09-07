#!/bin/sh

Xdialog --backtitle "Red Hat Software Linux" \
	--title "RADIOLIST BOX" \
        --radiolist "Hi, this is a radiolist box. You can use this\n\
to make a choice from a list of items.\n\
If there are more items than can fit on the\n\
screen, the list will be scrolled.\n\n\
Which of the following is a fruit ?" 26 46 5 \
        "Tiger"  "A dangerous animal." off \
        "Dog"    "No, that's not my dog." ON \
        "Orange" "Yeah, that's juicy." off \
        "Cat"    "No, never put a dog and a cat together!" off \
        "Fish"   "Cats like fish." off 2>/tmp/checklist.tmp.$$

retval=$?
choice=`cat /tmp/checklist.tmp.$$`
rm -f /tmp/checklist.tmp.$$

case $retval in
  0)
    echo "'$choice' chosen.";;
  1)
    echo "Cancel pressed.";;
  255)
    echo "Box closed.";;
esac
