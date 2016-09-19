#!/bin/sh

#bash build.sh
export DEBUGWINDOW=1
export DEBUG=1
export USEPZA=0

export KEEPFILES=1
export HEAD_OVER=1
export ANALYSETYPE=0
export BARCODE_ALGORTHIM=2
export SCALE=1.2

./ocrs "$1"
# ls /imagedata/*.jpg | parallel -j4 --round-robin './ocrs $f {}'
# 31000001446
# 31000001511
