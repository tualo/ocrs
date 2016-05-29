// ImageRecognize.h
#ifndef IMAGERECOGNIZE_H
#define IMAGERECOGNIZE_H


#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"


#include <iostream>
#include <stdio.h>
//#include <regex>
#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include <leptonica/allheaders.h>
#include <tesseract/baseapi.h>
#include <tesseract/strngs.h>
#include <zbar.h>




struct bcResult {
  cv::Point point;
  cv::Rect rect;
  std::string code;
  std::string type;
  bool found;
};

class ImageRecognize{
public:
  void open(const char* filename);
  void openPZA(const char* filename);
  bool showWindow;
  std::string code;
  std::string jsontext;
  std::string addresstext;
  int width;
  int height;
  cv::Mat resultMat;
  cv::Mat orignalImage;
  int resultThres;

private:
  cv::Mat largestContour(cv::Mat& src);
  void makeResultImage(cv::Mat& src);
  cv::Rect fittingROI(int x,int y,int w,int h, cv::Mat& m1);
  double getOrientation(std::vector<cv::Point> &pts, cv::Mat &img);

  bcResult barcode(cv::Mat& im);
  bcResult fast_barcode(cv::Mat& im);
  bcResult barcode_internal(cv::Mat &part);

  bool is_digits(const std::string &str);
  const char* text(cv::Mat& im);
  const char* getText(cv::Mat& im);

  bool usingLetterRoi(cv::Mat& im,cv::Rect roi2);
  bool usingLetterType1(cv::Mat& im);
  bool usingLetterType1_1(cv::Mat& im);

  bool usingLetterType3(cv::Mat& im);

  bool usingLetterType2(cv::Mat& im);
  bool usingLetterType2_1(cv::Mat& im);
  bool usingLetterType2_2(cv::Mat& im);

  cv::Mat getRectangle(cv::Mat& src);

  void getPZAText(cv::Mat& src);

  void rotate(cv::Mat& src, double angle, cv::Mat& dst);
  void showImage(cv::Mat& src);
  void showImage(cv::Mat& src,char* title);
  void rotateX(cv::Mat& src,float angle,cv::Point center);
  void linearize(cv::Mat& src);

  int oneCM;


  const char* fileName;
  const char* ocr_text;
  tesseract::TessBaseAPI* tess;
};
#endif
