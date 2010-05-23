#!/bin/bash
# to use:
# sudo apt-get install libnotify-bin

cd "`dirname $BASH_SOURCE`"
#notify-send -t 2000 "Building boomers..."
./autogen.sh
./configure
make -j
#gksu make install 
#notify-send -t 10000 "Done building boomers !"

