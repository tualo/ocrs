f=$1
name=`basename "$f"`
./ocrsex -dtw -h 127.0.0.1 -n deg -u root -x "" --blocksize=85 \
  --cmx 72 \
  --cmy 72 \
  -f "$f" \
  --substractmean 25 \
  --machine 22 \
  --savedb
#  --result /largedisk/result/ \
#  --machine 22 \
#  --rescaledfilename "/largedisk/rescaled/$name" \
#  --removeorignal
