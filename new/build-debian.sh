#!/bin/sh

git archive --format=tar.gz -o /tmp/ocrs-1.0.001.tar.gz --prefix=/ master
dh_make -p ocrs_1.0.001 -f /tmp/ocrs-1.0.001.tar.gz
cd debian
rm *.ex *.EX README.Debian README.source
cd ..
