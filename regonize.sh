#!/bin/bash


if [[ $# -eq 0 ]] ; then
  echo "ok, we need a filename"
  exit
fi

if [ -f $1 ]; then
  ./ocrs $1 #> /dev/null
  #echo "Code: $?"
  if [ $? -eq 0 ] ; then
    # all is fine
    echo "$f was ok"
  else
    # movin file into error folder
    echo "error in $f"
    name=`basename $1`
    mv $f $STORE_ERRORS$name
  fi
else
  echo "ok, we need a existing filename"
fi
