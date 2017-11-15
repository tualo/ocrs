f=$1
name=`basename "$f"`
./ocrsex -dtw -h 127.0.0.1 -n deg -u root -x "" \
  --blocksize=85 \
  --cmx 78 \
  --cmy 78 \
  -f "$f" \
  --substractmean 25 \
  --machine 21 \
  --savedb
#  --result /largedisk/result/ \
#  --machine 22 \
#  --rescaledfilename "/largedisk/rescaled/$name" \
#  --removeorignal
