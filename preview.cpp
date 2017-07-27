#include <iostream>
#include <stdio.h>
#include <vector>

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

int main( int argc, char** argv ){
  int window_wait=5000;
  if (argc<4){
    std::cout << "Usage " << argv[0] << " <originalfile> " << " <resultjpg> " << " <scale> " << " [resize] " << std::endl;
    return -1;
  }

  //double t1 = (double)cv::getTickCount();
  //double te1;

  const char* filename = argv[1];
  const char* newfile = argv[2];
  float scale = std::atof(argv[3]);
  int resize_by=3;
  if (argc>=5){
    resize_by= std::atoi(argv[4]);
  }

  cv::Mat orignalImage = cv::imread( filename, 1 );

  transpose(orignalImage, orignalImage);
  flip(orignalImage, orignalImage,1);
  //transpose+flip(1)=CW
  /*transpose(orignalImage, orignalImage);
  flip(orignalImage, orignalImage,1); //transpose+flip(1)=CW
  */
  //float scale=0.65;
  float px_cm = 2048/27;
  int x=(orignalImage.cols /resize_by)*scale;
  int y=orignalImage.rows /resize_by;
  cv::Mat res = cv::Mat(x, y, CV_32FC3);
  cv::resize(orignalImage, res, cv::Size(x, y), 0, 0, 3);
  /*
  cv::namedWindow("DEBUG", CV_WINDOW_AUTOSIZE );
  cv::imshow("DEBUG", res );
  cv::waitKey(window_wait);


  std::cout << "x " << x << "cm" << x/px_cm*3 << std::endl;
  std::cout << "y " << y << "cm" << y/px_cm*3  << std::endl;
  */
  std::vector<int> params;
  params.push_back(CV_IMWRITE_JPEG_QUALITY);
  params.push_back(80);
  cv::imwrite(newfile,res,params);
  //if (debug){
  //  te1 = ((double)cv::getTickCount() - t1)/cv::getTickFrequency();
  //  std::cout << "text passed in seconds: " << te1 << std::endl;
  //}


}
