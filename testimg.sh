#!/bin/sh

bash build.sh
export DEBUGWINDOW=1
export USEPZA=0
export KEEPFILES=1
./ocrs $1

# 31000001446
# 31000001511
