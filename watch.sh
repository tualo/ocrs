#! /bin/sh
export DB_NAME=sorter
export DB_USER=sorter
export DB_PASSWORD=sorter
export DB_HOST=localhost
export IMAGEPATH=Standardbriefsendungen
export USEPZA=1
export MANDANT=6575
FILES=/imagedata
extension=tiff

for f in $FILES/*.$extension
do
  if [ "$f" != "$FILES/*.$extension" ]
  then
  	echo "Processing $f"
    ocrs $f
  fi
done

inotifywait -m $FILES -e close_write |
  while read path action file; do
    echo "The file '$file' appeared in directory '$path' via '$action'"
    ocrs $FILES/$file &
    # do something with the file
  done
