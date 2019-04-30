// Grabber.h
#ifndef FINDCODES_H
#define FINDCODES_H


#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <zbar.h>

#include <stdio.h>
#include <cstdlib>
#include <fstream>

#include <boost/thread.hpp>
#include <boost/chrono.hpp>
#include <boost/asio.hpp>
#include <boost/format.hpp>

#include <boost/thread/scoped_thread.hpp>
#include <iostream>

#include "Barcode.h"

class FindCodes{
public:
  FindCodes();
  ~FindCodes();

  void detect(std::string filename);
  void detectCodes(cv::Mat image);


  void debugCodes();
  std::list<Barcode*> codes();

  void detectByAdaptiveThreshold(cv::Mat useimage,int blocksize, int subtractmean);

private:
  bool hasCode(std::string code, std::string type);

  int startSubtractMean;
  int stepSubtractMean;

  int startBlocksize;
  int stepBlocksize;



  int maxTasks;
  boost::mutex mutex;

  std::list<Barcode*> barcodes;
  std::hash<std::string > hash;


};
#endif
