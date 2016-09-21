
#include "ImageRecognize.h"
#include "ExtractAddress.h"
#include "opencv2/imgproc/imgproc.hpp"
#include <pthread.h>
#include <vector>
#include <string>


double contrast_measure( const cv::Mat&img )
{
    cv::Mat dx, dy;
    cv::Sobel( img, dx, CV_32F, 1, 0, 3 );
    cv::Sobel( img, dy, CV_32F, 0, 1, 3 );
    cv::magnitude( dx, dy, dx );
    return cv::sum(dx)[0];
}

int main( int argc, char** argv ){
  cv::Mat bgr_image = cv::imread(argv[1]);
// cv::GaussianBlur( bgr_image, bgr_image, cv::Size(7,7), 0);
  std::cout << "blurred image  : " << contrast_measure( bgr_image ) << std::endl;
}
