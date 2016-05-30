#! /bin/sh
export DB_NAME=sorter
export DB_USER=sorter
export DB_PASSWORD=sorter
export DB_HOST=localhost
export IMAGEPATH=Standardbriefsendungen
export USEPZA=1
export MANDANT=6575
FILES=/imagedata
for f in $FILES/*.tiff
do
	echo "Processing $f"
  ocrs $f
done

inotifywait -m $FILES -e close_write |
  while read path action file; do
    echo "The file '$file' appeared in directory '$path' via '$action'"
    ocrs $FILES/$file &
    # do something with the file
  done
