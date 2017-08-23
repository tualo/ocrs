// ImageRecognizeEx.h
#ifndef IMAGERECOGNIZEEX_H
#define IMAGERECOGNIZEEX_H

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
//#include "opencv2/imgcodecs.hpp"

#include <iostream>
#include <fstream>
#include <cstdlib>

#include <list>
#include <vector>
//#include <regex>
#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include <leptonica/allheaders.h>
#include <tesseract/baseapi.h>
#include <tesseract/strngs.h>
#include <zbar.h>

#define HAVE_BOOL 1
#define HAVE_UINT64 1
#define HAVE_INT64 1


#include <my_global.h>
#include <mysql.h>


#include "RegionOfInterest.h"
#include "ExtractAddress.h"

struct bcResult {
  cv::Point point;
  cv::Rect rect;
  std::string code;
  std::string type;
  bool found;
};

class ImageRecognizeEx{
public:
  ImageRecognizeEx();
  ~ImageRecognizeEx();
  void open(const char* filename);
  void setDebug(bool val);
  void setDebugWindow(bool val);
  void setDebugTime(bool val);

  void useBarcodeClahe(bool val);
  void setWait(int val);
  void rescale(int x_cm,int y_cm);
  void barcode();
  void texts();
  void setMachine(std::string val);
  void setDBConnection(MYSQL *connection);
  void setBlockSize(int value);
  void setSubtractMean(int value);
  bool is_digits(const std::string &str);


private:
  cv::Mat largestContour(cv::Mat& src);
  bool readMatBinary(std::ifstream& ifs, cv::Mat& in_mat);
  void showImage(cv::Mat& src);
  //bcResult fast_barcode(cv::Mat& im);
  bcResult barcode_internal(cv::Mat& im, bool forceFPCode);
  void rotateX(cv::Mat& src,float angle,cv::Point center);
  void rotate(cv::Mat& src, int direction);

  void initRegions();

  bool usingRoi(cv::Mat& im,cv::Rect roi2,int irotate,int istep_rotate);
  bool containsZipCode(cv::Mat& im,cv::Mat& orig);
  std::string getText(cv::Mat& im);
  void recalcSubstractMean(cv::Mat m);
  int linearize(cv::Mat& src,float multiply);

  std::vector<std::string> &isplit(const std::string &s, char delim, std::vector<std::string>  &elems);
  std::vector<std::string> isplit(const std::string &s,char delim);

  bool showDebug;
  bool showDebugWindow;
  bool showDebugTime;
  bool psmAuto;
  bool barcodeClahe;
  bool barcodeFP;

  MYSQL *con;


  cv::Mat orignalImage;
  cv::Mat roiImage;

  int oneCM;
  int windowWait;
  int subtractMean;
  int blockSize;
  int resultThres;

  const char* fileName;
  const char* ocrText;
  std::string resultText;
  std::string allTogether;
  std::string codes;
  std::string code;
  std::string machine;
  tesseract::TessBaseAPI* tess;

  ExtractAddress* extractAddress;

  std::list<RegionOfInterest*> barcodeRegions;
  std::list<RegionOfInterest*> addressRegions;

};
#endif
