#!/bin/bash

#pkg-config --libs tesseract
#pkg-config --libs opencv

#g++ ocrs.cpp ImageRecognize.cpp ExtractAddress.cpp glob_posix.cpp -o ocrs `mysql_config --cflags --libs` -I/usr/local/Cellar/opencv/2.4.11_1/include/opencv \
#-I/usr/local/Cellar/opencv/2.4.11_1/include -L/usr/local/Cellar/opencv/2.4.11_1/lib -lopencv_calib3d \
#-lopencv_core -lopencv_features2d -lopencv_flann -lopencv_gpu -lopencv_highgui -lopencv_imgproc \
#-lopencv_legacy -lopencv_ml -lopencv_nonfree -lopencv_objdetect -lopencv_ocl -lopencv_photo \
#-lopencv_stitching -lopencv_superres -lopencv_ts -lopencv_video -lopencv_videostab \
#-L/usr/local/Cellar/tesseract/3.04.00/lib -ltesseract -L/usr/local/Cellar/zbar/0.10_1/lib -lzbar \
#-lboost_regex \
#-lpthread

unamestr=`uname`
if [[ "$unamestr" == 'Darwin' ]]; then
  g++ ocrs.cpp ImageRecognize.cpp ExtractAddress.cpp glob_posix.cpp -o ocrs `mysql_config --cflags --libs` -I/usr/local/Cellar/opencv/3.0.0/include/opencv \
  -I/usr/local/Cellar/opencv/3.0.0/include -L/usr/local/Cellar/opencv3/3.0.0/lib -lopencv_calib3d -lopencv_core -lopencv_features2d -lopencv_flann -lopencv_highgui \
  -lopencv_imgcodecs -lopencv_imgproc -lopencv_ml -lopencv_objdetect -lopencv_photo -lopencv_shape -lopencv_stitching -lopencv_superres -lopencv_video -lopencv_videoio -lopencv_videostab \
  -I/usr/local/Cellar/leptonica/1.71_1/include \
  -I/usr/local/Cellar/tesseract/3.04.00/include \
  -I/usr/local/Cellar/zbar/0.10_1/include \
  -L/usr/local/Cellar/tesseract/3.04.00/lib -ltesseract -L/usr/local/Cellar/zbar/0.10_1/lib -lzbar \
  -I/usr/local/Cellar/boost/1.63.0/include -L/usr/local/Cellar/boost/1.63.0/lib -lboost_system -lboost_regex -lboost_filesystem \
  -lpthread


  g++ sharpness.cpp -o sharpness `mysql_config --cflags --libs` -I/usr/local/Cellar/opencv/3.0.0/include/opencv \
  -I/usr/local/Cellar/opencv/3.0.0/include -L/usr/local/Cellar/opencv3/3.0.0/lib -lopencv_calib3d -lopencv_core -lopencv_features2d -lopencv_flann -lopencv_highgui \
  -lopencv_imgcodecs -lopencv_imgproc -lopencv_ml -lopencv_objdetect -lopencv_photo -lopencv_shape -lopencv_stitching -lopencv_superres -lopencv_video -lopencv_videoio -lopencv_videostab \
  -I/usr/local/Cellar/leptonica/1.71_1/include \
  -I/usr/local/Cellar/tesseract/3.04.00/include \
  -I/usr/local/Cellar/zbar/0.10_1/include \
  -L/usr/local/Cellar/tesseract/3.04.00/lib -ltesseract -L/usr/local/Cellar/zbar/0.10_1/lib -lzbar \
  -I/usr/local/Cellar/boost/1.63.0/include -L/usr/local/Cellar/boost/1.63.0/lib -lboost_system -lboost_regex -lboost_filesystem \
  -lpthread

elif [[ "$unamestr" == 'Linux' ]]; then
  g++ ocrs.cpp ImageRecognize.cpp ExtractAddress.cpp glob_posix.cpp -o ocrs `mysql_config --cflags --libs` -std=c++11  \
  -I/usr/include/opencv2 \
  -I/usr/include/tesseract \
  -L/usr/local/lib \
  `pkg-config opencv --cflags --libs` \
  -L/usr/local/lib -ltesseract \
  -lzbar \
  -lboost_system -lboost_regex -lboost_filesystem \
  -lpthread
fi

#apt-get install libtesseract-dev
#apt-get install libleptonica-dev
#apt-get install libopencv2-dev

# linux
