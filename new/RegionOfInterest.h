// RegionOfInterest.h
#ifndef REGIONOFINTEREST_H
#define REGIONOFINTEREST_H

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <iostream>
#include <stdio.h>

#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include <leptonica/allheaders.h>
#include <tesseract/baseapi.h>
#include <tesseract/strngs.h>
#include <zbar.h>

class RegionOfInterest{
  public:
    RegionOfInterest(std::string name,double x,double y,double w, double h, int rotate, int rotate_steps);
    ~RegionOfInterest();
    void setDebug(bool val);
    void setImage(cv::Mat &useImage);
    void setPixelPerCM(int value);
    void setROI(double x,double y,double w, double h, int rotate, int rotate_steps);

    cv::Rect rect();
    std::string name();
    int rotate();
    int rotateSteps();

  private:
    int pixelPerCm;
    cv::Mat image;
    bool showDebug;
    std::string std_name;
    double ix;
    double iy;
    double iw;
    double ih;
    int irotate;
    int irotate_steps;
};
#endif
