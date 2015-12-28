// ImageRecognize.h
#ifndef IMAGERECOGNIZE_H
#define IMAGERECOGNIZE_H

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <stdio.h>
#include <regex>

#include <leptonica/allheaders.h>
#include <tesseract/baseapi.h>
#include <tesseract/strngs.h>
#include <zbar.h>

class ImageRecognize{
public:
  void open(char* filename);
private:
  cv::Mat largestContour(cv::Mat& src);
  double getOrientation(std::vector<cv::Point> &pts, cv::Mat &img);
  cv::Point barcode(cv::Mat& im);
  const char* text(cv::Mat& im);
  void rotate(cv::Mat& src, double angle, cv::Mat& dst);
  void showImage(cv::Mat& src);
  void showImage(cv::Mat& src,char* title);
  void rotateX(cv::Mat& src,float angle,cv::Point center);
  void linearize(cv::Mat& src);

  int oneCM;
  char* fileName;
  cv::Mat orignalImage;
  bool showWindow;
};
#endif
