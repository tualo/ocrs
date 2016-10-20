#!/bin/bash

g++ ocrs.cpp ImageRecognize.cpp ExtractAddress.cpp glob_posix.cpp -o ocrs `mysql_config --cflags --libs` -std=c++11  \
-I/usr/include/opencv2 \
-I/usr/include/tesseract \
-L/usr/local/lib \
-I/usr/local/include/opencv -I/usr/local/include -L/usr/local/lib -lopencv_shape -lopencv_stitching -lopencv_objdetect -lopencv_superres -lopencv_videostab -lopencv_calib3d -lopencv_features2d -lopencv_highgui -lopencv_videoio -lopencv_imgcodecs -lopencv_video -lopencv_photo -lopencv_ml -lopencv_imgproc -lopencv_flann -lopencv_core \
-L/usr/local/lib -ltesseract \
-lzbar \
-lboost_system -lboost_regex -lboost_filesystem \
-lpthread
