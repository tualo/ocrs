#!/bin/sh
#mkdir /imagedata/result/

export STORE_ORIGINAL=/imagedata/original/
export STORE_ERRORS=/imagedata/error/
export IMAGEPATH=/imagedata/

export WIDTH_CM=28
export SCALE=1.3

export MODELL=Clearing
export USEPZA=0
export MANDANT=0000

#if [[ $# -eq 0 ]] ; then
#  export X=1
#else
#  export IMAGEPATH=$1
#  export STORE_ERRORS=$1error/
#  export STORE_ORIGINAL=$1original/
#fi
echo "checking $IMAGEPATH";
while :
do
  if [ "$(ls -A $IMAGEPATH*.tiff 2> /dev/null )" ]; then
    ls $IMAGEPATH*.tiff | parallel --round-robin '/root/regonize.sh {}' > /dev/null
  else
    sleep 1
  fi
done
