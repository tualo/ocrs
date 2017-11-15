#!/bin/bash
for f in $1/*.tiff
do
  name=`basename $f`
  echo "Processing $f file ($name)..."
  ./recognize.sh "$f"
done
