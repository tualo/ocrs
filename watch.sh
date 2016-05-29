#! /bin/sh

inotifywait -m /imagedata -e close_write |
  while read path action file; do
    echo "The file '$file' appeared in directory '$path' via '$action'"
    /home/tualo/ocrs/ocrs /imagedata/$file &
    # do something with the file
  done
