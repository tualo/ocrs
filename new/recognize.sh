f=$1
name=`basename "$f"`
./ocrsex -dtw -h 127.0.0.1 -n deg -u root -x "" \
  -f "$f" \
  --machine $2 \
  --savedb \
  --dbsettings
#  --blocksize=85 \
#  --cmx 75 \
#  --cmy 66 \
#
#  --zipcoderegex "(D|O|7|I|i|Q|\\d){5}"
#  --result /largedisk/result/ \
#  --machine 22 \
#  --rescaledfilename "/largedisk/rescaled/$name" \
#  --removeorignal
#--cmx 71 \
#--cmy 99 \
