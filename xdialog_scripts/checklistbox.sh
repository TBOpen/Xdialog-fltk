#!/bin/sh

Xdialog --backtitle "Red Hat Software Linux" --title "CHECKLIST BOX" \
        --checklist "Hi, this is a checklist box. You can use this to\n\
present a list of choices which can be turned on or\n\
off. If there are more items than can fit on the\n\
screen, the list will be scrolled.\n\n\
Which of the following are fruits?" 30 61 6 \
        "Apple"  "It's an apple." off \
        "Dog"    "No, that's not my dog." ON \
        "Orange" "Yeah, that's juicy." off \
        "Cat"    "No, never put a dog and a cat together!" oN \
        "Fish"   "Cats like fish." On \
        "Lemon"  "You know how it tastes." on 2> /tmp/checklist.tmp.$$

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
