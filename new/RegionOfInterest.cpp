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
  /*
  std::cout << "image.rows: " << image.rows << std::endl;
  std::cout << "image.cols: " << image.cols << std::endl;

  std::cout << "ix: " << ix << "*" << pixelPerCm << std::endl;
  std::cout << "iy: " << iy << "*" << pixelPerCm << std::endl;
  std::cout << "ih: " << ih << "*" << pixelPerCm << std::endl;
  std::cout << "iw: " << iw << "*" << pixelPerCm << std::endl;
  */
  cv::Rect roi( pixelPerCm*ix , pixelPerCm*iy ,pixelPerCm*iw, pixelPerCm*ih);

  //std::cout << "roi: " << roi.x << " " << roi.y << " " << roi.width << " " << roi.height << std::endl;

  if (image.rows<(roi.y)){
    roi.y=image.rows-1;
  }
  if (image.rows<(roi.y+roi.height)){
    roi.height=1;
  }

  if (image.cols<(roi.x)){
    roi.x=image.cols-1;
  }
  if (image.cols<(roi.x+roi.width)){
    roi.width=1;
  }
//  std::cout << "roi: " << roi.x << " " << roi.y << " " << roi.width << " " << roi.height << std::endl;
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
