#!/bin/sh
mkdir /imagedata/result/

export STORE_ORIGINAL=/imagedata/original/
export STORE_ERRORS=/imagedata/error/
export IMAGEPATH=/imagedata/
export MODELL=Clearing
export USEPZA=0
export MANDANT=0000

for f in /tmp/*.tiff
do
	echo "checking $f"
  myfilesize=`stat -c %s $f`
  if [ $myfilesize -lt 5769678 ];then
    echo "the file size is to small"
    fname=`basename $f`
    mv $f $STORE_ERRORS$fname
  fi
  if [ $myfilesize -gt 31457280 ];then
    echo "the file size is to big"
    fname=`basename $f`
    mv $f $STORE_ERRORS$fname
  fi
done

ls /tmp/*.tiff | parallel --round-robin '/root/ocrs/ocrs {}'
