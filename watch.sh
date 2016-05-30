#! /bin/sh

FILES=/imagedata
for f in $FILES/*.tiff
do
	echo "Processing $f"
done

inotifywait -m $FILES -e close_write |
  while read path action file; do
    echo "The file '$file' appeared in directory '$path' via '$action'"
    ocrs $FILES/$file &
    # do something with the file
  done
