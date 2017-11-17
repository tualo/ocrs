f=$1
name=`basename "$f"`
./ocrsex -dtw -h 127.0.0.1 -n deg -u root -x "" \
  --blocksize=85 \
  --cmx 78 \
  --cmy 72 \
  -f "$f" \
  --substractmean 25 \
  --machine 21 \
  --savedb \
  --zipcoderegex "(D|O|7|I|i|Q|\\d){5}"
#  --result /largedisk/result/ \
#  --machine 22 \
#  --rescaledfilename "/largedisk/rescaled/$name" \
#  --removeorignal
