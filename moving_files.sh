#!/bin/sh

for f in /imagedata/result/*.jpg
do
  b = `basename $f`
  cp $f /server/result/$b && rm $f
done

for f in /imagedata/original/*.jpg
do
  b = `basename $f`
  cp $f /server/original/$b && rm $f
done
