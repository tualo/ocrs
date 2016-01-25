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


struct bcResult {
  cv::Point point;
  std::string code;
  std::string type;
};

class ImageRecognize{
public:
  void open(const char* filename);
  bool showWindow;
private:
  cv::Mat largestContour(cv::Mat& src);
  cv::Rect fittingROI(int x,int y,int w,int h, cv::Mat& m1);
  double getOrientation(std::vector<cv::Point> &pts, cv::Mat &img);
  bcResult barcode(cv::Mat& im);
  bool is_digits(const std::string &str);
  const char* text(cv::Mat& im);
  void rotate(cv::Mat& src, double angle, cv::Mat& dst);
  void showImage(cv::Mat& src);
  void showImage(cv::Mat& src,char* title);
  void rotateX(cv::Mat& src,float angle,cv::Point center);
  void linearize(cv::Mat& src);

  int oneCM;
  const char* fileName;
  cv::Mat orignalImage;
  tesseract::TessBaseAPI* tess;

};
#endif
