// ImageRecognizeEx.h
#ifndef IMAGERECOGNIZEEX_H
#define IMAGERECOGNIZEEX_H






#include "opencv2/highgui/highgui.hpp"
#include <opencv2/imgproc/imgproc.hpp>
//#include "opencv2/imgproc/imgproc.hpp"
//#include "opencv2/imgcodecs.hpp"
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <cstdlib>

#include <list>
#include <vector>
//#include <regex>
#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

#include <leptonica/allheaders.h>
#include <tesseract/baseapi.h>
#include <tesseract/strngs.h>
#include <zbar.h>


#define HAVE_INT64
#define HAVE_UINT64
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
  void setImage(cv::Mat mat);
  void showImage(cv::Mat& src,int ww);

  void open(const char* filename);
  void setDebug(bool val);
  void setDebugWindow(bool val);
  void setDebugTime(bool val);

  void setCalcMean(bool value);
  void useBarcodeClahe(bool val);
  void setWait(int val);
  void setPixelPerCM(int x_cm,int y_cm);

  void largestContour(bool useSlow);
  void rescale();
  void barcode();
  void checkPixels();
  void updateTimeStatistic(double tm);
  void updateResultfilename(std::string res);

  void setBarcodeStepSize(int value);
  void setBarcodeStartThreshold(int value);
  void setBarcodeStopThreshold(int value);

  void setBarcodeClahe(bool value);
  void setBarcodeFP(bool value);

  void correctSize();
  ExtractAddress* texts();
  void _debugTime(std::string str);


  void setMachine(std::string val);
  void setDBConnection(MYSQL *connection);
  void setBlockSize(int value);
  void setSubtractMean(int value);
  void setLightBlockSize(int value);
  void setLightSubtractMean(int value);

  void setMeanFactor(float value);
  bool is_digits(const std::string &str);

  void setKundennummer(std::string value);
  void setKostenstelle(std::string value);

  std::string getKundennummer();
  std::string getKostenstelle();

  void saveRescaledOriginal(std::string filename);

  std::string getBarcode();
  std::list<std::string> barcodelist();

  cv::Mat roiImage;

  cv::Mat getResultImage();
  cv::Mat getOriginalImage();
  void setZipcodeRegexText(std::string str);

  void setExplicitMachine(bool value);
  bool getExplicitMachine();

private:
  double getOrientation(std::vector<cv::Point> &pts, cv::Mat &img);
  cv::Mat largestComplexContour(cv::Mat& src);
  cv::Mat largestSimpleContour(cv::Mat& src);
  int calcmeanValue(cv::Mat m);
  void unpaper(cv::Mat& src);
  cv::Mat canny(cv::Mat src);
  cv::Mat sobel(cv::Mat gray);
  bool readMatBinary(std::ifstream& ifs, cv::Mat& in_mat);
  void showImage(cv::Mat& src);
  //bcResult fast_barcode(cv::Mat& im);
  bcResult barcode_internal(cv::Mat& im, bool forceFPCode);
  //bcResult barcode_internal_old(cv::Mat& im, bool forceFPCode);
  void rotateX(cv::Mat& src,float angle,cv::Point center);
  void rotate(cv::Mat& src, int direction);

  void initStatistics();
  void updateStatistics(std::string field,std::string val);
  void updateStatistics(std::string field,double val);
  void updateStatistics(std::string field,int val);

  void initBarcodeRegions();
  void initAddressRegions();
  void initZipCodeMap();

  bool usingRoi(cv::Mat& im,cv::Rect roi2,int irotate,int istep_rotate);
  bool containsZipCode(cv::Mat& im,cv::Mat& orig);
  bool containsZipCodePure(cv::Mat& im,cv::Mat& orig);
  std::string getText(cv::Mat& im);
  void recalcSubstractMean(cv::Mat m);
  int linearize(cv::Mat& src);

  bool forceFPNumber();

  std::vector<std::string> &isplit(const std::string &s, char delim, std::vector<std::string>  &elems);
  std::vector<std::string> isplit(const std::string &s,char delim);

  bool showDebug;
  bool showDebugWindow;
  bool showDebugWindowUnpaper;
  bool showDebugTime;
  bool psmAuto;
  bool barcodeClahe;
  bool barcodeFP;
  bool calcMean;

  MYSQL *con;

  bool bSaveRescaledOriginal;
  std::string sSaveRescaledOriginal;

  cv::Mat orignalImage;
  cv::Mat oImage;

  int oneCM;
  int x_cm;
  int y_cm;
  int windowWait;
  int subtractMean;
  int blockSize;
  int lightSubtractMean;
  int lightBlockSize;
  int resultThres;

  double debug_last_time;

  const char* fileName;
  const char* ocrText;
  std::string resultText;
  std::string allTogether;
  std::string codes;
  std::list<std::string> codelist;
  std::string code;
  std::string machine;
  std::string addressfield;
  std::string kundennummer;
  std::string kostenstelle;

  std::string zipcodeRegexText;

  float meanfactor;

  tesseract::TessBaseAPI* tess;

  ExtractAddress* extractAddress;

  std::list<RegionOfInterest*> barcodeRegions;
  std::list<RegionOfInterest*> addressRegions;

  int i_bc_thres_start;
  int i_bc_thres_stop;
  int i_bc_thres_step;

  int barcode_internal_counter;

  double rescale_cols;
  double rescale_rows;

  bool explicitMachine;
  boost::format updateStatisticsDouble;//("update ocrs_statistics set `%s` = '%s' where `code`='%9.6f' ");
  boost::format updateStatisticsString;//("update ocrs_statistics set `%s` = '%s' where `code`='%s' ");

};
#endif
