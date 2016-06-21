#!/bin/bash


if [[ $# -eq 0 ]] ; then
  echo "ok, we need a filename"
  exit
fi

if [ -f $1 ]; then
  /root/ocrs/ocrs $1 #> /dev/null
  #echo "Code: $?"
  if [ $? -eq 0 ] ; then
    # all is fine
    echo "$1 was ok"
  else
    # movin file into error folder
    echo "error in $1"
    name=`basename $1`
    mv $1 $STORE_ERRORS$name
  fi
else
  echo "ok, we need a existing filename"
fi
