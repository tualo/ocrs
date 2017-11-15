
void ImageRecognizeEx::unpaper(cv::Mat& src){


  cv::Scalar cm = cv::mean(src);

  std::cout << "mean "<< cm[0] << std::endl;
  //x_cm
  int bs = x_cm/10 * 2; // 3mm
  if (bs%2==0){
    bs+=1;
  }
  if (bs<3){
    bs=3;
  }
  subtractMean = (128-cm[0])/2;
  if (subtractMean<0){
    subtractMean*=-1;
  }
  cv::Mat test;
  cv::Mat test_inv;

  cv::GaussianBlur(src,test,cv::Size(13,13),2,2);
  cv::adaptiveThreshold(
      test,
      test,
      255,
      CV_ADAPTIVE_THRESH_GAUSSIAN_C,
      CV_THRESH_BINARY,//blockSize,calcmeanValue(src));/*,
      85,
      5
  );
  cv::GaussianBlur(test,test,cv::Size(3,3),2,2);

  //cv::imshow("1. \"blur\"", test);

//  cv::threshold(test, test_inv, 150, 255, cv::THRESH_BINARY_INV);
//  cv::imshow("2. \"blur thres\"", test_inv);

  cv::threshold(test, test, 180, 255, cv::THRESH_BINARY);
  //cv::imshow("2. \"blur thres norm\"", test);
  cv::Mat atMask;
  cv::adaptiveThreshold(
      src,
      src,
      255,
      CV_ADAPTIVE_THRESH_GAUSSIAN_C,
      CV_THRESH_BINARY,//blockSize,calcmeanValue(src));/*,
      3,
      1
  );
  int count_black = cv::countNonZero(src);

  std::cout << "black>>>" << count_black  << std::endl;
  double fb = ((double)count_black/((double)src.cols*(double)src.rows));
  std::cout << "black>>>" << fb  << std::endl;

  atMask=src.clone();
  cv::GaussianBlur(atMask,atMask,cv::Size(7,7),2,2);

//  cv::imshow("atMask ", atMask);
  if (fb<0.85){
    cv::threshold(atMask, atMask, 120, 255, cv::THRESH_BINARY);
  }else if (fb>0.95){
    cv::threshold(atMask, atMask, 190, 255, cv::THRESH_BINARY);
  }else{
    cv::threshold(atMask, atMask, 160, 255, cv::THRESH_BINARY);
  }
  int erosion_size=1;

  cv::Mat element = cv::getStructuringElement( cv::MORPH_ELLIPSE,
                                     cv::Size( 2*erosion_size + 1, 2*erosion_size+1 ),
                                     cv::Point( erosion_size, erosion_size ) );

 cv::erode( atMask, atMask, element );
 cv::erode( test, test, element );
//  cv::imshow("atMask threshold", atMask);
  cv::bitwise_or(src,atMask,src);
//  cv::imshow("adaptiveThreshold ", src);

//cv::waitKey(0);
  cv::bitwise_or(src,test,src);
}


cv::Mat ImageRecognizeEx::sobel(cv::Mat gray){
	cv::Mat edges;

	int scale = 1;
	int delta = 0;
	int ddepth = CV_16S;
	cv::Mat edges_x, edges_y;
	cv::Mat abs_edges_x, abs_edges_y;
	cv::Sobel(gray, edges_x, ddepth, 1, 0, 3, scale, delta, cv::BORDER_DEFAULT);
	cv::convertScaleAbs( edges_x, abs_edges_x );
	cv::Sobel(gray, edges_y, ddepth, 0, 1, 3, scale, delta, cv::BORDER_DEFAULT);
	cv::convertScaleAbs(edges_y, abs_edges_y);
	cv::addWeighted(abs_edges_x, 0.5, abs_edges_y, 0.5, 0, edges);

	return edges;
}

cv::Mat ImageRecognizeEx::canny(cv::Mat src)
{
	cv::Mat detected_edges;

	int edgeThresh = 1;
	int lowThreshold = 250;
	int highThreshold = 750;
	int kernel_size = 5;
	cv::Canny(src, detected_edges, lowThreshold, highThreshold, kernel_size);

	return detected_edges;
 }
