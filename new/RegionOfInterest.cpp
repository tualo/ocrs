#include "RegionOfInterest.h"

RegionOfInterest::RegionOfInterest(std::string name,double x,double y,double w, double h, int rotate, int rotate_steps) :
  pixelPerCm(23),
  showDebug(true)
{
  std_name = name;
  setROI(x,y,w,h,rotate,rotate_steps);
}

RegionOfInterest::~RegionOfInterest() {

}

void RegionOfInterest::setPixelPerCM(int value){
  pixelPerCm = value;
}

void RegionOfInterest::setImage(cv::Mat &useImage){
  image=useImage;
}

std::string RegionOfInterest::name(){
  return std_name;
}

int RegionOfInterest::rotate(){
  return irotate;
}

int RegionOfInterest::rotateSteps(){
  return irotate_steps;
}


cv::Rect RegionOfInterest::rect(){
  cv::Rect roi( pixelPerCm*ix , pixelPerCm*iy ,pixelPerCm*iw, pixelPerCm*ih);
  if (roi.x<0){
    roi.x=0;
  }
  if (roi.y<0){
    roi.y=0;
  }
  if (image.rows<(roi.y)){
    roi.y=image.rows-10;
  }
  if (image.rows<(roi.y+roi.height)){
    roi.height=image.rows-roi.y;
    std::cerr << "RegionOfInterest::rect height problem, setting to max" << std::endl;
  }

  if (image.cols<(roi.x)){
    roi.x=image.cols-10;
  }

  if (image.cols<(roi.x+roi.width)){
    roi.width=image.cols-roi.x;

    std::cerr << "RegionOfInterest::rect width problem, setting to max" << std::endl;
  }

  return roi;
}

void RegionOfInterest::setROI(double x,double y,double w, double h, int rotate, int rotate_steps){
  ix=x;
  iy=y;
  iw=w;
  ih=h;
  irotate=rotate;
  irotate_steps=rotate_steps;
}

void RegionOfInterest::setDebug(bool val){
  showDebug=val;
}
